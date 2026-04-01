#!/usr/bin/env bash
# build_web.sh - Build PhysEngine to WebAssembly and deploy to React UI
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_web"
OUTPUT_DIR="$SCRIPT_DIR/momentum-ui/public/web_dist"

# 1. Source Emscripten SDK environment if not already active
if ! command -v emcmake &> /dev/null; then
    # Try common emsdk locations
    EMSDK_PATHS=(
        "$HOME/emsdk/emsdk_env.sh"
        "/opt/emsdk/emsdk_env.sh"
        "/usr/local/emsdk/emsdk_env.sh"
    )
    SOURCED=0
    for EMSDK_ENV in "${EMSDK_PATHS[@]}"; do
        if [ -f "$EMSDK_ENV" ]; then
            echo "Sourcing Emscripten environment from: $EMSDK_ENV"
            # shellcheck source=/dev/null
            source "$EMSDK_ENV"
            SOURCED=1
            break
        fi
    done
    if [ $SOURCED -eq 0 ]; then
        echo "ERROR: emcmake not found and emsdk_env.sh not located."
        echo "Please install Emscripten: https://emscripten.org/docs/getting_started/downloads.html"
        echo "Or source emsdk_env.sh manually before running this script."
        exit 1
    fi
fi

echo "=== Building PhysEngine WebAssembly ==="

# 2. Prepare build directory
mkdir -p "$BUILD_DIR"

# 3. Configure with Emscripten toolchain
echo "--- Configuring with emcmake cmake ---"
emcmake cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release

# 4. Compile
echo "--- Compiling with emmake make ---"
emmake make -C "$BUILD_DIR" MomentumCore -j"$(nproc 2>/dev/null || echo 4)"

# 5. Copy output to momentum-ui/public/web_dist/
echo "--- Deploying to $OUTPUT_DIR ---"
mkdir -p "$OUTPUT_DIR"

# The CMakeLists.txt outputs to web_dist/ via RUNTIME_OUTPUT_DIRECTORY
WEB_DIST="$SCRIPT_DIR/web_dist"

for FILE in momentum.wasm momentum.js MomentumCore.wasm MomentumCore.js; do
    if [ -f "$WEB_DIST/$FILE" ]; then
        cp "$WEB_DIST/$FILE" "$OUTPUT_DIR/$FILE"
        echo "  Copied: $FILE -> $OUTPUT_DIR/"
    fi
done

# Verify expected outputs exist
MISSING=0
for FILE in MomentumCore.js MomentumCore.wasm; do
    if [ ! -f "$OUTPUT_DIR/$FILE" ]; then
        echo "WARNING: Expected output not found: $OUTPUT_DIR/$FILE"
        MISSING=1
    fi
done

if [ $MISSING -eq 0 ]; then
    echo ""
    echo "=== Build complete ==="
    echo "  MomentumCore.js   -> $OUTPUT_DIR/MomentumCore.js"
    echo "  MomentumCore.wasm -> $OUTPUT_DIR/MomentumCore.wasm"
else
    echo ""
    echo "=== Build finished with warnings - check output above ==="
fi
