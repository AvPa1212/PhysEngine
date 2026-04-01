#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/ClassicalEngine.hpp"
#include "core/Config.hpp"
#include <cmath>

// Helper: build a Task with controlled parameters
static Task makeTask(double urgency, double deadline, double mass,
                     double kineticFriction, double vx, double vy) {
    Task t;
    t.urgencyConstant  = urgency;
    t.deadlineTime     = deadline;
    t.mass             = mass;
    t.kineticFriction  = kineticFriction;
    t.velocity         = {vx, vy};
    return t;
}

// -----------------------------------------------------------------------
// Property 2: Deadline Force Formula Correctness
// Feature: classical-mechanics-task-system, Property 2: Deadline Force Formula Correctness
// Validates: Requirements 2.1
// -----------------------------------------------------------------------
RC_GTEST_PROP(PhysicsProperties, DeadlineForceFormulaCorrectness, ()) {
    // urgency in [1.0, 1000.0]
    const double urgency = *rc::gen::inRange(1, 1001).as("urgency_int") / 1.0;
    // deadline in [MIN_DEADLINE_TIME, 100.0] — use integer tenths to stay in range
    const int deadlineTenths = *rc::gen::inRange(
        static_cast<int>(Config::MIN_DEADLINE_TIME * 10 + 1), 1001);
    const double deadline = deadlineTenths / 10.0;

    Task task = makeTask(urgency, deadline, 1.0, 0.0, 0.0, 0.0);

    Vector2 force = ClassicalEngine::computeForces(task, {0.0, 0.0});

    // Effective t after clamping (deadline already >= MIN_DEADLINE_TIME)
    double t = deadline;
    double expectedForce = urgency / (t * t);
    // Clamp to MAX_DEADLINE_FORCE as the engine does
    if (expectedForce > Config::MAX_DEADLINE_FORCE)
        expectedForce = Config::MAX_DEADLINE_FORCE;

    RC_ASSERT(std::abs(force.x - expectedForce) < 1e-6);
}

// -----------------------------------------------------------------------
// Property 3: Deadline Force Direction
// Feature: classical-mechanics-task-system, Property 3: Deadline Force Direction
// Validates: Requirements 2.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(PhysicsProperties, DeadlineForceDirection, ()) {
    const double urgency  = *rc::gen::inRange(1, 1001).as("urgency_int") / 1.0;
    const double deadline = *rc::gen::inRange(1, 1001).as("deadline_int") / 10.0
                            + Config::MIN_DEADLINE_TIME;
    const double mass     = *rc::gen::inRange(1, 101).as("mass_int") / 10.0;
    const double kf       = *rc::gen::inRange(0, 11).as("kf_int") / 10.0;
    // Use zero velocity so only the deadline force contributes
    Task task = makeTask(urgency, deadline, mass, kf, 0.0, 0.0);

    Vector2 force = ClassicalEngine::computeForces(task, {0.0, 0.0});

    RC_ASSERT(force.x > 0.0);
    RC_ASSERT(force.y == 0.0);
}

// -----------------------------------------------------------------------
// Property 4: Deadline Force Ordering
// Feature: classical-mechanics-task-system, Property 4: Deadline Force Ordering
// Validates: Requirements 2.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(PhysicsProperties, DeadlineForceOrdering, ()) {
    const double urgency = *rc::gen::inRange(1, 1001).as("urgency_int") / 1.0;

    // t1 and t2 both > MIN_DEADLINE_TIME, and t1 < t2
    // Use integer tenths: t1 in [MIN+1 tenth, 500 tenths], t2 in [t1+1, 1000 tenths]
    const int minTenths = static_cast<int>(Config::MIN_DEADLINE_TIME * 10) + 1;
    const int t1Tenths  = *rc::gen::inRange(minTenths, 501).as("t1_tenths");
    const int t2Tenths  = *rc::gen::inRange(t1Tenths + 1, 1001).as("t2_tenths");

    const double t1 = t1Tenths / 10.0;
    const double t2 = t2Tenths / 10.0;

    Task task1 = makeTask(urgency, t1, 1.0, 0.0, 0.0, 0.0);
    Task task2 = makeTask(urgency, t2, 1.0, 0.0, 0.0, 0.0);

    Vector2 f1 = ClassicalEngine::computeForces(task1, {0.0, 0.0});
    Vector2 f2 = ClassicalEngine::computeForces(task2, {0.0, 0.0});

    // Shorter deadline → greater force (unless both clamped to MAX)
    // If both are clamped they are equal, which is still acceptable
    RC_ASSERT(f1.x >= f2.x);
}

// -----------------------------------------------------------------------
// Property 5: Friction Opposes Motion
// Feature: classical-mechanics-task-system, Property 5: Friction Opposes Motion
// Validates: Requirements 3.1, 3.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(PhysicsProperties, FrictionOpposesMotion, ()) {
    const double mass = *rc::gen::inRange(1, 101).as("mass_int") / 10.0;
    const double kf   = *rc::gen::inRange(1, 11).as("kf_int") / 10.0; // > 0 so friction exists
    // velocity components in [-10, 10], but ensure magnitude > 0.001
    // Use integers in [-100, 100] / 10.0 and reject near-zero
    const int vxInt = *rc::gen::inRange(-100, 101).as("vx_int");
    const int vyInt = *rc::gen::inRange(-100, 101).as("vy_int");
    const double vx = vxInt / 10.0;
    const double vy = vyInt / 10.0;

    RC_PRE(std::sqrt(vx * vx + vy * vy) > 0.001);

    // Zero deadline force to isolate friction
    Task task = makeTask(0.0, 10.0, mass, kf, vx, vy);

    Vector2 force = ClassicalEngine::computeForces(task, {vx, vy});

    // Friction force is the entire net force here (deadline urgency = 0)
    double dotProduct = force.x * vx + force.y * vy;
    RC_ASSERT(dotProduct < 0.0);
}

// -----------------------------------------------------------------------
// Property 6: Friction Force Magnitude
// Feature: classical-mechanics-task-system, Property 6: Friction Force Magnitude
// Validates: Requirements 3.2
// -----------------------------------------------------------------------
RC_GTEST_PROP(PhysicsProperties, FrictionForceMagnitude, ()) {
    const double mass = *rc::gen::inRange(1, 101).as("mass_int") / 10.0;
    const double kf   = *rc::gen::inRange(1, 11).as("kf_int") / 10.0;
    const int vxInt   = *rc::gen::inRange(-100, 101).as("vx_int");
    const int vyInt   = *rc::gen::inRange(-100, 101).as("vy_int");
    const double vx   = vxInt / 10.0;
    const double vy   = vyInt / 10.0;

    RC_PRE(std::sqrt(vx * vx + vy * vy) > 0.001);

    // Zero deadline force to isolate friction
    Task task = makeTask(0.0, 10.0, mass, kf, vx, vy);

    Vector2 force = ClassicalEngine::computeForces(task, {vx, vy});

    double actualMag   = std::sqrt(force.x * force.x + force.y * force.y);
    double expectedMag = kf * mass * Config::GRAVITY_CONSTANT;

    RC_ASSERT(std::abs(actualMag - expectedMag) < 1e-9);
}
