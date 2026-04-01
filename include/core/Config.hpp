/**
 * @file Config.hpp
 * @brief Compile-time simulation constants shared across all engine modules.
 *
 * Centralising these values in one place makes it easy to tune the
 * simulation without hunting through multiple source files.  All
 * constants are `constexpr` so they impose zero runtime cost and are
 * visible to the optimiser.
 */
#pragma once

/**
 * @namespace Config
 * @brief Global configuration constants for the PhysEngine simulation.
 *
 * Changing a value here recompiles every translation unit that includes
 * this header, ensuring the whole engine stays consistent.
 */
namespace Config {
    /// Fixed time-step used by all integrators (seconds).  At 1/60 s the
    /// simulation runs at 60 Hz, matching a typical display refresh rate.
    constexpr double TIME_STEP = 1.0 / 60.0;

    /// Standard acceleration due to gravity near Earth's surface (m/s²).
    /// Used to scale kinetic-friction forces: F_friction = μ·m·g.
    constexpr double GRAVITY_CONSTANT = 9.81;

    /// Reduced Planck constant ħ (set to 1.0 for dimensionless simulation
    /// units).  Appears implicitly in the Schrödinger time-evolution
    /// operator U ≈ I - iHΔt/ħ.
    constexpr double PLANCK_REDUCED = 1.0;

    /// Boltzmann-like constant used when computing Shannon entropy S.
    /// Set to 1.0 so entropy is measured in nats rather than joules/kelvin.
    constexpr double ENTROPY_CONSTANT = 1.0;

    /// Number of quantum basis states (dimension of the Hilbert space).
    /// Determines the size of Task::psi and the Matrix dimensions.
    constexpr int QUANTUM_DIM = 4;

    // -----------------------------------------------------------------------
    // Safety thresholds — prevent numerical blow-ups and runaway forces.
    // -----------------------------------------------------------------------

    /// Minimum remaining deadline time (seconds) before the engine clamps it.
    /// Prevents the deadline force F = k/t² from diverging as t → 0.
    constexpr double MIN_DEADLINE_TIME = 0.01;

    /// Upper bound on the deadline force magnitude (force units).
    /// Any calculated force exceeding this is clamped to avoid integration
    /// instability with large time-steps.
    constexpr double MAX_DEADLINE_FORCE = 10000.0;

    // -----------------------------------------------------------------------
    // Lorenz Attractor Constants — govern chaotic stress dynamics.
    // -----------------------------------------------------------------------

    /// σ (sigma) — Prandtl number.  Controls the rate at which the x
    /// component is driven toward y; classically σ = 10.
    constexpr double CHAOS_SIGMA = 10.0;

    /// ρ (rho) — Rayleigh number.  The primary bifurcation parameter;
    /// at ρ = 28 the Lorenz system exhibits the well-known "butterfly"
    /// strange attractor.
    constexpr double CHAOS_RHO = 28.0;

    /// β (beta) — geometric factor.  The classical value 8/3 comes from
    /// truncating the Fourier series of the convection equations.
    constexpr double CHAOS_BETA = 8.0 / 3.0;
}