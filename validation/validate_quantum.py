import numpy as np
from scipy.linalg import expm
import sys
from typing import Dict, Tuple

class QuantumValidationResults:
    """Track quantum validation results."""
    
    def __init__(self):
        self.passed = False
        self.ground_truth = None
        self.errors = []
        self.warnings = []
    
    def add_error(self, msg: str):
        self.errors.append(msg)
    
    def add_warning(self, msg: str):
        self.warnings.append(msg)
    
    def print_summary(self):
        print("\n" + "="*60)
        print("QUANTUM EVOLUTION VALIDATION RESULTS")
        print("="*60)
        
        if self.passed:
            print("✅ PASSED - Ground truth generated successfully")
        else:
            print("❌ FAILED - Validation errors:")
            for error in self.errors:
                print(f"  - {error}")
        
        if self.warnings:
            print("\n⚠️ WARNINGS:")
            for warning in self.warnings:
                print(f"  - {warning}")
        
        print("="*60)


def complex_to_string(c: complex) -> str:
    """Format complex number for display."""
    real_str = f"{c.real:10.6f}"
    imag_str = f"{abs(c.imag):10.6f}"
    sign = "+" if c.imag >= 0 else "-"
    return f"{real_str} {sign} {imag_str}i"


def validate_quantum_evolution() -> QuantumValidationResults:
    """
    Generate exact ground truth for quantum state evolution.
    Uses Schrödinger equation with matrix exponential (exact solution).
    
    Returns:
        QuantumValidationResults with validation data
    """
    results = QuantumValidationResults()
    
    try:
        print("="*60)
        print("Quantum State Evolution Ground Truth")
        print("Using Exact Schrödinger Evolution (Matrix Exponential)")
        print("="*60)
        
        # Matching your engine's default dt (60Hz)
        dt = 1.0 / 60.0
        
        print(f"\n[SETUP]")
        print(f"  Time step (dt): {dt:.8f}s (60Hz)")
        print(f"  Hilbert space dimension: 4")
        
        # 1. Setup Hamiltonian (diagonal - represents energy levels)
        H = np.array([
            [1.0, 0.0, 0.0, 0.0],
            [0.0, 2.0, 0.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0, 0.0]
        ], dtype=complex)
        
        print(f"\n[HAMILTONIAN]")
        print(f"  Diagonal elements (energy levels):")
        for i, h_ii in enumerate(np.diag(H)):
            print(f"    E_{i} = {h_ii.real:.1f}")
        print(f"  Form: Diagonal (time-independent)")
        
        # 2. Initial Wavefunction (state |0⟩)
        psi_0 = np.array([1.0, 0.0, 0.0, 0.0], dtype=complex)
        
        print(f"\n[INITIAL STATE]")
        print(f"  |ψ(0)⟩ = |0⟩ (pure state)")
        for i, amp in enumerate(psi_0):
            print(f"  ψ_{i}(0) = {complex_to_string(amp)}")
        
        # Validate normalization
        norm_sq = np.abs(psi_0).sum()**2
        print(f"  ||ψ(0)||² = {norm_sq:.10f}")
        if abs(norm_sq - 1.0) > 1e-10:
            results.add_warning(f"Initial state not normalized: {norm_sq}")
        
        # 3. Exact Unitary Evolution using matrix exponential
        # Schrödinger: |ψ(t)⟩ = exp(-i * H * t / ℏ) |ψ(0)⟩
        # (with ℏ = 1 in natural units)
        print(f"\n[COMPUTING EVOLUTION]")
        print(f"  Using: |ψ(dt)⟩ = exp(-i·H·dt) |ψ(0)⟩", end="")
        sys.stdout.flush()
        
        U_exact = expm(-1j * H * dt)
        psi_dt = U_exact @ psi_0
        
        print(" ✓")
        
        # 4. Validate result normalization
        norm_sq_final = np.abs(psi_dt).sum()**2
        print(f"\n[NORMALIZATION CHECK]")
        print(f"  ||ψ(dt)||² = {norm_sq_final:.15f}")
        print(f"  Deviation from 1.0: {abs(norm_sq_final - 1.0):.2e}")
        
        if abs(norm_sq_final - 1.0) > 1e-12:
            results.add_warning(f"Final state normalization: {norm_sq_final}")
        
        # 5. Probability and Entropy Calculation
        probs = np.abs(psi_dt)**2
        
        # Shannon Entropy: S = -Σ P_i * ln(P_i)
        # (with convention 0*ln(0) = 0)
        prob_nonzero = probs[probs > 1e-15]
        if len(prob_nonzero) > 0:
            entropy = -np.sum(prob_nonzero * np.log(prob_nonzero))
        else:
            entropy = 0.0
        
        print(f"\n[EVOLVED STATE AT t=dt]")
        print(f"  Time: {dt:.8f}s")
        for i, (amp, prob) in enumerate(zip(psi_dt, probs)):
            print(f"  ψ_{i}: {complex_to_string(amp)}")
            print(f"       Probability: {prob:.10f}")
        
        # 6. Entropy Analysis
        print(f"\n[ENTROPY ANALYSIS]")
        print(f"  Shannon Entropy: S = -Σ P_i·ln(P_i)")
        print(f"  S = {entropy:.10f} nats")
        
        # Convert to bits
        entropy_bits = entropy / np.log(2)
        print(f"  S = {entropy_bits:.10f} bits")
        
        # Maximum entropy for 4-state system: ln(4)
        max_entropy = np.log(4)
        print(f"  Maximum S (maximally mixed): {max_entropy:.10f} nats")
        print(f"  S/S_max ratio: {entropy/max_entropy:.10f}")
        
        # 7. Purity Analysis
        rho = np.outer(psi_dt, np.conj(psi_dt))  # Pure state density matrix
        purity = np.trace(rho @ rho).real
        print(f"\n[PURITY ANALYSIS]")
        print(f"  Purity: Tr(ρ²) = {purity:.10f}")
        print(f"  Expected for pure state: 1.0")
        print(f"  Deviation: {abs(purity - 1.0):.2e}")
        
        # 8. Store ground truth
        results.ground_truth = {
            'psi': psi_dt,
            'probabilities': probs,
            'entropy': entropy,
            'purity': purity,
            'hamiltonian': H,
            'dt': dt
        }
        
        # 9. Validation instructions
        print(f"\n[VALIDATION PROCEDURE]")
        print(f"  1. Initialize C++ Task with:")
        print(f"     psi = [1+0i, 0+0i, 0+0i, 0+0i]")
        print(f"     hamiltonian.data[0][0] = (1.0, 0.0)")
        print(f"     hamiltonian.data[1][1] = (2.0, 0.0)")
        print(f"     hamiltonian.data[2][2] = (0.0, 0.0)")
        print(f"     hamiltonian.data[3][3] = (0.0, 0.0)")
        print(f"  2. Call QuantumEngine::evolve(task) once")
        print(f"  3. Compare Task.psi with expected values above")
        print(f"  4. Acceptable tolerance: ±0.000001 (6 decimal places)")
        print(f"  5. Check Task.entropy against theoretical value")
        
        results.passed = True
    
    except ImportError as e:
        results.add_error(f"Missing dependency: {e}\nInstall scipy: pip install scipy")
    except Exception as e:
        results.add_error(f"Validation failed: {e}")
    
    return results


def main():
    """Main entry point."""
    try:
        results = validate_quantum_evolution()
        results.print_summary()
        sys.exit(0 if results.passed else 1)
    
    except KeyboardInterrupt:
        print("\n\n⚠️ Validation interrupted by user", file=sys.stderr)
        sys.exit(130)
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()