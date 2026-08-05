#pragma once

// CRC-32 (IEEE 802.3 / zlib-compatible: reflected polynomial 0xEDB88320).
// Every compressed stream ends with a little-endian CRC-32 over all bytes
// preceding the trailer, so any bit corruption is detected at decode time.

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "compresskit/constants.hpp"

namespace compresskit {

namespace detail {

inline constexpr std::array<uint32_t, 256> make_crc_table() {
    std::array<uint32_t, 256> table{};
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (int k = 0; k < BITS_PER_BYTE; ++k) {
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        }
        table[i] = c;
    }
    return table;
}

inline constexpr std::array<uint32_t, 256> CRC_TABLE = make_crc_table();

}  // namespace detail

inline uint32_t crc32(const uint8_t* data, std::size_t size) {
    uint32_t c = 0xFFFFFFFFu;
    for (std::size_t i = 0; i < size; ++i) {
        c = detail::CRC_TABLE[(c ^ data[i]) & 0xFFu] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFFu;
}

// Appends the CRC-32 of everything currently in `out`, little-endian.
inline void append_crc32(std::vector<uint8_t>& out) {
    uint32_t c = crc32(out.data(), out.size());
    for (std::size_t i = 0; i < CRC32_SIZE; ++i) {
        out.push_back(static_cast<uint8_t>((c >> (i * BITS_PER_BYTE)) & 0xFFu));
    }
}

// Verifies the trailing CRC-32 of `input`. Returns the content size
// (input minus trailer) on success; throws `<algo>: ...` otherwise.
inline std::size_t verify_crc32(const std::vector<uint8_t>& input, const char* algo_name) {
    if (input.size() < CRC32_SIZE) {
        throw std::runtime_error(std::string(algo_name) + ": truncated checksum");
    }
    std::size_t content = input.size() - CRC32_SIZE;
    uint32_t stored = 0;
    for (std::size_t i = 0; i < CRC32_SIZE; ++i) {
        stored |= static_cast<uint32_t>(input[content + i]) << (i * BITS_PER_BYTE);
    }
    if (crc32(input.data(), content) != stored) {
        throw std::runtime_error(std::string(algo_name) + ": checksum mismatch");
    }
    return content;
}

}  // namespace compresskit
