#pragma once

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <cstdint>

namespace rankcpp {

template <std::uint32_t LengthBits>
using BoostBigUint = typename boost::multiprecision::number<
    boost::multiprecision::cpp_int_backend<
        LengthBits, LengthBits, boost::multiprecision::unsigned_magnitude,
        boost::multiprecision::unchecked, void>>;

template <std::uint32_t Digits10>
using BoostBigReal = typename boost::multiprecision::number<
    boost::multiprecision::cpp_dec_float<Digits10>>;

template <std::uint32_t LengthBits>
auto log2(BoostBigUint<LengthBits> n) noexcept -> double {
  constexpr auto const FloatBits = 100;
  auto const realValue = static_cast<BoostBigReal<FloatBits>>(n);
  BoostBigReal<FloatBits> const two(2.0);
  auto const logTwo = boost::multiprecision::log(two);
  BoostBigReal<FloatBits> const logValue =
      boost::multiprecision::log(realValue) / logTwo;
  return static_cast<double>(logValue);
}

} /* namespace rankcpp */
