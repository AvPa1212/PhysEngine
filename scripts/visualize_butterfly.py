import sys
import os
import matplotlib.pyplot as plt

# 1. Setup Path to find momentum_api in the project root
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)
sys.path.append(project_root)

from momentum_api import get_momentum_lib

# 2. Initialize Library via Central Controller
momentum = get_momentum_lib()

# 3. Initialize Agent
task = momentum.Task_Create()
# Initial state: minimal stress to begin the attractor path
momentum.Task_SetStress(task, 0.1, 0.1, 0.1)

x_coords, y_coords, z_coords = [], [], []

# 4. Data Generation
print("Generating 10,000 points of chaos via Central API...")
for i in range(10000):
    momentum.Engine_UpdateChaos(task)
    x_coords.append(momentum.Task_GetStressX(task))
    y_coords.append(momentum.Task_GetStressY(task))
    z_coords.append(momentum.Task_GetStressZ(task))

# 5. Plotting
fig = plt.figure(figsize=(12, 8), facecolor='white')
ax = fig.add_subplot(111, projection='3d')

# Plotting the Lorenz Attractor "Butterfly Wings"
ax.plot(x_coords, y_coords, z_coords, lw=0.5, color='darkorange')

ax.set_title("Lorenz Attractor - Momentum Engine Output")
ax.set_xlabel("Stress X")
ax.set_ylabel("Stress Y")
ax.set_zlabel("Stress Z")

# Set a cinematic viewing angle
ax.view_init(elev=20, azim=45)

print("Rendering plot...")
# Note: When running from /scripts, the image will save in /scripts
output_path = "butterfly_attractor.png"
plt.savefig(output_path)
print(f"Static plot saved to: {os.path.abspath(output_path)}")

# Attempt to show window (requires X-Server/Display)
try:
    plt.show()
except Exception:
    print("GUI display not available; check the saved .png for results.")

# 6. Cleanup
momentum.Task_Destroy(task)