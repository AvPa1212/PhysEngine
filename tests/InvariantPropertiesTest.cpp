// InvariantPropertiesTest.cpp
// Property-based tests for task creation invariants
// Feature: classical-mechanics-task-system

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/Task.hpp"
#include <cmath>

// -----------------------------------------------------------------------
// Property 13: Task Creation Parameters Preserved
// Feature: classical-mechanics-task-system, Property 13: Task Creation Parameters Preserved
// Validates: Requirements 7.4, 7.5
//
// For any valid mass M > 0 and deadline D > 0, creating a Task and setting
// mass = M and deadlineTime = D SHALL result in task.mass == M and
// task.deadlineTime == D.
// -----------------------------------------------------------------------
RC_GTEST_PROP(InvariantProperties, TaskCreationParametersPreserved, ()) {
    // Generate random valid mass: positive, in range [0.01, 1000.0]
    const double mass = *rc::gen::inRange(1, 100001).as("mass_int") / 100.0;
    // Generate random valid deadline: positive, in range [0.01, 1000.0]
    const double deadline = *rc::gen::inRange(1, 100001).as("deadline_int") / 100.0;

    Task task;
    task.mass         = mass;
    task.deadlineTime = deadline;

    RC_ASSERT(std::abs(task.mass         - mass)     < 1e-12);
    RC_ASSERT(std::abs(task.deadlineTime - deadline) < 1e-12);
}

// -----------------------------------------------------------------------
// Property 14: Default Parameter Application
// Feature: classical-mechanics-task-system, Property 14: Default Parameter Application
// Validates: Requirements 7.6, 7.7
//
// A Task created with the default constructor SHALL have staticFriction == 0.5
// and kineticFriction == 0.3. When custom friction values are set, those
// values SHALL be used.
// -----------------------------------------------------------------------
RC_GTEST_PROP(InvariantProperties, DefaultParameterApplication, ()) {
    // Part 1: default constructor gives the expected friction defaults
    Task defaultTask;
    RC_ASSERT(std::abs(defaultTask.staticFriction  - 0.5) < 1e-12);
    RC_ASSERT(std::abs(defaultTask.kineticFriction - 0.3) < 1e-12);

    // Part 2: custom friction values are preserved
    // Generate non-negative friction values in [0.0, 2.0]
    const double customKinetic = *rc::gen::inRange(0, 201).as("kf_int") / 100.0;
    const double customStatic  = *rc::gen::inRange(0, 201).as("sf_int") / 100.0;

    Task customTask;
    customTask.kineticFriction = customKinetic;
    customTask.staticFriction  = customStatic;

    RC_ASSERT(std::abs(customTask.kineticFriction - customKinetic) < 1e-12);
    RC_ASSERT(std::abs(customTask.staticFriction  - customStatic)  < 1e-12);
}
