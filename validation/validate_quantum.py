import numpy as np
from scipy.linalg import expm

def validate_quantum_evolution():
    print("--- Quantum State Ground Truth (Schrödinger Evolution) ---")
    
    # Matching your engine's default dt (60Hz)
    dt = 1.0 / 60.0
    
    # 1. Setup Hamiltonian (Matching your MomentumCore test case)
    # This represents the energy levels of your 4-state system
    H = np.array([[1.0, 0.0, 0.0, 0.0],
                  [0.0, 2.0, 0.0, 0.0],
                  [0.0, 0.0, 0.0, 0.0],
                  [0.0, 0.0, 0.0, 0.0]], dtype=complex)
    
    # 2. Initial Wavefunction (State 0 fully occupied)
    psi_0 = np.array([1, 0, 0, 0], dtype=complex)
    
    # 3. Exact Unitary Evolution
    # Schrödinger: |ψ(t)> = exp(-i * H * t) |ψ(0)>
    # Using expm (matrix exponential) for the high-precision truth
    U_exact = expm(-1j * H * dt)
    psi_dt = U_exact @ psi_0
    
    # 4. Probability and Entropy Calculation
    # Probability P_i = |ψ_i|^2
    probs = np.abs(psi_dt)**2
    # Shannon Entropy S = -Σ P_i * ln(P_i)
    entropy = -np.sum(probs[probs > 1e-12] * np.log(probs[probs > 1e-12]))
    
    print(f"Time Step (dt): {dt:.6f}s")
    print("-" * 45)
    print("Exact State Vector (ψ) after 1 step:")
    for i, val in enumerate(psi_dt):
        print(f"  State {i}: {val.real:10.6f} + {val.imag:10.6f}i")
    
    print("-" * 45)
    print(f"Exact Probability Distribution: {probs}")
    print(f"Exact Theoretical Entropy:      {entropy:.10f}")
    print("-" * 45)
    print("Compare this 'Theoretical Entropy' with Task_GetEntropy() result.")

if __name__ == "__main__":
    validate_quantum_evolution()