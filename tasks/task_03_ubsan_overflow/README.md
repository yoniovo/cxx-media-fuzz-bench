# Task 03: Signed Integer Overflow (UndefinedBehaviorSanitizer)

## Vulnerability Overview
- **Vulnerability Type**: Signed Integer Overflow & Undefined Bit Shift
- **CWE**: [CWE-190](https://cwe.mitre.org/data/definitions/190.html) (Integer Overflow or Wraparound)
- **Component**: `src/riff_parser.cpp` -> `RiffParser::calculateSampleBufferSize()`
- **Severity**: Medium (Undefined Behavior / Denial of Service)

## Vulnerability Details
When computing buffer sizes for multi-channel sample streams, `calculateSampleBufferSize()` evaluates:
```cpp
int32_t total_samples = num_frames * samples_per_frame;
int32_t total_bytes = total_samples << channels;
```
Because `num_frames` and `samples_per_frame` are signed 32-bit integers derived from untrusted input, large values (e.g. `num_frames = 2,000,000`, `samples_per_frame = 2,000`) cause an arithmetic product of $4,000,000,000$, exceeding `INT32_MAX` ($2,147,483,647$). In ISO C++, signed integer overflow invokes Undefined Behavior (UB), which compilers can exploit to optimize away safety checks or produce negative allocation sizes.

UndefinedBehaviorSanitizer (UBSan) flags this condition:
```text
runtime error: signed integer overflow: 2000000 * 2000 cannot be represented in type 'int32_t'
```

## Reproduction
```bash
make fuzzers
UBSAN_OPTIONS=halt_on_error=1 ./bin/fuzzer_riff tasks/task_03_ubsan_overflow/poc.bin
```

## Remediation
Use checked 64-bit arithmetic or overflow-safe arithmetic primitives, clamping or returning an error if parameters exceed buffer limits:
```cpp
int64_t total_samples = static_cast<int64_t>(num_frames) * static_cast<int64_t>(samples_per_frame);
if (total_samples < 0 || total_samples > 100000000) {
    return -1;
}
if (channels < 0 || channels > 8) {
    return -1;
}
int64_t total_bytes = total_samples << channels;
if (total_bytes > 0x7FFFFFFF) {
    return -1;
}
return static_cast<int32_t>(total_bytes);
```
