#include <gtest/gtest.h>
#include "physics/ChaosEngine.hpp"
#include "physics/Task.hpp"
#include "core/Config.hpp"
#include <cmath>

// Helper: create a task at a specified stress state with zero entropy
static Task makeChaosTask(double sx, double sy, double sz) {
    Task task;
    task.stressX = sx;
    task.stressY = sy;
    task.stressZ = sz;
    task.entropy = 0.0;
    return task;
}

// ---- Stress components change after one update ----

TEST(ChaosEngineTest, StressComponentsChangeAfterUpdate) {
    Task task = makeChaosTask(1.0, 1.0, 1.0);
    const double sx0 = task.stressX;
    const double sy0 = task.stressY;
    const double sz0 = task.stressZ;

    ChaosEngine::update(task);

    bool changed = (task.stressX != sx0) || (task.stressY != sy0) || (task.stressZ != sz0);
    EXPECT_TRUE(changed);
}

// ---- Entropy increases after update from zero ----

TEST(ChaosEngineTest, EntropyIncreasesAfterUpdate) {
    Task task = makeChaosTask(1.0, 1.0, 1.0);
    task.entropy = 0.0;

    ChaosEngine::update(task);

    EXPECT_GT(task.entropy, 0.0);
}

// ---- Entropy is monotonically non-decreasing ----

TEST(ChaosEngineTest, EntropyMonotonicallyNonDecreasing) {
    Task task = makeChaosTask(1.0, 1.0, 1.0);
    task.entropy = 0.0;

    double prevEntropy = task.entropy;
    for (int step = 0; step < 50; ++step) {
        ChaosEngine::update(task);
        EXPECT_GE(task.entropy, prevEntropy)
            << "Entropy decreased at step " << step;
        prevEntropy = task.entropy;
    }
}

// ---- Stress components remain finite after many steps ----

TEST(ChaosEngineTest, StressRemainsFiniteAfterManySteps) {
    Task task = makeChaosTask(0.1, 0.1, 0.1);
    for (int step = 0; step < 1000; ++step) {
        ChaosEngine::update(task);
    }
    EXPECT_TRUE(std::isfinite(task.stressX));
    EXPECT_TRUE(std::isfinite(task.stressY));
    EXPECT_TRUE(std::isfinite(task.stressZ));
}

// ---- Sensitive dependence on initial conditions (butterfly effect) ----

TEST(ChaosEngineTest, NearbyTrajectoriesDiverge) {
    Task task1 = makeChaosTask(1.0, 1.0, 1.0);
    Task task2 = makeChaosTask(1.0 + 1e-5, 1.0, 1.0); // tiny perturbation

    // The Lorenz Lyapunov time is ~1.1 s; 2000 steps ≈ 33 s is well past saturation
    for (int step = 0; step < 2000; ++step) {
        ChaosEngine::update(task1);
        ChaosEngine::update(task2);
    }

    const double dx = task1.stressX - task2.stressX;
    const double dy = task1.stressY - task2.stressY;
    const double dz = task1.stressZ - task2.stressZ;
    const double dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    // After 2000 steps the two trajectories should be decorrelated (dist >> 1e-5)
    EXPECT_GT(dist, 1.0);
}

// ---- Identical initial conditions produce identical trajectories ----

TEST(ChaosEngineTest, IdenticalInitialConditionsProduceIdenticalTrajectories) {
    Task task1 = makeChaosTask(2.5, 3.0, 10.0);
    Task task2 = makeChaosTask(2.5, 3.0, 10.0);

    for (int step = 0; step < 200; ++step) {
        ChaosEngine::update(task1);
        ChaosEngine::update(task2);
    }

    EXPECT_DOUBLE_EQ(task1.stressX, task2.stressX);
    EXPECT_DOUBLE_EQ(task1.stressY, task2.stressY);
    EXPECT_DOUBLE_EQ(task1.stressZ, task2.stressZ);
    EXPECT_DOUBLE_EQ(task1.entropy, task2.entropy);
}

// ---- Fixed-point verification: known Lorenz fixed point stays put ----
// The non-trivial fixed points of the Lorenz system are:
//   x* = y* = sqrt(beta*(rho-1)),  z* = rho - 1
// At these points dx/dt = dy/dt = dz/dt = 0, so one integration step
// must leave the stress values unchanged.

TEST(ChaosEngineTest, LorenzFixedPointDoesNotMove) {
    const double val   = std::sqrt(Config::CHAOS_BETA * (Config::CHAOS_RHO - 1.0));
    const double zstar = Config::CHAOS_RHO - 1.0;

    Task task = makeChaosTask(val, val, zstar);
    const double sx0 = task.stressX;
    const double sy0 = task.stressY;
    const double sz0 = task.stressZ;

    ChaosEngine::update(task);

    EXPECT_NEAR(task.stressX, sx0, 1e-12);
    EXPECT_NEAR(task.stressY, sy0, 1e-12);
    EXPECT_NEAR(task.stressZ, sz0, 1e-12);
}

// ---- Entropy accumulation is proportional to phase-space movement ----

TEST(ChaosEngineTest, EntropyAccumulatesWithMovement) {
    // A state that moves more should accumulate entropy faster.
    // Large initial offset from origin → larger Lorenz derivatives → more entropy.
    Task task_far  = makeChaosTask(10.0, 10.0, 20.0);
    Task task_near = makeChaosTask(0.01, 0.01, 0.01);

    ChaosEngine::update(task_far);
    ChaosEngine::update(task_near);

    EXPECT_GT(task_far.entropy, task_near.entropy);
}
