// RoundTripPropertiesTest.cpp
// Property-based tests for API bridge and serialization round-trips
// Feature: classical-mechanics-task-system

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/MomentumBridge.h"
#include "physics/Task.hpp"
#include <cmath>
#include <cstring>

// -----------------------------------------------------------------------
// Property 17: API Bridge Round-Trip
// Feature: classical-mechanics-task-system, Property 17: API Bridge Round-Trip
// Validates: Requirements 11.2
// -----------------------------------------------------------------------
RC_GTEST_PROP(RoundTripProperties, APIBridgeRoundTrip, ()) {
    // Generate random position, velocity, mass
    const double x  = *rc::gen::inRange(-10000, 10001).as("x_int")  / 100.0;
    const double y  = *rc::gen::inRange(-10000, 10001).as("y_int")  / 100.0;
    const double vx = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    // mass must be > 0
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;

    Task* t = Task_Create();
    RC_ASSERT(t != nullptr);

    Task_SetPosition(t, x, y);
    Task_SetVelocity(t, vx, vy);
    Task_SetMass(t, mass);

    RC_ASSERT(std::abs(Task_GetPositionX(t) - x)   < 1e-12);
    RC_ASSERT(std::abs(Task_GetPositionY(t) - y)   < 1e-12);
    RC_ASSERT(std::abs(Task_GetVelocityX(t) - vx)  < 1e-12);
    RC_ASSERT(std::abs(Task_GetVelocityY(t) - vy)  < 1e-12);
    RC_ASSERT(std::abs(Task_GetMass(t)      - mass) < 1e-12);

    Task_Destroy(t);
}

// -----------------------------------------------------------------------
// Property 18: Serialization Round-Trip
// Feature: classical-mechanics-task-system, Property 18: Serialization Round-Trip
// Validates: Requirements 11.6
// -----------------------------------------------------------------------
RC_GTEST_PROP(RoundTripProperties, SerializationRoundTrip, ()) {
    // Generate random task state
    const double posX    = *rc::gen::inRange(-10000, 10001).as("posX_int")    / 100.0;
    const double posY    = *rc::gen::inRange(-10000, 10001).as("posY_int")    / 100.0;
    const double velX    = *rc::gen::inRange(-10000, 10001).as("velX_int")    / 100.0;
    const double velY    = *rc::gen::inRange(-10000, 10001).as("velY_int")    / 100.0;
    const double mass    = *rc::gen::inRange(1, 10001).as("mass_int")         / 100.0;
    const double entropy = *rc::gen::inRange(0, 10001).as("entropy_int")      / 100.0;
    const int stepCount  = *rc::gen::inRange(0, 10000).as("stepCount");

    Task* original = Task_Create();
    RC_ASSERT(original != nullptr);

    Task_SetPosition(original, posX, posY);
    Task_SetVelocity(original, velX, velY);
    Task_SetMass(original, mass);
    // Set entropy and stepCount directly via the Task struct
    // (no bridge setter for these, so we cast through the opaque pointer)
    {
        Task* raw = original;
        raw->entropy   = entropy;
        raw->stepCount = stepCount;
    }

    // Serialize
    const char* json = State_Serialize(original);
    RC_ASSERT(json != nullptr);
    RC_ASSERT(std::strlen(json) > 2); // at least "{}"

    // Deserialize into a fresh task
    Task* restored = Task_Create();
    RC_ASSERT(restored != nullptr);
    State_Deserialize(restored, json);

    // Verify all fields match within 1e-9
    RC_ASSERT(std::abs(Task_GetPositionX(restored) - posX)    < 1e-9);
    RC_ASSERT(std::abs(Task_GetPositionY(restored) - posY)    < 1e-9);
    RC_ASSERT(std::abs(Task_GetVelocityX(restored) - velX)    < 1e-9);
    RC_ASSERT(std::abs(Task_GetVelocityY(restored) - velY)    < 1e-9);
    RC_ASSERT(std::abs(Task_GetMass(restored)      - mass)    < 1e-9);
    RC_ASSERT(std::abs(Task_GetEntropy(restored)   - entropy) < 1e-9);
    RC_ASSERT(Task_GetStepCount(restored) == stepCount);

    Task_Destroy(original);
    Task_Destroy(restored);
}
