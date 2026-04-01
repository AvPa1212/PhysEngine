/**
 * @file ChaosEngine.hpp
 * @brief Lorenz chaotic attractor integration for task stress dynamics.
 *
 * Models chaotic behaviour by evolving the task's (stressX, stressY, stressZ)
 * triple as a point on the Lorenz strange attractor.  The Lorenz system is
 * governed by:
 *   dx/dt = σ(y − x)
 *   dy/dt = x(ρ − z) − y
 *   dz/dt = xy − βz
 *
 * With σ = 10, ρ = 28, β = 8/3 (Config defaults), the system exhibits
 * the well-known "butterfly" strange attractor and sensitive dependence on
 * initial conditions — small differences in initial stress values diverge
 * exponentially over time.
 *
 * A fourth-order Runge-Kutta integrator is used (same order as
 * ClassicalEngine) for consistency and numerical quality.
 *
 * The total displacement in phase space each step is also used to
 * accumulate entropy, coupling the chaos subsystem to the thermodynamic
 * entropy visible in the UI.
 */
#pragma once
#include "Task.hpp"
#include "core/Config.hpp"
#include <cmath> // Added for std::abs

/**
 * @brief Stateless chaos engine that advances the Lorenz stress state each frame.
 */
class ChaosEngine {
public:
    /**
     * @brief Advances the Lorenz stress state of a task by one time-step.
     *
     * Uses RK4 integration of the three Lorenz ODEs (dx/dt, dy/dt, dz/dt)
     * to move the stress point (stressX, stressY, stressZ) forward in phase
     * space by Config::TIME_STEP seconds.
     *
     * Intermediate midpoint coordinates for each RK4 stage (m1, m2, m3) are
     * pre-computed once and shared across all three derivative evaluations in
     * that stage to avoid redundant arithmetic.
     *
     * After the stress update, a small entropy increment proportional to the
     * total Lorenz displacement is added to task.entropy.  This causes tasks
     * with rapidly changing stress (high chaos) to accumulate entropy faster,
     * making them more likely to undergo quantum collapse.
     *
     * @param task The task whose stress components and entropy are updated
     *             in place.
     */
    static void update(Task& task) {
        const double dt = Config::TIME_STEP;
        const double dt_half = dt * 0.5;
        const double dt_sixth = dt / 6.0;

        // Snapshot current stress values so the RK4 stages all start from the
        // same base point rather than the partially-updated values.
        const double sX = task.stressX;
        const double sY = task.stressY;
        const double sZ = task.stressZ;

        // Lorenz derivative lambdas (inline closures — zero overhead with -O2).
        // dx/dt = σ(y - x)
        auto dx = [](double x, double y) { return Config::CHAOS_SIGMA * (y - x); };
        // dy/dt = x(ρ - z) - y
        auto dy = [](double x, double y, double z) { return x * (Config::CHAOS_RHO - z) - y; };
        // dz/dt = xy - βz
        auto dz = [](double x, double y, double z) { return x * y - Config::CHAOS_BETA * z; };

        // --- RK4 Integration for Lorenz ---

        // k1: derivatives at the current state.
        const double k1x = dx(sX, sY);
        const double k1y = dy(sX, sY, sZ);
        const double k1z = dz(sX, sY, sZ);

        // Precompute k2 midpoint coordinates once (reused for all three k2 evaluations)
        const double m1x = sX + k1x * dt_half;
        const double m1y = sY + k1y * dt_half;
        const double m1z = sZ + k1z * dt_half;
        // k2: derivatives at the half-step midpoint estimated by k1.
        const double k2x = dx(m1x, m1y);
        const double k2y = dy(m1x, m1y, m1z);
        const double k2z = dz(m1x, m1y, m1z);

        // Precompute k3 midpoint coordinates once (reused for all three k3 evaluations)
        const double m2x = sX + k2x * dt_half;
        const double m2y = sY + k2y * dt_half;
        const double m2z = sZ + k2z * dt_half;
        // k3: derivatives at the half-step midpoint estimated by k2.
        const double k3x = dx(m2x, m2y);
        const double k3y = dy(m2x, m2y, m2z);
        const double k3z = dz(m2x, m2y, m2z);

        // Precompute k4 endpoint coordinates once (reused for all three k4 evaluations)
        const double m3x = sX + k3x * dt;
        const double m3y = sY + k3y * dt;
        const double m3z = sZ + k3z * dt;
        // k4: derivatives at the full-step endpoint estimated by k3.
        const double k4x = dx(m3x, m3y);
        const double k4y = dy(m3x, m3y, m3z);
        const double k4z = dz(m3x, m3y, m3z);

        // Final weighted sums — RK4 formula: Δq = (k1 + 2k2 + 2k3 + k4)·Δt/6
        const double deltaX = dt_sixth * (k1x + 2.0 * k2x + 2.0 * k3x + k4x);
        const double deltaY = dt_sixth * (k1y + 2.0 * k2y + 2.0 * k3y + k4y);
        const double deltaZ = dt_sixth * (k1z + 2.0 * k2z + 2.0 * k3z + k4z);

        // Apply the computed deltas to advance the stress state.
        task.stressX += deltaX;
        task.stressY += deltaY;
        task.stressZ += deltaZ;

        // --- ENTROPY LOGIC ---
        // Accumulate entropy based on the total movement in phase space.
        // We use a small multiplier (0.001) so it doesn't collapse too instantly.
        // The L1 norm of the displacement vector is a cheap proxy for how much
        // the system moved; larger movements indicate more chaotic, energetic
        // behaviour and should raise entropy faster.
        task.entropy += (std::abs(deltaX) + std::abs(deltaY) + std::abs(deltaZ)) * 0.001;
    }
};