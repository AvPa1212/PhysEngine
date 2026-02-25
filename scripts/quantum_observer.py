import sys
import os

# 1. Setup Path to find momentum_api in the project root
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)
sys.path.append(project_root)

from momentum_api import get_momentum_lib

# 2. Initialize Library via Central Controller
momentum = get_momentum_lib()

# 3. Initialize Observation Task
task = momentum.Task_Create()
print("--- Starting Quantum Observer Loop (Centralized API) ---")

try:
    for i in range(1, 1001):
        momentum.Engine_UpdateChaos(task)
        
        # Every 100 steps, perform a measurement
        if i % 100 == 0:
            entropy = momentum.Task_GetEntropy(task)
            prob = momentum.Task_GetCollapseProbability(task)
            
            print(f"Step {i:4}: Entropy = {entropy:.4f} | Prob = {prob:6.2%}")
            
            # Threshold logic for observer intervention
            if prob > 0.5:
                print(">>> CRITICAL ENTROPY: Triggering Quantum Collapse...")
                momentum.Engine_PerformQuantumCollapse(task)
                
                new_entropy = momentum.Task_GetEntropy(task)
                print(f"Post-Collapse Entropy: {new_entropy:.4f}")

except KeyboardInterrupt:
    print("\nObservation interrupted by user.")

finally:
    # 4. Cleanup
    # This block ensures Task_Destroy is called even if the loop crashes or is interrupted
    momentum.Task_Destroy(task)
    print("C++ Resources Released. Simulation complete.")