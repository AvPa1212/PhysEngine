#include <gtest/gtest.h>
#include "physics/Task.hpp"

// ---- Default friction coefficients (Requirements 1.9, 1.10) ----

TEST(TaskTest, DefaultStaticFrictionIs0_5) {
    Task t;
    EXPECT_DOUBLE_EQ(t.staticFriction, 0.5);
}

TEST(TaskTest, DefaultKineticFrictionIs0_3) {
    Task t;
    EXPECT_DOUBLE_EQ(t.kineticFriction, 0.3);
}

TEST(TaskTest, FrictionCoefficientsAreNonNegativeByDefault) {
    Task t;
    EXPECT_GE(t.staticFriction, 0.0);
    EXPECT_GE(t.kineticFriction, 0.0);
}

// ---- Default urgency constant ----

TEST(TaskTest, DefaultUrgencyConstantIs100) {
    Task t;
    EXPECT_DOUBLE_EQ(t.urgencyConstant, 100.0);
}

// ---- Default mass (Requirements 7.6, 7.7) ----

TEST(TaskTest, DefaultMassIs1) {
    Task t;
    EXPECT_DOUBLE_EQ(t.mass, 1.0);
}

TEST(TaskTest, MassCanBeSetToPositiveValue) {
    Task t;
    t.mass = 5.0;
    EXPECT_DOUBLE_EQ(t.mass, 5.0);
}

TEST(TaskTest, MassCanBeSetToSmallPositiveValue) {
    Task t;
    t.mass = 0.001;
    EXPECT_DOUBLE_EQ(t.mass, 0.001);
}

// ---- Default kinematic vectors ----

TEST(TaskTest, DefaultPositionIsOrigin) {
    Task t;
    EXPECT_DOUBLE_EQ(t.position.x, 0.0);
    EXPECT_DOUBLE_EQ(t.position.y, 0.0);
}

TEST(TaskTest, DefaultVelocityIsZero) {
    Task t;
    EXPECT_DOUBLE_EQ(t.velocity.x, 0.0);
    EXPECT_DOUBLE_EQ(t.velocity.y, 0.0);
}

TEST(TaskTest, DefaultAccelerationIsZero) {
    Task t;
    EXPECT_DOUBLE_EQ(t.acceleration.x, 0.0);
    EXPECT_DOUBLE_EQ(t.acceleration.y, 0.0);
}

// ---- Default step count ----

TEST(TaskTest, DefaultStepCountIsZero) {
    Task t;
    EXPECT_EQ(t.stepCount, 0);
}

// ---- All fields are accessible (read/write) ----

TEST(TaskTest, AllFieldsAreReadWriteAccessible) {
    Task t;

    t.staticFriction = 0.7;
    t.kineticFriction = 0.4;
    t.urgencyConstant = 200.0;
    t.mass = 2.5;
    t.position = {1.0, 2.0};
    t.velocity = {3.0, 4.0};
    t.acceleration = {0.5, -0.5};
    t.stepCount = 42;

    EXPECT_DOUBLE_EQ(t.staticFriction, 0.7);
    EXPECT_DOUBLE_EQ(t.kineticFriction, 0.4);
    EXPECT_DOUBLE_EQ(t.urgencyConstant, 200.0);
    EXPECT_DOUBLE_EQ(t.mass, 2.5);
    EXPECT_DOUBLE_EQ(t.position.x, 1.0);
    EXPECT_DOUBLE_EQ(t.position.y, 2.0);
    EXPECT_DOUBLE_EQ(t.velocity.x, 3.0);
    EXPECT_DOUBLE_EQ(t.velocity.y, 4.0);
    EXPECT_DOUBLE_EQ(t.acceleration.x, 0.5);
    EXPECT_DOUBLE_EQ(t.acceleration.y, -0.5);
    EXPECT_EQ(t.stepCount, 42);
}
