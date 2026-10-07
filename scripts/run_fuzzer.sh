#!/usr/bin/env bash
set -euo pipefail

TARGET=${1:-"fuzzer_riff"}
MAX_TIME=${2:-"30"} # default 30 seconds

if [ ! -f "bin/${TARGET}" ]; then
    echo "[!] Target bin/${TARGET} not found. Running scripts/build_fuzzers.sh first..."
    ./scripts/build_fuzzers.sh
fi

echo "[*] Launching libFuzzer on bin/${TARGET} for max ${MAX_TIME}s with dictionary and seed corpus..."
./bin/${TARGET} \
    -dict=fuzzer/media.dict \
    -max_total_time=${MAX_TIME} \
    -print_final_stats=1 \
    fuzzer/corpus/
