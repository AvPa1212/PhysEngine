import numpy as np
from scipy.integrate import solve_ivp

# Lorenz Attractor Ground Truth (Matches ChaosEngine.hpp)
def lorenz(t, state, sigma, rho, beta):
    x, y, z = state
    return [sigma * (y - x), x * (rho - z) - y, x * y - beta * z]

def validate_chaos():
    # Matches Config.hpp constants
    params = (10.0, 28.0, 8/3)
    initial_state = [1.0, 1.0, 1.0]
    t_span = (0, 10)
    t_eval = np.linspace(0, 10, 600) # 60Hz matching

    sol = solve_ivp(lorenz, t_span, initial_state, args=params, t_eval=t_eval, method='RK45')
    
    print(f"Final Ground Truth Chaos State (t=10s):")
    print(f"X: {sol.y[0][-1]:.4f}, Y: {sol.y[1][-1]:.4f}, Z: {sol.y[2][-1]:.4f}")

if __name__ == "__main__":
    validate_chaos()