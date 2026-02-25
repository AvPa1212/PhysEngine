import sys
import os

# 1. Setup Path to find momentum_api in the project root
# Moves up from /validation to PhysEngine/
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)
sys.path.append(project_root)

try:
    from momentum_api import get_momentum_lib
except ImportError:
    print("Error: Could not find momentum_api.py in the project root.")
    sys.exit(1)

# 2. Initialize Library via Central Controller
try:
    momentum = get_momentum_lib()
    print(f"--- Bridge Verification Active ---")
except Exception as e:
    print(f"Bridge Failure: {e}")
    sys.exit(1)

# 3. Setup Test Case (Classical Physics)
task = momentum.Task_Create()

# Initialize at Origin (0,0)
# Note: Ensure your C++ Engine_IntegrateClassical uses a default velocity (e.g., 10.0)
momentum.Task_SetPosition(task, 0.0, 0.0)
initial_x = momentum.Task_GetPositionX(task)

print(f"Initial State -> X: {initial_x:.2f}")

# 4. Execute C++ RK4 Integration
print("Executing 100 iterations of C++ RK4 Physics...")
for _ in range(100):
    momentum.Engine_IntegrateClassical(task)

# 5. Retrieve and Validate Results
final_x = momentum.Task_GetPositionX(task)
print(f"Final State   -> X: {final_x:.6f}")

# Basic delta check (Assuming dt=0.01 and v=10.0, expect change of ~10.0)
displacement = abs(final_x - initial_x)
print(f"Total Displacement: {displacement:.4f}")

if displacement > 0:
    print("SUCCESS: C++ Engine state successfully mutated via Python Bridge.")
else:
    print("WARNING: No displacement detected. Check C++ Engine velocity/dt constants.")

# 6. Cleanup
momentum.Task_Destroy(task)
print("--- Bridge Test Complete: Memory Released ---")