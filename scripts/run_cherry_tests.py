#!/usr/bin/env python3
import argparse
import json
import os
import re
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any


ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
EMSDK_NOISE_PREFIXES = (
    "Setting up EMSDK environment",
    "Adding directories to PATH:",
    "PATH +=",
    "Setting environment variables:",
    "EMSDK =",
    "EMSDK_NODE =",
    "EMSDK_PYTHON =",
)


def normalize_output(text: str) -> str:
    text = ANSI_RE.sub("", text).replace("\r\n", "\n")
    kept = []
    for line in text.split("\n"):
        stripped = line.strip()
        if not stripped:
            kept.append("")
            continue
        if any(stripped.startswith(prefix) for prefix in EMSDK_NOISE_PREFIXES):
            continue
        kept.append(line)
    return "\n".join(kept).strip()


@dataclass
class TestResult:
    name: str
    ok: bool
    message: str
    duration_ms: int


def load_manifest(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as f:
        data = json.load(f)
    if "tests" not in data or not isinstance(data["tests"], list):
        raise ValueError("manifest must contain a top-level 'tests' array")
    return data


def assert_contains(haystack: str, needles: list[str], stream_name: str) -> list[str]:
    errors: list[str] = []
    for needle in needles:
        if needle not in haystack:
            errors.append(f"{stream_name} missing expected substring: {needle!r}")
    return errors


def assert_not_contains(haystack: str, needles: list[str], stream_name: str) -> list[str]:
    errors: list[str] = []
    for needle in needles:
        if needle in haystack:
            errors.append(f"{stream_name} unexpectedly contains substring: {needle!r}")
    return errors


def run_case(exe: Path, repo_root: Path, case: dict[str, Any]) -> TestResult:
    name = case["name"]
    rel_file = case["file"]
    path = repo_root / "test" / rel_file
    if not path.exists():
        return TestResult(name=name, ok=False, message=f"case file not found: {path}", duration_ms=0)

    cmd = [str(exe), str(path)]
    start = time.time()
    proc = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
    duration_ms = int((time.time() - start) * 1000)

    stdout = normalize_output(proc.stdout)
    stderr = normalize_output(proc.stderr)
    expected = case.get("expect", {})

    errors: list[str] = []
    allowed_exit_codes = case.get("allowed_exit_codes")
    if allowed_exit_codes is not None and proc.returncode not in allowed_exit_codes:
        errors.append(f"unexpected exit code {proc.returncode}, allowed={allowed_exit_codes}")

    errors += assert_contains(stdout, expected.get("stdout_contains", []), "stdout")
    errors += assert_not_contains(stdout, expected.get("stdout_not_contains", []), "stdout")
    errors += assert_contains(stderr, expected.get("stderr_contains", []), "stderr")
    errors += assert_not_contains(stderr, expected.get("stderr_not_contains", []), "stderr")

    if errors:
        details = [
            f"command: {' '.join(cmd)}",
            f"exit: {proc.returncode}",
            "stdout:",
            stdout if stdout else "<empty>",
            "stderr:",
            stderr if stderr else "<empty>",
            "errors:",
            *errors,
        ]
        return TestResult(name=name, ok=False, message="\n".join(details), duration_ms=duration_ms)
    return TestResult(name=name, ok=True, message="", duration_ms=duration_ms)


def main() -> int:
    parser = argparse.ArgumentParser(description="Run Cherry test suite")
    parser.add_argument("--manifest", default="test/tests_manifest.json", help="Path to tests manifest")
    parser.add_argument("--exe", default=None, help="Path to Cherry executable")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    manifest_path = (repo_root / args.manifest).resolve()
    manifest = load_manifest(manifest_path)

    if args.exe:
        exe = Path(args.exe).resolve()
    else:
        exe_name = "Cherry.exe" if os.name == "nt" else "Cherry"
        exe = (repo_root / "build" / exe_name).resolve()

    if not exe.exists():
        print(f"error: executable not found: {exe}", file=sys.stderr)
        return 2

    results: list[TestResult] = []
    for case in manifest["tests"]:
        result = run_case(exe, repo_root, case)
        results.append(result)
        tag = "PASS" if result.ok else "FAIL"
        print(f"[{tag}] {result.name} ({result.duration_ms}ms)")
        if not result.ok:
            print(result.message)
            print("-" * 72)

    passed = sum(1 for r in results if r.ok)
    total = len(results)
    failed = total - passed
    print(f"Summary: {passed}/{total} passed, {failed} failed")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
