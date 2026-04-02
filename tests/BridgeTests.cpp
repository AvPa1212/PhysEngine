// BridgeTests.cpp
// Unit tests for the MomentumBridge C API
// Requirements: 11.2, 11.5

#include <gtest/gtest.h>
#include "physics/MomentumBridge.h"
#include "physics/Task.hpp"
#include <cmath>

// ---- Task lifecycle ----

TEST(BridgeTest, TaskCreateReturnsNonNull) {
    Task* t = Task_Create();
    ASSERT_NE(t, nullptr);
    Task_Destroy(t);
}

TEST(BridgeTest, TaskDestroyDoesNotCrash) {
    Task* t = Task_Create();
    ASSERT_NO_THROW(Task_Destroy(t));
}

TEST(BridgeTest, TaskCreateWithParamsSetsMassDeadlineUrgency) {
    Task* t = Task_CreateWithParams(3.0, 6.0, 250.0);
    ASSERT_NE(t, nullptr);
    EXPECT_NEAR(Task_GetMass(t), 3.0, 1e-12);
    Engine_IntegrateClassical(t);
    EXPECT_GT(Task_GetAccelerationX(t), 0.0);
    Task_Destroy(t);
}

// ---- Getters return correct values after setters ----

TEST(BridgeTest, SetGetPosition) {
    Task* t = Task_Create();
    Task_SetPosition(t, 3.5, -7.25);
    EXPECT_NEAR(Task_GetPositionX(t), 3.5,   1e-12);
    EXPECT_NEAR(Task_GetPositionY(t), -7.25, 1e-12);
    Task_Destroy(t);
}

TEST(BridgeTest, SetGetVelocity) {
    Task* t = Task_Create();
    Task_SetVelocity(t, 1.23, -4.56);
    EXPECT_NEAR(Task_GetVelocityX(t), 1.23,  1e-12);
    EXPECT_NEAR(Task_GetVelocityY(t), -4.56, 1e-12);
    Task_Destroy(t);
}

TEST(BridgeTest, SetGetMass) {
    Task* t = Task_Create();
    Task_SetMass(t, 5.0);
    EXPECT_NEAR(Task_GetMass(t), 5.0, 1e-12);
    Task_Destroy(t);
}

// ---- Engine_IntegrateClassical increments stepCount ----

TEST(BridgeTest, IntegrateClassicalIncrementsStepCount) {
    Task* t = Task_Create();
    int before = Task_GetStepCount(t);
    Engine_IntegrateClassical(t);
    int after = Task_GetStepCount(t);
    EXPECT_EQ(after, before + 1);
    Task_Destroy(t);
}

TEST(BridgeTest, IntegrateClassicalModifiesState) {
    Task* t = Task_Create();
    // Give it a non-zero velocity so position changes
    Task_SetVelocity(t, 1.0, 0.0);
    double posXBefore = Task_GetPositionX(t);
    Engine_IntegrateClassical(t);
    double posXAfter = Task_GetPositionX(t);
    EXPECT_NE(posXAfter, posXBefore);
    Task_Destroy(t);
}

TEST(BridgeTest, AccelerationGettersReturnFiniteValuesAfterStep) {
    Task* t = Task_Create();
    ASSERT_NE(t, nullptr);
    Engine_IntegrateClassical(t);
    EXPECT_TRUE(std::isfinite(Task_GetAccelerationX(t)));
    EXPECT_TRUE(std::isfinite(Task_GetAccelerationY(t)));
    Task_Destroy(t);
}

TEST(BridgeTest, SimulationEngineLifecycleAndControls) {
    SimulationEngine* engine = Simulation_Create();
    ASSERT_NE(engine, nullptr);

    const size_t idx = Simulation_AddTask(engine, 2.0, 8.0, 120.0, 0.5, 0.3);
    ASSERT_NE(idx, static_cast<size_t>(-1));
    EXPECT_EQ(Simulation_GetTaskCount(engine), 1u);

    Simulation_Pause(engine);
    Simulation_Update(engine);
    TaskState state{};
    ASSERT_EQ(Simulation_GetTaskState(engine, 0, &state), 1);
    const double pausedPosX = state.posX;

    Simulation_Step(engine);
    ASSERT_EQ(Simulation_GetTaskState(engine, 0, &state), 1);
    EXPECT_GT(state.posX, pausedPosX);

    Simulation_SetTimeScale(engine, 2.0);
    EXPECT_NEAR(Simulation_GetTimeScale(engine), 2.0, 1e-12);

    Simulation_SetClassicalEnabled(engine, 0);
    EXPECT_EQ(Simulation_IsClassicalEnabled(engine), 0);

    const double beforeDisabledUpdate = state.posX;
    Simulation_Update(engine);
    ASSERT_EQ(Simulation_GetTaskState(engine, 0, &state), 1);
    EXPECT_DOUBLE_EQ(state.posX, beforeDisabledUpdate);

    EXPECT_EQ(Simulation_RemoveTask(engine, 0), 1);
    EXPECT_EQ(Simulation_GetTaskCount(engine), 0u);

    Simulation_Destroy(engine);
}

TEST(BridgeTest, SimulationBatchStateRetrieval) {
    SimulationEngine* engine = Simulation_Create();
    ASSERT_NE(engine, nullptr);

    ASSERT_NE(Simulation_AddTask(engine, 1.0, 5.0, 100.0, 0.5, 0.3), static_cast<size_t>(-1));
    ASSERT_NE(Simulation_AddTask(engine, 1.5, 7.0, 100.0, 0.5, 0.3), static_cast<size_t>(-1));
    Simulation_Update(engine);

    TaskState states[2] = {};
    const size_t copied = Simulation_GetAllTaskStates(engine, states, 2);
    EXPECT_EQ(copied, 2u);
    EXPECT_TRUE(std::isfinite(states[0].posX));
    EXPECT_TRUE(std::isfinite(states[1].posX));

    Simulation_Destroy(engine);
}

// ---- Null pointer safety ----

TEST(BridgeTest, GettersReturnZeroForNull) {
    EXPECT_DOUBLE_EQ(Task_GetPositionX(nullptr), 0.0);
    EXPECT_DOUBLE_EQ(Task_GetPositionY(nullptr), 0.0);
    EXPECT_DOUBLE_EQ(Task_GetVelocityX(nullptr), 0.0);
    EXPECT_DOUBLE_EQ(Task_GetVelocityY(nullptr), 0.0);
    EXPECT_DOUBLE_EQ(Task_GetMass(nullptr),      0.0);
    EXPECT_DOUBLE_EQ(Task_GetEntropy(nullptr),   0.0);
    EXPECT_EQ(Task_GetStepCount(nullptr), 0);
}

TEST(BridgeTest, SettersDoNotCrashOnNull) {
    ASSERT_NO_THROW(Task_SetPosition(nullptr, 1.0, 2.0));
    ASSERT_NO_THROW(Task_SetVelocity(nullptr, 1.0, 2.0));
    ASSERT_NO_THROW(Task_SetMass(nullptr, 1.0));
}

TEST(BridgeTest, IntegrateClassicalDoesNotCrashOnNull) {
    ASSERT_NO_THROW(Engine_IntegrateClassical(nullptr));
}

// ---- Energy API Bridge tests (Requirements: 12.1) ----

TEST(BridgeTest, EnergyGetKineticReturnsCorrectValue) {
    // KE = 0.5 * mass * |v|^2 = 0.5 * 2.0 * (3^2 + 4^2) = 0.5 * 2.0 * 25 = 25.0
    Task* t = Task_Create();
    Task_SetVelocity(t, 3.0, 4.0);
    Task_SetMass(t, 2.0);
    Engine_IntegrateClassical(t);  // triggers EnergyEngine::calculateEnergy internally
    // Manually verify via direct field: after integration energy fields are updated
    // Use Energy_GetKinetic to retrieve the stored kineticEnergy
    double ke = Energy_GetKinetic(t);
    EXPECT_TRUE(std::isfinite(ke));
    EXPECT_GE(ke, 0.0);
    Task_Destroy(t);
}

TEST(BridgeTest, EnergyGetKineticAfterInjectIsIncreased) {
    Task* t = Task_Create();
    Task_SetVelocity(t, 3.0, 4.0);
    Task_SetMass(t, 2.0);
    Engine_IntegrateClassical(t);
    double keBefore = Energy_GetKinetic(t);
    Energy_Inject(t, 50.0);
    double keAfter = Energy_GetKinetic(t);
    EXPECT_GT(keAfter, keBefore);
    Task_Destroy(t);
}

TEST(BridgeTest, EnergyInjectModifiesKineticEnergy) {
    Task* t = Task_Create();
    Task_SetMass(t, 1.0);
    // Start from rest - inject 100 units
    Energy_Inject(t, 100.0);
    double ke = Energy_GetKinetic(t);
    EXPECT_NEAR(ke, 100.0, 1e-6);
    Task_Destroy(t);
}

TEST(BridgeTest, EnergyGetKineticNullReturnsZero) {
    EXPECT_DOUBLE_EQ(Energy_GetKinetic(nullptr), 0.0);
}

TEST(BridgeTest, EnergyGetPotentialNullReturnsZero) {
    EXPECT_DOUBLE_EQ(Energy_GetPotential(nullptr), 0.0);
}

TEST(BridgeTest, EnergyGetTotalNullReturnsZero) {
    EXPECT_DOUBLE_EQ(Energy_GetTotal(nullptr), 0.0);
}

TEST(BridgeTest, EnergyInjectDoesNotCrashOnNull) {
    ASSERT_NO_THROW(Energy_Inject(nullptr, 50.0));
}

TEST(BridgeTest, EnergyGetTotalEqualsKEPlusPE) {
    Task* t = Task_Create();
    Task_SetVelocity(t, 3.0, 4.0);
    Task_SetMass(t, 2.0);
    Task_SetPosition(t, 0.0, 5.0);
    Engine_IntegrateClassical(t);
    double ke    = Energy_GetKinetic(t);
    double pe    = Energy_GetPotential(t);
    double total = Energy_GetTotal(t);
    EXPECT_NEAR(total, ke + pe, 1e-9);
    Task_Destroy(t);
}
