#include <rankcpp/Dimensions.hpp>

#include <rankcpp/BitSpan.hpp>

#include <catch2/catch.hpp>

#include <algorithm>
#include <array>
#include <cstdint>

namespace rankcpp {

TEST_CASE("Dimensions# fixed width constructor", "[Dimensions]") {
  SECTION("normal") {
    Dimensions const d(2, 4);
    CHECK(d.vectorCount() == 2);
    CHECK(d.vectorWidthBits(0) == 4);
    CHECK(d.vectorWidthBits(1) == 4);
    CHECK(d.keyLengthBits() == 8);
    CHECK(d.keyByteCount() == 1);
    CHECK(d.subkeyCount(0) == 16);
    CHECK(d.subkeyCount(1) == 16);
    CHECK(d.scoresCount() == 32);
    CHECK(d.scoresBeforeCount(0) == 0);
    CHECK(d.scoresBeforeCount(1) == 16);
    CHECK(d.bitOffset(0) == 0);
    CHECK(d.bitOffset(1) == 4);
  }
}

TEST_CASE("Dimensions# variable width constructor", "[Dimensions]") {
  auto runTest = [](Dimensions const &d) {
    CHECK(d.vectorCount() == 2);
    CHECK(d.vectorWidthBits(0) == 4);
    CHECK(d.vectorWidthBits(1) == 8);
    CHECK(d.keyLengthBits() == 12);
    CHECK(d.keyByteCount() == 2);
    CHECK(d.subkeyCount(0) == 16);
    CHECK(d.subkeyCount(1) == 256);
    CHECK(d.scoresCount() == 16 + 256);
    CHECK(d.scoresBeforeCount(0) == 0);
    CHECK(d.scoresBeforeCount(1) == 16);
    CHECK(d.bitOffset(0) == 0);
    CHECK(d.bitOffset(1) == 4);
  };
  SECTION("array constructor") {
    std::array<std::uint32_t, 2> const widths{4, 8};
    Dimensions const d(widths);
    runTest(d);
  }
  SECTION("initializer_list constructor") {
    Dimensions const d({4, 8});
    runTest(d);
  }
}

TEST_CASE("Dimensions# asSpans", "[Dimensions]") {
  Dimensions const d({4, 8});
  auto const &spans = d.asSpans();
  std::vector<BitSpan> const expected = {{0, 4}, {4, 8}};
  CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                   std::cbegin(spans)));
}

TEST_CASE("Dimensions# isEqualWidth", "[Dimensions]") {
  {
    Dimensions const d({4, 8});
    CHECK(false == d.isEqualWidth());
  }
  {
    Dimensions const d({8, 8});
    CHECK(true == d.isEqualWidth());
  }
  {
    Dimensions const d({8});
    CHECK(true == d.isEqualWidth());
  }
}

TEST_CASE("FixedDimensions# constructor", "[FixedDimensions]") {
  SECTION("runtime") {
    FixedDimensions<2, 4> const d;
    CHECK(d.vectorCount() == 2);
    CHECK(d.vectorWidthBits(0) == 4);
    CHECK(d.vectorWidthBits(1) == 4);
    CHECK(d.keyLengthBits() == 8);
    CHECK(d.keyByteCount() == 1);
    CHECK(d.subkeyCount(0) == 16);
    CHECK(d.subkeyCount(1) == 16);
    CHECK(d.scoresCount() == 32);
    CHECK(d.scoresBeforeCount(0) == 0);
    CHECK(d.scoresBeforeCount(1) == 16);
    CHECK(d.bitOffset(0) == 0);
    CHECK(d.bitOffset(1) == 4);
  }
  SECTION("constexpr") {
    constexpr FixedDimensions<2, 4> const d;
    static_assert(d.vectorCount() == 2);
    static_assert(d.vectorWidthBits(0) == 4);
    static_assert(d.vectorWidthBits(1) == 4);
    static_assert(d.keyLengthBits() == 8);
    static_assert(d.keyByteCount() == 1);
    static_assert(d.subkeyCount(0) == 16);
    static_assert(d.subkeyCount(1) == 16);
    static_assert(d.scoresCount() == 32);
    static_assert(d.scoresBeforeCount(0) == 0);
    static_assert(d.scoresBeforeCount(1) == 16);
    static_assert(d.bitOffset(0) == 0);
    static_assert(d.bitOffset(1) == 4);
  }
}

TEST_CASE("FixedDimensions# asSpans", "[FixedDimensions]") {
  FixedDimensions<2, 8> const d;
  auto const &spans = d.asSpans();
  std::array<BitSpan, 2> const expected = {BitSpan{0, 8}, BitSpan{8, 8}};
  CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                   std::cbegin(spans)));
}

TEST_CASE("FixedDimensions# isEqualWidth", "[FixedDimensions]") {
  SECTION("runtime") {
    FixedDimensions<2, 8> const d;
    CHECK(true == d.isEqualWidth());
  }
  SECTION("constexpr") {
    constexpr FixedDimensions<2, 8> const d;
    static_assert(d.isEqualWidth());
  }
}

} /* namespace rankcpp */