#pragma once

#include <rankcpp/BitSpan.hpp>
#include <rankcpp/Dimensions.hpp>
#include <rankcpp/Key.hpp>
#include <rankcpp/WeightTable.hpp>

#include <range/v3/all.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace rankcpp {

template <typename RankType, typename WeightType, class DimensionsType>
auto rank(WeightType maxWeight,
          WeightTable<WeightType, DimensionsType> const &weights) -> RankType {
  if (maxWeight == 0) {
    throw std::invalid_argument("The weight to rank to must be > 0");
  }

  std::vector<RankType> curr(maxWeight);
  std::vector<RankType> prev(maxWeight, RankType{1});

  auto const &dims = weights.dimensions();
  auto const vecRange = dims.vectorRange() | ranges::views::reverse;
  auto const &subkeys = dims.asSpans();

  for (auto vi : vecRange | ranges::views::drop_last(1)) {
    for (auto ski : subkeys[vi].subkeyRange() | ranges::views::reverse) {
      auto const weight = weights(vi, ski);
      if (maxWeight >= weight) {
        WeightType const currStart = maxWeight - weight;
        auto const currRange = ranges::views::iota(WeightType{0}, currStart) |
                               ranges::views::reverse;
        auto const prevRange =
            ranges::views::iota(weight, maxWeight) | ranges::views::reverse;
        for (auto const [cwi, pwi] : ranges::views::zip(currRange, prevRange)) {
          curr[cwi] += prev[pwi];
        }
      }
    }
    std::copy(std::cbegin(curr), std::cend(curr), std::begin(prev));
    std::fill(std::begin(curr), std::end(curr), 0);
  }

  // can skip all but nodes with weight 0 in the last vector
  for (auto ski : subkeys.front().subkeyRange() | ranges::views::reverse) {
    auto const weight = weights(vecRange.back(), ski);
    if (weight < maxWeight) {
      curr[0] += prev[weight];
    }
  }
  return curr[0];
}

template <std::uint32_t KeyLenBits, typename RankType, typename WeightType,
          class DimensionsType>
auto rank(Key<KeyLenBits> const &key,
          WeightTable<WeightType, DimensionsType> const &weights) -> RankType {
  auto const keyWeight = weights.weightForKey(key);
  if (keyWeight == 0) {
    throw std::invalid_argument("Weight for the known key must be > 0");
  }

  return rank<RankType, WeightType, DimensionsType>(keyWeight, weights);
}

template <typename RankType, typename WeightType, class DimensionsType>
auto rankLowMem(WeightType maxWeight,
                WeightTable<WeightType, DimensionsType> const &weights)
    -> RankType {
  if (maxWeight == 0) {
    throw std::invalid_argument("The weight to rank to must be > 0");
  }

  std::vector<RankType> curr(maxWeight);

  auto const &dims = weights.dimensions();
  auto const vecRange = dims.vectorRange() | ranges::views::reverse;
  auto const weightRange = ranges::views::iota(WeightType{0}, maxWeight);
  auto const &subkeys = dims.asSpans();

  // treat the last distinguishing vector separately
  for (auto wi : weightRange) {
    RankType temp{0};
    for (auto ski : subkeys.back().subkeyRange()) {
      auto const weight = weights(vecRange.front(), ski);
      if (wi + weight < maxWeight) {
        temp += RankType{1};
      }
    }
    curr[wi] = temp;
  }

  for (auto vi :
       vecRange | ranges::views::drop(1) | ranges::views::drop_last(1)) {
    for (auto wi : weightRange) {
      RankType temp{0};
      for (auto ski : subkeys[vi].subkeyRange()) {
        auto const weight = weights(vi, ski);
        auto const newWeight = wi + weight;
        if (newWeight < maxWeight) {
          temp += curr[newWeight];
        }
      }
      curr[wi] = temp;
    }
  }

  // only need to look at weight 0 in the zeroth distinguishing vector
  RankType temp{0};
  for (auto ski : subkeys.front().subkeyRange()) {
    auto const weight = weights(vecRange.back(), ski);
    if (weight < maxWeight) {
      temp += curr[weight];
    }
  }

  return temp;
}

template <typename RankType, typename WeightType, class DimensionsType>
auto rankAllWeights(WeightType maxWeight,
                    WeightTable<WeightType, DimensionsType> const &weights)
    -> std::vector<RankType> {
  if (maxWeight == 0) {
    throw std::invalid_argument("The max weight ranked up to must > 0");
  }

  auto const &dims = weights.dimensions();
  std::vector<RankType> curr(maxWeight);
  std::vector<RankType> prev(maxWeight, RankType{1});

  auto const vecRange = dims.vectorRange() | ranges::views::reverse;
  auto const &subkeys = dims.asSpans();

  for (auto vi : vecRange) {
    for (auto ski : subkeys[vi].subkeyRange() | ranges::views::reverse) {
      WeightType const weight = weights(vi, ski);
      if (maxWeight >= weight) {
        WeightType const currStart = maxWeight - weight;
        auto const currRange = ranges::views::iota(WeightType{0}, currStart) |
                               ranges::views::reverse;
        auto const prevRange =
            ranges::views::iota(weight, maxWeight) | ranges::views::reverse;
        for (auto const [cwi, pwi] : ranges::views::zip(currRange, prevRange)) {
          curr[cwi] += prev[pwi];
        }
      }
    }
    std::copy(std::cbegin(curr), std::cend(curr), std::begin(prev));
    std::fill(std::begin(curr), std::end(curr), RankType{0});
  }

  // the rank of each weight will be generated in a reverse order, so
  // reverse it to make the data more usable
  std::reverse(std::begin(prev), std::end(prev));

  return prev;
}

} /* namespace rankcpp */
