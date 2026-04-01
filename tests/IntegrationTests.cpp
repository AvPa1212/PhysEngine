// IntegrationTests.cpp
// Integration tests for multi-physics coexistence, C API bridge (simulating WASM/React and Python ctypes)
// Requirements: 15.1, 15.2, 15.3, 15.4, 15.5, 10.1, 10.2, 10.3, 10.4, 11.5

#include <gtest/gtest.h>
#include "physics/Task.hpp"
#include "physics/ClassicalEngine.hpp"
#include "physics/MomentumBridge.h"
#include <cmath>

// ============================================================
// Task 20.1: Multi-physics coexistence
// Requirements: 15.1, 15.2, 15.3, 15.4, 15.5
// ============================================================

TEST(IntegrationTest, MultiPhysicsCoexistence_ClassicalUpdatesOthersUnchanged) {
    Task task;

    // Set up classical state
    task.position   = { 1.0, 2.0 };
    task.velocity   = { 3.0, 0.5 };
    task.mass       = 2.0;
    task.deadlineTime = 5.0;

    // Set placeholder quantum/thermo/chaos state to known non-zero values
    task.entropy = 42.0;
    task.stressX = 7.0;
    task.stressY = 8.0;
    task.stressZ = 9.0;

    // Snapshot non-classical state before integration
    const double entropyBefore = task.entropy;
    const double stressXBefore = task.stressX;
    const double stressYBefore = task.stressY;
    const double stressZBefore = task.stressZ;
    const int    stepsBefore   = task.stepCount;

    // Snapshot classical state before integration
    const double posXBefore = task.position.x;

    // Run classical integration only
    ClassicalEngine::integrateRK4(task);

    // Classical state must have updated
    EXPECT_NE(task.position.x, posXBefore) << "position.x should change after RK4 integration";
    EXPECT_EQ(task.stepCount, stepsBefore + 1) << "stepCount should increment by 1";

    // Non-classical state must remain unchanged (isolation requirement)
    EXPECT_DOUBLE_EQ(task.entropy, entropyBefore) << "entropy must not be modified by ClassicalEngine";
    EXPECT_DOUBLE_EQ(task.stressX, stressXBefore) << "stressX must not be modified by ClassicalEngine";
    EXPECT_DOUBLE_EQ(task.stressY, stressYBefore) << "stressY must not be modified by ClassicalEngine";
    EXPECT_DOUBLE_EQ(task.stressZ, stressZBefore) << "stressZ must not be modified by ClassicalEngine";
}

TEST(IntegrationTest, MultiPhysicsCoexistence_MultipleStepsPreserveNonClassical) {
    Task task;
    task.velocity     = { 1.0, 1.0 };
    task.mass         = 1.0;
    task.deadlineTime = 10.0;
    task.entropy      = 100.0;
    task.stressX      = 3.14;
    task.stressY      = 2.71;
    task.stressZ      = 1.41;

    const double entropyBefore = task.entropy;
    const double stressXBefore = task.stressX;
    const double stressYBefore = task.stressY;
    const double stressZBefore = task.stressZ;

    // Run 10 classical integration steps
    for (int i = 0; i < 10; ++i) {
        ClassicalEngine::integrateRK4(task);
    }

    EXPECT_EQ(task.stepCount, 10);
    EXPECT_DOUBLE_EQ(task.entropy, entropyBefore);
    EXPECT_DOUBLE_EQ(task.stressX, stressXBefore);
    EXPECT_DOUBLE_EQ(task.stressY, stressYBefore);
    EXPECT_DOUBLE_EQ(task.stressZ, stressZBefore);
}

// ============================================================
// Task 20.2: WebAssembly + React (simulated via C API bridge)
// Requirements: 10.1, 10.2, 10.3, 10.4
// ============================================================

TEST(IntegrationTest, CApiBridge_CreateAndIntegrate) {
    // Simulate JS/WASM interaction: create task via bridge
    Task* t = Task_Create();
    ASSERT_NE(t, nullptr);

    // Set position, velocity, mass via bridge setters (as JS would)
    Task_SetPosition(t, 5.0, -3.0);
    Task_SetVelocity(t, 2.0, 0.0);
    Task_SetMass(t, 1.5);

    // Verify setters took effect via getters
    EXPECT_NEAR(Task_GetPositionX(t), 5.0,  1e-12);
    EXPECT_NEAR(Task_GetPositionY(t), -3.0, 1e-12);
    EXPECT_NEAR(Task_GetVelocityX(t), 2.0,  1e-12);
    EXPECT_NEAR(Task_GetMass(t),      1.5,  1e-12);

    const double posXBefore = Task_GetPositionX(t);
    const int    stepBefore = Task_GetStepCount(t);

    // Call Engine_IntegrateClassical via bridge (as WASM export would)
    Engine_IntegrateClassical(t);

    // Verify state updated correctly via bridge getters
    EXPECT_NE(Task_GetPositionX(t), posXBefore) << "position.x should change after integration";
    EXPECT_EQ(Task_GetStepCount(t), stepBefore + 1) << "stepCount should increment";

    Task_Destroy(t);
}

TEST(IntegrationTest, CApiBridge_MultipleTasksIndependent) {
    // Simulate React managing multiple tasks via WASM bridge
    Task* t1 = Task_Create();
    Task* t2 = Task_Create();
    ASSERT_NE(t1, nullptr);
    ASSERT_NE(t2, nullptr);

    Task_SetPosition(t1, 0.0, 0.0);
    Task_SetVelocity(t1, 1.0, 0.0);
    Task_SetMass(t1, 1.0);

    Task_SetPosition(t2, 10.0, 10.0);
    Task_SetVelocity(t2, -1.0, 0.0);
    Task_SetMass(t2, 2.0);

    Engine_IntegrateClassical(t1);
    Engine_IntegrateClassical(t2);

    // Both tasks should have advanced independently
    EXPECT_EQ(Task_GetStepCount(t1), 1);
    EXPECT_EQ(Task_GetStepCount(t2), 1);

    // Positions should differ from each other (different initial conditions)
    EXPECT_NE(Task_GetPositionX(t1), Task_GetPositionX(t2));

    Task_Destroy(t1);
    Task_Destroy(t2);
}

// ============================================================
// Task 20.3: Python ctypes binding (simulated via C API)
// Requirements: 11.5
// ============================================================

TEST(IntegrationTest, PythonCtypesSimulation_StepCountIncrements) {
    // Simulate Python ctypes: load bridge functions and call them directly
    Task* t = Task_Create();
    ASSERT_NE(t, nullptr);

    EXPECT_EQ(Task_GetStepCount(t), 0);

    Engine_IntegrateClassical(t);
    EXPECT_EQ(Task_GetStepCount(t), 1);

    Engine_IntegrateClassical(t);
    EXPECT_EQ(Task_GetStepCount(t), 2);

    Task_Destroy(t);
}

TEST(IntegrationTest, PythonCtypesSimulation_PositionChanges) {
    // Simulate Python ctypes workflow: create, configure, integrate, read back
    Task* t = Task_Create();
    ASSERT_NE(t, nullptr);

    Task_SetVelocity(t, 1.0, 0.0);
    Task_SetMass(t, 1.0);

    const double posXBefore = Task_GetPositionX(t);

    Engine_IntegrateClassical(t);

    const double posXAfter = Task_GetPositionX(t);
    EXPECT_NE(posXAfter, posXBefore) << "position.x should change after integration with non-zero velocity";
    EXPECT_EQ(Task_GetStepCount(t), 1);

    Task_Destroy(t);
}
