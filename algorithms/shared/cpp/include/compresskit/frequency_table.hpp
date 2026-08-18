#pragma once

#include <cstdint>
#include <vector>

namespace compresskit {

// In-memory frequency helpers (used by algorithms that operate on byte buffers).

// Counts byte frequencies of `data` into a SYMBOL_LIMIT-sized vector (EOF left at 0).
std::vector<uint32_t> count_frequencies(const std::vector<uint8_t>& data);

// Scales frequencies so the total does not exceed max_total.
// O(N) single pass: proportional reduction followed by at most one correction sweep.
void scale_frequencies(std::vector<uint32_t>& freq, uint32_t max_total);

// Builds cumulative frequency table of size freq.size()+1. Callers must
// ensure the table is non-zero (entropy entry points validate EOF presence).
std::vector<uint32_t> build_cumulative(const std::vector<uint32_t>& freq);

// 64-bit sum of a serialized frequency table. Entropy decoders require
// 1 <= total <= MAX_FREQ_TOTAL; tables outside that range cannot come from a
// valid encoder and would break the coders' division invariants.
uint64_t frequency_total(const std::vector<uint32_t>& freq);

// Binary search: first symbol with cumulative[symbol + 1] > value.
// Never selects a zero-width interval on a corrupt table.
uint32_t find_symbol(const std::vector<uint32_t>& cumulative, uint64_t value);

// Builds the entropy-coder frequency table: byte counts + EOF marker, scaled
// to fit `max_total`. Shared by arithmetic and range coders.
std::vector<uint32_t> build_entropy_frequencies(const std::vector<uint8_t>& data,
                                                uint32_t max_total);

}  // namespace compresskit
