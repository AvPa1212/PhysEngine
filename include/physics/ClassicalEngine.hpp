/**
 * @file ClassicalEngine.hpp
 * @brief Classical Newtonian mechanics integrators for Task objects.
 *
 * Provides two integration strategies:
 *  - integrateRK4   — fourth-order Runge-Kutta (primary, high accuracy)
 *  - integrateEuler — first-order forward Euler (baseline / benchmark only)
 *
 * Forces modelled per task:
 *  - Deadline force: F = k / t²  (models increasing urgency as deadline nears)
 *  - Kinetic friction: F = -μₖ·m·g·v̂  (opposes motion)
 *
 * All methods are static; the class serves as a stateless namespace.
 */
#pragma once
#include "Task.hpp"
#include "core/Config.hpp"
#include "physics/EnergyEngine.hpp"
#include <cmath>

/**
 * @brief Stateless classical-mechanics engine operating on Task instances.
 */
class ClassicalEngine {
public:
    /**
     * @brief Computes the net 2-D force acting on a task at a given velocity.
     *
     * Two forces are summed:
     *
     *  1. **Deadline force** — a scalar F = k / t² capped at MAX_DEADLINE_FORCE
     *     and directed along the positive X axis.  It models the "urgency pull"
     *     that accelerates a task toward completion as the deadline approaches.
     *     The denominator is clamped to MIN_DEADLINE_TIME to prevent division
     *     by values near zero.
     *
     *  2. **Kinetic friction** — opposes the supplied @p currentVelocity with
     *     magnitude μₖ·m·g.  A pre-computed reciprocal of the speed is used
     *     instead of calling normalize(), saving one sqrt() call per evaluation.
     *     No friction is applied below a small speed threshold (0.001 m/s) to
     *     avoid numerical jitter for essentially-stationary tasks.
     *
     * @param task            The task providing mass, friction coefficients,
     *                        deadline time, and urgency constant.
     * @param currentVelocity The velocity at which to evaluate friction
     *                        (differs from task.velocity at RK4 mid-points).
     * @return Net force vector (deadline + friction) in force units.
     */
    static Vector2 computeForces(const Task& task, const Vector2& currentVelocity) {
        // Deadline Force: F = k / t^2
        double t = task.deadlineTime > Config::MIN_DEADLINE_TIME ? task.deadlineTime : Config::MIN_DEADLINE_TIME;
        double deadlineForceMag = task.urgencyConstant / (t * t);
        if (deadlineForceMag > Config::MAX_DEADLINE_FORCE) {
            deadlineForceMag = Config::MAX_DEADLINE_FORCE;
        }

        Vector2 deadlineForce = { deadlineForceMag, 0.0 }; // Assuming it pulls in +X for this spec

        // Friction: compute magnitude once and reuse it to build the normalised direction,
        // avoiding the second sqrt that calling normalize() would trigger.
        const double velMag = magnitude(currentVelocity);
        Vector2 frictionForce = { 0.0, 0.0 };
        if (velMag > 0.001) {
            const double invVelMag = 1.0 / velMag;
            const Vector2 dir = { currentVelocity.x * invVelMag, currentVelocity.y * invVelMag };
            frictionForce = dir * (-task.kineticFriction * task.mass * Config::GRAVITY_CONSTANT);
        }

        Vector2 totalForce = deadlineForce + frictionForce;

        // Apply energy-based force scaling
        double scalingFactor = EnergyEngine::computeForceScalingFactor(task);
        totalForce = totalForce * scalingFactor;

        return totalForce;
    }

    /**
     * @brief Advances a task by one time-step using the fourth-order Runge-Kutta method.
     *
     * RK4 evaluates the equations of motion at four points within the interval
     * [t, t+Δt] and combines the slopes with weights (1, 2, 2, 1)/6 to obtain
     * a fourth-order accurate approximation of the new state.  The global
     * truncation error is O(Δt⁴), making it far more accurate than Euler at
     * the same step size.
     *
     * Stage summary:
     *  - k1: slope at the start of the interval (current state).
     *  - k2: slope at the midpoint using k1 to estimate mid-state.
     *  - k3: slope at the midpoint using k2 to estimate mid-state.
     *  - k4: slope at the end of the interval using k3 to estimate end-state.
     *
     * The inverse mass (1/m) is pre-computed once to replace the four
     * per-stage divisions with multiplications.
     *
     * After integration:
     *  - task.position and task.velocity are updated.
     *  - task.acceleration is set to the k1 acceleration (initial slope).
     *  - task.deadlineTime is decremented by Δt.
     *  - task.stepCount is incremented by 1.
     *
     * @param task The task to integrate (modified in place).
     */
    static void integrateRK4(Task& task) {
        integrateRK4(task, Config::TIME_STEP);
    }

    static void integrateRK4(Task& task, double dt) {
        if (!(task.mass > 0.0) || !std::isfinite(task.mass)) {
            task.acceleration = {0.0, 0.0};
            return;
        }
        if (!(dt > 0.0) || !std::isfinite(dt)) {
            return;
        }

        const double dt_half = dt * 0.5;
        const double dt_sixth = dt / 6.0;
        // Precompute inverse mass once — division is expensive; the four
        // accelerations below become multiplications instead.
        const double inv_mass = 1.0 / task.mass;

        // k1 — evaluate forces and acceleration at the current state.
        const Vector2 v1 = task.velocity;
        const Vector2 a1 = computeForces(task, v1) * inv_mass;

        // k2 — evaluate at the midpoint estimated using k1 slopes.
        const Vector2 v2 = task.velocity + a1 * dt_half;
        const Vector2 a2 = computeForces(task, v2) * inv_mass;

        // k3 — evaluate at the midpoint estimated using k2 slopes.
        const Vector2 v3 = task.velocity + a2 * dt_half;
        const Vector2 a3 = computeForces(task, v3) * inv_mass;

        // k4 — evaluate at the end-point estimated using k3 slopes.
        const Vector2 v4 = task.velocity + a3 * dt;
        const Vector2 a4 = computeForces(task, v4) * inv_mass;

        // Combine the four slopes with RK4 weights (1,2,2,1)/6 and integrate.
        task.position += (v1 + v2 * 2.0 + v3 * 2.0 + v4) * dt_sixth;
        task.velocity += (a1 + a2 * 2.0 + a3 * 2.0 + a4) * dt_sixth;
        task.acceleration = a1;

        if (!std::isfinite(task.position.x) || !std::isfinite(task.position.y) ||
            !std::isfinite(task.velocity.x) || !std::isfinite(task.velocity.y) ||
            !std::isfinite(task.acceleration.x) || !std::isfinite(task.acceleration.y)) {
            task.position = {0.0, 0.0};
            task.velocity = {0.0, 0.0};
            task.acceleration = {0.0, 0.0};
        }

        task.deadlineTime -= dt;
        task.stepCount++;
    }

    /**
     * @brief Advances a task by one time-step using the first-order forward Euler method.
     *
     * First-order forward Euler integrator.
     * Provided as a baseline for benchmarking accuracy and stability against
     * the primary RK4 integrator (see integrateRK4).  Euler requires only a
     * single force evaluation per step, making it faster per step but
     * accumulating O(dt) global error versus RK4's O(dt^4).
     *
     * Update equations:
     *   x(t+Δt) = x(t) + v(t)·Δt
     *   v(t+Δt) = v(t) + a(t)·Δt   where a(t) = F(t) / m
     *
     * @param task The task to integrate (modified in place).
     */
    static void integrateEuler(Task& task) {
        const double dt      = Config::TIME_STEP;
        const double inv_mass = 1.0 / task.mass;

        // Single force evaluation at the current state (Euler has only one stage).
        const Vector2 a = computeForces(task, task.velocity) * inv_mass;

        task.position     += task.velocity * dt;
        task.velocity     += a             * dt;
        task.acceleration  = a;

        task.deadlineTime -= dt;
        task.stepCount++;
    }
};