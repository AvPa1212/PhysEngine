import numpy as np
import sys
from typing import Tuple

class ValidationResults:
    """Track and report validation results with detailed logging."""
    
    def __init__(self):
        self.passed = False
        self.reference_values = None
        self.errors = []
    
    def add_error(self, msg: str):
        self.errors.append(msg)
    
    def print_summary(self):
        print("\n" + "="*60)
        print("RK4 CORE ALGORITHM VERIFICATION RESULTS")
        print("="*60)
        if self.passed:
            print("✅ PASSED - Results suitable for C++ comparison")
        else:
            print("❌ FAILED - See errors below:")
            for error in self.errors:
                print(f"  - {error}")
        print("="*60)


def lorenz_deriv(state: np.ndarray) -> np.ndarray:
    """
    Lorenz system derivatives (matches ChaosEngine.hpp)
    
    Args:
        state: [x, y, z] position in phase space
        
    Returns:
        [dx/dt, dy/dt, dz/dt] derivatives
    """
    x, y, z = state
    sigma, rho, beta = 10.0, 28.0, 8/3.0
    
    try:
        derivatives = np.array([
            sigma * (y - x),
            x * (rho - z) - y,
            x * y - beta * z
        ])
        
        # Validation: Check for NaN
        if np.any(np.isnan(derivatives)):
            raise ValueError(f"NaN in derivatives at state {state}")
        
        return derivatives
    
    except Exception as e:
        print(f"❌ Error computing derivatives: {e}", file=sys.stderr)
        raise


def rk4_step(state: np.ndarray, dt: float) -> np.ndarray:
    """
    Standard 4th Order Runge-Kutta Step
    
    Args:
        state: Current [x, y, z] position
        dt: Time step
        
    Returns:
        Updated [x, y, z] position
    """
    try:
        k1 = lorenz_deriv(state)
        k2 = lorenz_deriv(state + 0.5 * dt * k1)
        k3 = lorenz_deriv(state + 0.5 * dt * k2)
        k4 = lorenz_deriv(state + dt * k3)
        
        new_state = state + (dt / 6.0) * (k1 + 2*k2 + 2*k3 + k4)
        
        # Validation: Check for NaN in result
        if np.any(np.isnan(new_state)):
            raise ValueError(f"NaN in RK4 step: {new_state}")
        
        return new_state
    
    except Exception as e:
        print(f"❌ Error in RK4 step: {e}", file=sys.stderr)
        raise


def run_verification() -> ValidationResults:
    """
    Execute RK4 reference implementation and generate ground truth.
    
    Returns:
        ValidationResults object with reference values
    """
    results = ValidationResults()
    
    try:
        print("="*60)
        print("PhysEngine RK4 Core Algorithm Verification")
        print("="*60)
        
        # 1. Setup Initial Conditions (Must match Task_SetStress(1,1,1))
        state = np.array([1.0, 1.0, 1.0])
        dt = 1.0 / 60.0  # 60Hz matching Config.hpp
        
        print(f"\n[SETUP]")
        print(f"  Initial State: x={state[0]:.1f}, y={state[1]:.1f}, z={state[2]:.1f}")
        print(f"  Time Step (dt): {dt:.8f}s")
        print(f"  Integration Steps: 100")
        print(f"  Total Time: {100 * dt:.6f}s")
        
        # Lorenz parameters
        print(f"\n[LORENZ PARAMETERS]")
        print(f"  σ (sigma): 10.0")
        print(f"  ρ (rho):   28.0")
        print(f"  β (beta):  {8/3:.6f}")
        
        # 2. Execute Integration with progress tracking
        print(f"\n[INTEGRATION PROGRESS]")
        state_history = [state.copy()]
        
        for i in range(100):
            state = rk4_step(state, dt)
            state_history.append(state.copy())
            
            # Periodic logging
            if (i + 1) % 25 == 0:
                print(f"  Step {i+1:3d}/100: x={state[0]:12.8f}, y={state[1]:12.8f}, z={state[2]:12.8f}")
            
            # Sanity check for divergence
            state_magnitude = np.linalg.norm(state)
            if state_magnitude > 1e4:
                results.add_error(
                    f"State diverged to infinity at step {i+1}: "
                    f"magnitude = {state_magnitude:.6e}"
                )
                raise ValueError("State divergence detected")
        
        # 3. Output Results with detailed statistics
        print(f"\n[FINAL RESULTS]")
        print(f"  X: {state[0]:16.10f}")
        print(f"  Y: {state[1]:16.10f}")
        print(f"  Z: {state[2]:16.10f}")
        
        # Calculate trajectory statistics
        state_array = np.array(state_history)
        print(f"\n[TRAJECTORY STATISTICS]")
        print(f"  X Range: [{state_array[:, 0].min():.6f}, {state_array[:, 0].max():.6f}]")
        print(f"  Y Range: [{state_array[:, 1].min():.6f}, {state_array[:, 1].max():.6f}]")
        print(f"  Z Range: [{state_array[:, 2].min():.6f}, {state_array[:, 2].max():.6f}]")
        
        # Calculate velocity (approximate)
        final_vel = (state_array[-1] - state_array[-2]) / dt
        print(f"  Final Velocity: [{final_vel[0]:8.4f}, {final_vel[1]:8.4f}, {final_vel[2]:8.4f}]")
        
        # 4. Store reference values
        results.reference_values = {
            'x': state[0],
            'y': state[1],
            'z': state[2],
            'step': 100
        }
        results.passed = True
        
        # 5. Validation instructions
        print(f"\n[VALIDATION INSTRUCTIONS]")
        print(f"  1. Run your C++ MomentumCore with identical parameters:")
        print(f"     - Initial: Task_SetStress(1.0, 1.0, 1.0)")
        print(f"     - Steps: 100 iterations of Engine_UpdateChaos()")
        print(f"  2. Compare final values with results above")
        print(f"  3. Acceptable tolerance: ±0.000001 (6 decimal places)")
        print(f"  4. If C++ results differ significantly, check:")
        print(f"     - Config::TIME_STEP matches dt={dt:.8f}")
        print(f"     - Lorenz parameters in Config.hpp")
        print(f"     - RK4 implementation details")
        
    except Exception as e:
        print(f"\n❌ VERIFICATION FAILED: {e}", file=sys.stderr)
        results.passed = False
        return results
    
    return results


def main():
    """Main entry point."""
    try:
        results = run_verification()
        results.print_summary()
        
        # Exit with appropriate code
        sys.exit(0 if results.passed else 1)
    
    except KeyboardInterrupt:
        print("\n\n⚠️ Verification interrupted by user", file=sys.stderr)
        sys.exit(130)
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()