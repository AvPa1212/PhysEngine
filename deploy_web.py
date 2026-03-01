import os
import shutil
import subprocess

# --- CONFIGURATION ---
PROJECT_ROOT = os.getcwd()
BUILD_DIR = os.path.join(PROJECT_ROOT, "build_web")
DIST_DIR = os.path.join(PROJECT_ROOT, "web_dist")

def run_command(command, cwd=ROOT_DIR):
    """Helper to run shell commands and catch errors."""
    print(f"\n>>> Running: {command}")
    result = subprocess.run(command, shell=True, cwd=cwd)
    if result.returncode != 0:
        print(f"❌ Error executing: {command}")
        print("Did you forget to run 'source ~/emsdk/emsdk_env.sh' first?")
        sys.exit(1)

def build_wasm():
    print("\n--- STEP 1: Building WebAssembly Module ---")
    os.makedirs(BUILD_DIR, exist_ok=True)
    os.makedirs(DIST_DIR, exist_ok=True)
    
    # Run Emscripten CMake and Make
    run_command("emcmake cmake ..", cwd=BUILD_DIR)
    run_command("emmake make", cwd=BUILD_DIR)

def package_files():
    print("\n--- STEP 2: Packaging Files for Web ---")
    files_to_copy = ["MomentumCore.js", "MomentumCore.wasm"]
    
    for file_name in files_to_copy:
        src = os.path.join(BUILD_DIR, file_name)
        dst = os.path.join(DIST_DIR, file_name)
        
        if os.path.exists(src):
            shutil.copy2(src, dst)
            print(f"✅ Packaged: {file_name} -> web_dist/")
        else:
            print(f"❌ Missing expected build artifact: {src}")
            sys.exit(1)

def start_server():
    print("\n--- STEP 3: Starting Local Web Server ---")
    Handler = http.server.SimpleHTTPRequestHandler
    
    # Ensure .wasm files are served with the correct MIME type
    Handler.extensions_map.update({
        '.wasm': 'application/wasm',
    })

    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"🚀 Momentum Web Server running at: http://localhost:{PORT}")
        print("Press Ctrl+C to stop the server.")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server...")

if __name__ == "__main__":
    main()