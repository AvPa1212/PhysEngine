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
