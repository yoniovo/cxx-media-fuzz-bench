#include <cstdint>
#include <cstddef>
#include "media_stream.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < 2) {
        return 0;
    }

    tinymedia::MediaStream stream;

    /* Ingest sequence of frames */
    size_t cursor = 0;
    while (cursor + 2 <= size) {
        uint8_t payload_len = data[cursor + 1];
        size_t frame_len = 2 + payload_len;
        if (cursor + frame_len > size) {
            break;
        }

        stream.ingestFrame(data + cursor, frame_len);
        cursor += frame_len;
    }

    return 0;
}
