#include <rankcpp/BitSpan.hpp>

#include <catch2/catch.hpp>

#include <limits>
#include <stdexcept>

namespace rankcpp {

TEST_CASE("BitSpan#end", "[BitSpan]") {
  BitSpan const bitSpan1(0, 1);
  CHECK(0 == bitSpan1.end());
  BitSpan const bitSpan2(1, 4);
  CHECK(4 == bitSpan2.end());
  BitSpan const bitSpan3(7, 16);
  CHECK(22 == bitSpan3.end());
}

TEST_CASE("BitSpan# invalid construction", "[BitSpan]") {
  CHECK_THROWS_AS(BitSpan(0, 0), std::invalid_argument);
  auto const max = std::numeric_limits<std::uint32_t>::max();
  CHECK_THROWS_AS(BitSpan(max, 1), std::overflow_error);
}

TEMPLATE_TEST_CASE("BitSpan#valueCount", "[BitSpan]", std::size_t,
                   std::uint64_t, std::uint32_t) {
  BitSpan const bitSpan1(2, 1);
  CHECK(2 == bitSpan1.valueCount<TestType>());
  BitSpan const bitSpan2(2, 2);
  CHECK(4 == bitSpan2.valueCount<TestType>());
  BitSpan const bitSpan3(0, 8);
  CHECK(256 == bitSpan3.valueCount<TestType>());
  BitSpan const bitSpan4(12, 16);
  CHECK(65536 == bitSpan4.valueCount<TestType>());
  if constexpr (std::numeric_limits<TestType>::digits >= 64) {
    BitSpan const bitSpan5(18, 32);
    CHECK(4294967296UL == bitSpan5.valueCount<TestType>());
  }
  BitSpan const bitSpan6(0, std::numeric_limits<TestType>::digits + 1);
  CHECK_THROWS_AS(bitSpan6.valueCount<TestType>(), std::overflow_error);
}

TEST_CASE("BitSpan#encapsulates", "[BitSpan]") {
  {
    BitSpan const container(18, 32);
    BitSpan const bitSpan(18, 36);
    CHECK(false == container.encapsulates(bitSpan));
  }
  {
    BitSpan const container(0, 1);
    BitSpan const bitSpan(0, 1);
    CHECK(true == container.encapsulates(bitSpan));
  }
  {
    BitSpan const container(5, 5);
    BitSpan const bitSpan(9, 1);
    CHECK(true == container.encapsulates(bitSpan));
  }
  {
    BitSpan const container(5, 1);
    BitSpan const bitSpan(10, 1);
    CHECK(false == container.encapsulates(bitSpan));
  }
  {
    BitSpan const container(18, 32);
    BitSpan const bitSpan(17, 5);
    CHECK(false == container.encapsulates(bitSpan));
  }
  SECTION("constexpr") {
    constexpr BitSpan container(18, 32);
    constexpr BitSpan bitSpan(17, 5);
    static_assert(!container.encapsulates(bitSpan));
  }
}

TEST_CASE("BitSpan# equality", "[BitSpan]") {
  constexpr BitSpan const bitSpan1(0, 1);
  constexpr BitSpan const bitSpan2(1, 4);
  constexpr BitSpan const bitSpan3(1, 4);
  SECTION("runtime") {
    CHECK(bitSpan1 == bitSpan1);
    CHECK(bitSpan1 != bitSpan2);
    CHECK(bitSpan1 != bitSpan3);
    CHECK(bitSpan2 == bitSpan3);
  }
  SECTION("constexpr") {
    static_assert(bitSpan1 == bitSpan1);
    static_assert(bitSpan1 != bitSpan2);
    static_assert(bitSpan1 != bitSpan3);
    static_assert(bitSpan2 == bitSpan3);
  }
}

TEMPLATE_TEST_CASE("BitSpan#subkeyRange", "[BitSpan]", std::size_t,
                   std::uint64_t, std::uint32_t) {
  SECTION("1 bit / runtime") {
    BitSpan const bitSpan(2, 1);
    auto range = bitSpan.subkeyRange<TestType>();
    CHECK(2 == range.size());
    CHECK(0 == *(range.begin()));
    CHECK(1 == *(range.begin() + 1));
  }
  SECTION("2 bits / runtime") {
    BitSpan const bitSpan(2, 2);
    auto range = bitSpan.subkeyRange<TestType>();
    CHECK(4 == range.size());
    CHECK(0 == *(range.begin()));
    CHECK(1 == *(range.begin() + 1));
    CHECK(2 == *(range.begin() + 2));
    CHECK(3 == *(range.begin() + 3));
  }
  SECTION("32 bits / runtime") {
    if constexpr (std::numeric_limits<TestType>::digits >= 64) {
      BitSpan const bitSpan(18, 32);
      auto range = bitSpan.subkeyRange<TestType>();
      CHECK(4294967296UL == range.size());
      CHECK(0 == *(range.begin()));
      CHECK(4294967296UL == *(range.end()));
    }
  }
  SECTION("exception") {
    BitSpan const bitSpan(0, std::numeric_limits<TestType>::digits + 1);
    CHECK_THROWS_AS(bitSpan.subkeyRange<TestType>(), std::overflow_error);
  }
}

} /* namespace rankcpp */