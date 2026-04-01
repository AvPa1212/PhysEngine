// Feature: classical-mechanics-task-system, Property 19: Pause Prevents State Changes
// Validates: Requirements 12.1, 12.5

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "core/SimulationEngine.hpp"

// -----------------------------------------------------------------------
// Property 19: Pause Prevents State Changes
// Feature: classical-mechanics-task-system, Property 19: Pause Prevents State Changes
// Validates: Requirements 12.1, 12.5
//
// For any task in a paused simulation, calling update() SHALL NOT modify
// the task's position, velocity, or acceleration.
// -----------------------------------------------------------------------
RC_GTEST_PROP(PauseProperties, PausePreventsStateChanges, ()) {
    // Generate random but valid task parameters
    const double mass          = *rc::gen::inRange(1, 100).as("mass_x10") / 10.0;
    const double deadline      = *rc::gen::inRange(1, 200).as("deadline_x10") / 10.0;
    const double urgency       = *rc::gen::inRange(1, 1000).as("urgency");
    const double kFriction     = *rc::gen::inRange(0, 10).as("kFriction_x10") / 10.0;
    const double posX          = *rc::gen::inRange(-100, 100).as("posX");
    const double posY          = *rc::gen::inRange(-100, 100).as("posY");
    const double velX          = *rc::gen::inRange(-50, 50).as("velX");
    const double velY          = *rc::gen::inRange(-50, 50).as("velY");

    // Build a task with the generated state
    Task task;
    task.mass             = mass;
    task.deadlineTime     = deadline;
    task.urgencyConstant  = urgency;
    task.kineticFriction  = kFriction;
    task.position         = {posX, posY};
    task.velocity         = {velX, velY};
    task.acceleration     = {0.0, 0.0};

    // Create a paused simulation engine and add the task
    SimulationEngine engine;
    engine.paused = true;
    engine.tasks.push_back(task);

    // Capture state before update
    const double initPosX  = engine.tasks[0].position.x;
    const double initPosY  = engine.tasks[0].position.y;
    const double initVelX  = engine.tasks[0].velocity.x;
    const double initVelY  = engine.tasks[0].velocity.y;
    const double initAccX  = engine.tasks[0].acceleration.x;
    const double initAccY  = engine.tasks[0].acceleration.y;
    const int    initSteps = engine.tasks[0].stepCount;

    // Call update() while paused – state must not change
    engine.update();

    RC_ASSERT(engine.tasks[0].position.x     == initPosX);
    RC_ASSERT(engine.tasks[0].position.y     == initPosY);
    RC_ASSERT(engine.tasks[0].velocity.x     == initVelX);
    RC_ASSERT(engine.tasks[0].velocity.y     == initVelY);
    RC_ASSERT(engine.tasks[0].acceleration.x == initAccX);
    RC_ASSERT(engine.tasks[0].acceleration.y == initAccY);
    RC_ASSERT(engine.tasks[0].stepCount      == initSteps);
}
