#pragma once

#include <rankcpp/Dimensions.hpp>
#include <rankcpp/Key.hpp>
#include <rankcpp/ScoresTable.hpp>

#include <gsl/span>

#include <range/v3/all.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace rankcpp {

template <typename T, class DimensionsType = Dimensions> class WeightTable {
public:
  // TODO doesn't have to be unsigned
  static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>,
                "T must be an unsigned integral type");

  using WeightType = T;

  explicit WeightTable(DimensionsType dims)
      : dims_(dims), weights_(dims.scoresCount()) {}

  WeightTable(DimensionsType dims, std::vector<T> weights)
      : dims_(std::move(dims)), weights_(std::move(weights)) {}

  WeightTable(DimensionsType dims, std::initializer_list<T> const &list)
      : dims_(dims), weights_(dims.scoresCount()) {
    if (list.size() != dims.scoresCount()) {
      throw std::length_error("initializer_list needs to be of length " +
                              std::to_string(dims.scoresCount()) +
                              " bytes but is " + std::to_string(list.size()) +
                              " bytes");
    }
    std::copy(std::begin(list), std::end(list), std::begin(weights_));
  }

  auto weight(std::size_t vectorIndex, std::size_t subkeyIndex) const -> T {
    return weights_.at(dims_.scoresBeforeCount(vectorIndex) + subkeyIndex);
  }

  auto weight(std::size_t vectorIndex, std::size_t subkeyIndex) -> T & {
    return weights_.at(dims_.scoresBeforeCount(vectorIndex) + subkeyIndex);
  }

  auto operator()(std::size_t vectorIndex, std::size_t subkeyIndex) noexcept
      -> T & {
    return weights_[dims_.scoresBeforeCount(vectorIndex) + subkeyIndex];
  }

  auto operator()(std::size_t vectorIndex,
                  std::size_t subkeyIndex) const noexcept -> T {
    return weights_[dims_.scoresBeforeCount(vectorIndex) + subkeyIndex];
  }

  void rebase(T newMinWeight) noexcept {
    auto const minValue =
        *std::min_element(std::begin(weights_), std::end(weights_));

    if (minValue >= newMinWeight) {
      auto const shift = minValue - newMinWeight;
      std::transform(std::cbegin(weights_), std::cend(weights_),
                     std::begin(weights_),
                     [&shift](auto const &weight) { return weight - shift; });
    } else {
      auto const shift = newMinWeight - minValue;
      std::transform(std::cbegin(weights_), std::cend(weights_),
                     std::begin(weights_),
                     [&shift](auto const &weight) { return weight + shift; });
    }
  }

  void sortAscending() noexcept { sortEachSubkey<std::less<T>>(); }

  void sortDescending() noexcept { sortEachSubkey<std::greater<T>>(); }

  auto minimumWeight() const noexcept -> T {
    auto const vectorRange = dims_.vectorRange();
    return std::accumulate(
        std::cbegin(vectorRange), std::cend(vectorRange), T{0},
        [&](T current, auto const &vectorIndex) {
          auto const min = *std::min_element(
              std::cbegin(weights_) + static_cast<std::ptrdiff_t>(
                                         dims_.scoresBeforeCount(vectorIndex)),
              std::cbegin(weights_) +
                  static_cast<std::ptrdiff_t>(
                      dims_.scoresBeforeCount(vectorIndex)) +
                  static_cast<std::ptrdiff_t>(dims_.subkeyCount(vectorIndex)));
          return current + min;
        });
  }

  // TODO check all these noexcepts and rules for propagation down
  auto maximumWeight() const noexcept -> T {
    auto const vectorRange = dims_.vectorRange();
    return std::accumulate(
        std::cbegin(vectorRange), std::cend(vectorRange), T{0},
        [&](T current, auto const &vectorIndex) {
          auto const max = *std::max_element(
              std::cbegin(weights_) + static_cast<std::ptrdiff_t>(
                                         dims_.scoresBeforeCount(vectorIndex)),
              std::cbegin(weights_) +
                  static_cast<std::ptrdiff_t>(
                      dims_.scoresBeforeCount(vectorIndex)) +
                  static_cast<std::ptrdiff_t>(dims_.subkeyCount(vectorIndex)));
          return current + max;
        });
  }

  template <std::uint32_t KeyLenBits>
  auto weightForKey(Key<KeyLenBits> const &key) const -> T {
    auto indexedSubkeys =
        ranges::views::zip(dims_.vectorRange(), dims_.asSpans());
    return std::accumulate(std::cbegin(indexedSubkeys),
                           std::cend(indexedSubkeys), T{0},
                           [&](T current, auto const &indexedSubkey) {
                             auto const &[vectorIndex, subkey] = indexedSubkey;
                             auto const subkeyValue =
                                 key.template subkeyValue<std::size_t>(subkey);
                             return current + weight(vectorIndex, subkeyValue);
                           });
  }

  auto dimensions() const -> DimensionsType const & { return dims_; }

  auto allWeights() noexcept -> std::vector<T> & { return weights_; };

  auto allWeights() const noexcept -> std::vector<T> const & {
    return weights_;
  };

private:
  DimensionsType const dims_;
  std::vector<T> weights_;

  template <typename SortType> void sortEachSubkey() {
    for (std::size_t vectorIndex : dims_.vectorRange()) {
      std::sort(
          std::begin(weights_) +
              static_cast<std::ptrdiff_t>(dims_.scoresBeforeCount(vectorIndex)),
          std::begin(weights_) +
              static_cast<std::ptrdiff_t>(dims_.scoresBeforeCount(vectorIndex)) +
              static_cast<std::ptrdiff_t>(dims_.subkeyCount(vectorIndex)),
          SortType());
    }
  }
};

// TODO needs unit tests
template <typename ScoresType, typename WeightType, typename DimensionsType>
auto mapToWeight(ScoresTable<ScoresType, DimensionsType> const &table,
                 std::uint32_t precisionBits)
    -> WeightTable<WeightType, DimensionsType> {
  if (precisionBits < 2) {
    throw std::invalid_argument("Cannot run mapToWeight at less than"
                                " 2 bits of precision");
  }

  // find the maximum score and compute the multiplier to each score
  auto const &scores = table.allScores();
  auto const maxScore =
      *std::max_element(std::cbegin(scores), std::cend(scores));
  auto const alpha = std::log(maxScore) / std::log(2.0);
  if (std::isinf(alpha)) {
    throw std::logic_error("max score is 0.0; cannot apply mapToWeight");
  }
  auto const multiplier =
      std::pow(2.0, static_cast<ScoresType>(precisionBits) - alpha);

  // go back through the vectors, find and set the mapped weights
  WeightTable<WeightType, DimensionsType> weights(table.dimensions());
  std::transform(std::cbegin(scores), std::cend(scores),
                 std::begin(weights.allWeights()),
                 [&multiplier](ScoresType const &score) {
                   return static_cast<WeightType>(score * multiplier);
                 });

  // there's a considerable speed improvement from translating the weights
  // such that the most likely key has a weight of 1
  weights.rebase(1);
  return weights;
}

} /* namespace rankcpp */
