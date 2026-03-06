#pragma once
#include "math/Vector2.hpp"
#include "math/Matrix.hpp"
#include "core/Config.hpp"
#include <array>

struct Task {
    // Classical
    Vector2 position = { 0.0, 0.0 };
    Vector2 velocity = { 0.0, 0.0 };
    Vector2 acceleration = { 0.0, 0.0 };
    double mass = 1.0;

    double staticFriction = 0.5;
    double kineticFriction = 0.3;

    double deadlineTime = 10.0;
    double urgencyConstant = 100.0;

    // Quantum
    std::array<Complex, Config::QUANTUM_DIM> psi = {
        Complex{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}
    };
    Matrix hamiltonian;

    // Thermodynamics
    double internalEnergy = 0.0;
    double entropy = 0.0;

    // Chaos
    double stressX = 1.0;
    double stressY = 1.0;
    double stressZ = 1.0;

    // Simulation tracking
    int stepCount = 0;
};