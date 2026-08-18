#include "compresskit/buffer_api.hpp"

#include <cstdio>
#include <fstream>
#include <stdexcept>

#include "compresskit/constants.hpp"

namespace compresskit {
namespace {

bool write_file(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    if (!data.empty()) {
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }
    return static_cast<bool>(out);
}

std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) {
        throw std::runtime_error("cannot open input file");
    }
    std::streampos size = in.tellg();
    if (size < 0) {
        throw std::runtime_error("cannot determine input file size");
    }
    std::vector<uint8_t> data(static_cast<std::size_t>(size));
    in.seekg(0, std::ios::beg);
    if (!data.empty()) {
        in.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!in) {
            throw std::runtime_error("failed to read input file");
        }
    }
    return data;
}

template <typename Layer>
bool apply_file(BufferTransform transform, const std::string& input_path,
                const std::string& output_path, Layer layer) {
    try {
        std::vector<uint8_t> input = read_file(input_path);
        Result<std::vector<uint8_t>> result = layer(transform, input);
        if (!result.ok()) {
            return false;
        }
        return write_file(output_path, result.value);
    } catch (const std::exception&) {
        return false;
    }
}

}  // namespace

Result<std::vector<uint8_t>> encode_buffer(BufferTransform transform,
                                           const std::vector<uint8_t>& input) {
    if (input.size() >= MAX_RAW_SIZE) {
        return {StatusCode::ERR_SIZE_LIMIT, {}};
    }
    try {
        std::vector<uint8_t> out = transform(input);
        return {StatusCode::OK, std::move(out)};
    } catch (const std::exception& e) {
        std::fprintf(stderr, "encode failed: %s\n", e.what());
        return {StatusCode::ERR_CORRUPT, {}};
    }
}

Result<std::vector<uint8_t>> decode_buffer(BufferTransform transform,
                                           const std::vector<uint8_t>& input) {
    if (input.size() >= MAX_COMPRESSED_SIZE) {
        return {StatusCode::ERR_SIZE_LIMIT, {}};
    }
    try {
        std::vector<uint8_t> out = transform(input);
        if (out.size() > MAX_RAW_SIZE) {
            return {StatusCode::ERR_SIZE_LIMIT, {}};
        }
        return {StatusCode::OK, std::move(out)};
    } catch (const std::exception& e) {
        std::fprintf(stderr, "decode failed: %s\n", e.what());
        return {StatusCode::ERR_CORRUPT, {}};
    }
}

bool encode_file_via_buffer(BufferTransform transform, const std::string& input_path,
                            const std::string& output_path) {
    return apply_file(transform, input_path, output_path, encode_buffer);
}

bool decode_file_via_buffer(BufferTransform transform, const std::string& input_path,
                            const std::string& output_path) {
    return apply_file(transform, input_path, output_path, decode_buffer);
}

}  // namespace compresskit
