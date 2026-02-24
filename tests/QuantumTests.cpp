#include "physics/QuantumEngine.hpp"
#include "physics/ThermodynamicsEngine.hpp"
#include <iostream>
#include <cassert>

void testPsiNormalization() {
    Task task;
    // Set a non-standard Hamiltonian
    task.hamiltonian.data[0][0] = { 0.0, 1.0 };

    for (int i = 0; i < 100; ++i) {
        QuantumEngine::evolve(task);

        double normSq = 0.0;
        for (const auto& amp : task.psi) {
            normSq += amp.magnitudeSquared();
        }
        // Tolerance for floating point precision
        assert(std::abs(normSq - 1.0) < 1e-9);
    }
    std::cout << "[PASS] Psi Normalization Test" << std::endl;
}

void testEntropyBounds() {
    Task task;
    // Pure state should have zero entropy
    task.psi = { Complex{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0} };
    ThermodynamicsEngine::updateEntropy(task);
    assert(std::abs(task.entropy) < 1e-9);

    // Max mixed-ish state should have higher entropy
    for (auto& amp : task.psi) amp = { 0.5, 0.0 };
    ThermodynamicsEngine::updateEntropy(task);
    assert(task.entropy > 0.0);

    std::cout << "[PASS] Entropy Bounds Test" << std::endl;
}

int main() {
    testPsiNormalization();
    testEntropyBounds();
    return 0;
}