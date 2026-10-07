#include "riff_parser.hpp"
#include <cstring>
#include <iostream>

namespace tinymedia {

RiffParser::RiffParser() = default;
RiffParser::~RiffParser() = default;

static uint32_t readUint32LE(const uint8_t* ptr) {
    return static_cast<uint32_t>(ptr[0]) |
          (static_cast<uint32_t>(ptr[1]) << 8) |
          (static_cast<uint32_t>(ptr[2]) << 16) |
          (static_cast<uint32_t>(ptr[3]) << 24);
}

static uint16_t readUint16LE(const uint8_t* ptr) {
    return static_cast<uint16_t>(ptr[0]) |
          (static_cast<uint16_t>(ptr[1]) << 8);
}

bool RiffParser::parse(const uint8_t* data, size_t size) {
    if (!data || size < 12) {
        return false;
    }

    uint32_t riff_tag = readUint32LE(data);
    if (riff_tag != CHUNK_RIFF) {
        return false;
    }

    uint32_t file_len = readUint32LE(data + 4);
    (void)file_len;

    uint32_t wave_tag = readUint32LE(data + 8);
    if (wave_tag != CHUNK_WAVE) {
        return false;
    }

    size_t offset = 12;
    while (offset + 8 <= size) {
        if (!parseChunk(data, size, offset)) {
            break;
        }
    }

    return true;
}

bool RiffParser::parseChunk(const uint8_t* data, size_t size, size_t& offset) {
    if (offset + 8 > size) {
        return false;
    }

    uint32_t chunk_id = readUint32LE(data + offset);
    uint32_t chunk_len = readUint32LE(data + offset + 4);
    offset += 8;

    if (offset + chunk_len > size) {
        return false;
    }

    const uint8_t* chunk_data = data + offset;

    if (chunk_id == CHUNK_FMT) {
        parseFmtChunk(chunk_data, chunk_len);
    } else if (chunk_id == CHUNK_DATA) {
        m_audio_payload.assign(chunk_data, chunk_data + chunk_len);
    } else if (chunk_id == CHUNK_LIST) {
        parseListChunk(chunk_data, chunk_len);
    }

    offset += chunk_len;
    /* RIFF chunks are padded to 2-byte boundary */
    if (chunk_len % 2 != 0 && offset < size) {
        offset++;
    }

    return true;
}

bool RiffParser::parseFmtChunk(const uint8_t* chunk_data, size_t chunk_len) {
    if (chunk_len < 16) {
        return false;
    }

    m_format.format_tag      = readUint16LE(chunk_data);
    m_format.channels        = readUint16LE(chunk_data + 2);
    m_format.sample_rate     = readUint32LE(chunk_data + 4);
    m_format.bytes_per_sec   = readUint32LE(chunk_data + 8);
    m_format.block_align     = readUint16LE(chunk_data + 12);
    m_format.bits_per_sample = readUint16LE(chunk_data + 14);
    m_has_format = true;
    return true;
}

bool RiffParser::parseListChunk(const uint8_t* chunk_data, size_t chunk_len) {
    if (chunk_len < 4) {
        return false;
    }

    uint32_t list_type = readUint32LE(chunk_data);
    if (list_type != CHUNK_INFO) {
        return false;
    }

    size_t sub_offset = 4;
    while (sub_offset + 8 <= chunk_len) {
        char tag_chars[5] = {
            static_cast<char>(chunk_data[sub_offset]),
            static_cast<char>(chunk_data[sub_offset + 1]),
            static_cast<char>(chunk_data[sub_offset + 2]),
            static_cast<char>(chunk_data[sub_offset + 3]),
            '\0'
        };
        uint32_t tag_len = readUint32LE(chunk_data + sub_offset + 4);
        sub_offset += 8;

        if (sub_offset + tag_len > chunk_len) {
            break;
        }

        std::string tag_val;
        if (parseMetadataTag(chunk_data + sub_offset, tag_len, tag_val)) {
            m_metadata[tag_chars] = tag_val;
        }

        sub_offset += tag_len;
        if (tag_len % 2 != 0 && sub_offset < chunk_len) {
            sub_offset++;
        }
    }

    return true;
}

/* VULNERABILITY 1: Off-by-one heap buffer overwrite */
bool RiffParser::parseMetadataTag(const uint8_t* tag_data, size_t tag_len, std::string& out_val) {
    if (tag_len == 0 || tag_len > 1024) {
        return false;
    }

    /* VULNERABLE LOGIC:
     * Allocates 'tag_len' bytes, but writes null terminator at buf[tag_len].
     * Index tag_len is out of bounds by 1 byte!
     */
    char* buf = new char[tag_len];
    std::memcpy(buf, tag_data, tag_len);
    buf[tag_len] = '\0'; /* Off-by-one write! */

    out_val = std::string(buf);
    delete[] buf;
    return true;
}

std::string RiffParser::getMetadata(const std::string& key) const {
    auto it = m_metadata.find(key);
    if (it != m_metadata.end()) {
        return it->second;
    }
    return "";
}

/* VULNERABILITY 3: Signed Integer Overflow (UBSan) */
int32_t RiffParser::calculateSampleBufferSize(int32_t num_frames, int32_t samples_per_frame, int32_t channels) const {
    /* VULNERABLE LOGIC:
     * Signed integer arithmetic overflow if num_frames * samples_per_frame exceeds INT32_MAX
     * or shift out of bounds.
     */
    int32_t total_samples = num_frames * samples_per_frame;
    int32_t total_bytes = total_samples << channels;
    return total_bytes;
}

} // namespace tinymedia
