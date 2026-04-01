#include <gtest/gtest.h>
#include "core/SimulationEngine.hpp"

// Helper: build a default Task
static Task makeTask() {
    Task t;
    t.mass = 1.0;
    t.urgencyConstant = 100.0;
    t.deadlineTime = 10.0;
    t.kineticFriction = 0.3;
    return t;
}

// ---- Constructor reserves capacity >= 100 (Requirements 8.1) ----

TEST(SimulationEngineTest, ConstructorReservesCapacity) {
    SimulationEngine engine;
    EXPECT_GE(engine.tasks.capacity(), 100u);
}

// ---- Adding tasks increases collection size (Requirements 8.2) ----

TEST(SimulationEngineTest, AddingTaskIncreasesSize) {
    SimulationEngine engine;
    EXPECT_EQ(engine.tasks.size(), 0u);

    engine.tasks.push_back(makeTask());
    EXPECT_EQ(engine.tasks.size(), 1u);

    engine.tasks.push_back(makeTask());
    EXPECT_EQ(engine.tasks.size(), 2u);
}

// ---- Removing tasks decreases collection size (Requirements 8.3, 8.4) ----

TEST(SimulationEngineTest, RemovingTaskDecreasesSize) {
    SimulationEngine engine;
    engine.tasks.push_back(makeTask());
    engine.tasks.push_back(makeTask());
    EXPECT_EQ(engine.tasks.size(), 2u);

    engine.tasks.erase(engine.tasks.begin());
    EXPECT_EQ(engine.tasks.size(), 1u);
}

// ---- update() increments stepCount for all tasks (Requirements 8.5) ----

TEST(SimulationEngineTest, UpdateProcessesAllTasks) {
    SimulationEngine engine;
    engine.tasks.push_back(makeTask());
    engine.tasks.push_back(makeTask());
    engine.tasks.push_back(makeTask());

    // All tasks start at stepCount == 0
    for (const auto& t : engine.tasks) {
        EXPECT_EQ(t.stepCount, 0);
    }

    engine.update();

    // After one update, all tasks should have stepCount == 1
    for (const auto& t : engine.tasks) {
        EXPECT_EQ(t.stepCount, 1);
    }
}
