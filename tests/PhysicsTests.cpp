#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>

// Test result tracking
struct TestResults {
    int total = 0;
    int passed = 0;
    int failed = 0;
    
    void recordPass(const std::string& testName) {
        passed++;
        total++;
        std::cout << "  ✓ [PASS] " << testName << std::endl;
    }
    
    void recordFail(const std::string& testName, const std::string& reason = "") {
        failed++;
        total++;
        std::cout << "  ✗ [FAIL] " << testName;
        if (!reason.empty()) {
            std::cout << " - " << reason;
        }
        std::cout << std::endl;
    }
    
    void printSummary() const {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "TEST SUMMARY" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        std::cout << "Total:  " << total << std::endl;
        std::cout << "Passed: " << std::string(5, ' ') << "\033[32m" << passed << "\033[0m" << std::endl;
        std::cout << "Failed: " << std::string(5, ' ') << (failed > 0 ? "\033[31m" : "") 
                  << failed << "\033[0m" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
};

void testRK4Stability(TestResults& results) {
    std::cout << "\n[TEST] RK4 Stability Test" << std::endl;
    
    try {
        Task task;
        task.position = { 0, 0 };
        task.velocity = { 10, 0 };
        task.mass = 1.0;

        // Run for 100 steps
        for (int i = 0; i < 100; ++i) {
            ClassicalEngine::integrateRK4(task);
            
            // Check for NaN during iteration
            if (std::isnan(task.position.x) || std::isnan(task.position.y) ||
                std::isnan(task.velocity.x) || std::isnan(task.velocity.y)) {
                results.recordFail(
                    "RK4 Stability",
                    "NaN detected at step " + std::to_string(i)
                );
                return;
            }
        }

        // Final validation: Assert no NaNs produced
        assert(!std::isnan(task.position.x));
        assert(!std::isnan(task.position.y));
        assert(!std::isnan(task.velocity.x));
        assert(!std::isnan(task.velocity.y));
        
        std::cout << "    Position: (" << std::fixed << std::setprecision(6) 
                  << task.position.x << ", " << task.position.y << ")" << std::endl;
        std::cout << "    Velocity: (" << task.velocity.x << ", " << task.velocity.y << ")" << std::endl;
        
        results.recordPass("RK4 Stability");

    } catch (const std::exception& e) {
        results.recordFail("RK4 Stability", std::string(e.what()));
    }
}

void testChaosDivergence(TestResults& results) {
    std::cout << "\n[TEST] Chaos Divergence Test" << std::endl;
    
    try {
        Task t1, t2;
        
        // Reset states to a known sensitive region
        t1.stressX = 1.0; t1.stressY = 1.0; t1.stressZ = 1.0;
        t2 = t1;
        
        // Apply tiny perturbation to trigger divergence
        t2.stressX += 0.00001;

        std::cout << "    Initial separation: 0.00001" << std::endl;
        
        // Increase steps to 3000 to allow exponential divergence to hit the threshold
        for (int i = 0; i < 3000; ++i) {
            ChaosEngine::update(t1);
            ChaosEngine::update(t2);
            
            // Periodically check for NaN
            if ((i + 1) % 1000 == 0) {
                double delta = std::abs(t1.stressX - t2.stressX);
                std::cout << "    Step " << (i + 1) << ": Delta = " 
                          << std::scientific << std::setprecision(6) << delta << std::endl;
                
                if (std::isnan(delta)) {
                    results.recordFail(
                        "Chaos Divergence",
                        "NaN detected at step " + std::to_string(i + 1)
                    );
                    return;
                }
            }
        }

        double finalDelta = std::abs(t1.stressX - t2.stressX);
        std::cout << "    Final delta: " << std::scientific << std::setprecision(6) 
                  << finalDelta << std::endl;
        
        // Validate divergence threshold
        if (finalDelta > 0.1) {
            std::cout << "    ✓ Divergence threshold exceeded (> 0.1)" << std::endl;
            results.recordPass("Chaos Divergence");
        } else {
            results.recordFail(
                "Chaos Divergence",
                "Final delta (" + std::to_string(finalDelta) + ") did not exceed 0.1"
            );
        }

    } catch (const std::exception& e) {
        results.recordFail("Chaos Divergence", std::string(e.what()));
    }
}

void testEntropyAccumulation(TestResults& results) {
    std::cout << "\n[TEST] Entropy Accumulation Test" << std::endl;
    
    try {
        Task task;
        task.stressX = 1.0;
        task.stressY = 1.0;
        task.stressZ = 1.0;
        task.entropy = 0.0;
        
        double initialEntropy = task.entropy;
        
        // Run chaos engine for 500 steps
        for (int i = 0; i < 500; ++i) {
            ChaosEngine::update(task);
        }
        
        double finalEntropy = task.entropy;
        std::cout << "    Initial entropy: " << initialEntropy << std::endl;
        std::cout << "    Final entropy:   " << finalEntropy << std::endl;
        std::cout << "    Entropy increase: " << (finalEntropy - initialEntropy) << std::endl;
        
        // Entropy should monotonically increase
        if (finalEntropy >= initialEntropy) {
            results.recordPass("Entropy Accumulation");
        } else {
            results.recordFail(
                "Entropy Accumulation",
                "Entropy decreased instead of accumulating"
            );
        }
        
    } catch (const std::exception& e) {
        results.recordFail("Entropy Accumulation", std::string(e.what()));
    }
}

int main() {
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "PhysEngine Physics Test Suite" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    TestResults results;
    
    // Run all tests
    testRK4Stability(results);
    testChaosDivergence(results);
    testEntropyAccumulation(results);
    
    // Print summary
    results.printSummary();
    
    // Exit with appropriate code
    if (results.failed == 0) {
        std::cout << "\n✅ All tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ Some tests failed!" << std::endl;
        return 1;
    }
}