#pragma once

#include <rankcpp/BitSpan.hpp>
#include <rankcpp/utils/Encoding.hpp>

#include <gsl/span>

#include <range/v3/all.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>

namespace rankcpp {

template <std::uint32_t BitLen> class Key {
public:
  using ByteType = std::uint8_t;

  static_assert(BitLen > 0, "key length must be > 0 bits");

  static constexpr std::uint32_t const ByteCount =
      (BitLen % 8 != 0) ? (BitLen / 8) + 1 : BitLen / 8;

  constexpr Key() noexcept = default;

  constexpr explicit Key(
      std::array<ByteType, ByteCount> const &byteArray) noexcept
      : bytes(byteArray) {}

  explicit Key(std::string const &hex) {
    // check for the correct number of characters in the hex string
    if (hex.length() != ByteCount * 2) {
      throw std::length_error("hex string needs to be of length " +
                              std::to_string(ByteCount * 2) + " chars for a " +
                              std::to_string(BitLen) + "-bit key");
    }
    // convert the hex to the byte array
    hexToBytes(hex, gsl::span<ByteType>{bytes});
  }

  constexpr Key(std::initializer_list<ByteType> const &list) {
    if (list.size() != ByteCount) {
      throw std::length_error("initializer_list needs to be of length " +
                              std::to_string(ByteCount) + " bytes for a " +
                              std::to_string(BitLen) + "-bit key");
    }
    std::copy(std::cbegin(list), std::cend(list), std::begin(bytes));
  }

  constexpr auto asBytes() noexcept -> std::array<ByteType, ByteCount> & {
    return bytes;
  }

  constexpr auto asBytes() const noexcept
      -> std::array<ByteType, ByteCount> const & {
    return bytes;
  }

  template <typename IntType>
  auto subkeyValue(BitSpan subkey) const -> IntType {
    if (subkey.count() > std::numeric_limits<IntType>::digits) {
      throw std::out_of_range(
          "insufficient space in IntType to store subkey value");
    }

    IntType value{0};
    for (auto bit : ranges::views::iota(subkey.start(), subkey.end() + 1)) {
      std::uint32_t const byteIndex = bit / 8;
      std::uint32_t const bitOffset = bit % 8;
      std::uint32_t const bitValue =
          (bytes[byteIndex] & (1U << bitOffset)) >> bitOffset;
      auto const stateBitIndex = bit - subkey.start();
      value |= (static_cast<IntType>(bitValue) << stateBitIndex);
    }

    return value;
  }

  template <typename IntType> auto asLeIntegerValue() const -> IntType {
    if (BitLen > std::numeric_limits<IntType>::digits) {
      throw std::out_of_range("insufficient space in IntType to store key");
    }

    IntType value{0};
    for (auto b : ranges::views::iota(std::size_t{0}, bytes.size())) {
      IntType const byteValue{bytes[b]};
      value += byteValue << (b * 8);
    }
    return value;
  }

private:
  std::array<ByteType, ByteCount> bytes;
};

template <std::uint32_t BitLen, typename RngType>
auto randomKey(RngType &rng) -> Key<BitLen> {
  using ByteType = typename Key<BitLen>::ByteType;
  std::array<ByteType, Key<BitLen>::ByteCount> bytes{};
  auto const max = std::numeric_limits<ByteType>::max();
  std::uniform_int_distribution<ByteType> dist(0, max);
  std::generate(std::begin(bytes), std::end(bytes),
                [&dist, &rng]() { return dist(rng); });

  return Key<BitLen>(bytes);
}

} /* namespace rankcpp */
