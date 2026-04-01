#include <gtest/gtest.h>
#include "physics/ClassicalEngine.hpp"
#include "core/Config.hpp"
#include <cmath>

// Helper: create a task with configurable physics parameters
static Task makeEulerTask(double mass = 1.0, double urgency = 100.0,
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

// ---- Velocity increases after one step from rest ----
// Forward Euler updates position using the OLD velocity, so position.x stays 0
// after step 1 from rest.  The force is applied to velocity though, so velocity
// must become positive after the first step.

TEST(EulerIntegratorTest, ZeroVelocityConstantForceIncreasesVelocity) {
    Task task = makeEulerTask();
    ClassicalEngine::integrateEuler(task);

    EXPECT_GT(task.velocity.x, 0.0);
    EXPECT_DOUBLE_EQ(task.velocity.y, 0.0); // force is purely in +X
}

// ---- Position changes after two steps from rest ----
// Step 1: position unchanged (old v=0), but velocity updated to a*dt
// Step 2: position updated with the non-zero velocity from step 1

TEST(EulerIntegratorTest, ZeroVelocityConstantForceChangesPositionAfterTwoSteps) {
    Task task = makeEulerTask();
    ClassicalEngine::integrateEuler(task); // velocity updated, position stays 0
    ClassicalEngine::integrateEuler(task); // position now updated with non-zero velocity

    EXPECT_GT(task.position.x, 0.0);
    EXPECT_DOUBLE_EQ(task.position.y, 0.0);
}

// ---- Position changes in direction of initial velocity (zero force) ----

TEST(EulerIntegratorTest, NonZeroVelocityChangesPositionInVelocityDirection) {
    Task task = makeEulerTask(/*mass=*/1.0, /*urgency=*/0.0,
                               /*deadline=*/10.0, /*friction=*/0.0);
    task.velocity = {3.0, 4.0};

    ClassicalEngine::integrateEuler(task);

    EXPECT_GT(task.position.x, 0.0);
    EXPECT_GT(task.position.y, 0.0);
}

// ---- Deadline decrements by exactly TIME_STEP ----

TEST(EulerIntegratorTest, DeadlineDecrementsbyTimeStep) {
    Task task = makeEulerTask();
    const double initialDeadline = task.deadlineTime;

    ClassicalEngine::integrateEuler(task);

    EXPECT_NEAR(task.deadlineTime, initialDeadline - Config::TIME_STEP, 1e-12);
}

// ---- StepCount increments by 1 ----

TEST(EulerIntegratorTest, StepCountIncrementsAfterIntegration) {
    Task task = makeEulerTask();
    EXPECT_EQ(task.stepCount, 0);

    ClassicalEngine::integrateEuler(task);

    EXPECT_EQ(task.stepCount, 1);
}

TEST(EulerIntegratorTest, StepCountIncrementsMultipleTimes) {
    Task task = makeEulerTask();

    for (int i = 0; i < 10; ++i) {
        ClassicalEngine::integrateEuler(task);
    }

    EXPECT_EQ(task.stepCount, 10);
}

// ---- Acceleration field is updated ----

TEST(EulerIntegratorTest, AccelerationUpdatedAfterIntegration) {
    Task task = makeEulerTask(/*mass=*/2.0, /*urgency=*/100.0,
                               /*deadline=*/10.0, /*friction=*/0.0);

    // Expected: a = F(initial state) / mass
    const Vector2 initialForce = ClassicalEngine::computeForces(task, task.velocity);
    const Vector2 expectedAccel = initialForce * (1.0 / task.mass);

    ClassicalEngine::integrateEuler(task);

    EXPECT_NEAR(task.acceleration.x, expectedAccel.x, 1e-9);
    EXPECT_NEAR(task.acceleration.y, expectedAccel.y, 1e-9);
}

// ---- Position update follows forward-Euler formula: pos += vel * dt ----

TEST(EulerIntegratorTest, PositionUpdateMatchesEulerFormula) {
    // Zero force so velocity is constant → position change = vel * dt exactly
    Task task = makeEulerTask(/*mass=*/1.0, /*urgency=*/0.0,
                               /*deadline=*/10.0, /*friction=*/0.0);
    task.velocity = {5.0, 3.0};

    ClassicalEngine::integrateEuler(task);

    const double dt = Config::TIME_STEP;
    EXPECT_NEAR(task.position.x, 5.0 * dt, 1e-12);
    EXPECT_NEAR(task.position.y, 3.0 * dt, 1e-12);
}

// ---- Velocity update follows forward-Euler formula: vel += accel * dt ----

TEST(EulerIntegratorTest, VelocityUpdateMatchesEulerFormula) {
    Task task = makeEulerTask(/*mass=*/1.0, /*urgency=*/100.0,
                               /*deadline=*/10.0, /*friction=*/0.0);
    task.velocity = {0.0, 0.0};

    // Capture force before the step
    const Vector2 force = ClassicalEngine::computeForces(task, task.velocity);
    const Vector2 accel = force * (1.0 / task.mass);
    const double dt = Config::TIME_STEP;

    ClassicalEngine::integrateEuler(task);

    EXPECT_NEAR(task.velocity.x, accel.x * dt, 1e-9);
    EXPECT_NEAR(task.velocity.y, accel.y * dt, 1e-9);
}

// ---- Y components are unaffected when the only force is in +X ----

TEST(EulerIntegratorTest, PositionAndVelocityYUnchangedWhenForceIsXOnly) {
    Task task = makeEulerTask();
    task.velocity = {0.0, 0.0};

    ClassicalEngine::integrateEuler(task);

    EXPECT_DOUBLE_EQ(task.position.y, 0.0);
    EXPECT_DOUBLE_EQ(task.velocity.y, 0.0);
}

// ---- Euler and RK4 agree to first order for a single step (non-zero initial velocity) ----
// With a non-zero initial velocity, both integrators advance the particle by
// approximately v0*dt.  Their positions differ only by the higher-order terms
// (O(dt^2)), which is a small fraction of the total displacement.

TEST(EulerIntegratorTest, EulerAndRK4AgreeToFirstOrderForOneStep) {
    Task euler_task = makeEulerTask(/*mass=*/1.0, /*urgency=*/100.0,
                                    /*deadline=*/10.0, /*friction=*/0.0);
    euler_task.velocity = {5.0, 0.0}; // non-zero so both methods move the particle

    Task rk4_task = euler_task;

    ClassicalEngine::integrateEuler(euler_task);
    ClassicalEngine::integrateRK4(rk4_task);

    // Both produce position in +X; the difference is O(dt^2) ≈ 5e-4
    const double diff = std::abs(euler_task.position.x - rk4_task.position.x);
    EXPECT_LT(diff, 5e-4);
}
