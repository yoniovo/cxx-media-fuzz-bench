#!/usr/bin/env bash
set -euo pipefail

echo "[*] Compiling C++ tinymedia library and OSS-Fuzz harnesses with Clang++ ASan/UBSan..."
make clean
make all fuzzers
echo "[+] Build complete. Binaries available in bin/"
