// IntegrationTests.cpp
// Integration tests for multi-physics coexistence, C API bridge (simulating WASM/React and Python ctypes)
// Requirements: 15.1, 15.2, 15.3, 15.4, 15.5, 10.1, 10.2, 10.3, 10.4, 11.5

#include <gtest/gtest.h>
#include "physics/Task.hpp"
#include "physics/ClassicalEngine.hpp"
#include "physics/EnergyEngine.hpp"
#include "core/SimulationEngine.hpp"
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

// ============================================================
// Task 20.1: Energy + Classical Mechanics Integration
// Requirements: 10.1, 10.2, 10.3, 15.2, 15.3
// ============================================================

TEST(IntegrationTest, EnergyClassical_KineticEnergyReflectsVelocity) {
    // Use zero urgency and zero friction so only velocity drives position change
    SimulationEngine sim;
    Task& task = sim.addTask(
        /*mass=*/2.0,
        /*deadlineTime=*/1000.0,
        /*urgencyConstant=*/0.0,   // no deadline force
        /*staticFriction=*/0.0,
        /*kineticFriction=*/0.0
    );
    task.velocity = {3.0, 4.0};  // |v|^2 = 25, KE = 0.5 * 2 * 25 = 25

    // Run one update step (integrates + calculates energy)
    sim.update();

    // KE = 0.5 * mass * |v|^2
    double expectedKE = 0.5 * task.mass * (task.velocity.x * task.velocity.x
                                          + task.velocity.y * task.velocity.y);
    EXPECT_NEAR(task.kineticEnergy, expectedKE, 1e-9)
        << "kineticEnergy must equal 0.5 * mass * |v|^2 after integration";
    EXPECT_GE(task.kineticEnergy, 0.0) << "kineticEnergy must be non-negative";
}

TEST(IntegrationTest, EnergyClassical_EnergyConservationDuringFreeMotion) {
    // With zero urgency and zero friction, total mechanical energy is conserved
    SimulationEngine sim;
    Task& task = sim.addTask(
        /*mass=*/1.0,
        /*deadlineTime=*/1000.0,
        /*urgencyConstant=*/0.0,
        /*staticFriction=*/0.0,
        /*kineticFriction=*/0.0
    );
    task.position = {0.0, 5.0};
    task.velocity = {2.0, 0.0};

    // Compute initial total energy manually
    EnergyEngine::calculateEnergy(task);
    double initialTotal = task.totalEnergy;

    // Run 10 steps
    for (int i = 0; i < 10; ++i) {
        sim.update();
    }

    // Total energy should remain close to initial (no dissipation, no external forces)
    EXPECT_NEAR(task.totalEnergy, initialTotal, 1e-6)
        << "Total energy must be conserved during free motion (no friction, no urgency)";
}

// ============================================================
// Task 20.2: Energy Redistribution Workflow
// Requirements: 5.1, 5.2, 5.3, 5.4, 5.5
// ============================================================

TEST(IntegrationTest, EnergyRedistribution_ConservationAndProportionalDistribution) {
    SimulationEngine sim;

    // Add 3 tasks with different masses
    sim.addTask(/*mass=*/1.0, /*deadlineTime=*/100.0, /*urgencyConstant=*/0.0,
                /*staticFriction=*/0.0, /*kineticFriction=*/0.0);
    sim.addTask(/*mass=*/2.0, /*deadlineTime=*/100.0, /*urgencyConstant=*/0.0,
                /*staticFriction=*/0.0, /*kineticFriction=*/0.0);
    sim.addTask(/*mass=*/3.0, /*deadlineTime=*/100.0, /*urgencyConstant=*/0.0,
                /*staticFriction=*/0.0, /*kineticFriction=*/0.0);

    // Give the "completed" task (index 0) some energy
    sim.tasks[0].velocity = {10.0, 0.0};
    EnergyEngine::calculateEnergy(sim.tasks[0]);

    // Give remaining tasks a baseline energy
    sim.tasks[1].velocity = {1.0, 0.0};
    sim.tasks[2].velocity = {1.0, 0.0};
    EnergyEngine::calculateEnergy(sim.tasks[1]);
    EnergyEngine::calculateEnergy(sim.tasks[2]);

    // Record completed task energy and remaining tasks' energy before redistribution
    Task completedTask = sim.tasks[0];
    double completedEnergy = completedTask.totalEnergy;
    double energyBefore1 = sim.tasks[1].totalEnergy;
    double energyBefore2 = sim.tasks[2].totalEnergy;

    // Build remaining tasks vector
    std::vector<Task> remaining = {sim.tasks[1], sim.tasks[2]};
    double totalMass = remaining[0].mass + remaining[1].mass; // 2 + 3 = 5

    EnergyEngine::redistributeEnergy(completedTask, remaining);

    // Each task should receive a share proportional to its mass
    double expectedShare1 = (remaining[0].mass / totalMass) * completedEnergy;
    double expectedShare2 = (remaining[1].mass / totalMass) * completedEnergy;

    // After redistribution, KE should have increased by the expected share
    double newKE1 = EnergyEngine::computeKineticEnergy(remaining[0]);
    double newKE2 = EnergyEngine::computeKineticEnergy(remaining[1]);

    EXPECT_NEAR(newKE1, energyBefore1 + expectedShare1, 1e-6)
        << "Task 1 should receive mass-proportional share of completed task energy";
    EXPECT_NEAR(newKE2, energyBefore2 + expectedShare2, 1e-6)
        << "Task 2 should receive mass-proportional share of completed task energy";

    // Total energy of remaining tasks should have increased by completedEnergy (conservation)
    double totalAfter = remaining[0].totalEnergy + remaining[1].totalEnergy;
    double totalBefore = energyBefore1 + energyBefore2;
    EXPECT_NEAR(totalAfter, totalBefore + completedEnergy, 1e-6)
        << "Total energy must be conserved: remaining tasks gain exactly the completed task's energy";
}

// ============================================================
// Task 20.3: Focus Mode Workflow
// Requirements: 6.1, 6.2, 6.3, 6.4, 6.5
// ============================================================

TEST(IntegrationTest, FocusMode_InjectEnergyIncreasesKEAndSystemEnergy) {
    SimulationEngine sim;
    sim.addTask(/*mass=*/1.0, /*deadlineTime=*/100.0, /*urgencyConstant=*/0.0,
                /*staticFriction=*/0.0, /*kineticFriction=*/0.0);

    // Give task a small initial velocity so direction is defined
    sim.tasks[0].velocity = {1.0, 0.0};
    EnergyEngine::calculateEnergy(sim.tasks[0]);

    double keBefore = sim.tasks[0].kineticEnergy;
    double systemEnergyBefore = sim.getSystemEnergy();

    // Inject 50 units via focus mode
    const double injectionAmount = 50.0;
    EnergyEngine::injectEnergy(sim.tasks[0], injectionAmount);

    double keAfter = sim.tasks[0].kineticEnergy;
    double systemEnergyAfter = sim.getSystemEnergy();

    // KE should have increased by the injected amount
    EXPECT_NEAR(keAfter, keBefore + injectionAmount, 1e-6)
        << "Kinetic energy must increase by the injected amount";

    // Velocity magnitude should have increased
    double velMagBefore = std::sqrt(1.0); // initial |v| = 1
    double velMagAfter = std::sqrt(sim.tasks[0].velocity.x * sim.tasks[0].velocity.x
                                  + sim.tasks[0].velocity.y * sim.tasks[0].velocity.y);
    EXPECT_GT(velMagAfter, velMagBefore)
        << "Velocity magnitude must increase after energy injection";

    // System energy should have increased by the injected amount
    EXPECT_NEAR(systemEnergyAfter, systemEnergyBefore + injectionAmount, 1e-6)
        << "System energy must increase by the injected amount";
}

// ============================================================
// Task 20.4: Damping Workflow
// Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6
// ============================================================

TEST(IntegrationTest, Damping_VelocityAndEnergyDecreaseOverTime) {
    SimulationEngine sim;
    sim.addTask(/*mass=*/1.0, /*deadlineTime=*/1000.0, /*urgencyConstant=*/0.0,
                /*staticFriction=*/0.0, /*kineticFriction=*/0.0);

    // Give task a high initial velocity
    sim.tasks[0].velocity = {20.0, 0.0};
    EnergyEngine::calculateEnergy(sim.tasks[0]);

    double velMagBefore = std::abs(sim.tasks[0].velocity.x);
    double keBefore = sim.tasks[0].kineticEnergy;

    // Enable damping
    sim.enableDamping(Config::DEFAULT_DAMPING_COEFFICIENT);
    ASSERT_TRUE(sim.dampingEnabled);

    // Run 10 simulation steps
    for (int i = 0; i < 10; ++i) {
        sim.update();
    }

    double velMagAfter = std::sqrt(sim.tasks[0].velocity.x * sim.tasks[0].velocity.x
                                  + sim.tasks[0].velocity.y * sim.tasks[0].velocity.y);
    double keAfter = sim.tasks[0].kineticEnergy;

    EXPECT_LT(velMagAfter, velMagBefore)
        << "Velocity magnitude must decrease after damping over 10 steps";
    EXPECT_LT(keAfter, keBefore)
        << "Kinetic energy must decrease after damping over 10 steps";
    EXPECT_GE(keAfter, 0.0)
        << "Kinetic energy must remain non-negative";
}

// ============================================================
// Task 20.5: WebAssembly + React energy visualization
//            (simulated via C API bridge)
// Requirements: 12.1
// ============================================================

// The WASM/React frontend communicates with the physics engine through the
// SimulationEngine bridge (Simulation_Create/AddTask/Update) and the energy
// query bridge (Energy_GetKinetic, Energy_GetPotential, Energy_GetTotal,
// Energy_Inject).  Simulation_Update() calls ClassicalEngine + EnergyEngine
// together, which is exactly what the React animation loop does.
// These tests exercise that path, verifying that the values the bridge
// exposes are correct so that EnergyBar and SystemEnergyGauge components
// would display accurate data.

TEST(IntegrationTest, WasmReact_EnergyBar_KineticEnergyMatchesFormula) {
    // Simulate what the React frontend does:
    //   1. Create simulation via bridge (Module.Simulation_Create)
    //   2. Add a task (Module.Simulation_AddTask)
    //   3. Call Simulation_Update (animation frame)
    //   4. Query energy via Energy_Get* bridge functions
    //   5. Verify KE = 0.5 * mass * |v|^2  (what EnergyBar would render)

    SimulationEngine* sim = Simulation_Create();
    ASSERT_NE(sim, nullptr);

    // Add task: mass=3, no deadline force, no friction so only velocity drives KE
    size_t idx = Simulation_AddTask(sim, 3.0, 1000.0, 0.0, 0.0, 0.0);
    ASSERT_NE(idx, static_cast<size_t>(-1));

    // Set initial velocity directly (bridge doesn't expose per-task velocity setter
    // on SimulationEngine, so we set it on the underlying task)
    sim->tasks[idx].velocity = {4.0, 3.0};  // |v|^2 = 25

    // One simulation update: integrates + calculates energy
    Simulation_Update(sim);

    // Read energy via bridge (what useWasmModule hook calls)
    Task* t = &sim->tasks[idx];
    const double ke = Energy_GetKinetic(t);
    const double pe = Energy_GetPotential(t);
    const double te = Energy_GetTotal(t);

    // Compute expected KE from actual post-integration velocity
    const double vx = Task_GetVelocityX(t);
    const double vy = Task_GetVelocityY(t);
    const double m  = Task_GetMass(t);
    const double expectedKE = 0.5 * m * (vx * vx + vy * vy);

    EXPECT_NEAR(ke, expectedKE, 1e-9)
        << "Energy_GetKinetic must equal 0.5 * mass * |v|^2 (EnergyBar KE segment)";
    EXPECT_GE(ke, 0.0)
        << "Kinetic energy must be non-negative (EnergyBar must not show negative KE)";
    EXPECT_NEAR(te, ke + pe, 1e-9)
        << "Energy_GetTotal must equal KE + PE (SystemEnergyGauge total)";

    Simulation_Destroy(sim);
}

TEST(IntegrationTest, WasmReact_SystemEnergyGauge_EnergyIncreasesAfterInject) {
    // Simulate the focus-mode workflow that the React UI triggers:
    //   1. Create simulation, add task, run one update
    //   2. Read initial energy via bridge (what SystemEnergyGauge shows)
    //   3. Inject energy via Energy_Inject (FocusModeButton click)
    //   4. Verify bridge reports higher energy (gauge updates correctly)

    SimulationEngine* sim = Simulation_Create();
    ASSERT_NE(sim, nullptr);

    size_t idx = Simulation_AddTask(sim, 2.0, 1000.0, 0.0, 0.0, 0.0);
    ASSERT_NE(idx, static_cast<size_t>(-1));

    sim->tasks[idx].velocity = {1.0, 0.0};
    sim->tasks[idx].position = {0.0, 5.0};

    Simulation_Update(sim);

    Task* t = &sim->tasks[idx];
    const double keBefore = Energy_GetKinetic(t);
    const double teBefore = Energy_GetTotal(t);

    // Inject 50 units (FocusModeButton default amount)
    const double injectionAmount = 50.0;
    Energy_Inject(t, injectionAmount);

    const double keAfter = Energy_GetKinetic(t);
    const double teAfter = Energy_GetTotal(t);

    EXPECT_NEAR(keAfter, keBefore + injectionAmount, 1e-6)
        << "Energy_GetKinetic must increase by injected amount (EnergyBar KE segment grows)";
    EXPECT_NEAR(teAfter, teBefore + injectionAmount, 1e-6)
        << "Energy_GetTotal must increase by injected amount (SystemEnergyGauge updates)";
    EXPECT_GT(keAfter, keBefore)
        << "Kinetic energy must be strictly greater after injection";

    Simulation_Destroy(sim);
}

TEST(IntegrationTest, WasmReact_EnergyBar_PotentialEnergyReflectsHeight) {
    // Verify that the PE value the bridge exposes (used by EnergyBar's PE segment)
    // matches mass * g * y, which is what the React component would render.

    SimulationEngine* sim = Simulation_Create();
    ASSERT_NE(sim, nullptr);

    size_t idx = Simulation_AddTask(sim, 2.0, 1000.0, 0.0, 0.0, 0.0);
    ASSERT_NE(idx, static_cast<size_t>(-1));

    sim->tasks[idx].position = {0.0, 10.0};  // height = 10, PE = 2 * 9.81 * 10 = 196.2
    sim->tasks[idx].velocity = {0.0, 0.0};

    Simulation_Update(sim);

    Task* t = &sim->tasks[idx];
    const double pe = Energy_GetPotential(t);

    // Use actual post-integration position for expected PE
    const double actualY = Task_GetPositionY(t);
    const double m       = Task_GetMass(t);
    const double expectedPE = m * Config::GRAVITY_CONSTANT * actualY;

    EXPECT_NEAR(pe, expectedPE, 1e-9)
        << "Energy_GetPotential must equal mass * g * y (EnergyBar PE segment)";

    Simulation_Destroy(sim);
}

// ============================================================
// Task 20.6: Python ctypes energy binding
//            (simulated via C API bridge)
// Requirements: 12.1
// ============================================================

// Python ctypes loads the shared library and calls the same extern "C"
// symbols tested here.  The Python workflow uses SimulationEngine to drive
// integration + energy calculation together (Simulation_Update), then reads
// energy via Energy_GetKinetic/Potential/Total.

TEST(IntegrationTest, PythonCtypes_Energy_KineticEnergyFormula) {
    // Simulate Python ctypes workflow:
    //   sim = lib.Simulation_Create()
    //   idx = lib.Simulation_AddTask(sim, ...)
    //   lib.Simulation_Update(sim)
    //   ke  = lib.Energy_GetKinetic(task_ptr)

    SimulationEngine* sim = Simulation_Create();
    ASSERT_NE(sim, nullptr);

    size_t idx = Simulation_AddTask(sim, 4.0, 1000.0, 0.0, 0.0, 0.0);
    ASSERT_NE(idx, static_cast<size_t>(-1));

    sim->tasks[idx].velocity = {3.0, 4.0};  // |v|^2 = 25, KE = 0.5 * 4 * 25 = 50.0
    sim->tasks[idx].position = {0.0, 0.0};

    Simulation_Update(sim);

    Task* t = &sim->tasks[idx];
    const double ke = Energy_GetKinetic(t);
    const double vx = Task_GetVelocityX(t);
    const double vy = Task_GetVelocityY(t);
    const double m  = Task_GetMass(t);
    const double expectedKE = 0.5 * m * (vx * vx + vy * vy);

    EXPECT_NEAR(ke, expectedKE, 1e-9)
        << "Energy_GetKinetic must satisfy KE = 0.5 * mass * |v|^2";
    EXPECT_GE(ke, 0.0)
        << "Energy_GetKinetic must be non-negative";

    Simulation_Destroy(sim);
}

TEST(IntegrationTest, PythonCtypes_Energy_PotentialEnergyFormula) {
    // Simulate: pe = lib.Energy_GetPotential(task_ptr)

    SimulationEngine* sim = Simulation_Create();
    ASSERT_NE(sim, nullptr);

    size_t idx = Simulation_AddTask(sim, 5.0, 1000.0, 0.0, 0.0, 0.0);
    ASSERT_NE(idx, static_cast<size_t>(-1));

    sim->tasks[idx].position = {0.0, 8.0};
    sim->tasks[idx].velocity = {0.0, 0.0};

    Simulation_Update(sim);

    Task* t = &sim->tasks[idx];
    const double pe = Energy_GetPotential(t);
    const double y  = Task_GetPositionY(t);
    const double m  = Task_GetMass(t);
    const double expectedPE = m * Config::GRAVITY_CONSTANT * y;

    EXPECT_NEAR(pe, expectedPE, 1e-9)
        << "Energy_GetPotential must satisfy PE = mass * g * y";

    Simulation_Destroy(sim);
}

TEST(IntegrationTest, PythonCtypes_Energy_TotalEnergyEqualsKEPlusPE) {
    // Simulate: te = lib.Energy_GetTotal(task_ptr)
    // Python script would assert abs(te - (ke + pe)) < 1e-9

    SimulationEngine* sim = Simulation_Create();
    ASSERT_NE(sim, nullptr);

    size_t idx = Simulation_AddTask(sim, 2.5, 1000.0, 0.0, 0.0, 0.0);
    ASSERT_NE(idx, static_cast<size_t>(-1));

    sim->tasks[idx].velocity = {2.0, 1.0};
    sim->tasks[idx].position = {0.0, 4.0};

    Simulation_Update(sim);

    Task* t = &sim->tasks[idx];
    const double ke = Energy_GetKinetic(t);
    const double pe = Energy_GetPotential(t);
    const double te = Energy_GetTotal(t);

    EXPECT_NEAR(te, ke + pe, 1e-9)
        << "Energy_GetTotal must equal Energy_GetKinetic + Energy_GetPotential";

    Simulation_Destroy(sim);
}

TEST(IntegrationTest, PythonCtypes_Energy_NullPointerSafety) {
    // Python ctypes may pass NULL if task creation fails.
    // Verify the bridge returns 0.0 rather than crashing.

    EXPECT_DOUBLE_EQ(Energy_GetKinetic(nullptr),   0.0)
        << "Energy_GetKinetic(nullptr) must return 0.0";
    EXPECT_DOUBLE_EQ(Energy_GetPotential(nullptr), 0.0)
        << "Energy_GetPotential(nullptr) must return 0.0";
    EXPECT_DOUBLE_EQ(Energy_GetTotal(nullptr),     0.0)
        << "Energy_GetTotal(nullptr) must return 0.0";
}

TEST(IntegrationTest, PythonCtypes_Energy_ValuesCorrectAfterMultipleSteps) {
    // Simulate a Python script that runs several integration steps and
    // reads energy after each one, verifying the invariant KE + PE == TE.

    SimulationEngine* sim = Simulation_Create();
    ASSERT_NE(sim, nullptr);

    size_t idx = Simulation_AddTask(sim, 1.0, 1000.0, 0.0, 0.0, 0.0);
    ASSERT_NE(idx, static_cast<size_t>(-1));

    sim->tasks[idx].velocity = {5.0, 0.0};
    sim->tasks[idx].position = {0.0, 3.0};

    for (int step = 0; step < 5; ++step) {
        Simulation_Update(sim);

        Task* t = &sim->tasks[idx];
        const double ke = Energy_GetKinetic(t);
        const double pe = Energy_GetPotential(t);
        const double te = Energy_GetTotal(t);

        EXPECT_GE(ke, 0.0)
            << "KE must be non-negative at step " << step;
        EXPECT_NEAR(te, ke + pe, 1e-9)
            << "TE must equal KE + PE at step " << step;
    }

    Simulation_Destroy(sim);
}
