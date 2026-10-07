#ifndef RIFF_PARSER_HPP
#define RIFF_PARSER_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>

namespace tinymedia {

constexpr uint32_t FOURCC(char a, char b, char c, char d) {
    return (static_cast<uint32_t>(a)) |
           (static_cast<uint32_t>(b) << 8) |
           (static_cast<uint32_t>(c) << 16) |
           (static_cast<uint32_t>(d) << 24);
}

constexpr uint32_t CHUNK_RIFF = FOURCC('R', 'I', 'F', 'F');
constexpr uint32_t CHUNK_WAVE = FOURCC('W', 'A', 'V', 'E');
constexpr uint32_t CHUNK_FMT  = FOURCC('f', 'm', 't', ' ');
constexpr uint32_t CHUNK_DATA = FOURCC('d', 'a', 't', 'a');
constexpr uint32_t CHUNK_INFO = FOURCC('I', 'N', 'F', 'O');
constexpr uint32_t CHUNK_LIST = FOURCC('L', 'I', 'S', 'T');

struct AudioFormat {
    uint16_t format_tag{0};
    uint16_t channels{0};
    uint32_t sample_rate{0};
    uint32_t bytes_per_sec{0};
    uint16_t block_align{0};
    uint16_t bits_per_sample{0};
};

class RiffParser {
public:
    RiffParser();
    ~RiffParser();

    bool parse(const uint8_t* data, size_t size);
    bool hasAudioFormat() const { return m_has_format; }
    const AudioFormat& getFormat() const { return m_format; }
    std::string getMetadata(const std::string& key) const;
    const std::vector<uint8_t>& getAudioData() const { return m_audio_payload; }

    /* Calculation of total audio stream allocation (Task 3: UBSan target) */
    int32_t calculateSampleBufferSize(int32_t num_frames, int32_t samples_per_frame, int32_t channels) const;

private:
    bool parseChunk(const uint8_t* data, size_t size, size_t& offset);
    bool parseFmtChunk(const uint8_t* chunk_data, size_t chunk_len);
    bool parseListChunk(const uint8_t* chunk_data, size_t chunk_len);
    bool parseMetadataTag(const uint8_t* tag_data, size_t tag_len, std::string& out_val);

    bool m_has_format{false};
    AudioFormat m_format{};
    std::vector<uint8_t> m_audio_payload;
    std::unordered_map<std::string, std::string> m_metadata;
};

} // namespace tinymedia

#endif // RIFF_PARSER_HPP
