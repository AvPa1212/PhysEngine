import numpy as np

def lorenz_deriv(state):
    x, y, z = state
    sigma, rho, beta = 10, 28, 8/3
    return np.array([sigma * (y - x), x * (rho - z) - y, x * y - beta * z])

def rk4_step(state, dt):
    k1 = lorenz_deriv(state)
    k2 = lorenz_deriv(state + 0.5 * dt * k1)
    k3 = lorenz_deriv(state + 0.5 * dt * k2)
    k4 = lorenz_deriv(state + dt * k3)
    return state + (dt / 6.0) * (k1 + 2*k2 + 2*k3 + k4)

state = np.array([1.0, 1.0, 1.0])
dt = 1/60

for _ in range(100):
    state = rk4_step(state, dt)

print(f"Ground Truth (RK4) at Frame 100:")
print(f"X: {state[0]:.6f}")
print(f"Y: {state[1]:.6f}")