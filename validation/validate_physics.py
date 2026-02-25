import numpy as np
from scipy.integrate import solve_ivp

# Lorenz Attractor Ground Truth (Matches your ChaosEngine.hpp logic)
def lorenz(t, state, sigma, rho, beta):
    x, y, z = state
    # dx/dt = sigma * (y - x)
    # dy/dt = x * (rho - z) - y
    # dz/dt = x * y - beta * z
    return [sigma * (y - x), x * (rho - z) - y, x * y - beta * z]

def validate_chaos():
    print("--- Scientific Ground Truth Validation ---")
    
    # Constants matching your Config.hpp
    sigma = 10.0
    rho = 28.0
    beta = 8/3.0
    params = (sigma, rho, beta)
    
    # Initial state matching your Task_SetStress(1.0, 1.0, 1.0)
    initial_state = [1.0, 1.0, 1.0]
    
    # We want to simulate 10 seconds of time
    # In your C++ engine, if dt=0.01, that is 1000 steps.
    t_span = (0, 10.0)
    t_eval = np.linspace(0, 10.0, 1001) # Match 1000 steps + initial state

    # Solve using RK45 (Adaptive Dormand-Prince)
    sol = solve_ivp(
        lorenz, 
        t_span, 
        initial_state, 
        args=params, 
        t_eval=t_eval, 
        method='RK45', 
        rtol=1e-8, 
        atol=1e-10
    )
    
    print(f"Parameters: σ={sigma}, ρ={rho}, β={beta:.4f}")
    print(f"Integration Method: SciPy RK45 (High Precision)")
    print("-" * 40)
    
    # Extract final coordinates
    fx, fy, fz = sol.y[0][-1], sol.y[1][-1], sol.y[2][-1]
    
    print(f"Final State at t=10.0s (Step 1000):")
    print(f"X: {fx:>10.6f}")
    print(f"Y: {fy:>10.6f}")
    print(f"Z: {fz:>10.6f}")
    print("-" * 40)
    print("Use these values to verify your C++ MomentumCore output.")

if __name__ == "__main__":
    validate_chaos()