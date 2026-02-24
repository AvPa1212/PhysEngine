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

        // Friction
        double velMag = magnitude(currentVelocity);
        Vector2 frictionForce = { 0.0, 0.0 };
        if (velMag > 0.001) {
            Vector2 dir = normalize(currentVelocity);
            frictionForce = dir * (-task.kineticFriction * task.mass * Config::GRAVITY_CONSTANT);
        }

        return deadlineForce + frictionForce;
    }

    static void integrateRK4(Task& task) {
        double dt = Config::TIME_STEP;

        // k1
        Vector2 v1 = task.velocity;
        Vector2 a1 = computeForces(task, v1) * (1.0 / task.mass);

        // k2
        Vector2 v2 = task.velocity + a1 * (dt / 2.0);
        Vector2 a2 = computeForces(task, v2) * (1.0 / task.mass);

        // k3
        Vector2 v3 = task.velocity + a2 * (dt / 2.0);
        Vector2 a3 = computeForces(task, v3) * (1.0 / task.mass);

        // k4
        Vector2 v4 = task.velocity + a3 * dt;
        Vector2 a4 = computeForces(task, v4) * (1.0 / task.mass);

        task.position += (v1 + v2 * 2.0 + v3 * 2.0 + v4) * (dt / 6.0);
        task.velocity += (a1 + a2 * 2.0 + a3 * 2.0 + a4) * (dt / 6.0);
        task.acceleration = a1;

        task.deadlineTime -= dt;
    }
};