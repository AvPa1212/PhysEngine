#pragma once

namespace Config {
    constexpr double TIME_STEP = 1.0 / 60.0;
    constexpr double GRAVITY_CONSTANT = 9.81;
    constexpr double PLANCK_REDUCED = 1.0;
    constexpr double ENTROPY_CONSTANT = 1.0;
    constexpr int QUANTUM_DIM = 4;

    // Safety thresholds
    constexpr double MIN_DEADLINE_TIME = 0.01;
    constexpr double MAX_DEADLINE_FORCE = 10000.0;

    // Energy System Constants
    constexpr double MAX_KINETIC_ENERGY = 100000.0;
    constexpr double MIN_ENERGY_THRESHOLD = 0.001;
    constexpr double DEFAULT_DAMPING_COEFFICIENT = 0.1;
    constexpr double MAX_ENERGY_INJECTION_RATE = 100.0;

    // Lorenz Attractor Constants
    constexpr double CHAOS_SIGMA = 10.0;
    constexpr double CHAOS_RHO = 28.0;
    constexpr double CHAOS_BETA = 8.0 / 3.0;
}