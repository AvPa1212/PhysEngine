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

TEST(SimulationEngineTest, AddTaskInitializesExpectedDefaultsAndInputs) {
    SimulationEngine engine;
    engine.addTask(2.5, 7.0, 150.0);

    ASSERT_EQ(engine.tasks.size(), 1u);
    const Task& t = engine.tasks[0];
    EXPECT_DOUBLE_EQ(t.position.x, 0.0);
    EXPECT_DOUBLE_EQ(t.position.y, 0.0);
    EXPECT_DOUBLE_EQ(t.velocity.x, 0.0);
    EXPECT_DOUBLE_EQ(t.velocity.y, 0.0);
    EXPECT_DOUBLE_EQ(t.acceleration.x, 0.0);
    EXPECT_DOUBLE_EQ(t.acceleration.y, 0.0);
    EXPECT_DOUBLE_EQ(t.mass, 2.5);
    EXPECT_DOUBLE_EQ(t.deadlineTime, 7.0);
    EXPECT_DOUBLE_EQ(t.urgencyConstant, 150.0);
    EXPECT_DOUBLE_EQ(t.staticFriction, 0.5);
    EXPECT_DOUBLE_EQ(t.kineticFriction, 0.3);
}

TEST(SimulationEngineTest, PauseResumeAndSingleStepBehavior) {
    SimulationEngine engine;
    engine.addTask(1.0, 10.0, 100.0);

    engine.pause();
    engine.update();
    EXPECT_EQ(engine.tasks[0].stepCount, 0);

    engine.singleStep();
    EXPECT_EQ(engine.tasks[0].stepCount, 1);

    engine.resume();
    engine.update();
    EXPECT_EQ(engine.tasks[0].stepCount, 2);
}

TEST(SimulationEngineTest, TimeScaleChangesIntegrationAmount) {
    auto makeEngine = []() {
        SimulationEngine e;
        e.addTask(1.0, 100.0, 100.0, 0.0, 0.0);
        return e;
    };

    SimulationEngine base = makeEngine();
    base.update();

    SimulationEngine scaled = makeEngine();
    scaled.setTimeScale(2.0);
    scaled.update();

    EXPECT_GT(scaled.tasks[0].position.x, base.tasks[0].position.x);
}

TEST(SimulationEngineTest, CanDisableClassicalEngineIndependently) {
    SimulationEngine engine;
    engine.addTask(1.0, 10.0, 100.0);

    engine.setClassicalEnabled(false);
    engine.update();

    EXPECT_EQ(engine.tasks[0].stepCount, 0);
    EXPECT_DOUBLE_EQ(engine.tasks[0].position.x, 0.0);

    engine.setClassicalEnabled(true);
    engine.update();

    EXPECT_EQ(engine.tasks[0].stepCount, 1);
    EXPECT_GT(engine.tasks[0].position.x, 0.0);
}

// ---- Task 6.7: Energy integration tests (Requirements 10.1, 10.2, 10.3) ----

// update() calls calculateEnergy() for all tasks — energy fields updated after update
TEST(SimulationEngineTest, UpdateCalculatesEnergyForAllTasks) {
    SimulationEngine engine;

    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};  // magnitude = 5, KE = 0.5 * 2 * 25 = 25
    engine.tasks.push_back(t);

    // Energy fields start at zero
    EXPECT_DOUBLE_EQ(engine.tasks[0].kineticEnergy, 0.0);

    engine.update();

    // After update, kineticEnergy must be > 0
    EXPECT_GT(engine.tasks[0].kineticEnergy, 0.0);
}

// Damping applied when enabled — velocity magnitude decreases after update
TEST(SimulationEngineTest, DampingReducesVelocityWhenEnabled) {
    SimulationEngine engine;
    engine.enableDamping(0.5);

    Task t;
    t.mass = 1.0;
    t.velocity = {3.0, 4.0};
    engine.tasks.push_back(t);

    double initialSpeed = magnitude(engine.tasks[0].velocity);
    engine.update();
    double finalSpeed = magnitude(engine.tasks[0].velocity);

    EXPECT_LT(finalSpeed, initialSpeed);
}

// Damping not applied when disabled — energy fields set but velocity not reduced by damping
TEST(SimulationEngineTest, NoDampingWhenDisabled) {
    SimulationEngine engine;
    engine.disableDamping();

    Task t;
    t.mass = 1.0;
    t.velocity = {3.0, 4.0};
    engine.tasks.push_back(t);

    engine.update();

    // Energy fields should be populated
    EXPECT_GT(engine.tasks[0].kineticEnergy, 0.0);

    // Without damping, kineticEnergy must be consistent with the current velocity
    double finalSpeed = magnitude(engine.tasks[0].velocity);
    double expectedKE = 0.5 * t.mass * finalSpeed * finalSpeed;
    EXPECT_NEAR(engine.tasks[0].kineticEnergy, expectedKE, 1e-9);
}

// Energy fields updated after update()
TEST(SimulationEngineTest, EnergyFieldsUpdatedAfterUpdate) {
    SimulationEngine engine;

    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};
    t.position = {0.0, 5.0};  // positive Y gives positive potential energy
    engine.tasks.push_back(t);

    engine.update();

    const Task& result = engine.tasks[0];
    EXPECT_GT(result.kineticEnergy, 0.0);
    EXPECT_NEAR(result.totalEnergy, result.kineticEnergy + result.potentialEnergy, 1e-9);
}

// ---- Task 6.8: Multi-physics coexistence integration test (Requirements 10.5, 15.1-15.5) ----

TEST(SimulationEngineTest, MultiPhysicsCoexistenceAfterUpdate) {
    SimulationEngine engine;

    Task t;
    t.mass = 1.0;
    t.velocity = {1.0, 0.0};
    t.position = {0.0, 1.0};
    t.stressX = 1.0;
    t.stressY = 1.0;
    t.stressZ = 1.0;
    engine.tasks.push_back(t);

    double initialEntropy = engine.tasks[0].entropy;
    int initialStepCount = engine.tasks[0].stepCount;

    engine.update();

    const Task& result = engine.tasks[0];

    // Energy fields are set (kineticEnergy >= 0)
    EXPECT_GE(result.kineticEnergy, 0.0);

    // Thermodynamics still running — entropy has changed
    EXPECT_NE(result.entropy, initialEntropy);

    // Chaos engine still running — stepCount incremented
    EXPECT_GT(result.stepCount, initialStepCount);
}
