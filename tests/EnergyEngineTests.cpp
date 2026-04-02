#include <gtest/gtest.h>
#include "physics/EnergyEngine.hpp"
#include "physics/Task.hpp"
#include "core/Config.hpp"

// ============================================================
// Task 2.3 – computeKineticEnergy()
// Validates: Requirements 1.1, 1.2, 1.5
// ============================================================

// KE = 0.5 * mass * |v|^2
// velocity (3,4) -> |v|^2 = 9+16 = 25, mass=2.0 -> KE = 0.5*2*25 = 25.0
TEST(EnergyEngineTest, KineticEnergy_Velocity3_4_Mass2_Produces25) {
    Task t;
    t.velocity = {3.0, 4.0};
    t.mass = 2.0;
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(t), 25.0, 1e-9);
}

// Requirement 1.2: zero velocity -> KE = 0
TEST(EnergyEngineTest, KineticEnergy_ZeroVelocity_ProducesZero) {
    Task t;
    t.velocity = {0.0, 0.0};
    t.mass = 5.0;
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(t), 0.0, 1e-9);
}

// Requirement 1.5: KE is non-negative even with negative velocity components
TEST(EnergyEngineTest, KineticEnergy_NegativeComponents_ProducesPositiveKE) {
    Task t;
    t.velocity = {-3.0, -4.0};
    t.mass = 2.0;
    double ke = EnergyEngine::computeKineticEnergy(t);
    EXPECT_NEAR(ke, 25.0, 1e-9);
    EXPECT_GE(ke, 0.0);
}

// ============================================================
// Task 2.7 – computePotentialEnergy()
// Validates: Requirements 2.1, 2.2, 2.3
// ============================================================

// PE = mass * g * height, g = 9.81
// position (0,10), mass=2.0 -> PE = 2.0 * 9.81 * 10.0 = 196.2
TEST(EnergyEngineTest, PotentialEnergy_Height10_Mass2_Produces196_2) {
    Task t;
    t.position = {0.0, 10.0};
    t.mass = 2.0;
    EXPECT_NEAR(EnergyEngine::computePotentialEnergy(t), 196.2, 1e-9);
}

// Requirement 2.3: negative height -> negative PE
TEST(EnergyEngineTest, PotentialEnergy_NegativeHeight_ProducesNegativePE) {
    Task t;
    t.position = {0.0, -5.0};
    t.mass = 1.0;
    double pe = EnergyEngine::computePotentialEnergy(t);
    EXPECT_NEAR(pe, -5.0 * Config::GRAVITY_CONSTANT, 1e-9);
    EXPECT_LT(pe, 0.0);
}

// Requirement 2.2: height = y-coordinate; zero height -> PE = 0
TEST(EnergyEngineTest, PotentialEnergy_ZeroHeight_ProducesZero) {
    Task t;
    t.position = {5.0, 0.0};
    t.mass = 3.0;
    EXPECT_NEAR(EnergyEngine::computePotentialEnergy(t), 0.0, 1e-9);
}

// ============================================================
// Task 2.11 – calculateEnergy()
// Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.5
// ============================================================

// Requirement 3.3: energy fields are updated after call
TEST(EnergyEngineTest, CalculateEnergy_UpdatesEnergyFields) {
    Task t;
    t.velocity = {3.0, 4.0};
    t.position = {0.0, 10.0};
    t.mass = 2.0;

    EnergyEngine::calculateEnergy(t);

    EXPECT_NEAR(t.kineticEnergy,   25.0,  1e-9);
    EXPECT_NEAR(t.potentialEnergy, 196.2, 1e-9);
}

// Requirement 3.1 & 3.5: totalEnergy == KE + PE
TEST(EnergyEngineTest, CalculateEnergy_TotalEqualsSumOfKEAndPE) {
    Task t;
    t.velocity = {3.0, 4.0};
    t.position = {0.0, 10.0};
    t.mass = 2.0;

    EnergyEngine::calculateEnergy(t);

    EXPECT_NEAR(t.totalEnergy, t.kineticEnergy + t.potentialEnergy, 1e-9);
    EXPECT_NEAR(t.totalEnergy, 221.2, 1e-9);
}

// Requirement 3.3 / 10.3: classical fields (position, velocity) are unchanged
TEST(EnergyEngineTest, CalculateEnergy_ClassicalFieldsUnchanged) {
    Task t;
    t.velocity     = {3.0, 4.0};
    t.position     = {1.0, 2.0};
    t.acceleration = {0.5, -0.5};
    t.mass         = 2.0;

    EnergyEngine::calculateEnergy(t);

    EXPECT_DOUBLE_EQ(t.velocity.x,     3.0);
    EXPECT_DOUBLE_EQ(t.velocity.y,     4.0);
    EXPECT_DOUBLE_EQ(t.position.x,     1.0);
    EXPECT_DOUBLE_EQ(t.position.y,     2.0);
    EXPECT_DOUBLE_EQ(t.acceleration.x, 0.5);
    EXPECT_DOUBLE_EQ(t.acceleration.y, -0.5);
    EXPECT_DOUBLE_EQ(t.mass,           2.0);
}

// ============================================================
// Task 3.2 – injectEnergy()
// Validates: Requirements 6.1, 6.2, 6.3, 6.4
// ============================================================

// Requirement 6.1: injecting 50 into KE=25 produces KE=75
// mass=2.0, velocity=(3,4) -> KE = 0.5*2*(9+16) = 25.0; inject 50 -> KE=75
TEST(EnergyEngineTest, InjectEnergy_50IntoKE25_ProducesKE75) {
    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};
    EnergyEngine::injectEnergy(t, 50.0);
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(t), 75.0, 1e-6);
}

// Requirement 6.2: injecting into zero velocity creates velocity in +X direction
TEST(EnergyEngineTest, InjectEnergy_ZeroVelocity_CreatesPositiveXVelocity) {
    Task t;
    t.mass = 2.0;
    t.velocity = {0.0, 0.0};
    EnergyEngine::injectEnergy(t, 50.0);
    EXPECT_GT(t.velocity.x, 0.0);
    EXPECT_NEAR(t.velocity.y, 0.0, 1e-9);
}

// Requirement 6.3: injecting preserves velocity direction for non-zero velocity
TEST(EnergyEngineTest, InjectEnergy_NonZeroVelocity_PreservesDirection) {
    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};
    double ratioBeforeX = t.velocity.x;
    double ratioBeforeY = t.velocity.y;
    EnergyEngine::injectEnergy(t, 50.0);
    // Direction preserved: vx/vy ratio should be unchanged (3/4)
    EXPECT_NEAR(t.velocity.x / t.velocity.y, ratioBeforeX / ratioBeforeY, 1e-6);
}

// Requirement 6.4: injecting negative energy clamps KE to zero
TEST(EnergyEngineTest, InjectEnergy_NegativeLargeAmount_ClampsToZeroKE) {
    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};
    EnergyEngine::injectEnergy(t, -1000.0);
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(t), 0.0, 1e-9);
    EXPECT_NEAR(t.velocity.x, 0.0, 1e-9);
    EXPECT_NEAR(t.velocity.y, 0.0, 1e-9);
}

// ============================================================
// Task 3.7 – dissipateEnergy()
// Validates: Requirements 7.1, 7.2, 7.3, 7.4, 7.6
// ============================================================

// Requirement 7.1: damping reduces velocity magnitude
TEST(EnergyEngineTest, DissipateEnergy_ReducesVelocityMagnitude) {
    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};
    double speedBefore = std::sqrt(t.velocity.x * t.velocity.x + t.velocity.y * t.velocity.y);
    EnergyEngine::dissipateEnergy(t, 0.5);
    double speedAfter = std::sqrt(t.velocity.x * t.velocity.x + t.velocity.y * t.velocity.y);
    EXPECT_LT(speedAfter, speedBefore);
}

// Requirement 7.2: damping preserves velocity direction
TEST(EnergyEngineTest, DissipateEnergy_PreservesVelocityDirection) {
    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};
    EnergyEngine::dissipateEnergy(t, 0.5);
    // Direction preserved: vx/vy ratio should remain 3/4
    EXPECT_NEAR(t.velocity.x / t.velocity.y, 3.0 / 4.0, 1e-6);
}

// Requirement 7.3: zero damping coefficient produces no change
TEST(EnergyEngineTest, DissipateEnergy_ZeroDamping_NoChange) {
    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};
    EnergyEngine::dissipateEnergy(t, 0.0);
    EXPECT_NEAR(t.velocity.x, 3.0, 1e-9);
    EXPECT_NEAR(t.velocity.y, 4.0, 1e-9);
}

// Requirement 7.4 & 7.6: damping reduces KE monotonically
// energyLoss = coefficient * KE * TIME_STEP; KE after < KE before
TEST(EnergyEngineTest, DissipateEnergy_ReducesKEMonotonically) {
    Task t;
    t.mass = 2.0;
    t.velocity = {3.0, 4.0};
    double keBefore = EnergyEngine::computeKineticEnergy(t); // 25.0
    EnergyEngine::dissipateEnergy(t, 0.5);
    double keAfter = EnergyEngine::computeKineticEnergy(t);
    double expectedLoss = 0.5 * keBefore * Config::TIME_STEP;
    EXPECT_NEAR(keAfter, keBefore - expectedLoss, 1e-6);
    EXPECT_LT(keAfter, keBefore);
}

// ============================================================
// Task 3.13 – transferEnergy()
// Validates: Requirements 14.2, 14.3, 14.4
// ============================================================

// Requirement 14.2 & 14.3: transfer decreases source KE and increases target KE
// source KE=50 (mass=2, v=(5,5) -> KE=0.5*2*50=50), target KE=10 (mass=2, v=(sqrt(10),0))
TEST(EnergyEngineTest, TransferEnergy_DecreasesSourceIncreasesTarget) {
    Task source, target;
    source.mass = 2.0;
    // KE = 0.5*2*(vx^2+vy^2) = 50 -> vx^2+vy^2=50; use (5,5)
    source.velocity = {5.0, 5.0};

    target.mass = 2.0;
    // KE = 10 -> vx^2+vy^2=10; use (sqrt(10),0)
    target.velocity = {std::sqrt(10.0), 0.0};

    double sourceKEBefore = EnergyEngine::computeKineticEnergy(source); // ~50
    double targetKEBefore = EnergyEngine::computeKineticEnergy(target); // ~10

    EnergyEngine::transferEnergy(source, target, 30.0);

    double sourceKEAfter = EnergyEngine::computeKineticEnergy(source);
    double targetKEAfter = EnergyEngine::computeKineticEnergy(target);

    EXPECT_LT(sourceKEAfter, sourceKEBefore);
    EXPECT_GT(targetKEAfter, targetKEBefore);
}

// Requirement 14.4: transfer clamped to available source energy
// source KE=50, try to transfer 1000 -> source KE becomes 0, target gets at most 50
TEST(EnergyEngineTest, TransferEnergy_ClampedToAvailableSourceEnergy) {
    Task source, target;
    source.mass = 2.0;
    source.velocity = {5.0, 5.0}; // KE = 50

    target.mass = 2.0;
    target.velocity = {0.0, 0.0}; // KE = 0

    double sourceKEBefore = EnergyEngine::computeKineticEnergy(source); // 50

    EnergyEngine::transferEnergy(source, target, 1000.0);

    double sourceKEAfter = EnergyEngine::computeKineticEnergy(source);
    double targetKEAfter = EnergyEngine::computeKineticEnergy(target);

    EXPECT_NEAR(sourceKEAfter, 0.0, 1e-6);
    EXPECT_LE(targetKEAfter, sourceKEBefore + 1e-6);
}

// Requirement 14.4: transfer prevents negative energy in source
TEST(EnergyEngineTest, TransferEnergy_PreventsNegativeSourceEnergy) {
    Task source, target;
    source.mass = 1.0;
    source.velocity = {4.0, 3.0}; // KE = 0.5*1*25 = 12.5

    target.mass = 1.0;
    target.velocity = {0.0, 0.0};

    EnergyEngine::transferEnergy(source, target, 100.0);

    EXPECT_GE(EnergyEngine::computeKineticEnergy(source), 0.0);
}

// ============================================================
// Task 4.2 – computeSystemEnergy()
// Validates: Requirements 4.1
// ============================================================

// Empty task list returns 0.0
TEST(EnergyEngineTest, ComputeSystemEnergy_EmptyList_ReturnsZero) {
    std::vector<Task> tasks;
    EXPECT_NEAR(EnergyEngine::computeSystemEnergy(tasks), 0.0, 1e-9);
}

// Single task returns its totalEnergy
TEST(EnergyEngineTest, ComputeSystemEnergy_SingleTask_ReturnsTotalEnergy) {
    Task t;
    t.totalEnergy = 42.5;
    std::vector<Task> tasks = {t};
    EXPECT_NEAR(EnergyEngine::computeSystemEnergy(tasks), 42.5, 1e-9);
}

// Multiple tasks returns sum of all totalEnergy values
TEST(EnergyEngineTest, ComputeSystemEnergy_MultipleTasks_ReturnsSumOfTotalEnergies) {
    Task t1, t2, t3;
    t1.totalEnergy = 10.0;
    t2.totalEnergy = 20.0;
    t3.totalEnergy = 30.0;
    std::vector<Task> tasks = {t1, t2, t3};
    EXPECT_NEAR(EnergyEngine::computeSystemEnergy(tasks), 60.0, 1e-9);
}

// ============================================================
// Task 4.5 – redistributeEnergy()
// Validates: Requirements 5.2, 5.3, 5.4
// ============================================================

// Redistribution with masses [1,2,3] distributes proportionally
// completedTask.totalEnergy=60, totalMass=6 -> shares: 10, 20, 30
TEST(EnergyEngineTest, RedistributeEnergy_ProportionalToMass) {
    Task completed;
    completed.totalEnergy = 60.0;

    Task t1, t2, t3;
    t1.mass = 1.0; t1.velocity = {0.0, 0.0};
    t2.mass = 2.0; t2.velocity = {0.0, 0.0};
    t3.mass = 3.0; t3.velocity = {0.0, 0.0};
    std::vector<Task> remaining = {t1, t2, t3};

    EnergyEngine::redistributeEnergy(completed, remaining);

    // Each task's KE should equal its proportional share
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(remaining[0]), 10.0, 1e-6);
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(remaining[1]), 20.0, 1e-6);
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(remaining[2]), 30.0, 1e-6);
}

// Redistribution to empty list doesn't crash
TEST(EnergyEngineTest, RedistributeEnergy_EmptyList_NoCrash) {
    Task completed;
    completed.totalEnergy = 100.0;
    std::vector<Task> remaining;
    EXPECT_NO_THROW(EnergyEngine::redistributeEnergy(completed, remaining));
}

// Redistribution increases receiving task velocities (KE increases)
TEST(EnergyEngineTest, RedistributeEnergy_IncreasesReceivingTaskVelocities) {
    Task completed;
    completed.totalEnergy = 60.0;

    Task t1, t2, t3;
    t1.mass = 1.0; t1.velocity = {0.0, 0.0};
    t2.mass = 2.0; t2.velocity = {0.0, 0.0};
    t3.mass = 3.0; t3.velocity = {0.0, 0.0};
    std::vector<Task> remaining = {t1, t2, t3};

    double keBefore1 = EnergyEngine::computeKineticEnergy(remaining[0]);
    double keBefore2 = EnergyEngine::computeKineticEnergy(remaining[1]);
    double keBefore3 = EnergyEngine::computeKineticEnergy(remaining[2]);

    EnergyEngine::redistributeEnergy(completed, remaining);

    EXPECT_GT(EnergyEngine::computeKineticEnergy(remaining[0]), keBefore1);
    EXPECT_GT(EnergyEngine::computeKineticEnergy(remaining[1]), keBefore2);
    EXPECT_GT(EnergyEngine::computeKineticEnergy(remaining[2]), keBefore3);
}

// ============================================================
// Task 4.10 – sortByEnergy()
// Validates: Requirements 8.1, 8.3, 8.5
// ============================================================

// Tasks sorted by totalEnergy descending: [30, 10, 20] -> [30, 20, 10]
TEST(EnergyEngineTest, SortByEnergy_SortedDescendingByTotalEnergy) {
    Task t1, t2, t3;
    t1.totalEnergy = 30.0;
    t2.totalEnergy = 10.0;
    t3.totalEnergy = 20.0;
    std::vector<Task> tasks = {t1, t2, t3};

    auto sorted = EnergyEngine::sortByEnergy(tasks);

    ASSERT_EQ(sorted.size(), 3u);
    EXPECT_NEAR(sorted[0]->totalEnergy, 30.0, 1e-9);
    EXPECT_NEAR(sorted[1]->totalEnergy, 20.0, 1e-9);
    EXPECT_NEAR(sorted[2]->totalEnergy, 10.0, 1e-9);
}

// Tiebreaker uses kineticEnergy descending
TEST(EnergyEngineTest, SortByEnergy_TiebreakerUsesKineticEnergy) {
    Task t1, t2;
    t1.totalEnergy = 50.0; t1.kineticEnergy = 30.0;
    t2.totalEnergy = 50.0; t2.kineticEnergy = 40.0;
    std::vector<Task> tasks = {t1, t2};

    auto sorted = EnergyEngine::sortByEnergy(tasks);

    ASSERT_EQ(sorted.size(), 2u);
    EXPECT_NEAR(sorted[0]->kineticEnergy, 40.0, 1e-9);
    EXPECT_NEAR(sorted[1]->kineticEnergy, 30.0, 1e-9);
}

// Empty list returns empty result
TEST(EnergyEngineTest, SortByEnergy_EmptyList_ReturnsEmpty) {
    std::vector<Task> tasks;
    auto sorted = EnergyEngine::sortByEnergy(tasks);
    EXPECT_TRUE(sorted.empty());
}

// ============================================================
// Task 4.13 – computeEnergyDrift()
// Validates: Requirements 4.2, 13.2
// ============================================================

// Drift = 0 when energies are equal
TEST(EnergyEngineTest, ComputeEnergyDrift_EqualEnergies_ReturnsZero) {
    EXPECT_NEAR(EnergyEngine::computeEnergyDrift(100.0, 100.0), 0.0, 1e-9);
}

// Drift calculation with known values: |101-100|/100 = 0.01
TEST(EnergyEngineTest, ComputeEnergyDrift_KnownValues_ReturnsCorrectDrift) {
    EXPECT_NEAR(EnergyEngine::computeEnergyDrift(100.0, 101.0), 0.01, 1e-9);
}

// Zero initial energy returns 0.0 regardless of current energy
TEST(EnergyEngineTest, ComputeEnergyDrift_ZeroInitialEnergy_ReturnsZero) {
    EXPECT_NEAR(EnergyEngine::computeEnergyDrift(0.0, 50.0), 0.0, 1e-9);
    EXPECT_NEAR(EnergyEngine::computeEnergyDrift(0.0, 0.0),  0.0, 1e-9);
}

// ============================================================
// Task 14.2 – computeMeanEnergy(), computeEnergyStdDev(), identifyHighEnergyTasks()
// Validates: Requirements 11.3, 11.4
// ============================================================

// Mean of [10, 20, 30] = 20
TEST(EnergyEngineTest, ComputeMeanEnergy_KnownValues_Returns20) {
    Task t1, t2, t3;
    t1.totalEnergy = 10.0;
    t2.totalEnergy = 20.0;
    t3.totalEnergy = 30.0;
    std::vector<Task> tasks = {t1, t2, t3};
    EXPECT_NEAR(EnergyEngine::computeMeanEnergy(tasks), 20.0, 1e-9);
}

// Mean of empty list = 0
TEST(EnergyEngineTest, ComputeMeanEnergy_EmptyList_ReturnsZero) {
    std::vector<Task> tasks;
    EXPECT_NEAR(EnergyEngine::computeMeanEnergy(tasks), 0.0, 1e-9);
}

// Mean of single task = that task's totalEnergy
TEST(EnergyEngineTest, ComputeMeanEnergy_SingleTask_ReturnsTotalEnergy) {
    Task t;
    t.totalEnergy = 42.0;
    std::vector<Task> tasks = {t};
    EXPECT_NEAR(EnergyEngine::computeMeanEnergy(tasks), 42.0, 1e-9);
}

// StdDev of [10, 20, 30]: mean=20, variance=((10-20)^2+(20-20)^2+(30-20)^2)/3 = 200/3, stddev=sqrt(200/3)
TEST(EnergyEngineTest, ComputeEnergyStdDev_KnownValues_CorrectResult) {
    Task t1, t2, t3;
    t1.totalEnergy = 10.0;
    t2.totalEnergy = 20.0;
    t3.totalEnergy = 30.0;
    std::vector<Task> tasks = {t1, t2, t3};
    double expected = std::sqrt(200.0 / 3.0);
    EXPECT_NEAR(EnergyEngine::computeEnergyStdDev(tasks), expected, 1e-9);
}

// StdDev of identical values = 0
TEST(EnergyEngineTest, ComputeEnergyStdDev_IdenticalValues_ReturnsZero) {
    Task t1, t2, t3;
    t1.totalEnergy = 5.0;
    t2.totalEnergy = 5.0;
    t3.totalEnergy = 5.0;
    std::vector<Task> tasks = {t1, t2, t3};
    EXPECT_NEAR(EnergyEngine::computeEnergyStdDev(tasks), 0.0, 1e-9);
}

// StdDev of empty list = 0
TEST(EnergyEngineTest, ComputeEnergyStdDev_EmptyList_ReturnsZero) {
    std::vector<Task> tasks;
    EXPECT_NEAR(EnergyEngine::computeEnergyStdDev(tasks), 0.0, 1e-9);
}

// identifyHighEnergyTasks: threshold=25, tasks [10,20,30] -> only task with 30 returned
TEST(EnergyEngineTest, IdentifyHighEnergyTasks_Threshold25_ReturnsTasksAboveThreshold) {
    Task t1, t2, t3;
    t1.totalEnergy = 10.0;
    t2.totalEnergy = 20.0;
    t3.totalEnergy = 30.0;
    std::vector<Task> tasks = {t1, t2, t3};
    auto high = EnergyEngine::identifyHighEnergyTasks(tasks, 25.0);
    ASSERT_EQ(high.size(), 1u);
    EXPECT_NEAR(high[0]->totalEnergy, 30.0, 1e-9);
}

// identifyHighEnergyTasks: threshold includes boundary (>= threshold)
TEST(EnergyEngineTest, IdentifyHighEnergyTasks_IncludesBoundaryValue) {
    Task t1, t2;
    t1.totalEnergy = 25.0;
    t2.totalEnergy = 24.9;
    std::vector<Task> tasks = {t1, t2};
    auto high = EnergyEngine::identifyHighEnergyTasks(tasks, 25.0);
    ASSERT_EQ(high.size(), 1u);
    EXPECT_NEAR(high[0]->totalEnergy, 25.0, 1e-9);
}

// identifyHighEnergyTasks: empty list returns empty
TEST(EnergyEngineTest, IdentifyHighEnergyTasks_EmptyList_ReturnsEmpty) {
    std::vector<Task> tasks;
    auto high = EnergyEngine::identifyHighEnergyTasks(tasks, 10.0);
    EXPECT_TRUE(high.empty());
}

// identifyHighEnergyTasks: all tasks above threshold
TEST(EnergyEngineTest, IdentifyHighEnergyTasks_AllAboveThreshold_ReturnsAll) {
    Task t1, t2, t3;
    t1.totalEnergy = 50.0;
    t2.totalEnergy = 60.0;
    t3.totalEnergy = 70.0;
    std::vector<Task> tasks = {t1, t2, t3};
    auto high = EnergyEngine::identifyHighEnergyTasks(tasks, 10.0);
    EXPECT_EQ(high.size(), 3u);
}

// ============================================================
// Task 16.4 – injectEnergyRateLimited()
// Validates: Requirements 19.1, 19.2, 19.3
// ============================================================

// Requirement 19.1: injection within rate limit succeeds immediately (no queuing)
TEST(EnergyEngineTest, RateLimited_WithinLimit_InjectsImmediately) {
    Task t;
    t.mass = 1.0;
    t.velocity = {0.0, 0.0};

    double queue = 0.0;
    double lastTime = 0.0;
    double maxRate = Config::MAX_ENERGY_INJECTION_RATE; // 100.0

    // Budget per step = maxRate * TIME_STEP = 100 * (1/60) ≈ 1.667
    double budget = maxRate * Config::TIME_STEP;
    double amount = budget * 0.5; // well within limit

    EnergyEngine::injectEnergyRateLimited(t, amount, queue, lastTime, maxRate);

    // All energy should be injected immediately, nothing queued
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(t), amount, 1e-6);
    EXPECT_NEAR(queue, 0.0, 1e-9);
}

// Requirement 19.2: injection exceeding rate limit queues excess
TEST(EnergyEngineTest, RateLimited_ExceedsLimit_QueuesExcess) {
    Task t;
    t.mass = 1.0;
    t.velocity = {0.0, 0.0};

    double queue = 0.0;
    double lastTime = 0.0;
    double maxRate = Config::MAX_ENERGY_INJECTION_RATE; // 100.0

    double budget = maxRate * Config::TIME_STEP; // ≈ 1.667
    double amount = budget * 3.0; // 3x the budget

    EnergyEngine::injectEnergyRateLimited(t, amount, queue, lastTime, maxRate);

    // Only budget amount injected, remainder queued
    EXPECT_NEAR(EnergyEngine::computeKineticEnergy(t), budget, 1e-6);
    EXPECT_NEAR(queue, amount - budget, 1e-6);
}

// Requirement 19.3: queued energy distributed over subsequent steps
TEST(EnergyEngineTest, RateLimited_QueuedEnergyDistributedOverSteps) {
    Task t;
    t.mass = 1.0;
    t.velocity = {0.0, 0.0};

    double queue = 0.0;
    double lastTime = 0.0;
    double maxRate = Config::MAX_ENERGY_INJECTION_RATE;

    double budget = maxRate * Config::TIME_STEP;
    double totalAmount = budget * 4.0; // 4x budget, needs multiple steps

    // First injection: inject large amount
    EnergyEngine::injectEnergyRateLimited(t, totalAmount, queue, lastTime, maxRate);
    double queueAfterFirst = queue;
    EXPECT_GT(queueAfterFirst, 0.0); // should have queued remainder

    // Second step: inject 0 new energy, process queue
    EnergyEngine::injectEnergyRateLimited(t, 0.0, queue, lastTime, maxRate);
    double queueAfterSecond = queue;

    // Queue should have decreased
    EXPECT_LT(queueAfterSecond, queueAfterFirst);
}

// ============================================================
// Task 17.3 – computeForceScalingFactor()
// Validates: Requirements 20.1, 20.4
// ============================================================

// High energy -> scaling factor > 1 (or at least > low-energy factor)
TEST(EnergyEngineTest, ForceScaling_HighEnergy_IncreasesScalingFactor) {
    Task low, high;
    low.totalEnergy  = 1.0;
    high.totalEnergy = Config::MAX_KINETIC_ENERGY;  // Maximum energy

    double lowFactor  = EnergyEngine::computeForceScalingFactor(low);
    double highFactor = EnergyEngine::computeForceScalingFactor(high);

    EXPECT_GT(highFactor, lowFactor);
}

// Low energy (below threshold) -> scaling factor < 1.0 (Requirement 20.4)
TEST(EnergyEngineTest, ForceScaling_LowEnergy_FactorBelowOne) {
    Task t;
    // totalEnergy well below MAX_KINETIC_ENERGY -> sqrt(E/E_max) < 1
    t.totalEnergy = Config::MAX_KINETIC_ENERGY * 0.01;  // 1% of max

    double factor = EnergyEngine::computeForceScalingFactor(t);

    EXPECT_LT(factor, 1.0);
}

// Scaling factor always in [0, 2] regardless of energy value
TEST(EnergyEngineTest, ForceScaling_FactorAlwaysInValidRange) {
    Task t;

    // Zero energy
    t.totalEnergy = 0.0;
    double f0 = EnergyEngine::computeForceScalingFactor(t);
    EXPECT_GE(f0, 0.0);
    EXPECT_LE(f0, 2.0);

    // Max energy
    t.totalEnergy = Config::MAX_KINETIC_ENERGY;
    double fMax = EnergyEngine::computeForceScalingFactor(t);
    EXPECT_GE(fMax, 0.0);
    EXPECT_LE(fMax, 2.0);

    // Very large energy (should be clamped to 2.0)
    t.totalEnergy = Config::MAX_KINETIC_ENERGY * 100.0;
    double fLarge = EnergyEngine::computeForceScalingFactor(t);
    EXPECT_GE(fLarge, 0.0);
    EXPECT_LE(fLarge, 2.0);

    // Negative energy (below reference height)
    t.totalEnergy = -500.0;
    double fNeg = EnergyEngine::computeForceScalingFactor(t);
    EXPECT_GE(fNeg, 0.0);
    EXPECT_LE(fNeg, 2.0);
}
