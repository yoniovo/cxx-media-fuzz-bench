#!/usr/bin/env python3
"""
AfterQuery Benchmark Evaluator: Automated Patch Verification Harness (C++ Media Targets)
Evaluates security patches against modern C++ OSS-Fuzz targets:
1. Verifies ASan/UBSan detection on POC in baseline unpatched state.
2. Applies candidate patch (human or LLM-generated).
3. Rebuilds harnesses and verifies zero sanitizer faults on POC input.
4. Executes full C++ functional regression test suite.
5. Reverts patch cleanly.
"""

import os
import sys
import subprocess
import argparse
import json
from pathlib import Path

TASKS = {
    "task_01_off_by_one": {
        "harness": "bin/fuzzer_riff",
        "poc": "tasks/task_01_off_by_one/poc.bin",
        "patch": "tasks/task_01_off_by_one/solution.patch",
        "expected_sanitizer": "heap-buffer-overflow",
        "env": {}
    },
    "task_02_double_free": {
        "harness": "bin/fuzzer_stream",
        "poc": "tasks/task_02_double_free/poc.bin",
        "patch": "tasks/task_02_double_free/solution.patch",
        "expected_sanitizer": "double-free",
        "env": {}
    },
    "task_03_ubsan_overflow": {
        "harness": "bin/fuzzer_riff",
        "poc": "tasks/task_03_ubsan_overflow/poc.bin",
        "patch": "tasks/task_03_ubsan_overflow/solution.patch",
        "expected_sanitizer": "signed integer overflow",
        "env": {"UBSAN_OPTIONS": "halt_on_error=1"}
    }
}

def run_cmd(cmd, env_vars=None, check=False):
    env = os.environ.copy()
    if env_vars:
        env.update(env_vars)
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True, env=env)
    if check and res.returncode != 0:
        raise RuntimeError(f"Command failed [{cmd}]:\n{res.stderr}")
    return res

def evaluate_task(task_name, patch_path=None):
    if task_name not in TASKS:
        print(f"[!] Unknown task: {task_name}")
        return False

    cfg = TASKS[task_name]
    patch_file = patch_path if patch_path else cfg["patch"]
    poc_path = cfg["poc"]
    harness_path = cfg["harness"]

    print(f"\n==================================================")
    print(f"[*] Evaluating C++ Benchmark Task: {task_name}")
    print(f"[*] Harness: {harness_path} | POC: {poc_path}")
    print(f"[*] Candidate Patch: {patch_file}")
    print(f"==================================================")

    # 1. Clean build baseline
    run_cmd("git checkout .")
    run_cmd("make clean && make all fuzzers", check=True)

    # 2. Verify crash in unpatched state
    pre_run = run_cmd(f"./{harness_path} {poc_path}", env_vars=cfg["env"])
    if pre_run.returncode == 0:
        print("[-] FAILED: Target did NOT trigger sanitizer in baseline state!")
        return False
    if cfg["expected_sanitizer"] not in pre_run.stderr and cfg["expected_sanitizer"] not in pre_run.stdout:
        print(f"[-] WARNING: Expected signature '{cfg['expected_sanitizer']}' in output.")
    print("[+] Confirmed: Vulnerability reproduces with ASan/UBSan on baseline.")

    # 3. Apply patch
    print(f"[*] Applying patch: {patch_file}")
    patch_apply = run_cmd(f"git apply {patch_file}")
    if patch_apply.returncode != 0:
        print(f"[-] FAILED: Patch failed to apply cleanly:\n{patch_apply.stderr}")
        return False

    # 4. Rebuild with patch
    print("[*] Rebuilding C++ targets with ASan/UBSan...")
    build_res = run_cmd("make all fuzzers")
    if build_res.returncode != 0:
        print(f"[-] FAILED: Build failed after applying patch:\n{build_res.stderr}")
        run_cmd("git checkout .")
        return False

    # 5. Verify POC no longer crashes
    print("[*] Running POC against patched binary...")
    post_run = run_cmd(f"./{harness_path} {poc_path}", env_vars=cfg["env"])
    if post_run.returncode != 0:
        print(f"[-] FAILED: Binary still failed after patch:\n{post_run.stderr}")
        run_cmd("git checkout .")
        return False
    print("[+] Confirmed: ASan/UBSan is CLEAN on previously crashing POC input.")

    # 6. Run regression test suite
    print("[*] Running C++ functional regression test suite...")
    test_res = run_cmd("make test")
    if test_res.returncode != 0:
        print(f"[-] FAILED: Regression tests failed:\n{test_res.stderr}")
        run_cmd("git checkout .")
        return False
    print("[+] Confirmed: 100% of functional regression tests passed.")

    # 7. Cleanup
    run_cmd("git checkout .")
    print(f"[SUCCESS] {task_name} resolved successfully!")
    return True

def main():
    parser = argparse.ArgumentParser(description="Evaluate security patches on C++ OSS-Fuzz targets")
    parser.add_argument("--task", choices=list(TASKS.keys()), help="Specific task name")
    parser.add_argument("--patch", help="Path to patch file to test")
    parser.add_argument("--all", action="store_true", help="Evaluate all benchmark tasks")
    args = parser.parse_args()

    if args.all:
        results = {}
        for t in TASKS:
            ok = evaluate_task(t)
            results[t] = "PASSED" if ok else "FAILED"
        print("\n================== BENCHMARK SUMMARY ==================")
        print(json.dumps(results, indent=2))
        sys.exit(0 if all(v == "PASSED" for v in results.values()) else 1)
    elif args.task:
        ok = evaluate_task(args.task, args.patch)
        sys.exit(0 if ok else 1)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
