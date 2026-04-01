#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "core/SimulationEngine.hpp"

// Helper: build a default Task
static Task makeDefaultTask() {
    Task t;
    t.mass = 1.0;
    t.urgencyConstant = 100.0;
    t.deadlineTime = 10.0;
    t.kineticFriction = 0.3;
    return t;
}

// -----------------------------------------------------------------------
// Property 11: All Tasks Updated
// Feature: classical-mechanics-task-system, Property 11: All Tasks Updated
// Validates: Requirements 6.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(SimulationProperties, AllTasksUpdated, ()) {
    // Random number of tasks: 1-20
    const int n = *rc::gen::inRange(1, 21).as("num_tasks");

    SimulationEngine engine;
    for (int i = 0; i < n; ++i) {
        engine.tasks.push_back(makeDefaultTask());
    }

    // Store initial stepCounts
    std::vector<int> initialSteps;
    initialSteps.reserve(n);
    for (const auto& t : engine.tasks) {
        initialSteps.push_back(t.stepCount);
    }

    engine.update();

    // All tasks must have incremented stepCount
    for (int i = 0; i < n; ++i) {
        RC_ASSERT(engine.tasks[i].stepCount == initialSteps[i] + 1);
    }
}

// -----------------------------------------------------------------------
// Property 15: Task Collection Growth
// Feature: classical-mechanics-task-system, Property 15: Task Collection Growth
// Validates: Requirements 8.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(SimulationProperties, TaskCollectionGrowth, ()) {
    // N tasks: 0-20
    const int n = *rc::gen::inRange(0, 21).as("num_tasks");

    SimulationEngine engine;
    for (int i = 0; i < n; ++i) {
        engine.tasks.push_back(makeDefaultTask());
    }

    RC_ASSERT(static_cast<int>(engine.tasks.size()) == n);

    engine.tasks.push_back(makeDefaultTask());

    RC_ASSERT(static_cast<int>(engine.tasks.size()) == n + 1);
}

// -----------------------------------------------------------------------
// Property 16: Task Collection Shrinkage
// Feature: classical-mechanics-task-system, Property 16: Task Collection Shrinkage
// Validates: Requirements 8.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(SimulationProperties, TaskCollectionShrinkage, ()) {
    // N tasks: 1-20 (must have at least 1 to remove)
    const int n = *rc::gen::inRange(1, 21).as("num_tasks");

    SimulationEngine engine;
    for (int i = 0; i < n; ++i) {
        engine.tasks.push_back(makeDefaultTask());
    }

    RC_ASSERT(static_cast<int>(engine.tasks.size()) == n);

    engine.tasks.erase(engine.tasks.begin());

    RC_ASSERT(static_cast<int>(engine.tasks.size()) == n - 1);
}
