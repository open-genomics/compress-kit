#include <cassert>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <vector>

#include "compresskit/algorithms.hpp"
#include "compresskit/buffer_api.hpp"
#include "compresskit/frequency_table.hpp"

namespace {

struct AlgorithmCase {
    const char* name;
    compresskit::BufferTransform encode;
    compresskit::BufferTransform decode;
};

void test_roundtrip(const AlgorithmCase& algo, const std::vector<uint8_t>& input) {
    auto encoded = compresskit::encode_buffer(algo.encode, input);
    assert(encoded.status == compresskit::StatusCode::OK);

    auto decoded = compresskit::decode_buffer(algo.decode, encoded.value);
    assert(decoded.status == compresskit::StatusCode::OK);
    assert(decoded.value == input);
}

std::vector<uint8_t> make_sequential(std::size_t n) {
    std::vector<uint8_t> v(n);
    std::iota(v.begin(), v.end(), uint8_t{0});
    return v;
}

std::vector<uint8_t> make_lcg(std::size_t n) {
    std::vector<uint8_t> v(n);
    uint32_t state = 0xDEADBEEF;
    for (std::size_t i = 0; i < n; ++i) {
        state = state * 1664525u + 1013904223u;
        v[i] = static_cast<uint8_t>(state >> 24);
    }
    return v;
}

}  // namespace

int main() {
    const AlgorithmCase algorithms[] = {
        {"Huffman", compresskit::huffman_encode_buffer, compresskit::huffman_decode_buffer},
        {"Arithmetic", compresskit::arithmetic_encode_buffer,
         compresskit::arithmetic_decode_buffer},
        {"Range", compresskit::rangecoder_encode_buffer, compresskit::rangecoder_decode_buffer},
        {"RLE", compresskit::rle_encode_buffer, compresskit::rle_decode_buffer},
    };

    struct Corpus {
        const char* label;
        std::vector<uint8_t> data;
    };

    std::vector<uint8_t> all_256(256);
    std::iota(all_256.begin(), all_256.end(), 0);

    const Corpus corpus[] = {
        {"empty", {}},
        {"single_byte", {0x42}},
        {"ascii", {'s', 't', 'r', 'e', 'a', 'm', '-', 'a', 'p', 'i'}},
        {"all_same", std::vector<uint8_t>(4096, 0xAB)},
        {"all_256", all_256},
        {"sequential_1k", make_sequential(1024)},
        {"lcg_64k", make_lcg(65536)},
    };

    int passed = 0;
    for (const auto& algo : algorithms) {
        for (const auto& c : corpus) {
            test_roundtrip(algo, c.data);
            ++passed;
            std::printf("PASS %-12s %s\n", algo.name, c.label);
        }
    }

    std::printf("test_lifecycle: %d round-trip(s) passed\n", passed);
    return 0;
}
