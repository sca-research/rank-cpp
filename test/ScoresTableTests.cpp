#include <rankcpp/ScoresTable.hpp>

#include <rankcpp/Dimensions.hpp>

#include <catch2/catch.hpp>

#include <algorithm>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

namespace rankcpp {

TEMPLATE_TEST_CASE("ScoresTable#score", "[ScoresTable]", double) {
  Dimensions const dims(2, 2);
  std::vector<TestType> const scores = {3, 4, 6, 7, 0, 1, 3, 4};
  ScoresTable<TestType> table(dims, scores);
  SECTION("checked") {
    CHECK(3 == table.score(0, 0));
    CHECK(4 == table.score(0, 1));
    CHECK(6 == table.score(0, 2));
    CHECK(7 == table.score(0, 3));
    CHECK(0 == table.score(1, 0));
    CHECK(1 == table.score(1, 1));
    CHECK(3 == table.score(1, 2));
    CHECK(4 == table.score(1, 3));
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
    table.score(1, 0) = 6;
    CHECK(6 == table.score(1, 0));
  }
  SECTION("unchecked set") {
    table(1, 0) = 7;
    CHECK(7 == table(1, 0));
  }
}

TEMPLATE_TEST_CASE("ScoresTable# initializer_list", "[ScoresTable]", double,
                   float) {
  Dimensions const dims(2, 2);
  ScoresTable<TestType> table(dims, {3, 4, 6, 7, 0, 1, 3, 4});
  CHECK(3 == table.score(0, 0));
  CHECK(4 == table.score(0, 1));
  CHECK(6 == table.score(0, 2));
  CHECK(7 == table.score(0, 3));
  CHECK(0 == table.score(1, 0));
  CHECK(1 == table.score(1, 1));
  CHECK(3 == table.score(1, 2));
  CHECK(4 == table.score(1, 3));
}

TEMPLATE_TEST_CASE("ScoresTable#normaliseVectors", "[ScoresTable]", double,
                   float) {
  std::size_t const vectorSize = 8;
  Dimensions const dims(2, 3);
  std::vector<TestType> scores(2 * vectorSize);

  // Generate random data
  std::mt19937 generator(5);
  std::uniform_real_distribution<TestType> dist(-5.0, 5.0);
  std::generate(std::begin(scores), std::end(scores),
                [&generator, &dist] { return dist(generator); });

  ScoresTable<TestType> table(dims, scores);
  table.normaliseVectors();

  // Actual
  auto const &allScores = table.allScores();
  auto const sum1 = std::accumulate(std::cbegin(allScores),
                                    std::cbegin(allScores) + vectorSize, 0.0);
  auto const sum2 = std::accumulate(std::cbegin(allScores) + vectorSize,
                                    std::cend(allScores), 0.0);
  CHECK(Approx(1.0) == sum1);
  CHECK(Approx(1.0) == sum2);
}

TEMPLATE_TEST_CASE("ScoresTable#abs", "[ScoresTable]", double) {
  std::size_t const vectorSize = 8;
  Dimensions const dims(2, 3);
  std::vector<TestType> scores(2 * vectorSize);

  // Generate random data
  std::mt19937 generator(5);
  std::uniform_real_distribution<TestType> dist(-5.0, 5.0);
  std::generate(std::begin(scores), std::end(scores),
                [&generator, &dist] { return dist(generator); });

  ScoresTable<TestType> table(dims, scores);
  table.abs();

  auto const &allScores = table.allScores();
  auto const minElement =
      *std::min_element(std::cbegin(allScores), std::cend(allScores));
  auto const maxElement =
      *std::max_element(std::cbegin(allScores), std::cend(allScores));

  CHECK(minElement >= 0.0);
  CHECK(maxElement <= 5.0);
}

TEMPLATE_TEST_CASE("ScoresTable#translateVectorsToPositive", "[ScoresTable]",
                   double) {
  std::size_t const vectorSize = 8;
  Dimensions const dims(2, 3);
  std::vector<TestType> scores(2 * vectorSize);

  // Generate random data
  std::mt19937 generator(5);
  std::uniform_real_distribution<TestType> dist(-5.0, 5.0);
  std::generate(std::begin(scores), std::end(scores),
                [&generator, &dist] { return dist(generator); });

  ScoresTable<TestType> table(dims, scores);
  table.translateVectorsToPositive();

  auto const &allScores = table.allScores();
  auto const minElement =
      *std::min_element(std::cbegin(allScores), std::cend(allScores));
  auto const maxElement =
      *std::max_element(std::cbegin(allScores), std::cend(allScores));

  CHECK(Approx(ScoresTable<TestType>::epsilon) == minElement);
  CHECK(maxElement > 9.0);
}

TEMPLATE_TEST_CASE("ScoresTable#translateVectorsToPositive (already positive)",
                   "[ScoresTable]", double) {
  std::size_t const vectorSize = 8;
  Dimensions const dims(2, 3);
  std::vector<TestType> scores(2 * vectorSize);

  // Generate random data
  std::mt19937 generator(5);
  std::uniform_real_distribution<TestType> dist(1.0, 5.0);
  std::generate(std::begin(scores), std::end(scores),
                [&generator, &dist] { return dist(generator); });

  ScoresTable<TestType> table(dims, scores);
  table.translateVectorsToPositive();

  auto const &allScores = table.allScores();
  auto const minElement =
      *std::min_element(std::cbegin(allScores), std::cend(allScores));
  auto const maxElement =
      *std::max_element(std::cbegin(allScores), std::cend(allScores));

  CHECK(minElement >= 1.0);
  CHECK(maxElement <= 5.0);
}

TEMPLATE_TEST_CASE("ScoresTable#log", "[ScoresTable]", double) {
  std::size_t const vectorSize = 8;
  Dimensions const dims(2, 3);
  std::vector<TestType> scores(2 * vectorSize);

  // Generate random data
  std::mt19937 generator(5);
  std::uniform_real_distribution<TestType> dist(1.0, 5.0);
  std::generate(std::begin(scores), std::end(scores),
                [&generator, &dist] { return dist(generator); });

  std::vector<TestType> expected(scores);
  for (std::size_t index = 0; index < 2 * vectorSize; index++) {
    expected[index] = std::log(scores[index]) / std::log(2.0);
  }

  ScoresTable<TestType> table(dims, scores);
  table.log2();

  CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                   std::cbegin(table.allScores()),
                   [](auto x, auto y) -> bool { return x == Approx(y); }));
}

TEMPLATE_TEST_CASE("ScoresTable#mergeVectors", "[ScoresTable]", double) {
  Dimensions const dims(2, 2);
  ScoresTable<TestType> table(dims, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
  auto const merged = table.mergeVectors();

  std::vector<TestType> const expected = {5,  6,  7,  8,  10, 12, 14, 16,
                                          15, 18, 21, 24, 20, 24, 28, 32};
  CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                   std::cbegin(merged.allScores()),
                   [](auto x, auto y) -> bool { return x == Approx(y); }));
}

TEMPLATE_TEST_CASE("ScoresTable#mergeVectors 4 vectors", "[ScoresTable]",
                   double) {
  Dimensions const dims(4, 2);
  ScoresTable<TestType> table(dims, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0,
                                     1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});
  auto const merged = table.mergeVectors();

  std::vector<TestType> const expected = {
      5, 6, 7, 8, 10, 12, 14, 16, 15, 18, 21, 24, 20, 24, 28, 32,
      5, 6, 7, 8, 10, 12, 14, 16, 15, 18, 21, 24, 20, 24, 28, 32};
  CHECK(std::equal(std::cbegin(expected), std::cend(expected),
                   std::cbegin(merged.allScores()),
                   [](auto x, auto y) -> bool { return x == Approx(y); }));
}

TEST_CASE("ScoresTable#mergeVectors invalid", "[ScoresTable]") {
  {
    // odd number of vectors
    Dimensions const dims(3, 1);
    ScoresTable<double> table(dims, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    CHECK_THROWS_AS(table.mergeVectors(), std::invalid_argument);
  }
  {
    // uneven sizes
    std::array<std::uint32_t, 2> const vectorWidthsBits = {2, 3};
    Dimensions const dims(vectorWidthsBits);
    ScoresTable<double> table(
        dims, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0});
    CHECK_THROWS_AS(table.mergeVectors(), std::invalid_argument);
  }
}

TEMPLATE_TEST_CASE("ScoresTable#addScores", "[ScoresTable]", double) {
  std::vector<TestType> const scores = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8};

  Dimensions const dims(2, 2);
  ScoresTable<TestType> table(dims, scores);
  table.addScores(BitSpan(0, 2), std::cbegin(scores), std::cbegin(scores) + 4);
  table.addScores(BitSpan(2, 2), std::cbegin(scores) + 4, std::cend(scores));

  CHECK(std::equal(std::cbegin(scores), std::cend(scores),
                   std::cbegin(table.allScores()),
                   [](auto x, auto y) -> bool { return x == Approx(y); }));
}

TEST_CASE("ScoresTable#addScores invalid", "[ScoresTable]") {
  std::vector<double> const scores = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8};

  Dimensions const dims(2, 2);
  ScoresTable<double> table(dims, scores);
  // no such distinguishing vector
  CHECK_THROWS_AS(table.addScores(BitSpan(0, 3), std::cbegin(scores),
                                  std::cbegin(scores) + 8),
                  std::invalid_argument);
  CHECK_THROWS_AS(table.addScores(BitSpan(1, 2), std::cbegin(scores),
                                  std::cbegin(scores) + 4),
                  std::invalid_argument);
  // too many or few scores, correct BitSpan
  CHECK_THROWS_AS(table.addScores(BitSpan(0, 2), std::cbegin(scores),
                                  std::cbegin(scores) + 3),
                  std::length_error);
  CHECK_THROWS_AS(table.addScores(BitSpan(0, 2), std::cbegin(scores),
                                  std::cbegin(scores) + 5),
                  std::length_error);
}

} /* namespace rankcpp */
