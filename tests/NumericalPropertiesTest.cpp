#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/ClassicalEngine.hpp"
#include "core/Config.hpp"
#include <cmath>

// Helper: build a Task with controlled parameters
static Task makeNumericalTask(double urgency, double deadline, double mass,
                              double kineticFriction, double vx, double vy) {
    Task t;
    t.urgencyConstant = urgency;
    t.deadlineTime    = deadline;
    t.mass            = mass;
    t.kineticFriction = kineticFriction;
    t.velocity        = {vx, vy};
    t.position        = {0.0, 0.0};
    return t;
}

// -----------------------------------------------------------------------
// Property 7: Newton's Second Law
// Feature: classical-mechanics-task-system, Property 7: Newton's Second Law
// Validates: Requirements 4.1, 4.2, 4.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(NumericalProperties, NewtonsSecondLaw, ()) {
    // mass in [0.1, 10.0] via integer tenths [1, 100]
    const int massInt = *rc::gen::inRange(1, 101).as("mass_int");
    const double mass = massInt / 10.0;

    // urgency in [1, 1000]
    const int urgencyInt = *rc::gen::inRange(1, 1001).as("urgency_int");
    const double urgency = static_cast<double>(urgencyInt);

    // deadline in [MIN_DEADLINE_TIME, 100.0] via integer tenths
    const int minTenths = static_cast<int>(Config::MIN_DEADLINE_TIME * 10) + 1;
    const int deadlineTenths = *rc::gen::inRange(minTenths, 1001).as("deadline_tenths");
    const double deadline = deadlineTenths / 10.0;

    // kineticFriction in [0, 1] via integer hundredths [0, 100]
    const int kfInt = *rc::gen::inRange(0, 101).as("kf_int");
    const double kf = kfInt / 100.0;

    // velocity components in [-10, 10] via integer tenths
    const int vxInt = *rc::gen::inRange(-100, 101).as("vx_int");
    const int vyInt = *rc::gen::inRange(-100, 101).as("vy_int");
    const double vx = vxInt / 10.0;
    const double vy = vyInt / 10.0;

    Task task = makeNumericalTask(urgency, deadline, mass, kf, vx, vy);

    // Compute expected acceleration: a = F / m
    Vector2 netForce = ClassicalEngine::computeForces(task, task.velocity);
    Vector2 expectedAccel = netForce * (1.0 / mass);

    // Integrate one step — acceleration stored is a1 (force at initial state)
    ClassicalEngine::integrateRK4(task);

    RC_ASSERT(std::abs(task.acceleration.x - expectedAccel.x) < 1e-6);
    RC_ASSERT(std::abs(task.acceleration.y - expectedAccel.y) < 1e-6);
}

// -----------------------------------------------------------------------
// Property 8: RK4 Fourth-Order Accuracy
// Feature: classical-mechanics-task-system, Property 8: RK4 Fourth-Order Accuracy
// Validates: Requirements 5.7
// -----------------------------------------------------------------------
RC_GTEST_PROP(NumericalProperties, RK4FourthOrderAccuracy, ()) {
    // Use zero friction and a fixed urgency/deadline so force is constant.
    // Analytical solution: x(t) = 0.5 * (F/m) * t^2  (starting from rest at origin)
    const int massInt = *rc::gen::inRange(1, 101).as("mass_int");
    const double mass = massInt / 10.0;

    // Keep deadline very large so force stays nearly constant over 60 steps.
    // With deadline=1000, dt=1/60, and N=60 steps: deadline changes by only 1 second
    // out of 1000, so the constant-force approximation is accurate to ~0.1%.
    const double deadline = 1000.0;
    const double urgency  = 100.0;
    const double kf       = 0.0; // zero friction for clean analytical comparison

    Task task = makeNumericalTask(urgency, deadline, mass, kf, 0.0, 0.0);

    const int N = 60;
    const double dt = Config::TIME_STEP;

    // Integrate N steps with RK4
    for (int i = 0; i < N; ++i) {
        ClassicalEngine::integrateRK4(task);
    }

    // Analytical position: x(t) = 0.5 * (F/m) * t^2
    // Force at t=0: F = urgency / deadline^2 (nearly constant over N steps)
    const double F0 = urgency / (deadline * deadline);
    const double a0 = F0 / mass;
    const double t  = N * dt;
    const double analyticalX = 0.5 * a0 * t * t;

    // RK4 error should be small (< 1e-4) for this smooth, slowly-varying force
    RC_ASSERT(std::abs(task.position.x - analyticalX) < 1e-4);
}

// -----------------------------------------------------------------------
// Property 9: Time Evolution Advances State
// Feature: classical-mechanics-task-system, Property 9: Time Evolution Advances State
// Validates: Requirements 6.1
// -----------------------------------------------------------------------
RC_GTEST_PROP(NumericalProperties, TimeEvolutionAdvancesState, ()) {
    // urgency > 0 ensures non-zero deadline force
    const int urgencyInt = *rc::gen::inRange(1, 1001).as("urgency_int");
    const double urgency = static_cast<double>(urgencyInt);

    const int massInt = *rc::gen::inRange(1, 101).as("mass_int");
    const double mass = massInt / 10.0;

    // deadline well above MIN_DEADLINE_TIME
    const int deadlineTenths = *rc::gen::inRange(10, 1001).as("deadline_tenths");
    const double deadline = deadlineTenths / 10.0;

    const int kfInt = *rc::gen::inRange(0, 101).as("kf_int");
    const double kf = kfInt / 100.0;

    const int vxInt = *rc::gen::inRange(-100, 101).as("vx_int");
    const int vyInt = *rc::gen::inRange(-100, 101).as("vy_int");
    const double vx = vxInt / 10.0;
    const double vy = vyInt / 10.0;

    Task task = makeNumericalTask(urgency, deadline, mass, kf, vx, vy);

    const Vector2 initPos = task.position;
    const Vector2 initVel = task.velocity;

    ClassicalEngine::integrateRK4(task);

    // After one step with non-zero net force, position and/or velocity must change.
    // The deadline force is always non-zero (urgency > 0), so acceleration != 0,
    // meaning velocity always changes.
    const bool velChanged = (task.velocity.x != initVel.x) || (task.velocity.y != initVel.y);
    const bool posChanged = (task.position.x != initPos.x) || (task.position.y != initPos.y);

    RC_ASSERT(velChanged || posChanged);
}

// -----------------------------------------------------------------------
// Property 10: Deadline Decrements
// Feature: classical-mechanics-task-system, Property 10: Deadline Decrements
// Validates: Requirements 6.2
// -----------------------------------------------------------------------
RC_GTEST_PROP(NumericalProperties, DeadlineDecrements, ()) {
    // deadline strictly above MIN_DEADLINE_TIME so it can safely decrement
    const int minTenths = static_cast<int>(Config::MIN_DEADLINE_TIME * 10) + 2;
    const int deadlineTenths = *rc::gen::inRange(minTenths, 1001).as("deadline_tenths");
    const double deadline = deadlineTenths / 10.0;

    const int urgencyInt = *rc::gen::inRange(1, 1001).as("urgency_int");
    const double urgency = static_cast<double>(urgencyInt);

    const int massInt = *rc::gen::inRange(1, 101).as("mass_int");
    const double mass = massInt / 10.0;

    const int kfInt = *rc::gen::inRange(0, 101).as("kf_int");
    const double kf = kfInt / 100.0;

    Task task = makeNumericalTask(urgency, deadline, mass, kf, 0.0, 0.0);

    const double initialDeadline = task.deadlineTime;

    ClassicalEngine::integrateRK4(task);

    RC_ASSERT(std::abs(task.deadlineTime - (initialDeadline - Config::TIME_STEP)) < 1e-12);
}

// -----------------------------------------------------------------------
// Property 12: Integration Depends on Initial Conditions
// Feature: classical-mechanics-task-system, Property 12: Integration Depends on Initial Conditions
// Validates: Requirements 6.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(NumericalProperties, IntegrationDependsOnInitialConditions, ()) {
    const int urgencyInt = *rc::gen::inRange(1, 1001).as("urgency_int");
    const double urgency = static_cast<double>(urgencyInt);

    const int massInt = *rc::gen::inRange(1, 101).as("mass_int");
    const double mass = massInt / 10.0;

    const int deadlineTenths = *rc::gen::inRange(10, 1001).as("deadline_tenths");
    const double deadline = deadlineTenths / 10.0;

    const int kfInt = *rc::gen::inRange(0, 101).as("kf_int");
    const double kf = kfInt / 100.0;

    // Two different vx values that are guaranteed to differ
    const int vx1Int = *rc::gen::inRange(-100, 50).as("vx1_int");
    const int vx2Int = *rc::gen::inRange(51, 101).as("vx2_int");
    const double vx1 = vx1Int / 10.0;
    const double vx2 = vx2Int / 10.0;

    // Same vy for both
    const int vyInt = *rc::gen::inRange(-100, 101).as("vy_int");
    const double vy = vyInt / 10.0;

    Task task1 = makeNumericalTask(urgency, deadline, mass, kf, vx1, vy);
    Task task2 = makeNumericalTask(urgency, deadline, mass, kf, vx2, vy);

    ClassicalEngine::integrateRK4(task1);
    ClassicalEngine::integrateRK4(task2);

    // Different initial velocities must produce different final positions
    RC_ASSERT(task1.position.x != task2.position.x ||
              task1.position.y != task2.position.y);
}
