#include <gtest/gtest.h>
#include "physics/ClassicalEngine.hpp"
#include "core/Config.hpp"
#include <cmath>

// ---- Deadline force formula (Requirements 2.1) ----

TEST(ComputeForcesTest, DeadlineForceFormula) {
    Task task;
    task.urgencyConstant = 100.0;
    task.deadlineTime = 5.0;
    task.mass = 1.0;
    task.kineticFriction = 0.0; // isolate deadline force

    Vector2 force = ClassicalEngine::computeForces(task, {0.0, 0.0});

    double expected = 100.0 / (5.0 * 5.0); // 4.0
    EXPECT_NEAR(force.x, expected, 1e-9);
}

// ---- Deadline clamping at MIN_DEADLINE_TIME (Requirements 2.2) ----

TEST(ComputeForcesTest, DeadlineClampedAtMinDeadlineTime) {
    Task task;
    // Use a small urgency so the clamped force stays below MAX_DEADLINE_FORCE
    // MIN_DEADLINE_TIME = 0.01, so F = 0.5 / 0.0001 = 5000 < 10000
    task.urgencyConstant = 0.5;
    task.deadlineTime = 0.001; // below MIN_DEADLINE_TIME (0.01)
    task.mass = 1.0;
    task.kineticFriction = 0.0;

    Vector2 force = ClassicalEngine::computeForces(task, {0.0, 0.0});

    // Force should be computed using MIN_DEADLINE_TIME, not 0.001
    double expected = 0.5 / (Config::MIN_DEADLINE_TIME * Config::MIN_DEADLINE_TIME);
    EXPECT_NEAR(force.x, expected, 1e-6);
    // Also verify it differs from what we'd get without clamping
    double unclamped = 0.5 / (0.001 * 0.001);
    EXPECT_GT(unclamped, force.x); // unclamped would be much larger
}

// ---- Force clamping at MAX_DEADLINE_FORCE (Requirements 2.3) ----

TEST(ComputeForcesTest, DeadlineForceClamped) {
    Task task;
    task.urgencyConstant = 1e12; // enormous urgency → force would exceed MAX
    task.deadlineTime = 1.0;
    task.mass = 1.0;
    task.kineticFriction = 0.0;

    Vector2 force = ClassicalEngine::computeForces(task, {0.0, 0.0});

    EXPECT_NEAR(force.x, Config::MAX_DEADLINE_FORCE, 1e-6);
}

// ---- Deadline force direction: +X, zero Y (Requirements 2.4) ----

TEST(ComputeForcesTest, DeadlineForceDirectionPositiveX) {
    Task task;
    task.urgencyConstant = 100.0;
    task.deadlineTime = 10.0;
    task.mass = 1.0;
    task.kineticFriction = 0.0;

    Vector2 force = ClassicalEngine::computeForces(task, {0.0, 0.0});

    EXPECT_GT(force.x, 0.0);
    EXPECT_DOUBLE_EQ(force.y, 0.0);
}

// ---- Friction opposes velocity direction (Requirements 3.1) ----

TEST(ComputeForcesTest, FrictionOpposesVelocity) {
    Task task;
    task.urgencyConstant = 0.0; // zero deadline force to isolate friction
    task.deadlineTime = 10.0;
    task.mass = 1.0;
    task.kineticFriction = 0.3;

    Vector2 velocity = {3.0, 4.0};
    Vector2 force = ClassicalEngine::computeForces(task, velocity);

    // dot(friction, velocity) < 0 means friction opposes motion
    double dotProduct = force.x * velocity.x + force.y * velocity.y;
    EXPECT_LT(dotProduct, 0.0);
}

// ---- Friction magnitude formula (Requirements 3.2) ----

TEST(ComputeForcesTest, FrictionMagnitude) {
    Task task;
    task.urgencyConstant = 0.0;
    task.deadlineTime = 10.0;
    task.mass = 2.0;
    task.kineticFriction = 0.4;

    Vector2 velocity = {1.0, 0.0}; // unit velocity in X
    Vector2 force = ClassicalEngine::computeForces(task, velocity);

    double expectedMag = task.kineticFriction * task.mass * Config::GRAVITY_CONSTANT;
    double actualMag = std::sqrt(force.x * force.x + force.y * force.y);
    EXPECT_NEAR(actualMag, expectedMag, 1e-9);
}

// ---- Zero friction when velocity below threshold (Requirements 3.3) ----

TEST(ComputeForcesTest, ZeroFrictionWhenVelocityNearZero) {
    Task task;
    task.urgencyConstant = 0.0;
    task.deadlineTime = 10.0;
    task.mass = 1.0;
    task.kineticFriction = 0.5;

    Vector2 velocity = {0.0005, 0.0005}; // magnitude < 0.001
    Vector2 force = ClassicalEngine::computeForces(task, velocity);

    EXPECT_DOUBLE_EQ(force.x, 0.0);
    EXPECT_DOUBLE_EQ(force.y, 0.0);
}

// ---- Net force is sum of deadline and friction (Requirements 4.1) ----

TEST(ComputeForcesTest, NetForceIsSumOfComponents) {
    Task task;
    task.urgencyConstant = 100.0;
    task.deadlineTime = 5.0;
    task.mass = 1.0;
    task.kineticFriction = 0.3;

    Vector2 velocity = {1.0, 0.0};
    Vector2 force = ClassicalEngine::computeForces(task, velocity);

    double deadlineMag = 100.0 / (5.0 * 5.0);
    double frictionMag = 0.3 * 1.0 * Config::GRAVITY_CONSTANT;

    // Velocity is purely in +X, so friction is in -X
    double expectedX = deadlineMag - frictionMag;
    EXPECT_NEAR(force.x, expectedX, 1e-9);
    EXPECT_NEAR(force.y, 0.0, 1e-9);
}
