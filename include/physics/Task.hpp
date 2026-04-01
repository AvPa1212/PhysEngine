/**
 * @file Task.hpp
 * @brief Central data structure representing a single simulated task/particle.
 *
 * A Task bundles all of the state consumed and produced by the four physics
 * sub-systems (classical mechanics, quantum, thermodynamics, and chaos) into
 * one value type.  Keeping everything in a single struct improves cache
 * locality when the simulation engine iterates over the task list.
 *
 * All fields are default-initialised so a freshly constructed Task is
 * immediately valid and can be pushed into the simulation without extra setup.
 */
#pragma once
#include "math/Vector2.hpp"
#include "math/Matrix.hpp"
#include "core/Config.hpp"
#include <array>

/**
 * @brief Aggregates all per-task simulation state.
 *
 * The struct is divided into logical sections that map directly onto the
 * four engine modules that read and write it each simulation step.
 */
struct Task {
    // -----------------------------------------------------------------------
    // Classical mechanics fields — consumed/updated by ClassicalEngine
    // -----------------------------------------------------------------------

    /// 2-D position in world space (metres).  Updated by the RK4 integrator
    /// each frame using the current velocity.
    Vector2 position = { 0.0, 0.0 };

    /// 2-D velocity (m/s).  Driven by the net force (deadline + friction)
    /// divided by mass.
    Vector2 velocity = { 0.0, 0.0 };

    /// 2-D acceleration (m/s²) recorded at the start of the last RK4 step
    /// (k1 slope).  Useful for visualisation and derivative queries.
    Vector2 acceleration = { 0.0, 0.0 };

    /// Inertial mass (kg).  Scales the effect of all applied forces via
    /// Newton's second law: a = F / mass.
    double mass = 1.0;

    /// Coefficient of static friction μₛ.  Currently stored but not directly
    /// used by ClassicalEngine (kinetic friction is always applied when
    /// moving); reserved for future static-friction logic.
    double staticFriction = 0.5;

    /// Coefficient of kinetic friction μₖ.  The friction force opposes
    /// motion: F_friction = -μₖ · mass · g · v̂.
    double kineticFriction = 0.3;

    /// Time remaining until the task's deadline (seconds).  Drives the
    /// urgency force F = urgencyConstant / deadlineTime².  Decremented by
    /// Config::TIME_STEP each integration step.
    double deadlineTime = 10.0;

    /// Proportionality constant k for the deadline force F = k / t².
    /// Higher values model more urgent tasks that accelerate faster as
    /// the deadline approaches.
    double urgencyConstant = 100.0;

    // -----------------------------------------------------------------------
    // Quantum fields — consumed/updated by QuantumEngine
    // -----------------------------------------------------------------------

    /// Quantum state vector ψ ∈ ℂ^QUANTUM_DIM (wavefunction amplitudes).
    /// Default: pure |0⟩ state with amplitude 1 in the first basis element.
    /// Each component ψᵢ satisfies Σ|ψᵢ|² = 1 after normalisation.
    std::array<Complex, Config::QUANTUM_DIM> psi = {
        Complex{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}
    };

    /// Hermitian Hamiltonian matrix H that governs quantum time evolution.
    /// The engine computes U ≈ I - iHΔt - ½H²Δt² each step and applies
    /// it to psi.  Default: zero matrix (no quantum dynamics until set).
    Matrix hamiltonian;

    // -----------------------------------------------------------------------
    // Thermodynamics fields — consumed/updated by ThermodynamicsEngine
    // -----------------------------------------------------------------------

    /// Internal energy of the task (arbitrary units, currently unused by the
    /// default update path but available for future thermodynamic calculations).
    double internalEnergy = 0.0;

    /// Von-Neumann-like Shannon entropy S = -Σ pᵢ ln(pᵢ) computed from the
    /// squared wavefunction amplitudes.  Also accumulates contributions from
    /// Lorenz chaos dynamics (ChaosEngine).  High entropy signals an unstable,
    /// "spread-out" task that is a candidate for quantum collapse.
    double entropy = 0.0;

    // -----------------------------------------------------------------------
    // Chaos / stress fields — consumed/updated by ChaosEngine
    // -----------------------------------------------------------------------

    /// X component of the Lorenz state vector.  Represents task "stress" on
    /// the x-axis.  Initial value 1.0 seeds the Lorenz trajectory away from
    /// the unstable fixed point at the origin.
    double stressX = 1.0;

    /// Y component of the Lorenz state vector.
    double stressY = 1.0;

    /// Z component of the Lorenz state vector.
    double stressZ = 1.0;

    // -----------------------------------------------------------------------
    // Simulation tracking
    // -----------------------------------------------------------------------

    /// Number of integration steps completed since the task was created.
    /// Incremented by ClassicalEngine::integrateRK4 each frame.
    int stepCount = 0;
};