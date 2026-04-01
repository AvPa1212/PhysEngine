// StabilityPropertiesTest.cpp
// Property-based tests for stability and isolation properties
// Feature: classical-mechanics-task-system

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/ClassicalEngine.hpp"
#include "physics/Task.hpp"
#include "core/Config.hpp"
#include <cmath>

// -----------------------------------------------------------------------
// Property 21: State Remains Finite
// Feature: classical-mechanics-task-system, Property 21: State Remains Finite
// Validates: Requirements 13.4, 13.5, 13.6
//
// For any valid initial state (finite position, velocity, mass > 0),
// integrating for any number of steps SHALL keep position, velocity,
// and acceleration finite (not NaN or Inf).
// -----------------------------------------------------------------------
RC_GTEST_PROP(StabilityProperties, StateRemainsFinite, ()) {
    // Generate random valid initial state
    const double px   = *rc::gen::inRange(-1000, 1001).as("px_int") / 10.0;
    const double py   = *rc::gen::inRange(-1000, 1001).as("py_int") / 10.0;
    const double vx   = *rc::gen::inRange(-100, 101).as("vx_int") / 10.0;
    const double vy   = *rc::gen::inRange(-100, 101).as("vy_int") / 10.0;
    // mass > 0: [0.1, 100.0]
    const double mass = *rc::gen::inRange(1, 1001).as("mass_int") / 10.0;
    // urgency in [1, 1000]
    const double urgency = *rc::gen::inRange(1, 1001).as("urgency_int") * 1.0;
    // deadline in [MIN_DEADLINE_TIME, 100.0]
    const double deadline = *rc::gen::inRange(
        static_cast<int>(Config::MIN_DEADLINE_TIME * 100) + 1, 10001
    ).as("deadline_hundredths") / 100.0;
    // steps in [1, 1000]
    const int steps = *rc::gen::inRange(1, 1001).as("steps");

    Task task;
    task.position        = {px, py};
    task.velocity        = {vx, vy};
    task.mass            = mass;
    task.urgencyConstant = urgency;
    task.deadlineTime    = deadline;
    task.kineticFriction = 0.3;

    for (int i = 0; i < steps; ++i) {
        ClassicalEngine::integrateRK4(task);
    }

    RC_ASSERT(std::isfinite(task.position.x));
    RC_ASSERT(std::isfinite(task.position.y));
    RC_ASSERT(std::isfinite(task.velocity.x));
    RC_ASSERT(std::isfinite(task.velocity.y));
    RC_ASSERT(std::isfinite(task.acceleration.x));
    RC_ASSERT(std::isfinite(task.acceleration.y));
}

// -----------------------------------------------------------------------
// Property 20: Time Scaling Multiplies Displacement
// Feature: classical-mechanics-task-system, Property 20: Time Scaling Multiplies Displacement
// Validates: Requirements 12.4
//
// Integrating for more steps produces greater or equal displacement than
// fewer steps (monotonic scaling). For a task with zero friction and
// constant force, displacement scales monotonically with step count.
// -----------------------------------------------------------------------
RC_GTEST_PROP(StabilityProperties, TimeScalingMultipliesDisplacement, ()) {
    // Use zero friction and large deadline so force is nearly constant
    // urgency in [1, 100], deadline large [50, 200] so force stays small
    const double urgency  = *rc::gen::inRange(1, 101).as("urgency_int") * 1.0;
    const double deadline = *rc::gen::inRange(500, 2001).as("deadline_int") / 10.0;
    // N steps in [1, 50] to keep linearity regime
    const int N = *rc::gen::inRange(1, 51).as("N_steps");

    auto makeBaseTask = [&]() {
        Task t;
        t.position        = {0.0, 0.0};
        t.velocity        = {0.0, 0.0};
        t.mass            = 1.0;
        t.urgencyConstant = urgency;
        t.deadlineTime    = deadline;
        t.kineticFriction = 0.0;
        t.staticFriction  = 0.0;
        return t;
    };

    // Integrate N steps
    Task taskN = makeBaseTask();
    for (int i = 0; i < N; ++i) {
        ClassicalEngine::integrateRK4(taskN);
    }
    const double dispN = std::sqrt(taskN.position.x * taskN.position.x +
                                   taskN.position.y * taskN.position.y);

    // Integrate 2N steps from same initial state
    Task task2N = makeBaseTask();
    for (int i = 0; i < 2 * N; ++i) {
        ClassicalEngine::integrateRK4(task2N);
    }
    const double disp2N = std::sqrt(task2N.position.x * task2N.position.x +
                                    task2N.position.y * task2N.position.y);

    // Displacement must be monotonically non-decreasing with more steps
    RC_ASSERT(disp2N >= dispN);
}

// -----------------------------------------------------------------------
// Property 22: Classical Engine Isolation
// Feature: classical-mechanics-task-system, Property 22: Classical Engine Isolation
// Validates: Requirements 15.3
//
// ClassicalEngine::integrateRK4() SHALL NOT modify non-classical fields:
// entropy, stressX, stressY, stressZ.
// -----------------------------------------------------------------------
RC_GTEST_PROP(StabilityProperties, ClassicalEngineIsolation, ()) {
    // Generate random non-classical state values
    const double entropy = *rc::gen::inRange(-100, 101).as("entropy_int") / 10.0;
    const double stressX = *rc::gen::inRange(-100, 101).as("stressX_int") / 10.0;
    const double stressY = *rc::gen::inRange(-100, 101).as("stressY_int") / 10.0;
    const double stressZ = *rc::gen::inRange(-100, 101).as("stressZ_int") / 10.0;

    Task task;
    task.mass            = 1.0;
    task.urgencyConstant = 100.0;
    task.deadlineTime    = 10.0;
    task.kineticFriction = 0.3;
    task.entropy         = entropy;
    task.stressX         = stressX;
    task.stressY         = stressY;
    task.stressZ         = stressZ;

    ClassicalEngine::integrateRK4(task);

    RC_ASSERT(task.entropy == entropy);
    RC_ASSERT(task.stressX == stressX);
    RC_ASSERT(task.stressY == stressY);
    RC_ASSERT(task.stressZ == stressZ);
}

// -----------------------------------------------------------------------
// Property 1: Task Invariants Hold After Creation
// Feature: classical-mechanics-task-system, Property 1: Task Invariants Hold After Creation
// Validates: Requirements 1.9, 1.10
//
// For any valid task parameters (mass > 0, friction >= 0), after integration
// the mass SHALL remain > 0 and kineticFriction SHALL remain >= 0.
// -----------------------------------------------------------------------
RC_GTEST_PROP(StabilityProperties, TaskInvariantsHoldAfterCreation, ()) {
    // mass > 0: [0.01, 1000.0]
    const double mass = *rc::gen::inRange(1, 100001).as("mass_int") / 100.0;
    // friction >= 0: [0.0, 2.0]
    const double kf = *rc::gen::inRange(0, 201).as("kf_int") / 100.0;
    // steps in [1, 100]
    const int steps = *rc::gen::inRange(1, 101).as("steps");

    Task task;
    task.mass            = mass;
    task.kineticFriction = kf;
    task.urgencyConstant = 100.0;
    task.deadlineTime    = 10.0;

    for (int i = 0; i < steps; ++i) {
        ClassicalEngine::integrateRK4(task);
    }

    RC_ASSERT(task.mass > 0.0);
    RC_ASSERT(task.kineticFriction >= 0.0);
}
