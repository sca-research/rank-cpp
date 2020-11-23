#include <rankcpp/utils/Encoding.hpp>

#include <catch2/catch.hpp>

#include <gsl/span>

#include <algorithm>
#include <array>
#include <iterator>
#include <stdexcept>

namespace rankcpp {

TEST_CASE("hexToBytes", "[Encoding]") {
  std::array<std::uint8_t, 4> buf{};
  gsl::span<std::uint8_t, 4> out = buf;
  SECTION("good") {
    hexToBytes("01020304", out);
    std::array<std::uint8_t, 4> const expected1 = {0x01, 0x02, 0x03, 0x04};
    CHECK(std::equal(std::cbegin(expected1), std::cend(expected1),
                     std::cbegin(out)));
    hexToBytes("0102", out);
    std::array<std::uint8_t, 2> const expected2 = {0x01, 0x02};
    CHECK(std::equal(std::cbegin(expected2), std::cend(expected2),
                     std::cbegin(out)));
  }
  SECTION("zero") {
    hexToBytes("", out);
    std::array<std::uint8_t, 0> const expected = {};
    CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                     std::cbegin(out)));
  }
  SECTION("length error") {
    CHECK_THROWS_AS(hexToBytes("0102030405", out), std::length_error);
  }
}

} /* namespace rankcpp */