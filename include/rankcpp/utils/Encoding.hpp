#pragma once

#include <gsl/span>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace rankcpp {

inline void hexToBytes(const std::string &s, gsl::span<std::uint8_t> bytes) {
  if (((s.size() + 1) / 2) > bytes.size()) {
    throw std::length_error("output span is too small");
  }

  union {
    std::uint64_t binary;
    std::array<char, 8> byte;
  } value{};

  std::vector<std::uint8_t> out{};
  out.reserve((s.size() + 1) / 2);

  auto offset = s.size() % 16;
  if (offset != 0u) {
    value.binary = std::stoull(s.substr(0, offset), nullptr, 16);

    for (auto index = (offset + 1) / 2; (index--) != 0u;) {
      out.emplace_back(value.byte[index]);
    }
  }

  for (; offset < s.size(); offset += 16) {
    value.binary = std::stoull(s.substr(offset, 16), nullptr, 16);
    for (std::size_t index = 8; (index--) != 0u;) {
      out.emplace_back(value.byte[index]);
    }
  }
  std::copy(std::cbegin(out), std::cend(out), std::begin(bytes));
}

} /* namespace rankcpp */
