#pragma once

#include <rankcpp/BitSpan.hpp>

#include <range/v3/all.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <numeric>
#include <vector>

namespace rankcpp {

// TODO constexpr std::vector in C++20 should allow this to be written as a
// fully constexpr class
class Dimensions {
public:
  template <std::size_t VectorCount>
  explicit Dimensions(
      std::array<std::uint32_t, VectorCount> const &vectorWidthsBits) noexcept {
    initFromIter(std::cbegin(vectorWidthsBits), std::cend(vectorWidthsBits));
  }

  Dimensions(std::initializer_list<std::uint32_t> const &vectorWidthsBits) {
    initFromIter(std::cbegin(vectorWidthsBits), std::cend(vectorWidthsBits));
  }

  Dimensions(std::size_t vectorCount, std::size_t vectorWidthsBits) noexcept {
    for (auto vectorIndex : ranges::views::iota(std::size_t{0}, vectorCount)) {
      auto const offset = vectorIndex * vectorWidthsBits;
      spans.emplace_back(offset, vectorWidthsBits);
    }
  }

  auto vectorCount() const noexcept -> std::size_t { return spans.size(); }

  auto vectorRange() const noexcept {
    return ranges::views::iota(std::size_t{0}, spans.size());
  }

  auto vectorWidthBits(std::size_t index) const noexcept -> std::uint32_t {
    return spans[index].count();
  }

  auto keyLengthBits() const noexcept -> std::uint32_t {
    return std::accumulate(std::cbegin(spans), std::cend(spans),
                           std::uint32_t{0},
                           [](std::uint32_t sum, auto const &bitSpan) {
                             return sum + bitSpan.count();
                           });
  }

  auto keyByteCount() const noexcept -> std::size_t {
    auto const keyLenBits = keyLengthBits();
    return (keyLenBits % 8 != 0) ? (keyLenBits / 8) + 1 : keyLenBits / 8;
  }

  auto subkeyCount(std::size_t index) const noexcept -> std::size_t {
    return spans[index].valueCount<std::size_t>();
  }

  auto scoresCount() const noexcept -> std::size_t {
    return std::accumulate(std::cbegin(spans), std::cend(spans), std::size_t{0},
                           [](auto sum, auto const &bitSpan) {
                             return sum +
                                    bitSpan.template valueCount<std::size_t>();
                           });
  }

  auto scoresBeforeCount(std::size_t index) const noexcept -> std::size_t {
    auto end = std::cbegin(spans);
    std::advance(end, index);
    return std::accumulate(std::cbegin(spans), end, std::size_t{0},
                           [](auto sum, auto const &bitSpan) {
                             return sum +
                                    bitSpan.template valueCount<std::size_t>();
                           });
  }

  auto bitOffset(std::size_t index) const noexcept -> std::uint32_t {
    auto end = std::cbegin(spans);
    std::advance(end, index);
    return std::accumulate(
        std::cbegin(spans), end, std::uint32_t{0},
        [](auto sum, auto const &bitSpan) { return sum + bitSpan.count(); });
  }

  auto isEqualWidth() const noexcept -> bool {
    auto curr = std::cbegin(spans) + 1;
    while (curr != std::cend(spans)) {
      if ((*curr).count() != (*(curr - 1)).count()) {
        return false;
      }
      ++curr;
    }
    return true;
  }

  auto asSpans() const noexcept -> std::vector<BitSpan> const & {
    return spans;
  }

private:
  std::vector<BitSpan> spans;

  template <typename InputIt>
  void initFromIter(InputIt first, InputIt last) noexcept {
    std::for_each(first, last, [this](auto const &bitWidth) {
      if (spans.empty()) {
        spans.emplace_back(0, bitWidth);
      } else {
        auto const offset = spans.back().end() + 1;
        spans.emplace_back(offset, bitWidth);
      }
    });
  }
};

template <std::uint32_t VectorCount, std::uint32_t VectorWidthBits>
class FixedDimensions {
public:
  constexpr FixedDimensions() noexcept {
    // TODOcpp20 for a constexpr std::generate
    for (std::uint32_t vi = 0; vi < VectorCount; vi++) {
      auto const offset = vi * VectorWidthBits;
      spans[vi] = BitSpan(offset, VectorWidthBits);
    }
  }

  constexpr auto vectorCount() const noexcept -> std::size_t {
    return VectorCount;
  }

  auto vectorRange() const noexcept {
    return ranges::views::iota(std::size_t{0}, spans.size());
  }

  constexpr auto vectorWidthBits(std::size_t /*unused*/) const noexcept
      -> std::uint32_t {
    return VectorWidthBits;
  }

  constexpr auto keyLengthBits() const noexcept -> std::uint32_t {
    return VectorCount * VectorWidthBits;
  }

  constexpr auto keyByteCount() const noexcept -> std::size_t {
    auto const keyLenBits = keyLengthBits();
    return (keyLenBits % 8 != 0) ? (keyLenBits / 8) + 1 : keyLenBits / 8;
  }

  constexpr auto subkeyCount(std::size_t index) const noexcept -> std::size_t {
    return spans[index].template valueCount<std::size_t>();
  }

  constexpr auto scoresCount() const noexcept -> std::size_t {
    return VectorCount * (std::size_t{1} << VectorWidthBits);
  }

  constexpr auto scoresBeforeCount(std::size_t index) const noexcept
      -> std::size_t {
    return index * (std::size_t{1} << VectorWidthBits);
  }

  constexpr auto bitOffset(std::uint32_t index) const noexcept
      -> std::uint32_t {
    return index * VectorWidthBits;
  }

  constexpr auto isEqualWidth() const noexcept -> bool {
    auto curr = std::cbegin(spans) + 1;
    while (curr != std::cend(spans)) {
      if ((*curr).count() != (*(curr - 1)).count()) {
        return false;
      }
      ++curr;
    }
    return true;
  }

  constexpr auto asSpans() const noexcept
      -> std::array<BitSpan, VectorCount> const & {
    return spans;
  }

private:
  std::array<BitSpan, VectorCount> spans{};
};

} /* namespace rankcpp */
