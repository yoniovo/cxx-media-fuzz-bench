#include <cstdint>
#include <cstddef>
#include <cstring>
#include "riff_parser.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 4) {
        return 0;
    }

    tinymedia::RiffParser parser;
    parser.parse(data, size);

    /* Also exercise calculation method if input provides parameters */
    if (size >= 12) {
        int32_t f = static_cast<int32_t>(data[0]) | (static_cast<int32_t>(data[1]) << 8) | 
                   (static_cast<int32_t>(data[2]) << 16) | (static_cast<int32_t>(data[3]) << 24);
        int32_t s = static_cast<int32_t>(data[4]) | (static_cast<int32_t>(data[5]) << 8) | 
                   (static_cast<int32_t>(data[6]) << 16) | (static_cast<int32_t>(data[7]) << 24);
        int32_t c = data[8] % 4;
        parser.calculateSampleBufferSize(f, s, c);
    }

    return 0;
}
