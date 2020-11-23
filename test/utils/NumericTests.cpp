#include <rankcpp/utils/Numeric.hpp>

#include <catch2/catch.hpp>

#include <array>

namespace rankcpp {

TEST_CASE("Numeric #kahanSum", "[Numeric]") {
  SECTION("double") {
    std::array<double, 6> const data = {5.5, 4.5, 3.5, 2.4, 5.3, 3.5};
    double const expected{24.7};
    CHECK(expected == Approx(kahanSum(std::cbegin(data), std::cend(data))));
  }
  SECTION("float") {
    std::array<float, 6> const data = {5.5f, 4.5f, 3.5f, 2.4f, 5.3f, 3.5f};
    float const expected{24.7f};
    CHECK(expected == Approx(kahanSum(std::cbegin(data), std::cend(data))));
  }
}

} /* namespace rankcpp */
