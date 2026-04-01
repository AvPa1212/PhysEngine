#include <gtest/gtest.h>
#include "physics/ClassicalEngine.hpp"
#include "core/Config.hpp"
#include <cmath>

// Helper: create a task with zero friction and a given urgency/deadline
static Task makeTask(double mass = 1.0, double urgency = 100.0,
                     double deadline = 10.0, double friction = 0.0) {
    Task t;
    t.mass = mass;
    t.urgencyConstant = urgency;
    t.deadlineTime = deadline;
    t.kineticFriction = friction;
    t.staticFriction = 0.0;
    t.velocity = {0.0, 0.0};
    t.position = {0.0, 0.0};
    t.stepCount = 0;
    return t;
}

// ---- Requirement 5.1 / 6.1: position changes after one step with constant force ----

TEST(RK4IntegrationTest, ZeroVelocityConstantForceChangesPosition) {
    // With zero initial velocity and a constant deadline force in +X,
    // position.x must increase after one integration step.
    Task task = makeTask(/*mass=*/1.0, /*urgency=*/100.0, /*deadline=*/10.0, /*friction=*/0.0);

    ClassicalEngine::integrateRK4(task);

    EXPECT_GT(task.position.x, 0.0);
    EXPECT_DOUBLE_EQ(task.position.y, 0.0); // force is purely in X
}

// ---- Requirement 6.1: position increases in +X with zero friction ----

TEST(RK4IntegrationTest, PositionIncreasesInXWithZeroFriction) {
    Task task = makeTask(/*mass=*/2.0, /*urgency=*/200.0, /*deadline=*/5.0, /*friction=*/0.0);
    task.velocity = {0.0, 0.0};

    ClassicalEngine::integrateRK4(task);

    EXPECT_GT(task.position.x, 0.0);
}

// ---- Requirement 6.2: deadlineTime decrements by exactly TIME_STEP ----

TEST(RK4IntegrationTest, DeadlineTimeDecrementsbyTimeStep) {
    Task task = makeTask();
    double initialDeadline = task.deadlineTime;

    ClassicalEngine::integrateRK4(task);

    EXPECT_NEAR(task.deadlineTime, initialDeadline - Config::TIME_STEP, 1e-12);
}

// ---- Requirement 6.1 (spec task 4.1): stepCount increments by 1 ----

TEST(RK4IntegrationTest, StepCountIncrementsAfterIntegration) {
    Task task = makeTask();
    EXPECT_EQ(task.stepCount, 0);

    ClassicalEngine::integrateRK4(task);

    EXPECT_EQ(task.stepCount, 1);
}

TEST(RK4IntegrationTest, StepCountIncrementsMultipleTimes) {
    Task task = makeTask();

    for (int i = 0; i < 5; ++i) {
        ClassicalEngine::integrateRK4(task);
    }

    EXPECT_EQ(task.stepCount, 5);
}

// ---- Requirement 6.5: non-zero initial velocity changes position in velocity direction ----

TEST(RK4IntegrationTest, NonZeroVelocityChangesPositionInVelocityDirection) {
    Task task = makeTask(/*mass=*/1.0, /*urgency=*/0.0, /*deadline=*/10.0, /*friction=*/0.0);
    task.velocity = {3.0, 4.0};

    ClassicalEngine::integrateRK4(task);

    // With zero force, position change should be in the direction of initial velocity
    EXPECT_GT(task.position.x, 0.0);
    EXPECT_GT(task.position.y, 0.0);
}

// ---- Requirement 6.3: acceleration field updated after integration (equals a1 = F/m) ----

TEST(RK4IntegrationTest, AccelerationFieldUpdatedAfterIntegration) {
    Task task = makeTask(/*mass=*/2.0, /*urgency=*/100.0, /*deadline=*/10.0, /*friction=*/0.0);
    task.velocity = {0.0, 0.0};

    // Compute expected a1 = F(initial state) / mass
    Vector2 initialForce = ClassicalEngine::computeForces(task, task.velocity);
    Vector2 expectedAccel = initialForce * (1.0 / task.mass);

    ClassicalEngine::integrateRK4(task);

    EXPECT_NEAR(task.acceleration.x, expectedAccel.x, 1e-9);
    EXPECT_NEAR(task.acceleration.y, expectedAccel.y, 1e-9);
}
