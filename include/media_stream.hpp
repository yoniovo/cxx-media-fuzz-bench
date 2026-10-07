#ifndef MEDIA_STREAM_HPP
#define MEDIA_STREAM_HPP

#include <cstdint>
#include <cstddef>

namespace tinymedia {

/* Manages dynamically allocated frame buffers */
class StreamFrameBuffer {
public:
    StreamFrameBuffer();
    ~StreamFrameBuffer();

    bool allocate(size_t size);
    void release();
    uint8_t* data() { return m_data; }
    size_t size() const { return m_size; }

private:
    uint8_t* m_data{nullptr};
    size_t m_size{0};
};

/* Stateful media stream processor */
class MediaStream {
public:
    MediaStream();
    ~MediaStream();

    /* Ingest frame packet. Triggering Task 2 (Double Free) on specific malformed frame markers */
    bool ingestFrame(const uint8_t* data, size_t size);
    size_t frameCount() const { return m_frame_count; }

private:
    StreamFrameBuffer m_buffer;
    size_t m_frame_count{0};
};

} // namespace tinymedia

#endif // MEDIA_STREAM_HPP
