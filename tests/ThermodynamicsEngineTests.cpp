#include <gtest/gtest.h>
#include "physics/ThermodynamicsEngine.hpp"
#include "physics/Task.hpp"
#include "core/Config.hpp"
#include <cmath>

// Helper: task with all psi components equal to amp (uniform distribution)
static Task makeUniformTask() {
    Task task;
    const double amp = 0.5; // magnitudeSquared = 0.25 each; 4 * 0.25 = 1 (normalised)
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = { amp, 0.0 };
    }
    return task;
}

// ---- Pure state: entropy = 0 ----

TEST(ThermodynamicsEngineTest, PureStateHasZeroEntropy) {
    // psi = [1, 0, 0, 0]: S = -(1 * log(1)) = 0
    Task task;
    task.psi[0] = {1.0, 0.0};
    task.psi[1] = {0.0, 0.0};
    task.psi[2] = {0.0, 0.0};
    task.psi[3] = {0.0, 0.0};
    task.entropy = 99.0; // ensure the field is overwritten

    ThermodynamicsEngine::updateEntropy(task);

    EXPECT_NEAR(task.entropy, 0.0, 1e-12);
}

// ---- Uniform distribution: maximum entropy ----

TEST(ThermodynamicsEngineTest, UniformDistributionHasMaximumEntropy) {
    // p_i = 0.25 for each of 4 states  →  S = -4*(0.25*log(0.25)) = log(4)
    Task task = makeUniformTask();

    ThermodynamicsEngine::updateEntropy(task);

    double expected = -4.0 * Config::ENTROPY_CONSTANT * 0.25 * std::log(0.25);
    EXPECT_NEAR(task.entropy, expected, 1e-10);
}

// ---- All-zero psi: entropy = 0 (no probabilities above threshold) ----

TEST(ThermodynamicsEngineTest, AllZeroPsiHasZeroEntropy) {
    Task task;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        task.psi[i] = {0.0, 0.0};
    }
    task.entropy = 5.0;

    ThermodynamicsEngine::updateEntropy(task);

    EXPECT_NEAR(task.entropy, 0.0, 1e-12);
}

// ---- Entropy is always non-negative ----

TEST(ThermodynamicsEngineTest, EntropyIsNonNegative) {
    Task task = makeUniformTask();
    ThermodynamicsEngine::updateEntropy(task);
    EXPECT_GE(task.entropy, 0.0);
}

// ---- Two equal-weight components: S = log(2) ----

TEST(ThermodynamicsEngineTest, TwoComponentEqualSuperposition) {
    // p_0 = p_1 = 0.5, p_2 = p_3 = 0  →  S = -2*(0.5*log(0.5)) = log(2)
    Task task;
    const double amp = std::sqrt(0.5);
    task.psi[0] = {amp, 0.0};
    task.psi[1] = {amp, 0.0};
    task.psi[2] = {0.0, 0.0};
    task.psi[3] = {0.0, 0.0};

    ThermodynamicsEngine::updateEntropy(task);

    double expected = -2.0 * Config::ENTROPY_CONSTANT * 0.5 * std::log(0.5);
    EXPECT_NEAR(task.entropy, expected, 1e-10);
}

// ---- Mixed state has more entropy than pure state ----

TEST(ThermodynamicsEngineTest, MixedStateHasMoreEntropyThanPureState) {
    Task pure_task;
    pure_task.psi[0] = {1.0, 0.0};
    ThermodynamicsEngine::updateEntropy(pure_task);

    Task mixed_task = makeUniformTask();
    ThermodynamicsEngine::updateEntropy(mixed_task);

    EXPECT_GT(mixed_task.entropy, pure_task.entropy);
}

// ---- updateEntropy always overwrites the previous value ----

TEST(ThermodynamicsEngineTest, UpdateEntropyOverwritesPreviousValue) {
    Task task;
    task.psi[0] = {1.0, 0.0}; // pure state → entropy should become 0
    task.entropy = 999.0;

    ThermodynamicsEngine::updateEntropy(task);

    EXPECT_NEAR(task.entropy, 0.0, 1e-12);
}

// ---- Imaginary amplitudes contribute to probability ----

TEST(ThermodynamicsEngineTest, ImaginaryAmplitudesContributeToProbability) {
    // psi = [{0, 1}, {0, 0}, ...}: magnitudeSquared = 1  →  pure state, S = 0
    Task task;
    task.psi[0] = {0.0, 1.0};
    task.psi[1] = {0.0, 0.0};
    task.psi[2] = {0.0, 0.0};
    task.psi[3] = {0.0, 0.0};

    ThermodynamicsEngine::updateEntropy(task);

    EXPECT_NEAR(task.entropy, 0.0, 1e-12);
}

// ---- Entropy ordering: fewer non-zero components means lower entropy ----

TEST(ThermodynamicsEngineTest, TwoStateMixtureHasLessEntropyThanFourStateMixture) {
    Task two_state;
    const double amp2 = std::sqrt(0.5);
    two_state.psi[0] = {amp2, 0.0};
    two_state.psi[1] = {amp2, 0.0};
    two_state.psi[2] = {0.0, 0.0};
    two_state.psi[3] = {0.0, 0.0};
    ThermodynamicsEngine::updateEntropy(two_state);

    Task four_state = makeUniformTask();
    ThermodynamicsEngine::updateEntropy(four_state);

    EXPECT_LT(two_state.entropy, four_state.entropy);
}
