#pragma once

#include "compresskit/buffer_api.hpp"

namespace compresskit {
namespace cli {

// An algorithm exposes its in-memory encode/decode transforms; the launcher
// handles file I/O, size limits and error reporting via the buffer layer.
struct Algorithm {
    BufferTransform encode;
    BufferTransform decode;
};

int run(const Algorithm& algo, int argc, char** argv);

}  // namespace cli
}  // namespace compresskit
