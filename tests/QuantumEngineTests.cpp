#include <gtest/gtest.h>
#include "physics/QuantumEngine.hpp"
#include "physics/Task.hpp"
#include "core/Config.hpp"
#include <cmath>
#include <array>

// Helper: sum of |psi_i|^2
static double normSquared(const Task& task) {
    double sum = 0.0;
    for (const auto& amp : task.psi) {
        sum += amp.magnitudeSquared();
    }
    return sum;
}

// ---- normalize() ----

TEST(QuantumEngineTest, NormalizeAlreadyNormalizedStateIsUnchanged) {
    Task task; // default psi = [{1,0},{0,0},{0,0},{0,0}]  →  norm^2 = 1
    QuantumEngine::normalize(task);
    EXPECT_NEAR(normSquared(task), 1.0, 1e-12);
    EXPECT_NEAR(task.psi[0].real, 1.0, 1e-12);
    EXPECT_NEAR(task.psi[0].imag, 0.0, 1e-12);
}

TEST(QuantumEngineTest, NormalizeUnnormalizedState) {
    Task task;
    task.psi[0] = {2.0, 0.0}; // norm^2 = 4 before normalization
    task.psi[1] = {0.0, 0.0};
    task.psi[2] = {0.0, 0.0};
    task.psi[3] = {0.0, 0.0};

    QuantumEngine::normalize(task);

    EXPECT_NEAR(normSquared(task), 1.0, 1e-12);
    EXPECT_NEAR(task.psi[0].real, 1.0, 1e-12);
}

TEST(QuantumEngineTest, NormalizeUniformSuperpositionStaysNormalized) {
    Task task;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = {0.5, 0.0}; // 4 * 0.25 = 1  →  already normalised
    }
    QuantumEngine::normalize(task);
    EXPECT_NEAR(normSquared(task), 1.0, 1e-12);
}

TEST(QuantumEngineTest, NormalizeZeroStateDoesNotCrash) {
    Task task;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = {0.0, 0.0};
    }
    EXPECT_NO_THROW(QuantumEngine::normalize(task));
}

TEST(QuantumEngineTest, NormalizeScalesImaginaryComponents) {
    Task task;
    task.psi[0] = {0.0, 2.0}; // norm^2 = 4
    task.psi[1] = {0.0, 0.0};
    task.psi[2] = {0.0, 0.0};
    task.psi[3] = {0.0, 0.0};

    QuantumEngine::normalize(task);

    EXPECT_NEAR(normSquared(task), 1.0, 1e-12);
    EXPECT_NEAR(task.psi[0].imag, 1.0, 1e-12);
    EXPECT_NEAR(task.psi[0].real, 0.0, 1e-12);
}

// ---- evolve() ----

TEST(QuantumEngineTest, EvolvePreservesNormalization) {
    Task task; // zero Hamiltonian  →  U = I
    QuantumEngine::evolve(task);
    EXPECT_NEAR(normSquared(task), 1.0, 1e-10);
}

TEST(QuantumEngineTest, EvolveWithZeroHamiltonianPreservesState) {
    // H = 0  →  U = I - 0 - 0 = I  →  state unchanged
    Task task; // default Hamiltonian is the zero matrix
    const double psi0_real = task.psi[0].real;

    QuantumEngine::evolve(task);

    EXPECT_NEAR(task.psi[0].real, psi0_real, 1e-10);
    EXPECT_NEAR(normSquared(task), 1.0, 1e-10);
}

TEST(QuantumEngineTest, EvolveWithNonZeroHamiltonianChangesState) {
    Task task;
    // Real diagonal Hamiltonian
    task.hamiltonian.data[0][0] = {1.0, 0.0};
    task.hamiltonian.data[1][1] = {2.0, 0.0};
    task.hamiltonian.data[2][2] = {3.0, 0.0};
    task.hamiltonian.data[3][3] = {4.0, 0.0};

    // Uniform superposition
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = {0.5, 0.0};
    }
    const double psi0_real_before = task.psi[0].real;

    QuantumEngine::evolve(task);

    // At least one amplitude should have changed
    bool changed = false;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        if (std::abs(task.psi[i].real - psi0_real_before) > 1e-10 ||
            std::abs(task.psi[i].imag) > 1e-10) {
            changed = true;
            break;
        }
    }
    EXPECT_TRUE(changed);
    EXPECT_NEAR(normSquared(task), 1.0, 1e-10);
}

TEST(QuantumEngineTest, EvolveRepeatedlyRemainsNormalized) {
    Task task;
    // Off-diagonal coupling induces mixing
    task.hamiltonian.data[0][1] = {0.5, 0.0};
    task.hamiltonian.data[1][0] = {0.5, 0.0};

    for (int step = 0; step < 100; ++step) {
        QuantumEngine::evolve(task);
        EXPECT_NEAR(normSquared(task), 1.0, 1e-8)
            << "Normalization failed at step " << step;
    }
}

// ---- calculateCollapseProbability() ----

TEST(QuantumEngineTest, CollapseProbabilityZeroForZeroEntropy) {
    Task task;
    task.entropy = 0.0;
    EXPECT_NEAR(QuantumEngine::calculateCollapseProbability(task), 0.0, 1e-12);
}

TEST(QuantumEngineTest, CollapseProbabilityOneForMaxEntropy) {
    Task task;
    const double maxEntropy =
        std::log(static_cast<double>(Config::QUANTUM_DIM)) * Config::ENTROPY_CONSTANT;
    task.entropy = maxEntropy;
    EXPECT_NEAR(QuantumEngine::calculateCollapseProbability(task), 1.0, 1e-6);
}

TEST(QuantumEngineTest, CollapseProbabilityClampedAtZeroForNegativeEntropy) {
    Task task;
    task.entropy = -1.0;
    EXPECT_NEAR(QuantumEngine::calculateCollapseProbability(task), 0.0, 1e-12);
}

TEST(QuantumEngineTest, CollapseProbabilityClampedAtOneForExcessEntropy) {
    Task task;
    task.entropy = 100.0;
    EXPECT_NEAR(QuantumEngine::calculateCollapseProbability(task), 1.0, 1e-12);
}

TEST(QuantumEngineTest, CollapseProbabilityIsProportionalToEntropy) {
    Task task1, task2;
    task1.entropy = 0.5;
    task2.entropy = 1.0;
    EXPECT_GT(QuantumEngine::calculateCollapseProbability(task2),
              QuantumEngine::calculateCollapseProbability(task1));
}

TEST(QuantumEngineTest, CollapseProbabilityIsBetweenZeroAndOne) {
    Task task;
    task.entropy = 0.69; // log(2) — half-way mixed state
    double prob = QuantumEngine::calculateCollapseProbability(task);
    EXPECT_GE(prob, 0.0);
    EXPECT_LE(prob, 1.0);
}

// ---- collapse() ----

TEST(QuantumEngineTest, CollapseProducesNormalizedPureState) {
    Task task;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = {0.5, 0.0};
    }
    QuantumEngine::collapse(task);
    EXPECT_NEAR(normSquared(task), 1.0, 1e-12);
}

TEST(QuantumEngineTest, CollapseProducesExactlyOneNonZeroComponent) {
    Task task;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = {0.5, 0.0};
    }
    QuantumEngine::collapse(task);

    int nonZeroCount = 0;
    for (const auto& amp : task.psi) {
        if (amp.magnitudeSquared() > 1e-12) {
            nonZeroCount++;
        }
    }
    EXPECT_EQ(nonZeroCount, 1);
}

TEST(QuantumEngineTest, CollapseSelectsHighestProbabilityState) {
    Task task;
    task.psi[0] = {0.1, 0.0};
    task.psi[1] = {0.2, 0.0};
    task.psi[2] = {0.9, 0.0}; // highest probability
    task.psi[3] = {0.1, 0.0};

    QuantumEngine::collapse(task);

    // Index 2 should be the surviving basis state
    EXPECT_NEAR(task.psi[2].real, 1.0, 1e-12);
    EXPECT_NEAR(task.psi[2].imag, 0.0, 1e-12);
    EXPECT_NEAR(task.psi[0].magnitudeSquared(), 0.0, 1e-12);
    EXPECT_NEAR(task.psi[1].magnitudeSquared(), 0.0, 1e-12);
    EXPECT_NEAR(task.psi[3].magnitudeSquared(), 0.0, 1e-12);
}

TEST(QuantumEngineTest, CollapseResetsEntropyToZero) {
    Task task;
    task.entropy = 1.0;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = {0.5, 0.0};
    }
    QuantumEngine::collapse(task);
    EXPECT_DOUBLE_EQ(task.entropy, 0.0);
}

TEST(QuantumEngineTest, CollapseDefaultStateSurvivesAsFirstBasisState) {
    Task task; // default psi = [{1,0},{0,0},{0,0},{0,0}]
    QuantumEngine::collapse(task);

    EXPECT_NEAR(task.psi[0].real, 1.0, 1e-12);
    EXPECT_NEAR(task.psi[0].imag, 0.0, 1e-12);
    EXPECT_DOUBLE_EQ(task.entropy, 0.0);
}

TEST(QuantumEngineTest, CollapseNonZeroEntropyAfterCollapseIsZero) {
    Task task;
    // Set a mixed state and compute entropy
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = {0.5, 0.0};
    }
    task.entropy = 1.386; // approximately max entropy

    QuantumEngine::collapse(task);

    EXPECT_DOUBLE_EQ(task.entropy, 0.0);
}
