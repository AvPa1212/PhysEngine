import os
import shutil
import subprocess
import sys

# --- CONFIGURATION ---
PROJECT_ROOT = os.getcwd()
BUILD_DIR = os.path.join(PROJECT_ROOT, "build_web")
DIST_DIR = os.path.join(PROJECT_ROOT, "web_dist")

def run_cmd(cmd, cwd=None, fail_on_error=True):
    """
    Execute a shell command with proper error handling and reporting.
    
    Args:
        cmd (str): Command to execute
        cwd (str): Working directory (optional)
        fail_on_error (bool): Raise exception if command fails (default: True)
        
    Returns:
        int: Return code of the command
        
    Raises:
        RuntimeError: If fail_on_error=True and command fails
    """
    print(f"\n>>> Running: {cmd}")
    try:
        result = subprocess.run(
            cmd,
            shell=True,
            cwd=cwd,
            capture_output=False,
            text=True
        )
        
        if result.returncode != 0:
            error_msg = f"❌ Error executing: {cmd}\nReturn code: {result.returncode}"
            if fail_on_error:
                print(error_msg, file=sys.stderr)
                raise RuntimeError(error_msg)
            else:
                print(error_msg, file=sys.stderr)
                return result.returncode
        
        print(f"✅ Command succeeded: {cmd}")
        return result.returncode
        
    except FileNotFoundError as e:
        error_msg = f"❌ Command not found: {cmd}\nError: {e}"
        print(error_msg, file=sys.stderr)
        if fail_on_error:
            raise RuntimeError(error_msg)
        return 1
    except Exception as e:
        error_msg = f"❌ Unexpected error executing: {cmd}\nError: {e}"
        print(error_msg, file=sys.stderr)
        if fail_on_error:
            raise RuntimeError(error_msg)
        return 1


def main():
    """Main deployment pipeline with comprehensive error handling."""
    try:
        # 1. Force Clean Slate
        print("--- STEP 0: Cleaning previous builds ---")
        for folder in [BUILD_DIR, DIST_DIR]:
            if os.path.exists(folder):
                try:
                    shutil.rmtree(folder)
                    print(f"✅ Removed: {folder}")
                except Exception as e:
                    print(f"❌ Failed to remove {folder}: {e}", file=sys.stderr)
                    raise RuntimeError(f"Failed to clean {folder}")
            os.makedirs(folder)
            print(f"✅ Created: {folder}")

        # 2. Build the C++ Engine via Emcmake
        print("\n--- STEP 1: Building WebAssembly Module ---")
        
        # Check if emcmake is available
        check_emcmake = run_cmd("emcmake --version", fail_on_error=False)
        if check_emcmake != 0:
            raise RuntimeError(
                "❌ Emcmake not found. Please install Emscripten:\n"
                "   https://emscripten.org/docs/getting_started/downloads.html"
            )
        
        em_cmake_cmd = (
            'emcmake cmake .. '
            '-DCMAKE_CXX_FLAGS="-lembind" '
            '-DCMAKE_EXE_LINKER_FLAGS="-lembind -sALLOW_MEMORY_GROWTH=1"'
        )
        run_cmd(em_cmake_cmd, cwd=BUILD_DIR, fail_on_error=True)
        
        print("\n--- STEP 1b: Compiling with emmake ---")
        run_cmd("emmake make", cwd=BUILD_DIR, fail_on_error=True)

        # 3. Verify Artifacts
        print("\n--- STEP 2: Verifying Engine Output ---")
        js_path = os.path.join(DIST_DIR, "MomentumCore.js")
        wasm_path = os.path.join(DIST_DIR, "MomentumCore.wasm")

        required_files = [
            (js_path, "MomentumCore.js"),
            (wasm_path, "MomentumCore.wasm")
        ]
        
        all_exist = True
        for file_path, file_name in required_files:
            if os.path.exists(file_path):
                file_size = os.path.getsize(file_path) / (1024 * 1024)  # MB
                print(f"✅ {file_name} verified ({file_size:.2f} MB)")
            else:
                print(f"❌ Missing: {file_name} at {file_path}", file=sys.stderr)
                all_exist = False
        
        if not all_exist:
            raise RuntimeError(
                f"Missing build artifacts in {DIST_DIR}!\n"
                "Build may have failed. Check Emscripten configuration."
            )

        # FINAL SANITY CHECK
        try:
            with open(js_path, 'r') as f:
                content = f.read()
                if "PhysEngine" in content:
                    print("✅ Export Name 'PhysEngine' confirmed in JS.")
                else:
                    print("⚠️ Warning: 'PhysEngine' not found in JS. Check CMakeLinker flags.", 
                          file=sys.stderr)
        except IOError as e:
            raise RuntimeError(f"Failed to read {js_path}: {e}")

        # 4. Final instructions
        print("\n--- STEP 3: DEPLOYMENT COMPLETE ---")
        print("1. Kill any existing server (Ctrl+C)")
        print("2. Run: python3 -m http.server 8000")
        print("3. CRITICAL: Hard Refresh Browser (Ctrl+F5) to clear cached WASM")
        
        return True

    except RuntimeError as e:
        print(f"\n❌ DEPLOYMENT FAILED: {e}", file=sys.stderr)
        return False
    except Exception as e:
        print(f"\n❌ UNEXPECTED ERROR: {e}", file=sys.stderr)
        return False


def deploy_to_react():
    """Deploy WASM binaries to React UI with error handling."""
    try:
        print("\n--- Deploying WASM to React UI ---")
        
        source_dir = "web_dist"
        dest_dir = "momentum-ui/public/web_dist"
        files_to_copy = ["MomentumCore.js", "MomentumCore.wasm"]

        # 1. Ensure the destination directory exists
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)
            print(f"✅ Created directory: {dest_dir}")

        # 2. Copy the files with error handling
        failed_files = []
        for filename in files_to_copy:
            src_path = os.path.join(source_dir, filename)
            dest_path = os.path.join(dest_dir, filename)
            
            if os.path.exists(src_path):
                try:
                    shutil.copy2(src_path, dest_path)
                    file_size = os.path.getsize(dest_path) / (1024 * 1024)
                    print(f"✅ Deployed: {filename} ({file_size:.2f} MB) → {dest_dir}")
                except Exception as e:
                    print(f"❌ Failed to copy {filename}: {e}", file=sys.stderr)
                    failed_files.append(filename)
            else:
                print(f"❌ Source not found: {src_path}", file=sys.stderr)
                failed_files.append(filename)
        
        if failed_files:
            raise RuntimeError(
                f"Failed to deploy files: {', '.join(failed_files)}\n"
                "Build artifacts may not have been created."
            )
        
        print("✅ React deployment successful!")
        return True

    except RuntimeError as e:
        print(f"❌ React deployment failed: {e}", file=sys.stderr)
        return False
    except Exception as e:
        print(f"❌ Unexpected error during React deployment: {e}", file=sys.stderr)
        return False


if __name__ == "__main__":
    print("="*60)
    print("PhysEngine WebAssembly Deployment Pipeline")
    print("="*60)
    
    success = main()
    
    if success:
        success = deploy_to_react()
    
    if success:
        print("\n" + "="*60)
        print("✅ ALL DEPLOYMENT STEPS COMPLETED SUCCESSFULLY")
        print("="*60)
        sys.exit(0)
    else:
        print("\n" + "="*60)
        print("❌ DEPLOYMENT FAILED - See errors above")
        print("="*60)
        sys.exit(1)