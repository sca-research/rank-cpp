#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace rankcpp {

template <typename InputIt>
constexpr auto kahanSum(InputIt first, InputIt last) ->
    typename std::iterator_traits<InputIt>::value_type {
  using T = typename std::iterator_traits<InputIt>::value_type;
  static_assert(std::is_arithmetic_v<T>,
                "value type must be an arithmetic type");

  T sum = {0};
  T sumC = {0};

  std::for_each(first, last, [&sum, &sumC](auto const &score) {
    auto const sumY = score - sumC;
    auto const sumT = sum + sumY;
    sumC = (sumT - sum) - sumY;
    sum = sumT;
  });

  return sum;
}

} /* namespace rankcpp */
