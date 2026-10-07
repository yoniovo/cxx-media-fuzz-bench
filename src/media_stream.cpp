#include "media_stream.hpp"
#include <cstring>
#include <iostream>

namespace tinymedia {

StreamFrameBuffer::StreamFrameBuffer() = default;

StreamFrameBuffer::~StreamFrameBuffer() {
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
}

bool StreamFrameBuffer::allocate(size_t size) {
    release();
    m_data = new (std::nothrow) uint8_t[size];
    if (!m_data) {
        m_size = 0;
        return false;
    }
    m_size = size;
    return true;
}

/* VULNERABILITY 2: Double Free on error unwinding */
void StreamFrameBuffer::release() {
    if (m_data) {
        delete[] m_data;
        /* VULNERABLE LOGIC:
         * Fails to reset m_data to nullptr!
         * Leaves dangling pointer in m_data.
         * When ~StreamFrameBuffer() runs on destruction, delete[] is called again!
         */
        /* Missing: m_data = nullptr; */
    }
    m_size = 0;
}

MediaStream::MediaStream() = default;
MediaStream::~MediaStream() = default;

bool MediaStream::ingestFrame(const uint8_t* data, size_t size) {
    if (!data || size < 2) {
        return false;
    }

    uint8_t frame_type = data[0];
    uint8_t payload_len = data[1];

    if (size < static_cast<size_t>(2 + payload_len)) {
        return false;
    }

    if (frame_type == 0x01) {
        /* Standard frame ingestion */
        if (!m_buffer.allocate(payload_len)) {
            return false;
        }
        std::memcpy(m_buffer.data(), data + 2, payload_len);
        m_frame_count++;
        return true;
    }

    if (frame_type == 0xEE) {
        /* Malformed / abort frame: triggers cleanup */
        m_buffer.release();
        /* Returns false: object teardown will follow */
        return false;
    }

    return true;
}

} // namespace tinymedia
