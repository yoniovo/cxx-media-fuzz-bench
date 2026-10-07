# Task 02: Double Free on Error Recovery

## Vulnerability Overview
- **Vulnerability Type**: Incomplete Pointer Invalidation leading to Double Free
- **CWE**: [CWE-415](https://cwe.mitre.org/data/definitions/415.html) (Double Free)
- **Component**: `src/media_stream.cpp` -> `StreamFrameBuffer::release()` / `MediaStream::ingestFrame()`
- **Severity**: Critical (Memory Corruption / Arbitrary Write)

## Vulnerability Details
`StreamFrameBuffer` manages dynamic memory allocation for incoming media frames. When an invalid frame marker (`0xEE`) is received, `MediaStream::ingestFrame()` initiates error recovery and invokes `StreamFrameBuffer::release()`:
```cpp
void StreamFrameBuffer::release() {
    if (m_data) {
        delete[] m_data;
        // Missing: m_data = nullptr;
    }
    m_size = 0;
}
```
Because `m_data` is freed but not reset to `nullptr`, the object retains a dangling pointer. When the parent `MediaStream` object is subsequently destructed or torn down, the `StreamFrameBuffer::~StreamFrameBuffer()` destructor runs:
```cpp
StreamFrameBuffer::~StreamFrameBuffer() {
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
}
```
This triggers an AddressSanitizer `attempting double-free on 0x...` fatal error.

## Reproduction
```bash
make fuzzers
./bin/fuzzer_stream tasks/task_02_double_free/poc.bin
```

## Remediation
Properly zero the pointer immediately upon deallocation in `StreamFrameBuffer::release()`:
```cpp
void StreamFrameBuffer::release() {
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
    m_size = 0;
}
```
Or refactor `m_data` into `std::unique_ptr<uint8_t[]>` to adhere strictly to C++ RAII resource management.
