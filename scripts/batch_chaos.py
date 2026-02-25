import sys
import os

# 1. Setup Path to find momentum_api in the project root
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)
sys.path.append(project_root)

from momentum_api import get_momentum_lib

# 2. Initialize Library via Central Controller
momentum = get_momentum_lib()

# 3. Create 100 "Agents"
# The API handles Task_Create and Task_Destroy signatures automatically
agents = [momentum.Task_Create() for _ in range(100)]

# Initialize each with a slightly different starting X stress
# This demonstrates the "Butterfly Effect" where small changes lead to massive divergence
for i, agent in enumerate(agents):
    momentum.Task_SetStress(agent, 1.0 + (i * 0.001), 1.0, 1.0)

# 4. Run 2000 steps for all agents
print(f"Evolving {len(agents)} chaotic systems using centralized API...")
for _ in range(2000):
    for agent in agents:
        momentum.Engine_UpdateChaos(agent)

# 5. Sample the results
print("\nFinal X-Stress Samples (Verifying Divergence):")
for i in [0, 25, 50, 75, 99]:
    val = momentum.Task_GetStressX(agents[i])
    print(f"Agent {i:02}: {val:>10.6f}")

# 6. Cleanup
# Crucial: This releases the C++ memory allocated for each agent
for agent in agents:
    momentum.Task_Destroy(agent)

print("\nCleanup complete. All C++ heap allocations released.")