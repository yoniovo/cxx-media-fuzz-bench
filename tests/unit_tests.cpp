#include <cassert>
#include <iostream>
#include <vector>
#include <cstring>
#include "riff_parser.hpp"
#include "media_stream.hpp"

using namespace tinymedia;

static std::vector<uint8_t> createValidWav() {
    std::vector<uint8_t> wav;
    // RIFF header
    const char riff[4] = {'R', 'I', 'F', 'F'};
    wav.insert(wav.end(), riff, riff + 4);
    uint32_t file_len = 36;
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&file_len), reinterpret_cast<uint8_t*>(&file_len) + 4);
    const char wave[4] = {'W', 'A', 'V', 'E'};
    wav.insert(wav.end(), wave, wave + 4);

    // fmt  chunk
    const char fmt[4] = {'f', 'm', 't', ' '};
    wav.insert(wav.end(), fmt, fmt + 4);
    uint32_t fmt_len = 16;
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&fmt_len), reinterpret_cast<uint8_t*>(&fmt_len) + 4);
    uint16_t tag = 1; // PCM
    uint16_t channels = 2;
    uint32_t rate = 44100;
    uint32_t bytes_sec = 176400;
    uint16_t align = 4;
    uint16_t bits = 16;
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&tag), reinterpret_cast<uint8_t*>(&tag) + 2);
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&channels), reinterpret_cast<uint8_t*>(&channels) + 2);
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&rate), reinterpret_cast<uint8_t*>(&rate) + 4);
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&bytes_sec), reinterpret_cast<uint8_t*>(&bytes_sec) + 4);
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&align), reinterpret_cast<uint8_t*>(&align) + 2);
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&bits), reinterpret_cast<uint8_t*>(&bits) + 2);

    // data chunk
    const char data[4] = {'d', 'a', 't', 'a'};
    wav.insert(wav.end(), data, data + 4);
    uint32_t data_len = 4;
    wav.insert(wav.end(), reinterpret_cast<uint8_t*>(&data_len), reinterpret_cast<uint8_t*>(&data_len) + 4);
    uint8_t samples[4] = {0x00, 0x11, 0x22, 0x33};
    wav.insert(wav.end(), samples, samples + 4);

    return wav;
}

static void test_valid_wave_parse() {
    std::cout << "[*] Running test_valid_wave_parse...\n";
    auto wav = createValidWav();
    RiffParser parser;
    bool ok = parser.parse(wav.data(), wav.size());
    assert(ok);
    assert(parser.hasAudioFormat());
    assert(parser.getFormat().channels == 2);
    assert(parser.getFormat().sample_rate == 44100);
    assert(parser.getAudioData().size() == 4);
    std::cout << "    -> PASS\n";
}

static void test_stream_ingest_valid() {
    std::cout << "[*] Running test_stream_ingest_valid...\n";
    MediaStream stream;
    uint8_t frame[6] = {0x01, 0x04, 'T', 'E', 'S', 'T'};
    bool ok = stream.ingestFrame(frame, sizeof(frame));
    assert(ok);
    assert(stream.frameCount() == 1);
    std::cout << "    -> PASS\n";
}

static void test_safe_sample_calc() {
    std::cout << "[*] Running test_safe_sample_calc...\n";
    RiffParser parser;
    int32_t sz = parser.calculateSampleBufferSize(100, 480, 1);
    assert(sz == (100 * 480 * 2));
    std::cout << "    -> PASS\n";
}

int main() {
    std::cout << "=== Running tinymedia C++ Unit Test Suite ===\n";
    test_valid_wave_parse();
    test_stream_ingest_valid();
    test_safe_sample_calc();
    std::cout << "=== All C++ Unit Tests Passed Successfully! ===\n";
    return 0;
}
