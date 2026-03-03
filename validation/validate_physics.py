import numpy as np
from scipy.integrate import solve_ivp
import sys
from typing import Dict, Tuple

class PhysicsValidationResults:
    """Track physics validation results."""
    
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
        print("LORENZ CHAOS VALIDATION RESULTS")
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


def lorenz(t, state, sigma, rho, beta):
    """
    Lorenz attractor system (ground truth implementation)
    
    dx/dt = sigma * (y - x)
    dy/dt = x * (rho - z) - y
    dz/dt = x * y - beta * z
    """
    x, y, z = state
    return [
        sigma * (y - x),
        x * (rho - z) - y,
        x * y - beta * z
    ]


def validate_chaos() -> PhysicsValidationResults:
    """
    Generate high-precision ground truth for Lorenz attractor.
    
    Returns:
        PhysicsValidationResults with validation data
    """
    results = PhysicsValidationResults()
    
    try:
        print("="*60)
        print("Lorenz Attractor Ground Truth Validation")
        print("Using SciPy RK45 (Dormand-Prince) - High Precision")
        print("="*60)
        
        # Constants matching Config.hpp
        sigma = 10.0
        rho = 28.0
        beta = 8.0 / 3.0
        params = (sigma, rho, beta)
        
        # Initial state matching Task_SetStress(1.0, 1.0, 1.0)
        initial_state = [1.0, 1.0, 1.0]
        
        print(f"\n[PARAMETERS]")
        print(f"  σ (sigma): {sigma}")
        print(f"  ρ (rho):   {rho}")
        print(f"  β (beta):  {beta:.10f}")
        
        print(f"\n[INITIAL CONDITIONS]")
        print(f"  x₀ = {initial_state[0]:.1f}")
        print(f"  y₀ = {initial_state[1]:.1f}")
        print(f"  z₀ = {initial_state[2]:.1f}")
        
        # Simulation span
        t_span = (0, 10.0)
        t_eval = np.linspace(0, 10.0, 1001)  # 1000 steps + initial
        
        print(f"\n[INTEGRATION SETUP]")
        print(f"  Time span: {t_span[0]:.1f}s to {t_span[1]:.1f}s")
        print(f"  Number of evaluation points: {len(t_eval)}")
        print(f"  Method: SciPy RK45 (Adaptive Runge-Kutta)")
        print(f"  Relative tolerance: 1e-8")
        print(f"  Absolute tolerance: 1e-10")
        
        print(f"\n[COMPUTING GROUND TRUTH]", end="")
        sys.stdout.flush()
        
        # Solve using high-precision RK45
        sol = solve_ivp(
            lorenz,
            t_span,
            initial_state,
            args=params,
            t_eval=t_eval,
            method='RK45',
            rtol=1e-8,
            atol=1e-10,
            dense_output=True
        )
        
        print(" ✓")
        
        # Check integration success
        if not sol.status == 0:
            results.add_error(f"Integration failed: {sol.message}")
            return results
        
        print(f"\n[INTEGRATION SUCCESS]")
        print(f"  Status: Completed successfully")
        print(f"  Number of steps: {len(sol.t)}")
        print(f"  Dense output available: Yes")
        
        # Extract final state
        fx, fy, fz = sol.y[0][-1], sol.y[1][-1], sol.y[2][-1]
        
        # Calculate trajectory statistics
        min_x, max_x = sol.y[0].min(), sol.y[0].max()
        min_y, max_y = sol.y[1].min(), sol.y[1].max()
        min_z, max_z = sol.y[2].min(), sol.y[2].max()
        
        print(f"\n[TRAJECTORY STATISTICS]")
        print(f"  X Range: [{min_x:10.6f}, {max_x:10.6f}]")
        print(f"  Y Range: [{min_y:10.6f}, {max_y:10.6f}]")
        print(f"  Z Range: [{min_z:10.6f}, {max_z:10.6f}]")
        
        # Calculate state magnitude
        state_magnitude = np.sqrt(fx**2 + fy**2 + fz**2)
        print(f"  Final State Magnitude: {state_magnitude:.6f}")
        
        print(f"\n[FINAL STATE AT t=10.0s]")
        print(f"  X: {fx:16.10f}")
        print(f"  Y: {fy:16.10f}")
        print(f"  Z: {fz:16.10f}")
        
        # Store ground truth
        results.ground_truth = {
            'x': fx,
            'y': fy,
            'z': fz,
            'time': 10.0,
            'steps': 1000,
            'solution': sol
        }
        
        # Validation details
        print(f"\n[COMPARISON TOLERANCE]")
        print(f"  Acceptable error: ±0.000001 (6 decimal places)")
        print(f"  Critical error: >0.01 (indicates algorithmic mismatch)")
        
        print(f"\n[VALIDATION PROCEDURE]")
        print(f"  1. Run C++ engine with:")
        print(f"     Task_SetStress(1.0, 1.0, 1.0)")
        print(f"     Config::TIME_STEP = {1.0/60.0:.8f}")
        print(f"     1000 iterations of Engine_UpdateChaos()")
        print(f"  2. Compare final [x, y, z] with values above")
        print(f"  3. Run 'test_divergence.py' for detailed comparison")
        
        results.passed = True
    
    except ImportError as e:
        results.add_error(f"Missing dependency: {e}\nInstall scipy: pip install scipy")
    except Exception as e:
        results.add_error(f"Validation failed: {e}")
    
    return results


def main():
    """Main entry point."""
    try:
        results = validate_chaos()
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