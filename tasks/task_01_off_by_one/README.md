# Task 01: Off-by-One Heap Buffer Overwrite

## Vulnerability Overview
- **Vulnerability Type**: Off-by-one Error leading to Heap Buffer Overflow Write
- **CWE**: [CWE-193](https://cwe.mitre.org/data/definitions/193.html) (Off-by-one Error), [CWE-122](https://cwe.mitre.org/data/definitions/122.html) (Heap-based Buffer Overflow)
- **Component**: `src/riff_parser.cpp` -> `RiffParser::parseMetadataTag()`
- **Severity**: High (Memory Corruption / Potential Code Execution)

## Vulnerability Details
When parsing metadata tags (such as `INAM` or `IART` inside an INFO LIST chunk), `parseMetadataTag` allocates a dynamic character buffer sized exactly to the declared tag length `tag_len`:
```cpp
char* buf = new char[tag_len];
std::memcpy(buf, tag_data, tag_len);
buf[tag_len] = '\0';
```
In C/C++, valid indices for an array of size $N$ are $0 \dots N-1$. Writing `buf[tag_len]` writes the null terminator 1 byte past the end of the heap allocation. AddressSanitizer intercepts this as `heap-buffer-overflow (WRITE of size 1)`.

## Reproduction
```bash
make fuzzers
./bin/fuzzer_riff tasks/task_01_off_by_one/poc.bin
```

## Remediation
Allocate space for the null terminator (`new char[tag_len + 1]`), or directly construct `std::string` using pointer range constructor:
```cpp
out_val = std::string(reinterpret_cast<const char*>(tag_data), tag_len);
```
