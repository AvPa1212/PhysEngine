import numpy as np
from scipy.linalg import expm

def validate_quantum_evolution():
    dt = 1.0 / 60.0
    # Example Hamiltonian (Match your test case in main.cpp)
    H = np.array([[1.0, 0.0, 0.0, 0.0],
                  [0.0, 2.0, 0.0, 0.0],
                  [0.0, 0.0, 0.0, 0.0],
                  [0.0, 0.0, 0.0, 0.0]], dtype=complex)
    
    psi_0 = np.array([1, 0, 0, 0], dtype=complex)
    
    # Exact Unitary Evolution: U = exp(-i * H * dt)
    U_exact = expm(-1j * H * dt)
    psi_dt = U_exact @ psi_0
    
    print("Exact Quantum State after 1 step (60Hz):")
    print(psi_dt)
    
    # Entropy calculation: -sum(p * log(p))
    probs = np.abs(psi_dt)**2
    entropy = -np.sum(probs[probs > 0] * np.log(probs[probs > 0]))
    print(f"Exact Entropy: {entropy:.6f}")

if __name__ == "__main__":
    validate_quantum_evolution()