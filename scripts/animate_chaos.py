import sys
import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# 1. Setup Path to find momentum_api in the project root
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)
sys.path.append(project_root)

from momentum_api import get_momentum_lib

# 2. Initialize Library via Central Controller
momentum = get_momentum_lib()

# 3. Setup Agent and Data
# Using signatures defined in momentum_api.py
task = momentum.Task_Create()
momentum.Task_SetStress(task, 0.1, 0.1, 0.1)

x_data, y_data, z_data = [], [], []

# 4. Setup Figure
fig = plt.figure(figsize=(10, 7), facecolor='black')
ax = fig.add_subplot(111, projection='3d', facecolor='black')
line, = ax.plot([], [], [], lw=0.8, color='cyan')

# Aesthetic limits for the Lorenz system
ax.set_xlim(-25, 25)
ax.set_ylim(-35, 35)
ax.set_zlim(0, 50)
ax.axis('off') 

def init():
    line.set_data([], [])
    line.set_3d_properties([])
    return line,

def update(frame):
    # Run 5 engine steps per frame for smooth speed
    for _ in range(5):
        momentum.Engine_UpdateChaos(task)
        x_data.append(momentum.Task_GetStressX(task))
        y_data.append(momentum.Task_GetStressY(task))
        z_data.append(momentum.Task_GetStressZ(task))
    
    line.set_data(x_data, y_data)
    line.set_3d_properties(z_data)
    
    # Rotate camera for the 3D 'flying' effect
    ax.view_init(elev=20, azim=frame * 0.5)
    return line,

# 5. Run Animation
print(f"Recording animation using central API: {os.path.join(project_root, 'momentum_api.py')}")
print("Generating 300 frames...")
ani = FuncAnimation(fig, update, frames=300, init_func=init, blit=True)

# Save the video
output_file = "chaos_evolution.mp4"
try:
    # Note: Saves in the directory where you run the command
    ani.save(output_file, fps=30, extra_args=['-vcodec', 'libx264'])
    print(f"Final video saved at: {os.path.abspath(output_file)}")
except Exception as e:
    print(f"Error saving video: {e}")
    plt.savefig("chaos_frame.png")

# 6. Cleanup C++ memory
momentum.Task_Destroy(task)