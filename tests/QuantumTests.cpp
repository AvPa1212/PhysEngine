#include "physics/QuantumEngine.hpp"
#include "physics/ThermodynamicsEngine.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <sstream>
#include <vector>

// Test result tracking
struct TestResults {
    int total = 0;
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failureDetails;
    
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
        failureDetails.push_back(testName + (reason.empty() ? "" : ": " + reason));
    }
    
    void printSummary() const {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "QUANTUM TEST SUMMARY" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        std::cout << "Total:  " << total << std::endl;
        std::cout << "Passed: " << std::string(5, ' ') << "\033[32m" << passed << "\033[0m" << std::endl;
        std::cout << "Failed: " << std::string(5, ' ') << (failed > 0 ? "\033[31m" : "") 
                  << failed << "\033[0m" << std::endl;
        
        if (failed > 0) {
            std::cout << "\nFailed Tests:" << std::endl;
            for (const auto& detail : failureDetails) {
                std::cout << "  - " << detail << std::endl;
            }
        }
        std::cout << std::string(60, '=') << std::endl;
    }
};

// Helper function to print complex number
std::string complexToString(const Complex& c) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6);
    if (c.imag >= 0) {
        ss << c.real << " + " << c.imag << "i";
    } else {
        ss << c.real << " - " << std::abs(c.imag) << "i";
    }
    return ss.str();
}

// Helper function to print quantum state
std::string stateToString(const std::array<Complex, Config::QUANTUM_DIM>& psi) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < psi.size(); ++i) {
        ss << complexToString(psi[i]);
        if (i < psi.size() - 1) ss << ", ";
    }
    ss << "]";
    return ss.str();
}

void testPsiNormalization(TestResults& results) {
    std::cout << "\n[TEST] Psi Normalization Test" << std::endl;
    std::cout << "    Purpose: Verify quantum state remains normalized after evolution" << std::endl;
    
    try {
        Task task;
        // Set a non-standard Hamiltonian
        task.hamiltonian.data[0][0] = { 0.0, 1.0 };
        
        std::cout << "    Initial state: " << stateToString(task.psi) << std::endl;

        double maxDeviation = 0.0;
        int worstStep = -1;
        
        for (int i = 0; i < 100; ++i) {
            QuantumEngine::evolve(task);

            // Calculate norm squared
            double normSq = 0.0;
            for (const auto& amp : task.psi) {
                normSq += amp.magnitudeSquared();
            }
            
            double deviation = std::abs(normSq - 1.0);
            
            // Track worst case
            if (deviation > maxDeviation) {
                maxDeviation = deviation;
                worstStep = i;
            }
            
            // Check for catastrophic deviation
            if (deviation > 1e-6) {
                results.recordFail(
                    "Psi Normalization",
                    "Norm squared = " + std::to_string(normSq) + 
                    " at step " + std::to_string(i) + 
                    " (deviation: " + std::to_string(deviation) + ")"
                );
                std::cout << "    State at failure: " << stateToString(task.psi) << std::endl;
                return;
            }
            
            // Periodic detailed logging
            if ((i + 1) % 25 == 0) {
                std::cout << "    Step " << std::setw(3) << (i + 1) 
                          << ": ||psi||² = " << std::fixed << std::setprecision(12) 
                          << normSq << " (deviation: " << std::scientific << deviation << ")" << std::endl;
            }
        }
        
        // Validate final state
        double finalNormSq = 0.0;
        for (const auto& amp : task.psi) {
            finalNormSq += amp.magnitudeSquared();
        }
        
        assert(std::abs(finalNormSq - 1.0) < 1e-9);
        
        std::cout << "    Final state: " << stateToString(task.psi) << std::endl;
        std::cout << "    Final ||psi||²: " << std::fixed << std::setprecision(12) << finalNormSq << std::endl;
        std::cout << "    Max deviation: " << std::scientific << maxDeviation 
                  << " at step " << worstStep << std::endl;
        
        results.recordPass("Psi Normalization");

    } catch (const std::exception& e) {
        results.recordFail("Psi Normalization", std::string(e.what()));
    }
}

void testEntropyBounds(TestResults& results) {
    std::cout << "\n[TEST] Entropy Bounds Test" << std::endl;
    std::cout << "    Purpose: Verify entropy bounds for quantum states" << std::endl;
    
    try {
        // Test 1: Pure state entropy should be zero
        std::cout << "    --- Subtest 1: Pure State (entropy should ≈ 0) ---" << std::endl;
        Task task1;
        task1.psi = { Complex{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0} };
        
        std::cout << "    Initial state: " << stateToString(task1.psi) << std::endl;
        ThermodynamicsEngine::updateEntropy(task1);
        
        std::cout << "    Entropy: " << std::fixed << std::setprecision(10) 
                  << task1.entropy << std::endl;
        
        if (std::abs(task1.entropy) > 1e-9) {
            results.recordFail(
                "Entropy Bounds (Pure State)",
                "Pure state entropy = " + std::to_string(task1.entropy) + 
                " (expected ≈ 0)"
            );
            return;
        }
        
        std::cout << "    ✓ Pure state entropy correctly near zero" << std::endl;

        // Test 2: Mixed state entropy should be higher
        std::cout << "    --- Subtest 2: Mixed State (entropy should > 0) ---" << std::endl;
        Task task2;
        for (auto& amp : task2.psi) amp = { 0.5, 0.0 };
        
        // Normalize the mixed state
        double norm = 0.0;
        for (const auto& amp : task2.psi) {
            norm += amp.magnitudeSquared();
        }
        norm = std::sqrt(norm);
        for (auto& amp : task2.psi) {
            amp.real /= norm;
            amp.imag /= norm;
        }
        
        std::cout << "    Initial state: " << stateToString(task2.psi) << std::endl;
        ThermodynamicsEngine::updateEntropy(task2);
        
        std::cout << "    Entropy: " << std::fixed << std::setprecision(10) 
                  << task2.entropy << std::endl;
        
        if (task2.entropy <= 0.0) {
            results.recordFail(
                "Entropy Bounds (Mixed State)",
                "Mixed state entropy = " + std::to_string(task2.entropy) + 
                " (expected > 0)"
            );
            return;
        }
        
        std::cout << "    ✓ Mixed state entropy correctly positive" << std::endl;

        // Test 3: Entropy should respect maximum bounds
        std::cout << "    --- Subtest 3: Maximum Entropy Bound ---" << std::endl;
        double maxEntropy = 1.38629436; // ln(4) for 4-dimensional system
        std::cout << "    Theoretical max entropy: " << std::fixed << std::setprecision(10) 
                  << maxEntropy << std::endl;
        std::cout << "    Actual entropy: " << task2.entropy << std::endl;
        
        if (task2.entropy > maxEntropy * 1.01) { // Allow 1% tolerance
            results.recordFail(
                "Entropy Bounds (Max Bound)",
                "Entropy " + std::to_string(task2.entropy) + 
                " exceeds theoretical max " + std::to_string(maxEntropy)
            );
            return;
        }
        
        std::cout << "    ✓ Entropy within theoretical bounds" << std::endl;
        
        results.recordPass("Entropy Bounds");

    } catch (const std::exception& e) {
        results.recordFail("Entropy Bounds", std::string(e.what()));
    }
}

void testQuantumCollapse(TestResults& results) {
    std::cout << "\n[TEST] Quantum Collapse Test" << std::endl;
    std::cout << "    Purpose: Verify quantum state collapse to pure state" << std::endl;
    
    try {
        Task task;
        // Create an arbitrary superposition
        task.psi[0] = { 0.8, 0.0 };
        task.psi[1] = { 0.6, 0.0 };
        task.psi[2] = { 0.0, 0.0 };
        task.psi[3] = { 0.0, 0.0 };
        
        // Normalize
        double norm = 0.0;
        for (const auto& amp : task.psi) {
            norm += amp.magnitudeSquared();
        }
        norm = std::sqrt(norm);
        for (auto& amp : task.psi) {
            amp.real /= norm;
            amp.imag /= norm;
        }
        
        std::cout << "    Pre-collapse state: " << stateToString(task.psi) << std::endl;
        
        // Calculate pre-collapse entropy
        ThermodynamicsEngine::updateEntropy(task);
        double preCollapseEntropy = task.entropy;
        std::cout << "    Pre-collapse entropy: " << std::fixed << std::setprecision(10) 
                  << preCollapseEntropy << std::endl;
        
        // Perform collapse
        QuantumEngine::collapse(task);
        
        std::cout << "    Post-collapse state: " << stateToString(task.psi) << std::endl;
        
        // Verify it's a pure state (only one non-zero amplitude)
        int nonZeroCount = 0;
        int maxIdx = -1;
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            if (std::abs(task.psi[i].real) > 1e-6 || std::abs(task.psi[i].imag) > 1e-6) {
                nonZeroCount++;
                maxIdx = i;
            }
        }
        
        if (nonZeroCount != 1) {
            results.recordFail(
                "Quantum Collapse",
                "Post-collapse state has " + std::to_string(nonZeroCount) + 
                " non-zero amplitudes (expected 1)"
            );
            return;
        }
        
        std::cout << "    ✓ Collapsed to basis state |" << maxIdx << ">" << std::endl;
        
        // Verify amplitude of pure state is ~1.0
        double maxAmp = task.psi[maxIdx].magnitudeSquared();
        std::cout << "    Collapsed state amplitude squared: " << std::fixed << std::setprecision(10) 
                  << maxAmp << std::endl;
        
        if (std::abs(maxAmp - 1.0) > 1e-6) {
            results.recordFail(
                "Quantum Collapse",
                "Collapsed amplitude = " + std::to_string(maxAmp) + 
                " (expected 1.0)"
            );
            return;
        }
        
        // Verify entropy decreased
        ThermodynamicsEngine::updateEntropy(task);
        double postCollapseEntropy = task.entropy;
        std::cout << "    Post-collapse entropy: " << std::fixed << std::setprecision(10) 
                  << postCollapseEntropy << std::endl;
        
        if (postCollapseEntropy > preCollapseEntropy + 1e-9) {
            results.recordFail(
                "Quantum Collapse",
                "Entropy increased after collapse: " + 
                std::to_string(preCollapseEntropy) + " → " + 
                std::to_string(postCollapseEntropy)
            );
            return;
        }
        
        std::cout << "    ✓ Entropy decreased as expected (collapse reduces uncertainty)" << std::endl;
        
        results.recordPass("Quantum Collapse");

    } catch (const std::exception& e) {
        results.recordFail("Quantum Collapse", std::string(e.what()));
    }
}

void testCollapseProbability(TestResults& results) {
    std::cout << "\n[TEST] Collapse Probability Calculation Test" << std::endl;
    std::cout << "    Purpose: Verify collapse probability scales with entropy" << std::endl;
    
    try {
        // Test 1: Low entropy → low collapse probability
        std::cout << "    --- Subtest 1: Low Entropy State ---" << std::endl;
        Task task1;
        task1.psi[0] = { 1.0, 0.0 }; // Pure state
        task1.psi[1] = { 0.0, 0.0 };
        task1.psi[2] = { 0.0, 0.0 };
        task1.psi[3] = { 0.0, 0.0 };
        task1.entropy = 0.0;
        
        double prob1 = QuantumEngine::calculateCollapseProbability(task1);
        std::cout << "    Entropy: " << std::fixed << std::setprecision(6) << task1.entropy << std::endl;
        std::cout << "    Collapse probability: " << std::fixed << std::setprecision(6) << prob1 << std::endl;
        
        if (prob1 > 0.01) {
            results.recordFail(
                "Collapse Probability (Low Entropy)",
                "Probability " + std::to_string(prob1) + 
                " too high for pure state (expected ≈ 0)"
            );
            return;
        }
        
        std::cout << "    ✓ Low entropy correctly maps to low probability" << std::endl;

        // Test 2: High entropy → high collapse probability
        std::cout << "    --- Subtest 2: High Entropy State ---" << std::endl;
        Task task2;
        task2.entropy = 1.38; // Near maximum
        
        double prob2 = QuantumEngine::calculateCollapseProbability(task2);
        std::cout << "    Entropy: " << std::fixed << std::setprecision(6) << task2.entropy << std::endl;
        std::cout << "    Collapse probability: " << std::fixed << std::setprecision(6) << prob2 << std::endl;
        
        if (prob2 < 0.99) {
            results.recordFail(
                "Collapse Probability (High Entropy)",
                "Probability " + std::to_string(prob2) + 
                " too low for high entropy state (expected ≈ 1.0)"
            );
            return;
        }
        
        std::cout << "    ✓ High entropy correctly maps to high probability" << std::endl;

        // Test 3: Probability is clamped to [0, 1]
        std::cout << "    --- Subtest 3: Probability Bounds ---" << std::endl;
        Task task3;
        task3.entropy = 10.0; // Unreasonably high
        
        double prob3 = QuantumEngine::calculateCollapseProbability(task3);
        std::cout << "    Entropy (invalid): " << std::fixed << std::setprecision(6) << task3.entropy << std::endl;
        std::cout << "    Clamped probability: " << std::fixed << std::setprecision(6) << prob3 << std::endl;
        
        if (prob3 < 0.0 || prob3 > 1.0) {
            results.recordFail(
                "Collapse Probability (Bounds)",
                "Probability " + std::to_string(prob3) + 
                " out of valid range [0, 1]"
            );
            return;
        }
        
        std::cout << "    ✓ Probability correctly clamped to [0, 1]" << std::endl;
        
        results.recordPass("Collapse Probability");

    } catch (const std::exception& e) {
        results.recordFail("Collapse Probability", std::string(e.what()));
    }
}

int main() {
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "PhysEngine Quantum Test Suite" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    TestResults results;
    
    // Run all quantum tests
    testPsiNormalization(results);
    testEntropyBounds(results);
    testQuantumCollapse(results);
    testCollapseProbability(results);
    
    // Print summary
    results.printSummary();
    
    // Exit with appropriate code
    if (results.failed == 0) {
        std::cout << "\n✅ All quantum tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ Some quantum tests failed!" << std::endl;
        return 1;
    }
}