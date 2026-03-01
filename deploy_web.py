import os
import shutil
import subprocess

# --- CONFIGURATION ---
PROJECT_ROOT = os.getcwd()
BUILD_DIR = os.path.join(PROJECT_ROOT, "build_web")
DIST_DIR = os.path.join(PROJECT_ROOT, "web_dist")

def run_cmd(cmd, cwd=None):
    print(f"\n>>> Running: {cmd}")
    # Using env to ensure Emscripten is found if not in global path
    result = subprocess.run(cmd, shell=True, cwd=cwd)
    if result.returncode != 0:
        print(f"❌ Error executing: {cmd}")
        exit(1)

def main():
    # 1. Force Clean Slate
    # This is crucial because Emscripten can cache old 'void*' type information
    print("--- STEP 0: Cleaning previous builds ---")
    for folder in [BUILD_DIR, DIST_DIR]:
        if os.path.exists(folder):
            shutil.rmtree(folder)
        os.makedirs(folder)

    # 2. Build the C++ Engine via Emcmake
    # Note: We pass -lembind through CXXFLAGS to ensure it's picked up
    print("\n--- STEP 1: Building WebAssembly Module ---")
    
    # We set -lembind here as a backup in case CMakeLists doesn't have it
    em_cmake_cmd = 'emcmake cmake .. -DCMAKE_CXX_FLAGS="-lembind" -DCMAKE_EXE_LINKER_FLAGS="-lembind -sALLOW_MEMORY_GROWTH=1"'

    run_cmd(em_cmake_cmd, cwd=BUILD_DIR)
    run_cmd("emmake make", cwd=BUILD_DIR)

    # 3. Verify Artifacts
    print("\n--- STEP 2: Verifying Engine Output ---")
    js_path = os.path.join(DIST_DIR, "MomentumCore.js")
    wasm_path = os.path.join(DIST_DIR, "MomentumCore.wasm")

    if os.path.exists(js_path) and os.path.exists(wasm_path):
        print(f"✅ Engine components verified in {DIST_DIR}")
        
        # FINAL SANITY CHECK: Look for the name in the generated JS
        with open(js_path, 'r') as f:
            content = f.read()
            if "PhysEngine" in content:
                print("✅ Export Name 'PhysEngine' confirmed in JS.")
            else:
                print("⚠️ Warning: 'PhysEngine' not found in JS. Check CMakeLinker flags.")
    else:
        print(f"❌ Missing build artifacts in {DIST_DIR}!")
        print(f"Expected: {js_path}")
        exit(1)

    # 4. Final instructions
    print("\n--- STEP 3: DEPLOYMENT COMPLETE ---")
    print("1. Kill any existing server (Ctrl+C)")
    print("2. Run: python3 -m http.server 8000")
    print("3. CRITICAL: Hard Refresh Browser (Ctrl+F5) to clear cached WASM")

def deploy_to_react():
    print("\n--- Deploying WASM to React UI ---")
    
    # Define Source and Destination paths
    source_dir = "web_dist"
    dest_dir = "momentum-ui/public/web_dist"
    files_to_copy = ["MomentumCore.js", "MomentumCore.wasm"]

    # 1. Ensure the destination directory exists
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
        print(f"Created directory: {dest_dir}")

    # 2. Copy the files
    for filename in files_to_copy:
        src_path = os.path.join(source_dir, filename)
        dest_path = os.path.join(dest_dir, filename)
        
        if os.path.exists(src_path):
            shutil.copy2(src_path, dest_path)
            print(f"Successfully deployed: {filename} -> {dest_dir}")
        else:
            print(f"Warning: {src_path} not found. Build may have failed.")

if __name__ == "__main__":
    main()
    deploy_to_react()