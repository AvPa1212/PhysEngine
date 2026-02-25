import ctypes
import os
import sys

def get_momentum_lib():
    # 1. Locate the Library relative to this file (Project Root)
    project_root = os.path.dirname(os.path.abspath(__file__))

    if sys.platform.startswith("win"):
        potential_paths = [
            os.path.join(project_root, "build", "MomentumCore.dll"),
            os.path.join(project_root, "build", "Debug", "MomentumCore.dll"),
        ]
        lib_path = next((p for p in potential_paths if os.path.exists(p)), potential_paths[0])
    else:
        lib_path = os.path.join(project_root, "build", "libMomentumCore.so")

    if not os.path.exists(lib_path):
        raise FileNotFoundError(f"MomentumCore library not found at: {lib_path}")

    # 2. Load Library
    lib = ctypes.CDLL(lib_path)

    # 3. Define All Signatures
    # Lifecycle
    lib.Task_Create.restype = ctypes.c_void_p
    lib.Task_Destroy.argtypes = [ctypes.c_void_p]

    # Data Access (Getters/Setters)
    lib.Task_SetStress.argtypes = [ctypes.c_void_p, ctypes.c_double, ctypes.c_double, ctypes.c_double]
    lib.Task_GetStressX.restype = ctypes.c_double
    lib.Task_GetStressY.restype = ctypes.c_double
    lib.Task_GetStressZ.restype = ctypes.c_double
    lib.Task_GetEntropy.restype = ctypes.c_double
    lib.Task_GetCollapseProbability.restype = ctypes.c_double

    # Engine Commands
    lib.Engine_UpdateChaos.argtypes = [ctypes.c_void_p]
    lib.Engine_PerformQuantumCollapse.argtypes = [ctypes.c_void_p]

    return lib