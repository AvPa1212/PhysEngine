import os
import sys
import ctypes

def get_momentum_lib():
    """
    Load the MomentumCore shared library with robust error handling.
    
    Returns:
        ctypes.CDLL: The loaded library, or None if loading fails.
        
    Raises:
        RuntimeError: If the library cannot be found or loaded.
    """
    try:
        # 1. Locate the Library relative to this file (Project Root)
        project_root = os.path.dirname(os.path.abspath(__file__))

        if sys.platform.startswith("win"):
            potential_paths = [
                os.path.join(project_root, "build", "MomentumCore.dll"),
                os.path.join(project_root, "build", "Debug", "MomentumCore.dll"),
            ]
            lib_path = next((p for p in potential_paths if os.path.exists(p)), None)
        else:
            lib_path = os.path.join(project_root, "build", "libMomentumCore.so")
            if not os.path.exists(lib_path):
                lib_path = None

        # Check if library exists
        if lib_path is None or not os.path.exists(lib_path):
            error_msg = (
                "MomentumCore library not found.\n"
                "Expected locations:\n"
            )
            if sys.platform.startswith("win"):
                error_msg += (
                    f"  - {os.path.join(project_root, 'build', 'MomentumCore.dll')}\n"
                    f"  - {os.path.join(project_root, 'build', 'Debug', 'MomentumCore.dll')}\n"
                )
            else:
                error_msg += f"  - {os.path.join(project_root, 'build', 'libMomentumCore.so')}\n"
            error_msg += "\nPlease build the project first by running:\n"
            error_msg += "  python3 deploy_web.py\n"
            raise RuntimeError(error_msg)

        # 2. Load Library with error handling
        try:
            lib = ctypes.CDLL(lib_path)
        except OSError as e:
            raise RuntimeError(
                f"Failed to load library from {lib_path}.\n"
                f"Error: {e}\n"
                f"This may indicate a missing dependency or incompatible architecture."
            )

        # 3. Define All Signatures
        # Lifecycle
        lib.Task_Create.restype = ctypes.c_void_p
        lib.Task_Destroy.argtypes = [ctypes.c_void_p]

        # Data Access (Getters/Setters)
        lib.Task_GetPositionX.argtypes = [ctypes.c_void_p]
        lib.Task_GetPositionX.restype = ctypes.c_double
        # ... (add remaining function signatures)

        print("[INFO] MomentumCore library loaded successfully from: " + lib_path)
        return lib

    except RuntimeError as e:
        print(f"[ERROR] {e}", file=sys.stderr)
        raise
    except Exception as e:
        print(f"[ERROR] Unexpected error loading momentum library: {e}", file=sys.stderr)
        raise RuntimeError(f"Failed to initialize momentum_api: {e}")


# Wrapper function for safe library initialization
def safe_get_momentum_lib():
    """
    Safely load the momentum library and provide helpful error messages.
    
    Returns:
        ctypes.CDLL: The loaded library
        
    Raises:
        RuntimeError: If initialization fails
    """
    try:
        return get_momentum_lib()
    except RuntimeError:
        raise
    except Exception as e:
        print(f"[ERROR] Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


# Usage example in scripts:
# try:
#     momentum = safe_get_momentum_lib()
#     # ... use momentum library ...
# except RuntimeError as e:
#     print(f"Failed to initialize physics engine: {e}")
#     sys.exit(1)