#pragma once

// In-memory little-endian serialization helpers shared by the buffer-based
// algorithm implementations.

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "compresskit/constants.hpp"

namespace compresskit {

// Appends a 32-bit unsigned integer in little-endian to `out`.
inline void write_u32_le(std::vector<uint8_t>& out, uint32_t value) {
    for (std::size_t i = 0; i < U32_SIZE; ++i) {
        out.push_back(static_cast<uint8_t>((value >> (i * BITS_PER_BYTE)) & 0xFFu));
    }
}

// Appends a 4-byte magic prefix.
inline void write_magic(std::vector<uint8_t>& out, const char* magic) {
    for (std::size_t i = 0; i < MAGIC_SIZE; ++i) {
        out.push_back(static_cast<uint8_t>(magic[i]));
    }
}

// Writes a static-model file header: magic + frequency table
// (count: u32 LE, then count x u32 LE entries).
inline void write_frequency_header(std::vector<uint8_t>& out, const char* magic,
                                   const std::vector<uint32_t>& freq) {
    write_magic(out, magic);
    write_u32_le(out, static_cast<uint32_t>(freq.size()));
    for (uint32_t v : freq) {
        write_u32_le(out, v);
    }
}

// Verifies the leading magic prefix. On success, advances `pos` past it.
// Throws std::runtime_error with `<algo_name>: ...` on mismatch or truncation.
//
// For Huffman/Arithmetic/Range (use_legacy_check = true):
//   - v2 magic: proceed
//   - v1 magic (HFMN/AENC/RCNC): "unsupported legacy format"
//   - other: "bad magic"
//
// For RLE (use_legacy_check = false):
//   - v2 magic (RLE2): proceed
//   - other: "bad magic" (v1 RLE had no magic, nothing to recognise as legacy)
inline void verify_magic(const uint8_t* data, std::size_t size, std::size_t& pos,
                         const char* expected_magic, const char* algo_name,
                         bool use_legacy_check = true) {
    if (size < MAGIC_SIZE) {
        throw std::runtime_error(std::string(algo_name) + ": input too short");
    }
    if (bytes_equal(data, expected_magic)) {
        pos = MAGIC_SIZE;
        return;
    }
    if (use_legacy_check) {
        // Check if it's a recognizable v1 magic (any of the three legacy magics).
        if (bytes_equal(data, HUFFMAN_LEGACY_MAGIC) || bytes_equal(data, ARITHMETIC_LEGACY_MAGIC) ||
            bytes_equal(data, RANGE_LEGACY_MAGIC)) {
            throw std::runtime_error(std::string(algo_name) + ": unsupported legacy format");
        }
    }
    throw std::runtime_error(std::string(algo_name) + ": bad magic");
}

// Reads a frequency table following the magic prefix.
// `pos` starts after the magic; `algo_name` prefixes error messages.
inline std::vector<uint32_t> read_frequency_header(const uint8_t* data, std::size_t size,
                                                   std::size_t& pos, const char* algo_name) {
    if (pos + U32_SIZE > size) {
        throw std::runtime_error(std::string(algo_name) + ": truncated frequency count");
    }
    uint32_t count = 0;
    for (std::size_t i = 0; i < U32_SIZE; ++i) {
        count |= static_cast<uint32_t>(data[pos + i]) << (i * BITS_PER_BYTE);
    }
    pos += U32_SIZE;
    if (count != SYMBOL_LIMIT) {
        throw std::runtime_error(std::string(algo_name) + ": bad frequency count");
    }
    std::vector<uint32_t> freq(count, 0);
    for (uint32_t i = 0; i < count; ++i) {
        if (pos + U32_SIZE > size) {
            throw std::runtime_error(std::string(algo_name) + ": truncated frequency entry");
        }
        uint32_t v = 0;
        for (std::size_t b = 0; b < U32_SIZE; ++b) {
            v |= static_cast<uint32_t>(data[pos + b]) << (b * BITS_PER_BYTE);
        }
        freq[i] = v;
        pos += U32_SIZE;
    }
    return freq;
}

// Convenience: verify magic and read the trailing frequency table in one call.
// `pos` starts at 0 and is advanced past magic + frequency table on success.
inline std::vector<uint32_t> read_magic_and_frequency_header(const uint8_t* data, std::size_t size,
                                                             std::size_t& pos,
                                                             const char* expected_magic,
                                                             const char* algo_name,
                                                             bool use_legacy_check = true) {
    verify_magic(data, size, pos, expected_magic, algo_name, use_legacy_check);
    return read_frequency_header(data, size, pos, algo_name);
}

// Pre-checks the leading magic before CRC verification, so that legacy
// formats are classified correctly instead of being reported as checksum
// errors.  Throws "unsupported legacy format" for recognizable v1 magic,
// "bad magic" for unknown magic, or returns silently for v2 magic.
inline void precheck_magic(const std::vector<uint8_t>& input, const char* expected_v2_magic,
                           const char* algo_name, bool use_legacy_check = true) {
    if (input.size() < MAGIC_SIZE) {
        return;  // Let verify_crc32/verify_magic handle short input
    }
    if (bytes_equal(input.data(), expected_v2_magic)) {
        return;  // v2 magic — proceed to CRC and body
    }
    if (use_legacy_check && (bytes_equal(input.data(), HUFFMAN_LEGACY_MAGIC) ||
                             bytes_equal(input.data(), ARITHMETIC_LEGACY_MAGIC) ||
                             bytes_equal(input.data(), RANGE_LEGACY_MAGIC))) {
        throw std::runtime_error(std::string(algo_name) + ": unsupported legacy format");
    }
    throw std::runtime_error(std::string(algo_name) + ": bad magic");
}

}  // namespace compresskit
