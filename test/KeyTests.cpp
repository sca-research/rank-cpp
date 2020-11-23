#include <rankcpp/Key.hpp>

#include <rankcpp/BitSpan.hpp>

#include <catch2/catch.hpp>

#include <cstdint>

#include <algorithm>
#include <array>
#include <iterator>
#include <random>
#include <string>

namespace rankcpp {

TEST_CASE("Key# array constructor", "[Key]") {
  SECTION("runtime") {
    std::array<std::uint8_t, 5> const bytes = {0x00, 0x01, 0x02, 0x03, 0x04};
    Key<40> const key(bytes);
    CHECK(std::equal(std::cbegin(bytes), std::cend(bytes),
                     std::cbegin(key.asBytes())));
  }
  SECTION("constexpr") {
    constexpr std::array<std::uint8_t, 5> const bytes = {0x00, 0x01, 0x02, 0x03,
                                                         0x04};
    constexpr Key<40> const key(bytes);
    // TODO20: c++20 for a constexpr std::equal
    static_assert(key.asBytes().size() == 5);
    static_assert(bytes[0] == key.asBytes()[0]);
    static_assert(bytes[1] == key.asBytes()[1]);
    static_assert(bytes[2] == key.asBytes()[2]);
    static_assert(bytes[3] == key.asBytes()[3]);
    static_assert(bytes[4] == key.asBytes()[4]);
  }
}

TEST_CASE("Key# hex constructor", "[Key]") {
  {
    std::array<std::uint8_t, 5> const bytes = {0x00, 0x01, 0x02, 0x03, 0x04};
    Key<40> const key("0001020304");
    CHECK(std::equal(std::cbegin(bytes), std::cend(bytes),
                     std::cbegin(key.asBytes())));
  }
  {
    std::array<std::uint8_t, 8> const bytes = {0x00, 0x01, 0x02, 0x03,
                                               0x04, 0x05, 0x06, 0x07};
    Key<64> const key("0001020304050607");
    CHECK(std::equal(std::cbegin(bytes), std::cend(bytes),
                     std::cbegin(key.asBytes())));
  }
  {
    std::array<std::uint8_t, 1> const bytes = {0x05};
    Key<4> const key("05");
    CHECK(std::equal(std::cbegin(bytes), std::cend(bytes),
                     std::cbegin(key.asBytes())));
  }
  {
    CHECK_THROWS_AS(Key<40>("000102030405"), std::length_error);
    CHECK_THROWS_AS(Key<10>("01"), std::length_error);
  }
}

TEST_CASE("Key# initializer_list constructor", "[Key]") {
  SECTION("runtime") {
    Key<40> const key = {0x00, 0x01, 0x02, 0x03, 0x04};
    std::array<std::uint8_t, 5> const bytes = {0x00, 0x01, 0x02, 0x03, 0x04};
    CHECK(std::equal(std::cbegin(bytes), std::cend(bytes),
                     std::cbegin(key.asBytes())));
  }
  SECTION("errors") {
    CHECK_THROWS_AS(Key<40>({0x00, 0x01, 0x02, 0x03}), std::length_error);
    CHECK_THROWS_AS(Key<40>({0x00, 0x01, 0x02, 0x03, 0x04, 0x05}),
                    std::length_error);
  }
}

TEST_CASE("Key# asLeIntegerValue", "[Key]") {
  SECTION("40") {
    Key<40> const key({0x00, 0x01, 0x02, 0x03, 0x04});
    CHECK(17230332160UL == key.asLeIntegerValue<std::uint64_t>());
  }
  SECTION("60") {
    Key<64> const key({0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07});
    CHECK(506097522914230528UL == key.asLeIntegerValue<std::uint64_t>());
    CHECK_THROWS_AS(key.asLeIntegerValue<std::uint32_t>(), std::out_of_range);
  }
  SECTION("40 2") {
    Key<40> const key({0x00, 0x01, 0x02, 0x03, 0x04});
    CHECK(17230332160UL == key.asLeIntegerValue<std::uint64_t>());
  }
}

TEST_CASE("randomKey", "[Key]") {
  // Seed the RNGs with the same value -- the produced keys should be equal
  std::mt19937_64 rng1(1394);
  std::mt19937_64 rng2(1394);
  auto const key1 = randomKey<256, decltype(rng1)>(rng1);
  auto const key2 = randomKey<256, decltype(rng2)>(rng2);
  CHECK(key1.asBytes().size() == 32);
  CHECK(key2.asBytes().size() == 32);
  CHECK(std::equal(std::cbegin(key1.asBytes()), std::cend(key1.asBytes()),
                   std::cbegin(key2.asBytes())));
}

TEST_CASE("Key# subkeyValue (single byte)", "[Key]") {
  SECTION("runtime") {
    std::array<std::uint8_t, 16> const keyBytes = {
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07};
    std::uint64_t const expected = 7;
    Key<128> const key(keyBytes);
    auto const actual = key.subkeyValue<std::uint64_t>(BitSpan{0, 8});
    CHECK(expected == actual);
  }
}

TEST_CASE("Key# subkeyValue (two bytes)", "[Key]") {
  std::array<uint8_t, 4> const keyBytes = {0x01, 0x02, 0x03, 0x04};
  std::uint64_t const expected = 770;
  Key<32> const key(keyBytes);
  auto const actual = key.subkeyValue<std::uint64_t>(BitSpan{8, 16});
  CHECK(expected == actual);
}

TEST_CASE("Key# subkeyValue (single bit)", "[Key]") {
  std::array<uint8_t, 4> const keyBytes = {0x01, 0x02, 0x03, 0x04};
  std::uint64_t const expected = 1;
  Key<32> const key(keyBytes);
  auto const actual = key.subkeyValue<std::uint64_t>(BitSpan{0, 1});
  CHECK(expected == actual);
}

TEST_CASE("Key# subkeyValue (truncated)", "[Key]") {
  Key<11> const key("6502");
  // Bits 6, 7, 8 and 9 (so last 2 from the first byte, and the first 2 from
  // the second byte) = 0b1001
  std::uint64_t const expected = 9;
  auto const actual = key.subkeyValue<std::uint64_t>(BitSpan{6, 4});
  CHECK(expected == actual);
}

} /* namespace rankcpp */
