#include <rankcpp/WeightTable.hpp>

#include <rankcpp/Dimensions.hpp>
#include <rankcpp/Key.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <iterator>
#include <random>
#include <vector>

namespace rankcpp {

// TODO variable width tests
// TODO error cases

TEMPLATE_TEST_CASE("WeightTable#weight", "[WeightTable]", std::uint64_t,
                   std::uint32_t, std::uint16_t, std::uint8_t) {
  Dimensions const dims(2, 2);
  std::vector<TestType> weights = {3, 4, 6, 7, 0, 1, 3, 4};
  WeightTable<TestType> table(dims, weights);
  SECTION("checked") {
    CHECK(3 == table.weight(0, 0));
    CHECK(4 == table.weight(0, 1));
    CHECK(6 == table.weight(0, 2));
    CHECK(7 == table.weight(0, 3));
    CHECK(0 == table.weight(1, 0));
    CHECK(1 == table.weight(1, 1));
    CHECK(3 == table.weight(1, 2));
    CHECK(4 == table.weight(1, 3));
  }
  SECTION("unchecked") {
    CHECK(3 == table(0, 0));
    CHECK(4 == table(0, 1));
    CHECK(6 == table(0, 2));
    CHECK(7 == table(0, 3));
    CHECK(0 == table(1, 0));
    CHECK(1 == table(1, 1));
    CHECK(3 == table(1, 2));
    CHECK(4 == table(1, 3));
  }
  SECTION("checked set") {
    table.weight(1, 0) = 6;
    CHECK(6 == table.weight(1, 0));
  }
  SECTION("unchecked set") {
    table(1, 0) = 7;
    CHECK(7 == table(1, 0));
  }
}

TEMPLATE_TEST_CASE("WeightTable# initializer_list constructor", "[WeightTable]",
                   std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
  Dimensions const dims(2, 2);
  WeightTable<TestType> table(dims, {3, 4, 6, 7, 0, 1, 3, 4});
  CHECK(3 == table.weight(0, 0));
  CHECK(4 == table.weight(0, 1));
  CHECK(6 == table.weight(0, 2));
  CHECK(7 == table.weight(0, 3));
  CHECK(0 == table.weight(1, 0));
  CHECK(1 == table.weight(1, 1));
  CHECK(3 == table.weight(1, 2));
  CHECK(4 == table.weight(1, 3));
}

TEMPLATE_TEST_CASE("WeightTable#get weight of key", "[WeightTable]",
                   std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
  TestType const expected = 3 + 3 + 5;
  SECTION("6 bits") {
    Dimensions const dims(3, 2);
    WeightTable<TestType> const table(dims,
                                      {4, 3, 1, 1, 6, 4, 3, 1, 5, 7, 8, 9});
    Key<6> const secretKey("09");
    CHECK(expected == table.weightForKey(secretKey));
  }
  SECTION("36 bits") {
    Dimensions const dims(3, 12);
    auto const vectorSize = 1 << 12;
    std::vector<TestType> weights(12288);
    weights[0 * vectorSize + 513] = 3;
    weights[1 * vectorSize + 48] = 3;
    weights[2 * vectorSize + 772] = 5;
    WeightTable<TestType> const table(dims, weights);
    Key<36> const secretKey("0102030403");
    CHECK(expected == table.weightForKey(secretKey));
  }
}

TEMPLATE_TEST_CASE("WeightTable#rebase (minus, 0)", "[WeightTable]",
                   std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
  Dimensions const dims(3, 2);
  WeightTable<TestType> table(dims, {9, 3, 4, 1, 6, 4, 3, 1, 5, 7, 4, 1});
  table.rebase(0);

  std::vector<TestType> const expected = {8, 2, 3, 0, 5, 3, 2, 0, 4, 6, 3, 0};
  CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                   std::cbegin(table.allWeights())));
}

TEMPLATE_TEST_CASE("WeightTable#rebase (to 0, minus) vector 2", "[WeightTable]",
                   std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
  Dimensions const dims(3, 2);
  WeightTable<TestType> table(dims, {10, 4, 5, 2, 7, 5, 4, 2, 6, 8, 5, 2});
  table.rebase(0);
  std::vector<TestType> const expected = {8, 2, 3, 0, 5, 3, 2, 0, 4, 6, 3, 0};
  CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                   std::cbegin(table.allWeights())));
}

TEMPLATE_TEST_CASE("WeightTable#rebase (to 1, minus)", "[WeightTable]",
                   std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
  Dimensions const dims(3, 2);
  WeightTable<TestType> table(dims, {9, 3, 4, 2, 6, 4, 3, 2, 5, 7, 4, 2});
  table.rebase(1);
  std::vector<TestType> const expected = {8, 2, 3, 1, 5, 3, 2, 1, 4, 6, 3, 1};
  CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                   std::cbegin(table.allWeights())));
}
// TODO more tests

TEMPLATE_TEST_CASE("WeightTable#minimum/maximum weight", "[WeightTable]",
                   std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
  Dimensions const dims(3, 2);
  WeightTable<TestType> const table(dims, {4, 3, 1, 1, 6, 4, 3, 1, 5, 7, 4, 1});
  CHECK(3 == table.minimumWeight());
  CHECK(17 == table.maximumWeight());
}

TEMPLATE_TEST_CASE("WeightTable#minimum weight (with zero weights)",
                   "[WeightTable]", std::uint64_t, std::uint32_t, std::uint16_t,
                   std::uint8_t) {
  Dimensions const dims(3, 2);
  WeightTable<TestType> const table(dims, {4, 3, 1, 0, 6, 4, 3, 0, 5, 7, 4, 1});
  CHECK(1 == table.minimumWeight());
}

TEMPLATE_TEST_CASE("WeightTable#maximum weight (with zero weights)",
                   "[WeightTable]", std::uint64_t, std::uint32_t, std::uint16_t,
                   std::uint8_t) {
  Dimensions const dims(3, 2);
  WeightTable<TestType> const table(dims, {0, 0, 0, 0, 6, 4, 3, 1, 5, 7, 4, 1});
  CHECK(13 == table.maximumWeight());
}

TEMPLATE_TEST_CASE("WeightTable#sort ascending / descending", "[WeightTable]",
                   std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
  Dimensions const dims(3, 2);
  WeightTable<TestType> table(dims, {0, 3, 4, 1, 6, 4, 3, 1, 5, 7, 4, 1});

  SECTION("ascending") {
    table.sortAscending();
    std::vector<TestType> const expected = {0, 1, 3, 4, 1, 3, 4, 6, 1, 4, 5, 7};
    CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                     std::cbegin(table.allWeights())));
  }

  SECTION("descending") {
    table.sortDescending();
    std::vector<TestType> const expected = {4, 3, 1, 0, 6, 4, 3, 1, 7, 5, 4, 1};
    CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                     std::cbegin(table.allWeights())));
  }
}

/*TEMPLATE_TEST_CASE("WeightTable#sort ascending and track indexes",
"[WeightTable]", std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
    std::array<TestType, 12> const weights = {0, 3, 4, 1, 6, 4,
                                              3, 1, 5, 7, 4, 1};
    WeightTable<TestType> table(weights);
    std::array<uint8_t, 3 * 4> indexes;
    table.template sortAscendingAndTrackIndexes<uint8_t>(indexes);

    std::array<TestType, 12> const expected = {0, 1, 3, 4, 1, 3,
                                               4, 6, 1, 4, 5, 7};
    CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                     std::cbegin(table.allWeights())));

    std::vector<uint8_t> const expectedIndexes = {0, 3, 1, 2, 3, 2,
                                                  1, 0, 3, 2, 0, 1};
    CHECK(std::equal(std::cbegin(expectedIndexes), std::cend(expectedIndexes),
                     std::cbegin(indexes)));
}*/

TEMPLATE_TEST_CASE("WeightTable#mapToWeight (4 bits)", "[WeightTable]",
                   std::uint64_t, std::uint32_t, std::uint16_t, std::uint8_t) {
  using ScoresType = double;
  using WeightType = TestType;
  std::size_t const vectorSize = 8;
  Dimensions const dims(2, 3);
  std::vector<ScoresType> scores(2 * vectorSize);

  // Generate random data
  std::mt19937 generator(5);
  std::uniform_real_distribution<ScoresType> dist(-5.0, 5.0);
  std::generate(std::begin(scores), std::end(scores),
                [&generator, &dist] { return dist(generator); });

  ScoresTable<ScoresType> scoresTable(dims, scores);
  scoresTable.translateVectorsToPositive();
  scoresTable.normaliseVectors();
  scoresTable.log2();
  scoresTable.abs();

  std::uint32_t const precisionBits = 4;
  auto const weightTable =
      mapToWeight<ScoresType, WeightType>(scoresTable, precisionBits);
  auto const &weights = weightTable.allWeights();

  auto const maxScore =
      *std::max_element(std::cbegin(weights), std::cend(weights));
  CHECK(maxScore < 16);
}

} /* namespace rankcpp */
