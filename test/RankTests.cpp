#include <rankcpp/Rank.hpp>

#include <rankcpp/Dimensions.hpp>
#include <rankcpp/Key.hpp>
#include <rankcpp/ScoresTable.hpp>
#include <rankcpp/WeightTable.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <cstdint>
#include <iterator>
#include <vector>

namespace rankcpp {

/**
 * A hand-worked example given two 2-bit distinguishing vectors dv0 and dv1.
 * Each vector consists of four subkeys sk0,...,sk3.  Weights:
 *
 *  |     | dv0 | dv1 |
 *  |-----|-----|-----|
 *  | sk0 | 0   | 0   |
 *  | sk1 | 1   | 2   |
 *  | sk2 | 3   | 3   |
 *  | sk3 | 0   | 0   |
 *
 * The correct key is 0b0110 or 0x06.  The maximum weight of all key candidates
 * is 3+3=6, and so if we rank all weights strictly less than 7 we will generate
 * the rank of every single weight, and thus every key candidate.
 *
 * The following diagram captures the state of the 'current' vector of ranks as
 * the algorithm progresses when ranking all of the weights.
 *
 *  | dv | sk | weight               |
 *  |    |    |----------------------|
 *  |    |    | 0  1  2  3  4  5  6  |
 *  |----|---------------------------|
 *  |    | 0  | 16 15 14 13 8  6  4  | no skips
 *  | 0  | 1  | 12 11 10 9  5  4  2  | 6
 *  |    | 2  | 8  7  6  6  3  2  2  | 6, 5, 4
 *  |    | 3  | 4  4  4  4  3  2  2  | no skips
 *  |----|----|----------------------|
 *  |    | 0  | 4  4  4  4  3  2  2  | no skips
 *  | 1  | 1  | 3  3  3  3  2  1  1  | 6, 5
 *  |    | 2  | 2  2  2  2  1  1  1  | 6, 5, 4
 *  |    | 3  | 1  1  1  1  1  1  1  | no skips
 */
TEST_CASE("Rank#rank two vectors", "[Rank]") {
  using WeightType = std::uint64_t;
  using RankType = std::uint32_t;
  Dimensions const dims(2, 2);
  Key<4> const key("06");
  WeightTable<WeightType> const table(dims, {0, 1, 3, 0, 0, 2, 3, 0});
  auto const keyWeight = table.weightForKey<4>(key); // 5
  SECTION("standard") {
    auto const actual = rank<RankType>(keyWeight, table);
    CHECK(14 == actual);
  }
  SECTION("standard, using key") {
    auto const actual = rank<4, RankType>(key, table);
    CHECK(14 == actual);
  }
  SECTION("lowmem") {
    auto const actual = rankLowMem<RankType>(keyWeight, table);
    CHECK(14 == actual);
  }
  SECTION("rankAllWeights") {
    auto const actual = rankAllWeights<RankType, WeightType>(7, table);
    std::array<RankType, 7> const expected = {4, 6, 8, 13, 14, 15, 16};
    CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                     std::cbegin(actual)));
  }
}

/**
 * A hand-worked example given three 2-bit distinguishing vectors dv0, dv1, dv2.
 * Each vector consists of four subkeys sk0,...,sk3. Weights:
 *
 *  |     | dv0 | dv1 | dv2 |
 *  |-----|-----|-----|-----|
 *  | sk0 | 1   | 1   | 1   |
 *  | sk1 | 2   | 3   | 1   |
 *  | sk2 | 4   | 4   | 2   |
 *  | sk3 | 1   | 1   | 2   |
 *
 * The correct key is (1, 2, 1): 0b011001 or 0x19.  The maximum weight of all
 * key candidates is 4+4+2=10, and so if we rank all weights strictly less than
 * 11 we will generate the rank of every single weight, and thus every key
 * candidate.
 *
 * The following diagram captures the state of the 'current' vector of ranks as
 * the algorithm progresses when ranking all of the weights.
 *
 *  | dv | sk | weight                           |
 *  |    |    |----------------------------------|
 *  |    |    | 0  1  2  3  4  5  6  7  8  9  10 |
 *  |----|---------------------------------------|
 *  |    | 0  | 64 62 58 54 42 28 20 8  0  0  0  |
 *  | 0  | 1  | 48 46 42 38 28 18 12 4  0  0  0  |
 *  |    | 2  | 32 30 26 24 18 10 8  4  0  0  0  |
 *  |    | 3  | 16 16 16 16 14 10 8  4  0  0  0  |
 *  |----|----|----------------------------------|
 *  |    | 0  | 16 16 16 16 16 14 10 8  4  0  0  |
 *  | 1  | 1  | 12 12 12 12 12 10 6  4  2  0  0  |
 *  |    | 2  | 8  8  8  8  8  6  4  4  2  0  0  |
 *  |    | 3  | 4  4  4  4  4  4  4  4  2  0  0  |
 *  |----|----|----------------------------------|
 *  |    | 0  | 4  4  4  4  4  4  4  4  4  2  0  |
 *  | 2  | 1  | 3  3  3  3  3  3  3  3  3  1  0  |
 *  |    | 2  | 2  2  2  2  2  2  2  2  2  0  0  |
 *  |    | 3  | 1  1  1  1  1  1  1  1  1  0  0  |
 */
TEST_CASE("Rank# three vectors", "[Rank]") {
  using WeightType = std::uint64_t;
  using RankType = std::uint32_t;
  Dimensions const dims(3, 2);
  WeightTable<WeightType> const table(dims,
                                      {1, 2, 4, 1, 1, 3, 4, 1, 1, 1, 2, 2});
  Key<6> const key("19");
  auto const keyWeight = table.weightForKey<6>(key);
  SECTION("standard") {
    auto const actual = rank<RankType>(keyWeight, table);
    CHECK(42 == actual);
  }
  SECTION("lowmem") {
    auto const actual = rankLowMem<RankType>(keyWeight, table);
    CHECK(42 == actual);
  }
  SECTION("rankAllWeights") {
    auto const actual = rankAllWeights<RankType, WeightType>(11, table);
    std::array<RankType, 11> const expected = {0,  0,  0,  8,  20, 28,
                                               42, 54, 58, 62, 64};
    CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                     std::cbegin(actual)));
  }
}

/**
 * A hand-worked example given one 3-bit distinguishing vectors dv0 and one
 * 2-bit distinguishing vector dv1.  dv0 consists of eight subkeys sk0,...,sk7,
 * and dv1 four subkeys sk0,...,sk3.  Weights:
 *
 *  |     | dv0 | dv1 |     | dv0 | dv1 |
 *  |-----|-----|-----|-----|-----|-----|
 *  | sk0 | 1   | 1   | sk4 | 2   | -   |
 *  | sk1 | 1   | 2   | sk5 | 1   | -   |
 *  | sk2 | 3   | 3   | sk6 | 2   | -   |
 *  | sk3 | 1   | 1   | sk7 | 1   | -   |
 *
 * The correct key is (2,3): 0b01010 or 0x1A.
 *
 * The following diagram captures the state of the 'current' vector of ranks as
 * the algorithm progresses when ranking all of the weights.   The maximum
 * weight of all key candidates is 3+3=6, and so if we rank all weights strictly
 * less than 7 we will generate the rank of every single weight, and thus every
 * key candidate.
 *
 *  | dv | sk | weight               |
 *  |    |    |----------------------|
 *  |    |    | 0  1  2  3  4  5  6  |
 *  |----|---------------------------|
 *  |    | 0  | 32 31 28 19 10 0  0  |
 *  | 0  | 1  | 28 27 24 16 8  0  0  |
 *  |    | 2  | 24 23 20 13 6  0  0  |
 *  |    | 3  | 20 20 18 13 6  0  0  |
 *  |    | 4  | 16 16 14 10 4  0  0  |
 *  |    | 5  | 12 12 11 8  4  0  0  |
 *  |    | 6  | 8  8  7  5  2  0  0  |
 *  |    | 7  | 4  4  4  3  2  0  0  |
 *  |----|----|----------------------|
 *  |    | 0  | 4  4  4  4  3  2  0  |
 *  | 1  | 1  | 3  3  3  3  2  1  0  |
 *  |    | 2  | 2  2  2  2  1  1  0  |
 *  |    | 3  | 1  1  1  1  1  1  0  |
 */
TEST_CASE("Rank#rank unbalanced vectors", "[Rank]") {
  using WeightType = std::uint64_t;
  using RankType = std::uint32_t;
  Dimensions const dims({3, 2});
  WeightTable<WeightType> const table(dims,
                                      {1, 1, 3, 1, 2, 1, 2, 1, 1, 2, 3, 1});
  Key<6> const key("1A");
  auto const keyWeight = table.weightForKey<6>(key);
  SECTION("standard") {
    auto const actual = rank<RankType>(keyWeight, table);
    CHECK(19 == actual);
  }
  SECTION("standard, using key") {
    auto const actual = rank<6, RankType>(key, table);
    CHECK(19 == actual);
  }
  SECTION("lowmem") {
    auto const actual = rankLowMem<RankType>(keyWeight, table);
    CHECK(19 == actual);
  }
  SECTION("rankAllWeights") {
    auto const actual = rankAllWeights<RankType, WeightType>(7, table);
    std::array<RankType, 7> const expected = {0, 0, 10, 19, 28, 31, 32};
    CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                     std::cbegin(actual)));
  }
}

TEST_CASE("Rank#rank, rank = 0", "[Rank]") {
  using WeightType = std::uint64_t;
  using RankType = std::uint32_t;
  Dimensions const dims(2, 2);
  Key<4> const key("06");
  WeightTable<WeightType> const table(dims, {11, 15, 3, 6, 7, 2, 6, 19});

  auto const keyWeight = table.weightForKey(key);
  SECTION("standard") {
    auto const actual = rank<RankType>(keyWeight, table);
    CHECK(0 == actual);
  }
  SECTION("standard, using key") {
    auto const actual = rank<4, RankType>(key, table);
    CHECK(0 == actual);
  }
  SECTION("lowmem") {
    auto const actual = rankLowMem<RankType>(keyWeight, table);
    CHECK(0 == actual);
  }
}

} /* namespace rankcpp */
