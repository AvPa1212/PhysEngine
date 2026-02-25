import numpy as np

def lorenz_deriv(state):
    """Lorenz system derivatives (matches ChaosEngine.hpp)"""
    x, y, z = state
    sigma, rho, beta = 10.0, 28.0, 8/3.0
    return np.array([
        sigma * (y - x),
        x * (rho - z) - y,
        x * y - beta * z
    ])

def rk4_step(state, dt):
    """Standard 4th Order Runge-Kutta Step"""
    k1 = lorenz_deriv(state)
    k2 = lorenz_deriv(state + 0.5 * dt * k1)
    k3 = lorenz_deriv(state + 0.5 * dt * k2)
    k4 = lorenz_deriv(state + dt * k3)
    return state + (dt / 6.0) * (k1 + 2*k2 + 2*k3 + k4)

def run_verification():
    print("--- Core Algorithm Verification (RK4 Python Reference) ---")
    
    # 1. Setup Initial Conditions (Must match Task_SetStress(1,1,1))
    state = np.array([1.0, 1.0, 1.0])
    dt = 1.0 / 60.0  # 60Hz matching Config.hpp
    
    print(f"Integrating 100 steps with dt={dt:.6f}...")

    # 2. Execute Integration
    for i in range(100):
        state = rk4_step(state, dt)

    # 3. Output Results
    print("-" * 45)
    print(f"Python Reference Results at Step 100:")
    print(f"  X: {state[0]:12.8f}")
    print(f"  Y: {state[1]:12.8f}")
    print(f"  Z: {state[2]:12.8f}")
    print("-" * 45)
    print("ACTION: Run 'test_bridge.py' and compare these values.")
    print("They should match your C++ engine up to the 6th decimal place.")

if __name__ == "__main__":
    run_verification()