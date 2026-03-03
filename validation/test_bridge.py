import sys
import os
from typing import Dict

class BridgeTestResults:
    """Track bridge test results."""
    
    def __init__(self):
        self.passed = False
        self.errors = []
        self.warnings = []
        self.measurements = {}
    
    def add_error(self, msg: str):
        self.errors.append(msg)
    
    def add_warning(self, msg: str):
        self.warnings.append(msg)
    
    def add_measurement(self, name: str, value):
        self.measurements[name] = value
    
    def print_summary(self):
        print("\n" + "="*60)
        print("BRIDGE TEST RESULTS")
        print("="*60)
        
        if self.passed:
            print("✅ PASSED - C++/Python bridge functional")
            print("\nMeasurements:")
            for name, value in self.measurements.items():
                print(f"  {name}: {value}")
        else:
            print("❌ FAILED - Bridge errors detected:")
            for error in self.errors:
                print(f"  - {error}")
        
        if self.warnings:
            print("\n⚠️ WARNINGS:")
            for warning in self.warnings:
                print(f"  - {warning}")
        
        print("="*60)


def setup_momentum_api():
    """
    Setup Python path and load momentum API.
    
    Returns:
        Loaded momentum library module
    """
    # Setup Path to find momentum_api in the project root
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    sys.path.insert(0, project_root)
    
    try:
        from momentum_api import safe_get_momentum_lib
        print("[SETUP] Attempting to load momentum library...")
        momentum = safe_get_momentum_lib()
        print("[SETUP] ✓ Library loaded successfully")
        return momentum
    
    except ImportError as e:
        print(f"[ERROR] Could not find momentum_api.py", file=sys.stderr)
        print(f"  Expected location: {os.path.join(project_root, 'momentum_api.py')}", 
              file=sys.stderr)
        raise RuntimeError(f"momentum_api import failed: {e}")
    
    except RuntimeError as e:
        print(f"[ERROR] Library initialization failed: {e}", file=sys.stderr)
        raise


def test_basic_bridge(momentum, results: BridgeTestResults) -> bool:
    """Test basic C++/Python bridge functionality."""
    
    print("\n" + "="*60)
    print("Bridge Verification Test")
    print("="*60)
    
    try:
        print("\n[TEST 1] Task Lifecycle (Create/Destroy)")
        
        # Create task
        task = momentum.Task_Create()
        if task is None:
            results.add_error("Task_Create() returned None")
            return False
        
        print("  ✓ Task created successfully")
        print(f"  Task pointer: {hex(task) if isinstance(task, int) else task}")
        
        # Test initial state
        initial_x = momentum.Task_GetPositionX(task)
        print(f"  Initial X position: {initial_x:.2f}")
        results.add_measurement("initial_x", initial_x)
        
        # Set position
        print("\n[TEST 2] Position Setters/Getters")
        test_x, test_y = 5.0, 3.0
        momentum.Task_SetPosition(task, test_x, test_y)
        
        retrieved_x = momentum.Task_GetPositionX(task)
        if abs(retrieved_x - test_x) < 1e-6:
            print(f"  ✓ Position X: set={test_x}, got={retrieved_x}")
        else:
            results.add_warning(f"Position X mismatch: set={test_x}, got={retrieved_x}")
        
        # Test RK4 integration
        print("\n[TEST 3] Classical Physics Integration (RK4)")
        print("  Executing 100 iterations of Engine_IntegrateClassical()...")
        
        for i in range(100):
            momentum.Engine_IntegrateClassical(task)
            if (i + 1) % 25 == 0:
                current_x = momentum.Task_GetPositionX(task)
                print(f"    Step {i+1:3d}/100: x={current_x:10.6f}")
        
        final_x = momentum.Task_GetPositionX(task)
        displacement = final_x - test_x
        
        print(f"\n  Initial X: {test_x:.6f}")
        print(f"  Final X:   {final_x:.6f}")
        print(f"  Displacement: {displacement:.6f}")
        
        results.add_measurement("final_x", final_x)
        results.add_measurement("displacement", displacement)
        
        if abs(displacement) < 1e-6:
            results.add_warning("No displacement detected - check velocity/dt in engine")
        else:
            print(f"  ✓ State successfully mutated")
        
        # Test chaos engine
        print("\n[TEST 4] Chaos Engine (Lorenz)")
        
        momentum.Task_SetStress(task, 1.0, 1.0, 1.0)
        print("  Initial stress: (1.0, 1.0, 1.0)")
        
        for i in range(50):
            momentum.Engine_UpdateChaos(task)
        
        stress_x = momentum.Task_GetStressX(task)
        stress_y = momentum.Task_GetStressY(task)
        stress_z = momentum.Task_GetStressZ(task)
        
        print(f"  After 50 steps:")
        print(f"    Stress X: {stress_x:.6f}")
        print(f"    Stress Y: {stress_y:.6f}")
        print(f"    Stress Z: {stress_z:.6f}")
        
        results.add_measurement("stress_x", stress_x)
        results.add_measurement("stress_y", stress_y)
        results.add_measurement("stress_z", stress_z)
        
        # Check for chaos divergence
        stress_magnitude = (stress_x**2 + stress_y**2 + stress_z**2)**0.5
        if stress_magnitude > 1.0:
            print(f"  ✓ Chaotic behavior detected (magnitude={stress_magnitude:.4f})")
        else:
            results.add_warning(f"Magnitude unexpectedly small: {stress_magnitude:.4f}")
        
        # Test entropy
        print("\n[TEST 5] Entropy Retrieval")
        entropy = momentum.Task_GetEntropy(task)
        print(f"  Current entropy: {entropy:.6f}")
        results.add_measurement("entropy", entropy)
        
        if entropy < 0:
            results.add_error(f"Negative entropy detected: {entropy}")
            return False
        
        # Cleanup
        print("\n[TEST 6] Memory Cleanup")
        momentum.Task_Destroy(task)
        print("  ✓ Task destroyed, memory released")
        
        results.passed = True
        return True
    
    except Exception as e:
        results.add_error(f"Test failed: {e}")
        print(f"\n[ERROR] {e}", file=sys.stderr)
        return False


def main():
    """Main entry point."""
    results = BridgeTestResults()
    
    try:
        momentum = setup_momentum_api()
        test_basic_bridge(momentum, results)
        
    except RuntimeError as e:
        results.add_error(str(e))
    except KeyboardInterrupt:
        print("\n\n⚠️ Test interrupted by user", file=sys.stderr)
        sys.exit(130)
    except Exception as e:
        results.add_error(f"Unexpected error: {e}")
        print(f"\n[ERROR] {e}", file=sys.stderr)
    
    results.print_summary()
    sys.exit(0 if results.passed else 1)


if __name__ == "__main__":
    main()