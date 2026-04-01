#pragma once
#include "Task.hpp"
#include "core/Config.hpp"

class ClassicalEngine {
public:
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

        return deadlineForce + frictionForce;
    }

    static void integrateRK4(Task& task) {
        const double dt = Config::TIME_STEP;
        const double dt_half = dt * 0.5;
        const double dt_sixth = dt / 6.0;
        // Precompute inverse mass once — division is expensive; the four
        // accelerations below become multiplications instead.
        const double inv_mass = 1.0 / task.mass;

        // k1
        const Vector2 v1 = task.velocity;
        const Vector2 a1 = computeForces(task, v1) * inv_mass;

        // k2
        const Vector2 v2 = task.velocity + a1 * dt_half;
        const Vector2 a2 = computeForces(task, v2) * inv_mass;

        // k3
        const Vector2 v3 = task.velocity + a2 * dt_half;
        const Vector2 a3 = computeForces(task, v3) * inv_mass;

        // k4
        const Vector2 v4 = task.velocity + a3 * dt;
        const Vector2 a4 = computeForces(task, v4) * inv_mass;

        task.position += (v1 + v2 * 2.0 + v3 * 2.0 + v4) * dt_sixth;
        task.velocity += (a1 + a2 * 2.0 + a3 * 2.0 + a4) * dt_sixth;
        task.acceleration = a1;

        task.deadlineTime -= dt;
    }
};