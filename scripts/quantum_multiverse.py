import sys
import os

# 1. Setup Path to find momentum_api in the project root
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)
sys.path.append(project_root)

from momentum_api import get_momentum_lib

# 2. Initialize Library via Central Controller
momentum = get_momentum_lib()

# 3. Initialize 50 Parallel Realities
num_agents = 50
agents = [momentum.Task_Create() for _ in range(num_agents)]

for i, a in enumerate(agents):
    # Each starts with a tiny variation in X stress to trigger divergence
    momentum.Task_SetStress(a, 0.1 + (i * 0.001), 0.1, 0.1)

print(f"Evolving {num_agents} parallel realities using centralized API...")

# 4. Simulation Loop with Global Observer Logic
for step in range(2000):
    total_entropy = 0
    for a in agents:
        momentum.Engine_UpdateChaos(a)
        total_entropy += momentum.Task_GetEntropy(a)
    
    avg_entropy = total_entropy / num_agents
    
    # Global Quantum Observer: If the average disorder is too high, reset everyone
    if avg_entropy > 0.8:
        print(f"Step {step:4}: Global Entropy {avg_entropy:.4f} exceeds threshold! Collapsing Multiverse...")
        for a in agents:
            momentum.Engine_PerformQuantumCollapse(a)

# 5. Final State Check
print("\nFinal State of Reality 0 vs Reality 49:")
print(f"Agent 00 Entropy: {momentum.Task_GetEntropy(agents[0]):.4f}")
print(f"Agent 49 Entropy: {momentum.Task_GetEntropy(agents[49]):.4f}")

# 6. Cleanup
for a in agents:
    momentum.Task_Destroy(a)

print("\nMultiverse simulation complete. Memory cleared.")