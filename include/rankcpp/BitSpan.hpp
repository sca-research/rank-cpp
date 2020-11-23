#pragma once

#include <range/v3/all.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>

/** \file
 * \brief Used to define portions of a key when viewed as a bitstring
 *
 */

namespace rankcpp {

class BitSpan {
public:
  constexpr BitSpan(std::uint32_t startIndex, std::uint32_t bitCount)
      : startIndex_(startIndex), bitCount_(bitCount) {
    if (bitCount_ == 0) {
      throw std::invalid_argument("BitSpan cannot have a bit count of zero");
    }
    if (startIndex_ + bitCount_ < startIndex_) {
      throw std::overflow_error("definition overflows uint32_t bounds");
    }
  }

  constexpr BitSpan() noexcept {}

  constexpr auto start() const noexcept -> std::uint32_t { return startIndex_; }

  constexpr auto count() const noexcept -> std::uint32_t { return bitCount_; }

  constexpr auto end() const noexcept -> std::uint32_t {
    return startIndex_ + bitCount_ - 1;
  }

  template <typename IntType = std::size_t>
  constexpr auto valueCount() const -> IntType {
    if (bitCount_ > std::numeric_limits<IntType>::digits - 1) {
      throw std::overflow_error("value count too large for an IntType");
    }
    return IntType{1} << bitCount_;
  }

  template <typename SubkeyType = std::size_t>
  constexpr auto subkeyRange() const {
    if (bitCount_ > std::numeric_limits<SubkeyType>::digits - 1) {
      throw std::overflow_error("value count too large for a SubkeyType");
    }
    return ranges::views::iota(SubkeyType{0}, valueCount<SubkeyType>());
  }

  constexpr auto encapsulates(BitSpan const &other) const noexcept -> bool {
    return startIndex_ <= other.start() && end() >= other.end();
  }

  constexpr friend auto operator==(BitSpan lhs, BitSpan rhs) noexcept -> bool {
    return lhs.startIndex_ == rhs.startIndex_ && lhs.bitCount_ == rhs.bitCount_;
  }

  constexpr friend auto operator!=(BitSpan lhs, BitSpan rhs) noexcept -> bool {
    return !(lhs == rhs);
  }

private:
  std::uint32_t startIndex_{0};
  std::uint32_t bitCount_{0};
};

} /* namespace rankcpp */
