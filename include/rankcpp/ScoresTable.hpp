#pragma once

#include <rankcpp/Dimensions.hpp>
#include <rankcpp/utils/Numeric.hpp>

#include <range/v3/all.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace rankcpp {

template <typename T, typename DimensionsType = Dimensions> class ScoresTable {
  static_assert(std::is_floating_point_v<T>, "T must be a floating point type");

public:
  static constexpr T const epsilon = static_cast<T>(0.000001);

  explicit ScoresTable(DimensionsType dims)
      : dims_(dims), scores_(dims.scoresCount()) {}

  ScoresTable(DimensionsType dims, std::vector<T> scores)
      : dims_(std::move(dims)), scores_(std::move(scores)) {}

  ScoresTable(DimensionsType dims, std::initializer_list<T> const &list)
      : dims_(dims), scores_(dims.scoresCount()) {
    if (list.size() != dims.scoresCount()) {
      throw std::length_error("initializer_list needs to be of length " +
                              std::to_string(dims.scoresCount()) +
                              " bytes but is " + std::to_string(list.size()) +
                              " bytes");
    }
    std::copy(std::cbegin(list), std::cend(list), std::begin(scores_));
  }

  auto score(std::size_t vectorIndex, std::size_t subkeyIndex) -> T & {
    return scores_.at(dims_.scoresBeforeCount(vectorIndex) + subkeyIndex);
  }

  auto score(std::size_t vectorIndex, std::size_t subkeyIndex) const -> T {
    return scores_.at(dims_.scoresBeforeCount(vectorIndex) + subkeyIndex);
  }

  auto operator()(std::size_t vectorIndex, std::size_t subkeyIndex) -> T & {
    return scores_[dims_.scoresBeforeCount(vectorIndex) + subkeyIndex];
  }

  auto operator()(std::size_t vectorIndex, std::size_t subkeyIndex) const -> T {
    return scores_[dims_.scoresBeforeCount(vectorIndex) + subkeyIndex];
  }

  auto dimensions() const -> DimensionsType const & { return dims_; }

  void normaliseVectors() {
    for (std::size_t vectorIndex : dims_.vectorRange()) {
      auto first = std::begin(scores_);
      std::advance(first, dims_.scoresBeforeCount(vectorIndex));
      auto last = first;
      std::advance(last, dims_.subkeyCount(vectorIndex));
      auto const sum = kahanSum(first, last);
      auto const constant = T{1.0} / sum;

      std::transform(first, last, first, [&constant](auto const &score) {
        return score * constant;
      });
    }
  }

  void abs() {
    std::transform(std::cbegin(scores_), std::cend(scores_), std::begin(scores_),
                   [](auto const &score) { return std::fabs(score); });
  }

  void log2() { log(2.0); }

  void log(T base) {
    std::transform(std::cbegin(scores_), std::cend(scores_), std::begin(scores_),
                   [&base](auto const &score) {
                     return std::log(score) / std::log(base);
                   });
  }

  void translateVectorsToPositive() {
    // find the minimum value
    auto const minValue =
        *std::min_element(std::cbegin(scores_), std::cend(scores_));

    // if the minimum value is 0.0 then the vector elements are already all
    // positive and we don't need to do anything
    if (minValue <= T{0.0}) {
      // if we need to shift the scores, then add a small epsilon as a
      // fudge to ensure that no score is 0.0 after translation
      std::transform(
          std::cbegin(scores_), std::cend(scores_), std::begin(scores_),
          [&minValue](T const &score) { return (score - minValue) + epsilon; });
    }
  }

  template <typename InputIt>
  void addScores(BitSpan const &subkey, InputIt first, InputIt last) {
    using InputType = typename std::iterator_traits<InputIt>::value_type;
    static_assert(std::is_same_v<InputType, T>,
                  "supplied scores are not of the same type as those held "
                  "in the ScoresTable");
    // have to check that the supplied subkey definition matches one
    // specified by the dimensions of the table
    auto const &subkeys = dims_.asSpans();
    auto found = std::find(std::cbegin(subkeys), std::cend(subkeys), subkey);
    if (found == std::cend(subkeys)) {
      throw std::invalid_argument(
          "subkey does not match any specified by the table dimensions");
    }

    // check size of scores matches the specified subkey
    auto const supplied = std::distance(first, last);
    auto const required = subkey.valueCount();
    if (static_cast<decltype(required)>(supplied) != required) {
      throw std::length_error("required " + std::to_string(required) +
                              " scores, supplied " + std::to_string(supplied));
    }

    // copy in scores
    auto const vectorIndex = found - std::cbegin(subkeys);
    auto const offset =
        dims_.scoresBeforeCount(static_cast<std::size_t>(vectorIndex));
    auto end = std::begin(scores_);
    std::advance(first, offset);
    std::copy(first, last, end);
  }

  // TODO enable if here
  auto mergeVectors() const -> ScoresTable<T, Dimensions> {
    if (!dims_.isEqualWidth()) {
      throw std::invalid_argument(
          "all distinguishing vectors must be of equal width to merge");
    }
    auto const vectorCount = dims_.vectorCount();
    if (vectorCount % 2 != 0) {
      throw std::invalid_argument(
          "can only merge an even number of distinguishing vectors");
    }
    auto const vectorWidthBits = dims_.vectorWidthBits(0);
    Dimensions const mergedDims(vectorCount / 2, vectorWidthBits * 2);
    std::size_t const mask = (std::size_t{1} << vectorWidthBits) - 1;

    ScoresTable<T, Dimensions> merged(mergedDims);

    for (std::size_t vecIndex : dims_.vectorRange() | ranges::views::drop(1) |
                                    ranges::views::stride(2)) {
      std::size_t newVecIndex = (vecIndex - 1) / 2;
      auto const rearVecIndex = vecIndex - 1;

      for (std::size_t ski : mergedDims.asSpans()[newVecIndex].subkeyRange()) {
        auto const frontSubkeyIndex = ski & mask;
        auto const rearSubkeyIndex = (ski >> vectorWidthBits) & mask;

        auto const frontScore = this->operator()(vecIndex, frontSubkeyIndex);
        auto const rearScore = this->operator()(rearVecIndex, rearSubkeyIndex);
        merged(newVecIndex, ski) = rearScore * frontScore;
      }
    }

    return merged;
  }

  auto allScores() -> std::vector<T> & { return scores_; }

  auto allScores() const -> std::vector<T> const & { return scores_; }

private:
  DimensionsType const dims_;
  std::vector<T> scores_;
};

} /* namespace rankcpp */
