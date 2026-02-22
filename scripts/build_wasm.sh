#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build-wasm}"
OUT_DIR="${2:-web/wasm}"
CONFIG="${3:-Release}"
USE_PRESET="${4:-0}"

command -v emcmake >/dev/null 2>&1 || { echo "emcmake not found in PATH"; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo "cmake not found in PATH"; exit 1; }

if [ "$USE_PRESET" = "1" ]; then
  echo "Configuring Emscripten build using CMake preset 'wasm'..."
  cmake --preset wasm

  echo "Building Cherry (WASM) using CMake build preset 'wasm'..."
  cmake --build --preset wasm
else
  echo "Configuring Emscripten build..."
  emcmake cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$CONFIG"

  echo "Building Cherry (WASM)..."
  cmake --build "$BUILD_DIR" --config "$CONFIG"
fi

mkdir -p "$OUT_DIR"

JS_PATH=""
if [ -f "$BUILD_DIR/Cherry.js" ]; then
  JS_PATH="$BUILD_DIR/Cherry.js"
elif [ -f "$BUILD_DIR/$CONFIG/Cherry.js" ]; then
  JS_PATH="$BUILD_DIR/$CONFIG/Cherry.js"
else
  echo "Could not find Cherry.js in $BUILD_DIR"
  exit 1
fi

BASE_DIR="$(dirname "$JS_PATH")"
cp "$JS_PATH" "$OUT_DIR/Cherry.js"

for asset in Cherry.wasm Cherry.data Cherry.worker.js; do
  if [ -f "$BASE_DIR/$asset" ]; then
    cp "$BASE_DIR/$asset" "$OUT_DIR/$asset"
  fi
done

echo "WASM artifacts copied to '$OUT_DIR'."
