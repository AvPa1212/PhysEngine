import os
import shutil
import subprocess

# --- CONFIGURATION ---
PROJECT_ROOT = os.getcwd()
BUILD_DIR = os.path.join(PROJECT_ROOT, "build_web")
DIST_DIR = os.path.join(PROJECT_ROOT, "web_dist")

def run_cmd(cmd, cwd=None):
    print(f"\n>>> Running: {cmd}")
    result = subprocess.run(cmd, shell=True, cwd=cwd)
    if result.returncode != 0:
        print(f"❌ Error executing: {cmd}")
        exit(1)

def main():
    # 1. Ensure directories exist
    if not os.path.exists(DIST_DIR):
        os.makedirs(DIST_DIR)
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)

    # 2. Build the C++ Engine
    print("\n--- STEP 1: Building WebAssembly Module ---")
    run_cmd("emcmake cmake ..", cwd=BUILD_DIR)
    run_cmd("emmake make", cwd=BUILD_DIR)

    # 3. Verify Artifacts (Check web_dist directly since CMake puts them there)
    print("\n--- STEP 2: Verifying Engine Output ---")
    js_path = os.path.join(DIST_DIR, "MomentumCore.js")
    wasm_path = os.path.join(DIST_DIR, "MomentumCore.wasm")

    if os.path.exists(js_path) and os.path.exists(wasm_path):
        print(f"✅ Engine components verified in {DIST_DIR}")
    else:
        print(f"❌ Missing build artifacts in {DIST_DIR}!")
        exit(1)

    # 4. Final instructions
    print("\n--- STEP 3: DEPLOYMENT COMPLETE ---")
    print("Run the server with: python3 -m http.server 8000")
    print("Then open: http://localhost:8000")

if __name__ == "__main__":
    main()