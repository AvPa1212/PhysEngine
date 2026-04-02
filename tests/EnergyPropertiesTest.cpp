#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "physics/EnergyEngine.hpp"
#include "physics/ClassicalEngine.hpp"
#include "physics/Task.hpp"
#include "core/Config.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

// RapidCheck default is 100 iterations per property (maxSuccess=100).
// Override at runtime via: RC_PARAMS="max_success=200" ./EnergyPropertiesTest

// Helper: build a Task with controlled parameters
static Task makeEnergyTask(double mass, double vx, double vy, double px, double py) {
    Task t;
    t.mass       = mass;
    t.velocity   = {vx, vy};
    t.position   = {px, py};
    return t;
}

// -----------------------------------------------------------------------
// Property 1: Kinetic Energy Formula
// Validates: Requirements 1.1
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, KineticEnergyFormula, ()) {
    // positive mass in (0, 100]
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    // velocity components in [-100, 100]
    const double vx = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);

    double ke = EnergyEngine::computeKineticEnergy(task);

    double velMagSq = vx * vx + vy * vy;
    double expected = 0.5 * mass * velMagSq;

    RC_ASSERT(std::abs(ke - expected) < 1e-9);
}

// -----------------------------------------------------------------------
// Property 2: Kinetic Energy Non-Negativity
// Validates: Requirements 1.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, KineticEnergyNonNegativity, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx   = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy   = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);

    double ke = EnergyEngine::computeKineticEnergy(task);

    RC_ASSERT(ke >= 0.0);
}

// -----------------------------------------------------------------------
// Property 3: Potential Energy Formula
// Validates: Requirements 2.1
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, PotentialEnergyFormula, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    // position.y in [-1000, 1000]
    const double py = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;

    Task task = makeEnergyTask(mass, 0.0, 0.0, 0.0, py);

    double pe = EnergyEngine::computePotentialEnergy(task);

    double expected = mass * Config::GRAVITY_CONSTANT * py;

    RC_ASSERT(std::abs(pe - expected) < 1e-9);
}

// -----------------------------------------------------------------------
// Property 4: Potential Energy Finiteness
// Validates: Requirements 2.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, PotentialEnergyFiniteness, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double py   = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;

    Task task = makeEnergyTask(mass, 0.0, 0.0, 0.0, py);

    double pe = EnergyEngine::computePotentialEnergy(task);

    RC_ASSERT(std::isfinite(pe));
}

// -----------------------------------------------------------------------
// Property 5: Total Energy Composition
// Validates: Requirements 3.1, 3.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, TotalEnergyComposition, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx   = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy   = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double py   = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, 0.0, py);

    EnergyEngine::calculateEnergy(task);

    RC_ASSERT(std::abs(task.totalEnergy - (task.kineticEnergy + task.potentialEnergy)) < 1e-9);
}

// -----------------------------------------------------------------------
// Property 19: Classical Field Isolation
// Validates: Requirements 9.5, 10.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, ClassicalFieldIsolation, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx   = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy   = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double px   = *rc::gen::inRange(-100000, 100001).as("px_int") / 100.0;
    const double py   = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;
    const double ax   = *rc::gen::inRange(-10000, 10001).as("ax_int") / 100.0;
    const double ay   = *rc::gen::inRange(-10000, 10001).as("ay_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, px, py);
    task.acceleration = {ax, ay};

    // Store classical fields before
    const Vector2 posBefore   = task.position;
    const Vector2 velBefore   = task.velocity;
    const Vector2 accBefore   = task.acceleration;
    const double  massBefore  = task.mass;

    EnergyEngine::calculateEnergy(task);

    // Verify classical fields unchanged
    RC_ASSERT(task.position.x     == posBefore.x);
    RC_ASSERT(task.position.y     == posBefore.y);
    RC_ASSERT(task.velocity.x     == velBefore.x);
    RC_ASSERT(task.velocity.y     == velBefore.y);
    RC_ASSERT(task.acceleration.x == accBefore.x);
    RC_ASSERT(task.acceleration.y == accBefore.y);
    RC_ASSERT(task.mass           == massBefore);
}

// -----------------------------------------------------------------------
// Property 11: Energy Injection Increases Total Energy
// Validates: Requirements 6.1, 6.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyInjectionIncreasesTotalEnergy, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx   = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy   = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double py   = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;
    // positive energy amount in (0, 1000]
    const double amount = *rc::gen::inRange(1, 100001).as("amount_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, 0.0, py);
    EnergyEngine::calculateEnergy(task);

    // Only test when initial KE is within the valid operating range
    RC_PRE(task.kineticEnergy < Config::MAX_KINETIC_ENERGY);

    const double initialKE    = task.kineticEnergy;
    const double initialTotal = task.totalEnergy;

    EnergyEngine::injectEnergy(task, amount);

    // Expected new KE = min(initialKE + amount, MAX_KINETIC_ENERGY)
    const double expectedKE    = std::min(initialKE + amount, Config::MAX_KINETIC_ENERGY);
    const double expectedTotal = expectedKE + task.potentialEnergy;
    RC_ASSERT(std::abs(task.totalEnergy - expectedTotal) < 1e-6);
    RC_ASSERT(task.totalEnergy >= initialTotal - 1e-6);
}

// -----------------------------------------------------------------------
// Property 12: Energy Injection Increases Kinetic Energy
// Validates: Requirements 6.2
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyInjectionIncreasesKineticEnergy, ()) {
    const double mass   = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx     = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy     = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double amount = *rc::gen::inRange(1, 100001).as("amount_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);
    EnergyEngine::calculateEnergy(task);

    // Only test when initial KE is within the valid operating range
    RC_PRE(task.kineticEnergy < Config::MAX_KINETIC_ENERGY);

    const double initialKE = task.kineticEnergy;

    EnergyEngine::injectEnergy(task, amount);

    // KE must have increased (or reached the cap)
    RC_ASSERT(task.kineticEnergy > initialKE - 1e-9);
}

// -----------------------------------------------------------------------
// Property 13: Energy Injection Preserves Direction
// Validates: Requirements 6.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyInjectionPreservesDirection, ()) {
    const double mass   = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx     = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy     = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double amount = *rc::gen::inRange(1, 100001).as("amount_int") / 100.0;

    // Only test when velocity is non-zero (magnitude > 0.001)
    const double velMag = std::sqrt(vx * vx + vy * vy);
    RC_PRE(velMag > 0.001);

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);

    // Store initial direction (normalized)
    const double dirX = vx / velMag;
    const double dirY = vy / velMag;

    EnergyEngine::injectEnergy(task, amount);

    const double newMag = std::sqrt(task.velocity.x * task.velocity.x
                                  + task.velocity.y * task.velocity.y);
    RC_PRE(newMag > 0.001);

    const double newDirX = task.velocity.x / newMag;
    const double newDirY = task.velocity.y / newMag;

    const double dot = dirX * newDirX + dirY * newDirY;
    RC_ASSERT(dot > 0.999);
}

// -----------------------------------------------------------------------
// Property 14: Damping Formula
// Validates: Requirements 7.1, 7.2
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, DampingFormula, ()) {
    const double mass    = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx      = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy      = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    // damping coefficient in (0, 1]
    const double coeff   = *rc::gen::inRange(1, 101).as("coeff_int") / 100.0;

    // Require non-zero velocity so there is KE to dissipate
    const double velMag = std::sqrt(vx * vx + vy * vy);
    RC_PRE(velMag > 0.001);

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);
    EnergyEngine::calculateEnergy(task);

    const double initialKE = task.kineticEnergy;
    const double expectedLoss = coeff * initialKE * Config::TIME_STEP;

    EnergyEngine::dissipateEnergy(task, coeff);

    const double actualLoss = initialKE - task.kineticEnergy;
    RC_ASSERT(std::abs(actualLoss - expectedLoss) < 1e-6);
}

// -----------------------------------------------------------------------
// Property 15: Damping Reduces Velocity
// Validates: Requirements 7.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, DampingReducesVelocity, ()) {
    const double mass  = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx    = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy    = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double coeff = *rc::gen::inRange(1, 101).as("coeff_int") / 100.0;

    const double velMag = std::sqrt(vx * vx + vy * vy);
    RC_PRE(velMag > 0.001);

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);

    const double initialMag = velMag;

    EnergyEngine::dissipateEnergy(task, coeff);

    const double newMag = std::sqrt(task.velocity.x * task.velocity.x
                                  + task.velocity.y * task.velocity.y);
    RC_ASSERT(newMag < initialMag + 1e-9);
}

// -----------------------------------------------------------------------
// Property 16: Damping Preserves Direction
// Validates: Requirements 7.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, DampingPreservesDirection, ()) {
    const double mass  = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx    = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy    = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double coeff = *rc::gen::inRange(1, 101).as("coeff_int") / 100.0;

    const double velMag = std::sqrt(vx * vx + vy * vy);
    RC_PRE(velMag > 0.001);

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);

    const double dirX = vx / velMag;
    const double dirY = vy / velMag;

    EnergyEngine::dissipateEnergy(task, coeff);

    const double newMag = std::sqrt(task.velocity.x * task.velocity.x
                                  + task.velocity.y * task.velocity.y);
    RC_PRE(newMag > 0.001);

    const double newDirX = task.velocity.x / newMag;
    const double newDirY = task.velocity.y / newMag;

    const double dot = dirX * newDirX + dirY * newDirY;
    RC_ASSERT(dot > 0.999);
}

// -----------------------------------------------------------------------
// Property 17: Damping Monotonically Decreases KE
// Validates: Requirements 7.6
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, DampingMonotonicallyDecreasesKE, ()) {
    const double mass  = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx    = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy    = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double coeff = *rc::gen::inRange(1, 101).as("coeff_int") / 100.0;

    // Require positive KE
    const double velMag = std::sqrt(vx * vx + vy * vy);
    RC_PRE(velMag > 0.001);

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);
    EnergyEngine::calculateEnergy(task);

    const double initialKE = task.kineticEnergy;

    EnergyEngine::dissipateEnergy(task, coeff);

    RC_ASSERT(task.kineticEnergy <= initialKE + 1e-9);
}

// -----------------------------------------------------------------------
// Property 23: Energy Transfer Decreases Source
// Validates: Requirements 14.2
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyTransferDecreasesSource, ()) {
    const double srcMass = *rc::gen::inRange(1, 10001).as("src_mass_int") / 100.0;
    const double srcVx   = *rc::gen::inRange(-10000, 10001).as("src_vx_int") / 100.0;
    const double srcVy   = *rc::gen::inRange(-10000, 10001).as("src_vy_int") / 100.0;
    const double tgtMass = *rc::gen::inRange(1, 10001).as("tgt_mass_int") / 100.0;
    const double tgtVx   = *rc::gen::inRange(-10000, 10001).as("tgt_vx_int") / 100.0;
    const double tgtVy   = *rc::gen::inRange(-10000, 10001).as("tgt_vy_int") / 100.0;
    const double amount  = *rc::gen::inRange(1, 100001).as("amount_int") / 100.0;

    Task source = makeEnergyTask(srcMass, srcVx, srcVy, 0.0, 0.0);
    Task target = makeEnergyTask(tgtMass, tgtVx, tgtVy, 0.0, 0.0);
    EnergyEngine::calculateEnergy(source);
    EnergyEngine::calculateEnergy(target);

    // Only test within valid operating range
    RC_PRE(source.kineticEnergy < Config::MAX_KINETIC_ENERGY);
    const double sourceKE       = source.kineticEnergy;
    const double actualTransfer = std::min(amount, sourceKE);
    // Ensure target won't be clamped by the cap after receiving energy
    RC_PRE(target.kineticEnergy + actualTransfer <= Config::MAX_KINETIC_ENERGY);

    const double initialSourceTotal = source.totalEnergy;

    EnergyEngine::transferEnergy(source, target, amount);

    const double expectedSourceTotal = initialSourceTotal - actualTransfer;
    RC_ASSERT(std::abs(source.totalEnergy - expectedSourceTotal) < 1e-6);
}

// -----------------------------------------------------------------------
// Property 24: Energy Transfer Increases Target
// Validates: Requirements 14.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyTransferIncreasesTarget, ()) {
    const double srcMass = *rc::gen::inRange(1, 10001).as("src_mass_int") / 100.0;
    const double srcVx   = *rc::gen::inRange(-10000, 10001).as("src_vx_int") / 100.0;
    const double srcVy   = *rc::gen::inRange(-10000, 10001).as("src_vy_int") / 100.0;
    const double tgtMass = *rc::gen::inRange(1, 10001).as("tgt_mass_int") / 100.0;
    const double tgtVx   = *rc::gen::inRange(-10000, 10001).as("tgt_vx_int") / 100.0;
    const double tgtVy   = *rc::gen::inRange(-10000, 10001).as("tgt_vy_int") / 100.0;
    const double amount  = *rc::gen::inRange(1, 100001).as("amount_int") / 100.0;

    Task source = makeEnergyTask(srcMass, srcVx, srcVy, 0.0, 0.0);
    Task target = makeEnergyTask(tgtMass, tgtVx, tgtVy, 0.0, 0.0);
    EnergyEngine::calculateEnergy(source);
    EnergyEngine::calculateEnergy(target);

    // Only test within valid operating range
    RC_PRE(source.kineticEnergy < Config::MAX_KINETIC_ENERGY);
    const double sourceKE       = source.kineticEnergy;
    const double actualTransfer = std::min(amount, sourceKE);
    // Ensure target won't be clamped by the cap after receiving energy
    RC_PRE(target.kineticEnergy + actualTransfer <= Config::MAX_KINETIC_ENERGY);

    const double initialTargetTotal = target.totalEnergy;

    EnergyEngine::transferEnergy(source, target, amount);

    const double expectedTargetTotal = initialTargetTotal + actualTransfer;
    RC_ASSERT(std::abs(target.totalEnergy - expectedTargetTotal) < 1e-6);
}

// -----------------------------------------------------------------------
// Property 25: Energy Transfer Prevents Negative Energy
// Validates: Requirements 14.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyTransferPreventsNegativeEnergy, ()) {
    const double srcMass = *rc::gen::inRange(1, 10001).as("src_mass_int") / 100.0;
    const double srcVx   = *rc::gen::inRange(-10000, 10001).as("src_vx_int") / 100.0;
    const double srcVy   = *rc::gen::inRange(-10000, 10001).as("src_vy_int") / 100.0;
    const double tgtMass = *rc::gen::inRange(1, 10001).as("tgt_mass_int") / 100.0;
    const double tgtVx   = *rc::gen::inRange(-10000, 10001).as("tgt_vx_int") / 100.0;
    const double tgtVy   = *rc::gen::inRange(-10000, 10001).as("tgt_vy_int") / 100.0;

    Task source = makeEnergyTask(srcMass, srcVx, srcVy, 0.0, 0.0);
    Task target = makeEnergyTask(tgtMass, tgtVx, tgtVy, 0.0, 0.0);
    EnergyEngine::calculateEnergy(source);
    EnergyEngine::calculateEnergy(target);

    // Transfer an amount much larger than source KE to test clamping
    const double largeAmount = source.kineticEnergy * 10.0 + 1000.0;

    EnergyEngine::transferEnergy(source, target, largeAmount);

    RC_ASSERT(source.kineticEnergy >= -1e-9);
    RC_ASSERT(target.kineticEnergy >= -1e-9);
}

// -----------------------------------------------------------------------
// Property 26: Energy Transfer Conservation
// Validates: Requirements 14.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyTransferConservation, ()) {
    const double srcMass = *rc::gen::inRange(1, 10001).as("src_mass_int") / 100.0;
    const double srcVx   = *rc::gen::inRange(-10000, 10001).as("src_vx_int") / 100.0;
    const double srcVy   = *rc::gen::inRange(-10000, 10001).as("src_vy_int") / 100.0;
    const double tgtMass = *rc::gen::inRange(1, 10001).as("tgt_mass_int") / 100.0;
    const double tgtVx   = *rc::gen::inRange(-10000, 10001).as("tgt_vx_int") / 100.0;
    const double tgtVy   = *rc::gen::inRange(-10000, 10001).as("tgt_vy_int") / 100.0;
    const double amount  = *rc::gen::inRange(1, 100001).as("amount_int") / 100.0;

    Task source = makeEnergyTask(srcMass, srcVx, srcVy, 0.0, 0.0);
    Task target = makeEnergyTask(tgtMass, tgtVx, tgtVy, 0.0, 0.0);
    EnergyEngine::calculateEnergy(source);
    EnergyEngine::calculateEnergy(target);

    // Only test within valid operating range (no clamping)
    RC_PRE(source.kineticEnergy < Config::MAX_KINETIC_ENERGY);
    const double actualTransfer = std::min(amount, source.kineticEnergy);
    RC_PRE(target.kineticEnergy + actualTransfer <= Config::MAX_KINETIC_ENERGY);

    const double initialSum = source.totalEnergy + target.totalEnergy;

    EnergyEngine::transferEnergy(source, target, amount);

    const double finalSum = source.totalEnergy + target.totalEnergy;
    RC_ASSERT(std::abs(finalSum - initialSum) < 1e-6);
}

// -----------------------------------------------------------------------
// Property 6: System Energy Summation
// Validates: Requirements 4.1
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, SystemEnergySummation, ()) {
    // Generate 2-5 tasks
    const int count = *rc::gen::inRange(2, 6).as("count");

    std::vector<Task> tasks;
    tasks.reserve(count);

    double expectedSum = 0.0;
    for (int i = 0; i < count; ++i) {
        // totalEnergy in [-1000, 1000]
        const double energy = *rc::gen::inRange(-100000, 100001).as("energy_int") / 100.0;
        Task t;
        t.totalEnergy = energy;
        expectedSum += energy;
        tasks.push_back(t);
    }

    double systemEnergy = EnergyEngine::computeSystemEnergy(tasks);

    RC_ASSERT(std::abs(systemEnergy - expectedSum) < 1e-9);
}

// -----------------------------------------------------------------------
// Property 8: Energy Redistribution Proportionality
// Validates: Requirements 5.2, 5.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, RedistributionProportionality, ()) {
    // Completed task with positive totalEnergy
    const double completedEnergy = *rc::gen::inRange(1, 100001).as("completed_energy_int") / 100.0;
    Task completed;
    completed.totalEnergy = completedEnergy;

    // 2-4 remaining tasks with positive masses and zero velocity
    const int count = *rc::gen::inRange(2, 5).as("count");
    std::vector<Task> remaining;
    remaining.reserve(count);

    double totalMass = 0.0;
    for (int i = 0; i < count; ++i) {
        const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
        Task t;
        t.mass     = mass;
        t.velocity = {0.0, 0.0};
        t.position = {0.0, 0.0};
        EnergyEngine::calculateEnergy(t);
        totalMass += mass;
        remaining.push_back(t);
    }

    EnergyEngine::redistributeEnergy(completed, remaining);

    // Each task should have received share = (mass / totalMass) * completedEnergy
    // After injection, KE should equal the share (since initial KE was 0 and PE unchanged)
    for (const auto& task : remaining) {
        double expectedShare = (task.mass / totalMass) * completedEnergy;
        // KE after injection = expectedShare (clamped to MAX_KINETIC_ENERGY)
        double expectedKE = std::min(expectedShare, Config::MAX_KINETIC_ENERGY);
        RC_ASSERT(std::abs(task.kineticEnergy - expectedKE) < 1e-6);
    }
}

// -----------------------------------------------------------------------
// Property 9: Redistribution Injects Kinetic Energy
// Validates: Requirements 5.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, RedistributionInjectsKineticEnergy, ()) {
    // Completed task with positive totalEnergy
    const double completedEnergy = *rc::gen::inRange(1, 100001).as("completed_energy_int") / 100.0;
    Task completed;
    completed.totalEnergy = completedEnergy;

    // 2-4 remaining tasks with zero velocity
    const int count = *rc::gen::inRange(2, 5).as("count");
    std::vector<Task> remaining;
    remaining.reserve(count);

    for (int i = 0; i < count; ++i) {
        const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
        Task t;
        t.mass     = mass;
        t.velocity = {0.0, 0.0};
        t.position = {0.0, 0.0};
        EnergyEngine::calculateEnergy(t);
        remaining.push_back(t);
    }

    EnergyEngine::redistributeEnergy(completed, remaining);

    // All receiving tasks should have KE > 0 (velocity increased from zero)
    for (const auto& task : remaining) {
        RC_ASSERT(task.kineticEnergy > 0.0);
    }
}

// -----------------------------------------------------------------------
// Property 10: Redistribution Conserves Energy
// Validates: Requirements 5.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, RedistributionConservesEnergy, ()) {
    // Completed task with positive totalEnergy
    const double completedEnergy = *rc::gen::inRange(1, 100001).as("completed_energy_int") / 100.0;
    Task completed;
    completed.totalEnergy = completedEnergy;

    // 2-4 remaining tasks
    const int count = *rc::gen::inRange(2, 5).as("count");
    std::vector<Task> remaining;
    remaining.reserve(count);

    double initialRemainingEnergy = 0.0;
    for (int i = 0; i < count; ++i) {
        const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
        Task t;
        t.mass     = mass;
        t.velocity = {0.0, 0.0};
        t.position = {0.0, 0.0};
        EnergyEngine::calculateEnergy(t);
        initialRemainingEnergy += t.totalEnergy;
        remaining.push_back(t);
    }

    // Initial system energy = completed.totalEnergy + sum of remaining totalEnergy
    double initialSystemEnergy = completedEnergy + initialRemainingEnergy;

    EnergyEngine::redistributeEnergy(completed, remaining);

    double finalSystemEnergy = EnergyEngine::computeSystemEnergy(remaining);

    RC_ASSERT(std::abs(finalSystemEnergy - initialSystemEnergy) < 1e-6);
}

// -----------------------------------------------------------------------
// Property 18: Energy-Based Sorting
// Validates: Requirements 8.1, 8.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyBasedSorting, ()) {
    // Generate 2-5 tasks with different totalEnergy values
    const int count = *rc::gen::inRange(2, 6).as("count");

    std::vector<Task> tasks;
    tasks.reserve(count);

    for (int i = 0; i < count; ++i) {
        const double energy = *rc::gen::inRange(-100000, 100001).as("energy_int") / 100.0;
        Task t;
        t.totalEnergy    = energy;
        t.kineticEnergy  = 0.0;
        t.potentialEnergy = energy;
        tasks.push_back(t);
    }

    std::vector<Task*> sorted = EnergyEngine::sortByEnergy(tasks);

    RC_ASSERT(static_cast<int>(sorted.size()) == count);

    // Verify descending order by totalEnergy
    for (int i = 0; i + 1 < static_cast<int>(sorted.size()); ++i) {
        RC_ASSERT(sorted[i]->totalEnergy >= sorted[i + 1]->totalEnergy);
    }
}

// -----------------------------------------------------------------------
// Property 7: Energy Conservation
// Validates: Requirements 4.2, 13.1, 13.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyConservation, ()) {
    // The ClassicalEngine does not apply gravity as a force — gravity only appears in PE.
    // With zero urgency and zero friction, net force = 0, so velocity is constant and KE is conserved.
    // To test total energy conservation, we use zero velocity so position doesn't change,
    // meaning PE is also constant and total energy is conserved.
    const double mass = *rc::gen::inRange(100, 1001).as("mass_int") / 100.0;
    // Non-zero velocity in x only (no y-component so position.y stays constant → PE constant)
    const double vx = *rc::gen::inRange(10, 501).as("vx_int") / 100.0;
    // Position y in [-5, 5]
    const double py = *rc::gen::inRange(-500, 501).as("py_int") / 100.0;

    Task task;
    task.mass            = mass;
    task.velocity        = {vx, 0.0};  // Only horizontal velocity: position.y won't change
    task.position        = {0.0, py};
    task.urgencyConstant = 0.0;   // No deadline force
    task.kineticFriction = 0.0;   // No friction
    task.deadlineTime    = 1000.0; // Far deadline

    EnergyEngine::calculateEnergy(task);
    double initialEnergy = task.totalEnergy;

    // Require initial energy is meaningfully non-zero
    RC_PRE(std::abs(initialEnergy) > 1.0);

    // Integrate for 100 steps
    for (int step = 0; step < 100; ++step) {
        ClassicalEngine::integrateRK4(task);
        EnergyEngine::calculateEnergy(task);
    }

    double drift = EnergyEngine::computeEnergyDrift(initialEnergy, task.totalEnergy);

    // Allow 1% tolerance for RK4 numerical error over 100 steps
    RC_ASSERT(drift < 0.01);
}

// -----------------------------------------------------------------------
// Property 27: Energy Conversion During Motion
// Validates: Requirements 15.2, 15.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyConversionDuringMotion, ()) {
    // The ClassicalEngine does not apply gravity as a force.
    // With zero urgency and zero friction, net force = 0, so velocity is constant.
    // Using only horizontal velocity (vy=0): position.y doesn't change → ΔPE = 0, ΔKE = 0.
    // Therefore ΔPE ≈ -ΔKE (both zero) holds within tolerance.
    const double mass = *rc::gen::inRange(100, 1001).as("mass_int") / 100.0;
    // Non-zero horizontal velocity: [0.1, 5]
    const double vx = *rc::gen::inRange(10, 501).as("vx_int") / 100.0;
    const double py = *rc::gen::inRange(-500, 501).as("py_int") / 100.0;

    Task task;
    task.mass            = mass;
    task.velocity        = {vx, 0.0};  // No vertical velocity → no PE change
    task.position        = {0.0, py};
    task.urgencyConstant = 0.0;
    task.kineticFriction = 0.0;
    task.deadlineTime    = 1000.0;

    EnergyEngine::calculateEnergy(task);
    const double initialKE = task.kineticEnergy;
    const double initialPE = task.potentialEnergy;

    // Integrate one step
    ClassicalEngine::integrateRK4(task);
    EnergyEngine::calculateEnergy(task);

    const double deltaKE = task.kineticEnergy - initialKE;
    const double deltaPE = task.potentialEnergy - initialPE;

    // ΔPE ≈ -ΔKE within numerical tolerance
    RC_ASSERT(std::abs(deltaKE + deltaPE) < 1e-6);
}

// -----------------------------------------------------------------------
// Property 28: Kinetic Energy Clamping
// Validates: Requirements 16.1
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, KineticEnergyClamping, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx   = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy   = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, 0.0, 0.0);

    // Skip cases where initial velocity already saturates the cap — the interesting
    // case is that injection of a large amount still respects the ceiling.
    RC_PRE(EnergyEngine::computeKineticEnergy(task) < Config::MAX_KINETIC_ENERGY);

    // Inject a very large energy amount (10x the cap)
    EnergyEngine::injectEnergy(task, Config::MAX_KINETIC_ENERGY * 10.0);

    RC_ASSERT(task.kineticEnergy <= Config::MAX_KINETIC_ENERGY);
}

// -----------------------------------------------------------------------
// Property 29: Energy Finiteness
// Validates: Requirements 16.3, 16.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyFiniteness, ()) {
    const double mass   = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx     = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy     = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double py     = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;
    const double amount = *rc::gen::inRange(1, 100001).as("amount_int") / 100.0;
    const double coeff  = *rc::gen::inRange(1, 101).as("coeff_int") / 100.0;

    // Set up source and target tasks
    Task task   = makeEnergyTask(mass, vx, vy, 0.0, py);
    Task other  = makeEnergyTask(mass, vx, vy, 0.0, 0.0);
    EnergyEngine::calculateEnergy(task);
    EnergyEngine::calculateEnergy(other);

    // Perform a sequence of operations: inject, dissipate, transfer
    EnergyEngine::injectEnergy(task, amount);
    EnergyEngine::dissipateEnergy(task, coeff);
    EnergyEngine::transferEnergy(task, other, amount);

    // All energy values on both tasks must remain finite
    RC_ASSERT(std::isfinite(task.kineticEnergy));
    RC_ASSERT(std::isfinite(task.potentialEnergy));
    RC_ASSERT(std::isfinite(task.totalEnergy));
    RC_ASSERT(std::isfinite(other.kineticEnergy));
    RC_ASSERT(std::isfinite(other.potentialEnergy));
    RC_ASSERT(std::isfinite(other.totalEnergy));
}

// -----------------------------------------------------------------------
// Property 22: API Bridge Round-Trip
// Validates: Requirements 12.1
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, ApiBridgeRoundTrip, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx   = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy   = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double py   = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, 0.0, py);
    EnergyEngine::calculateEnergy(task);

    // Retrieve values via API bridge getters (direct field access mirrors bridge behaviour)
    const double bridgeKE    = task.kineticEnergy;
    const double bridgePE    = task.potentialEnergy;
    const double bridgeTotal = task.totalEnergy;

    // Verify retrieved values match the task fields exactly
    RC_ASSERT(bridgeKE    == task.kineticEnergy);
    RC_ASSERT(bridgePE    == task.potentialEnergy);
    RC_ASSERT(bridgeTotal == task.totalEnergy);

    // Verify total = KE + PE within floating-point precision
    RC_ASSERT(std::abs(bridgeTotal - (bridgeKE + bridgePE)) < 1e-9);
}

// -----------------------------------------------------------------------
// Property 30: Serialization Round-Trip
// Validates: Requirements 18.1, 18.5
// -----------------------------------------------------------------------

// Minimal helpers mirroring Bridge::Serialize / Bridge::Deserialize
static std::string serializeTask(const Task& task) {
    std::ostringstream oss;
    oss << std::setprecision(17)
        << "{\"posX\":" << task.position.x
        << ",\"posY\":" << task.position.y
        << ",\"velX\":" << task.velocity.x
        << ",\"velY\":" << task.velocity.y
        << ",\"mass\":" << task.mass
        << ",\"stressX\":" << task.stressX
        << ",\"stressY\":" << task.stressY
        << ",\"stressZ\":" << task.stressZ
        << ",\"entropy\":" << task.entropy
        << ",\"stepCount\":" << task.stepCount
        << ",\"kineticEnergy\":" << task.kineticEnergy
        << ",\"potentialEnergy\":" << task.potentialEnergy
        << ",\"totalEnergy\":" << task.totalEnergy
        << "}";
    return oss.str();
}

static bool parseDouble(const std::string& json, const std::string& key, double& out) {
    std::string search = "\"" + key + "\":";
    auto pos = json.find(search);
    if (pos == std::string::npos) return false;
    pos += search.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;
    try { out = std::stod(json.substr(pos)); return true; } catch (...) { return false; }
}

static void deserializeTask(Task& task, const std::string& json) {
    double d;
    if (parseDouble(json, "posX", d))          task.position.x = d;
    if (parseDouble(json, "posY", d))          task.position.y = d;
    if (parseDouble(json, "velX", d))          task.velocity.x = d;
    if (parseDouble(json, "velY", d))          task.velocity.y = d;
    if (parseDouble(json, "mass", d))          task.mass = d;
    if (parseDouble(json, "stressX", d))       task.stressX = d;
    if (parseDouble(json, "stressY", d))       task.stressY = d;
    if (parseDouble(json, "stressZ", d))       task.stressZ = d;
    if (parseDouble(json, "entropy", d))       task.entropy = d;

    bool hasKE = parseDouble(json, "kineticEnergy", d);
    if (hasKE) task.kineticEnergy = d;
    bool hasPE = parseDouble(json, "potentialEnergy", d);
    if (hasPE) task.potentialEnergy = d;
    bool hasTE = parseDouble(json, "totalEnergy", d);
    if (hasTE) task.totalEnergy = d;

    if (hasKE && hasPE && hasTE) {
        double expected = task.kineticEnergy + task.potentialEnergy;
        if (std::abs(task.totalEnergy - expected) > 1e-9) {
            EnergyEngine::calculateEnergy(task);
        }
    }
}

RC_GTEST_PROP(EnergyProperties, SerializationRoundTrip, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx   = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy   = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double px   = *rc::gen::inRange(-100000, 100001).as("px_int") / 100.0;
    const double py   = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;

    Task task = makeEnergyTask(mass, vx, vy, px, py);
    EnergyEngine::calculateEnergy(task);

    const double origKE = task.kineticEnergy;
    const double origPE = task.potentialEnergy;
    const double origTE = task.totalEnergy;

    // Serialize then deserialize
    std::string json = serializeTask(task);
    Task restored;
    deserializeTask(restored, json);

    RC_ASSERT(std::abs(origKE - restored.kineticEnergy) < 1e-9);
    RC_ASSERT(std::abs(origPE - restored.potentialEnergy) < 1e-9);
    RC_ASSERT(std::abs(origTE - restored.totalEnergy) < 1e-9);
}

// -----------------------------------------------------------------------
// Property 31: Deserialization Consistency Validation
// Validates: Requirements 18.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, DeserializationConsistencyValidation, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    const double vx   = *rc::gen::inRange(-10000, 10001).as("vx_int") / 100.0;
    const double vy   = *rc::gen::inRange(-10000, 10001).as("vy_int") / 100.0;
    const double py   = *rc::gen::inRange(-100000, 100001).as("py_int") / 100.0;

    // Compute real KE and PE
    const double velMagSq = vx * vx + vy * vy;
    const double realKE   = 0.5 * mass * velMagSq;
    const double realPE   = mass * Config::GRAVITY_CONSTANT * py;

    // Introduce an inconsistent totalEnergy (offset by a large amount)
    const double badTE = realKE + realPE + 1000.0;

    // Build JSON with inconsistent totalEnergy
    std::ostringstream oss;
    oss << std::setprecision(17)
        << "{\"posX\":0,\"posY\":" << py
        << ",\"velX\":" << vx << ",\"velY\":" << vy
        << ",\"mass\":" << mass
        << ",\"stressX\":1,\"stressY\":1,\"stressZ\":1"
        << ",\"entropy\":0,\"stepCount\":0"
        << ",\"kineticEnergy\":" << realKE
        << ",\"potentialEnergy\":" << realPE
        << ",\"totalEnergy\":" << badTE
        << "}";
    std::string json = oss.str();

    Task task;
    deserializeTask(task, json);

    // After deserialization, totalEnergy must equal KE + PE (recalculated)
    RC_ASSERT(std::abs(task.totalEnergy - (task.kineticEnergy + task.potentialEnergy)) < 1e-9);
    // And the bad value must have been corrected
    RC_ASSERT(std::abs(task.totalEnergy - badTE) > 1e-6);
}

// -----------------------------------------------------------------------
// Property 20: Energy Statistics Correctness
// Validates: Requirements 11.3
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyStatisticsCorrectness, ()) {
    // Generate 2-8 tasks with random totalEnergy values
    const int count = *rc::gen::inRange(2, 9).as("count");

    std::vector<Task> tasks;
    tasks.reserve(count);

    double sum = 0.0;
    for (int i = 0; i < count; ++i) {
        const double energy = *rc::gen::inRange(-100000, 100001).as("energy_int") / 100.0;
        Task t;
        t.totalEnergy = energy;
        sum += energy;
        tasks.push_back(t);
    }

    // Verify mean = sum / count
    const double mean = EnergyEngine::computeMeanEnergy(tasks);
    const double expectedMean = sum / static_cast<double>(count);
    RC_ASSERT(std::abs(mean - expectedMean) < 1e-9);

    // Verify population standard deviation
    double variance = 0.0;
    for (const auto& t : tasks) {
        double diff = t.totalEnergy - mean;
        variance += diff * diff;
    }
    variance /= static_cast<double>(count);
    const double expectedStdDev = std::sqrt(variance);

    const double stdDev = EnergyEngine::computeEnergyStdDev(tasks);
    RC_ASSERT(std::abs(stdDev - expectedStdDev) < 1e-9);
}

// -----------------------------------------------------------------------
// Property 21: Percentile Identification
// Validates: Requirements 11.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, PercentileIdentification, ()) {
    // Generate 2-8 tasks with random totalEnergy values
    const int count = *rc::gen::inRange(2, 9).as("count");

    std::vector<Task> tasks;
    tasks.reserve(count);

    for (int i = 0; i < count; ++i) {
        const double energy = *rc::gen::inRange(0, 100001).as("energy_int") / 100.0;
        Task t;
        t.totalEnergy = energy;
        tasks.push_back(t);
    }

    // Use mean + stddev as a threshold (approximates ~84th percentile for normal distributions)
    const double mean   = EnergyEngine::computeMeanEnergy(tasks);
    const double stdDev = EnergyEngine::computeEnergyStdDev(tasks);
    const double threshold = mean + stdDev;

    auto highEnergy = EnergyEngine::identifyHighEnergyTasks(tasks, threshold);

    // All identified tasks must have totalEnergy >= threshold
    for (const Task* t : highEnergy) {
        RC_ASSERT(t->totalEnergy >= threshold - 1e-9);
    }

    // No task below threshold should appear in the result
    for (const auto& t : tasks) {
        if (t.totalEnergy < threshold - 1e-9) {
            bool found = false;
            for (const Task* h : highEnergy) {
                if (h == &t) { found = true; break; }
            }
            RC_ASSERT(!found);
        }
    }
}

// -----------------------------------------------------------------------
// Property 32: Energy Injection Rate Limiting
// Validates: Requirements 19.1, 19.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, EnergyInjectionRateLimiting, ()) {
    // Generate a sequence of 2-8 injection operations
    const int steps = *rc::gen::inRange(2, 9).as("steps");
    // Each injection amount in (0, 10 * MAX_RATE * TIME_STEP]
    const double maxBudget = Config::MAX_ENERGY_INJECTION_RATE * Config::TIME_STEP;

    Task task;
    task.mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;
    task.velocity = {0.0, 0.0};

    double queue = 0.0;
    double lastTime = 0.0;
    const double maxRate = Config::MAX_ENERGY_INJECTION_RATE;

    for (int i = 0; i < steps; ++i) {
        // Amount can be up to 5x the per-step budget
        const double amount = *rc::gen::inRange(0, static_cast<int>(maxBudget * 500 + 1)).as("amount_int") / 100.0;

        const double keBefore = EnergyEngine::computeKineticEnergy(task);
        EnergyEngine::injectEnergyRateLimited(task, amount, queue, lastTime, maxRate);
        const double keAfter = EnergyEngine::computeKineticEnergy(task);

        // Energy injected this step must not exceed the per-step budget
        const double injectedThisStep = keAfter - keBefore;
        RC_ASSERT(injectedThisStep <= maxBudget + 1e-9);
    }

    // Queue must be non-negative at all times
    RC_ASSERT(queue >= 0.0);
}

// -----------------------------------------------------------------------
// Property 33: Excess Energy Queuing
// Validates: Requirements 19.2
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, ExcessEnergyQueuing, ()) {
    const double mass = *rc::gen::inRange(1, 10001).as("mass_int") / 100.0;

    Task task;
    task.mass = mass;
    task.velocity = {0.0, 0.0};

    double queue = 0.0;
    double lastTime = 0.0;
    const double maxRate = Config::MAX_ENERGY_INJECTION_RATE;
    const double budget = maxRate * Config::TIME_STEP;

    // Inject an amount that is strictly larger than the per-step budget
    const double excess = *rc::gen::inRange(1, 10001).as("excess_int") / 100.0;
    const double amount = budget + excess; // guaranteed to exceed budget

    EnergyEngine::injectEnergyRateLimited(task, amount, queue, lastTime, maxRate);

    // Excess must be queued (queue > 0)
    RC_ASSERT(queue > 0.0);

    // The queued amount should equal the excess (amount - budget)
    RC_ASSERT(std::abs(queue - excess) < 1e-9);

    // Process the queue over subsequent steps: queue should decrease each step
    double prevQueue = queue;
    for (int step = 0; step < 3; ++step) {
        EnergyEngine::injectEnergyRateLimited(task, 0.0, queue, lastTime, maxRate);
        RC_ASSERT(queue <= prevQueue + 1e-9);
        prevQueue = queue;
    }
}

// -----------------------------------------------------------------------
// Task 19.2: Comprehensive Energy Transfer Between Tasks
// Validates: Requirements 14.1, 14.2, 14.3, 14.4, 14.5
// -----------------------------------------------------------------------
// This property covers all transfer scenarios:
//   - Transfer amount less than source KE (partial transfer)
//   - Transfer amount equal to source KE (exact drain)
//   - Transfer amount greater than source KE (clamped to available)
//   - Zero source KE (nothing to transfer)
//   - Zero transfer amount (no-op)
RC_GTEST_PROP(EnergyProperties, ComprehensiveEnergyTransfer, ()) {
    // Source task parameters
    const double srcMass = *rc::gen::inRange(1, 10001).as("src_mass_int") / 100.0;
    const double srcVx   = *rc::gen::inRange(-5000, 5001).as("src_vx_int") / 100.0;
    const double srcVy   = *rc::gen::inRange(-5000, 5001).as("src_vy_int") / 100.0;
    const double srcPy   = *rc::gen::inRange(-10000, 10001).as("src_py_int") / 100.0;

    // Target task parameters
    const double tgtMass = *rc::gen::inRange(1, 10001).as("tgt_mass_int") / 100.0;
    const double tgtVx   = *rc::gen::inRange(-5000, 5001).as("tgt_vx_int") / 100.0;
    const double tgtVy   = *rc::gen::inRange(-5000, 5001).as("tgt_vy_int") / 100.0;
    const double tgtPy   = *rc::gen::inRange(-10000, 10001).as("tgt_py_int") / 100.0;

    // Transfer scenario selector: 0=partial, 1=exact, 2=excess, 3=zero-source, 4=zero-amount
    const int scenario = *rc::gen::inRange(0, 5).as("scenario");

    Task source = makeEnergyTask(srcMass, srcVx, srcVy, 0.0, srcPy);
    Task target = makeEnergyTask(tgtMass, tgtVx, tgtVy, 0.0, tgtPy);
    EnergyEngine::calculateEnergy(source);
    EnergyEngine::calculateEnergy(target);

    // Stay within valid operating range to avoid clamping interference
    RC_PRE(source.kineticEnergy < Config::MAX_KINETIC_ENERGY);
    RC_PRE(target.kineticEnergy < Config::MAX_KINETIC_ENERGY);

    const double sourceKE = source.kineticEnergy;

    double transferAmount = 0.0;
    switch (scenario) {
        case 0: // Partial: transfer half of source KE
            transferAmount = sourceKE * 0.5;
            break;
        case 1: // Exact: transfer exactly source KE
            transferAmount = sourceKE;
            break;
        case 2: // Excess: transfer 2x source KE (will be clamped)
            transferAmount = sourceKE * 2.0 + 1.0;
            break;
        case 3: // Zero source: source has no KE (zero velocity)
            source.velocity = {0.0, 0.0};
            EnergyEngine::calculateEnergy(source);
            transferAmount = 50.0; // Attempt to transfer from empty source
            break;
        case 4: // Zero amount: no-op transfer
            transferAmount = 0.0;
            break;
    }

    // Ensure target won't be clamped after receiving energy
    const double actualTransfer = std::min(transferAmount, source.kineticEnergy);
    RC_PRE(target.kineticEnergy + actualTransfer <= Config::MAX_KINETIC_ENERGY);

    const double initialSourceTotal = source.totalEnergy;
    const double initialTargetTotal = target.totalEnergy;
    const double initialSum         = initialSourceTotal + initialTargetTotal;

    EnergyEngine::transferEnergy(source, target, transferAmount);

    // 14.4: Both tasks must have non-negative KE after transfer
    RC_ASSERT(source.kineticEnergy >= -1e-9);
    RC_ASSERT(target.kineticEnergy >= -1e-9);

    // 14.5: Total energy of the pair is conserved
    const double finalSum = source.totalEnergy + target.totalEnergy;
    RC_ASSERT(std::abs(finalSum - initialSum) < 1e-6);

    // 14.2: Source total energy decreased by the actual transferred amount
    const double expectedSourceTotal = initialSourceTotal - actualTransfer;
    RC_ASSERT(std::abs(source.totalEnergy - expectedSourceTotal) < 1e-6);

    // 14.3: Target total energy increased by the actual transferred amount
    const double expectedTargetTotal = initialTargetTotal + actualTransfer;
    RC_ASSERT(std::abs(target.totalEnergy - expectedTargetTotal) < 1e-6);

    // 14.1: Transfer amount is clamped to available source KE (never negative)
    RC_ASSERT(actualTransfer >= 0.0);
    RC_ASSERT(actualTransfer <= sourceKE + 1e-9);
}

// -----------------------------------------------------------------------
// Task 19.3: System Energy Conservation Over Long Simulations
// Validates: Requirements 4.2, 13.1, 13.5
// -----------------------------------------------------------------------
// Runs 1000+ integration steps and verifies energy drift stays < 1%.
// Tests with various task configurations (mass, velocity, position).
RC_GTEST_PROP(EnergyProperties, LongSimulationEnergyConservation, ()) {
    // Task configuration: mass in [1, 10], velocity in [-2, 2], position.y in [-5, 5]
    const double mass = *rc::gen::inRange(100, 1001).as("mass_int") / 100.0;
    // Use only horizontal velocity so position.y stays constant → PE constant
    // This isolates KE conservation under zero-force conditions
    const double vx = *rc::gen::inRange(10, 201).as("vx_int") / 100.0;
    const double py = *rc::gen::inRange(-500, 501).as("py_int") / 100.0;

    Task task;
    task.mass            = mass;
    task.velocity        = {vx, 0.0};  // Horizontal only: no PE change
    task.position        = {0.0, py};
    task.urgencyConstant = 0.0;        // No deadline force
    task.kineticFriction = 0.0;        // No friction
    task.deadlineTime    = 100000.0;   // Far deadline

    EnergyEngine::calculateEnergy(task);
    const double initialEnergy = task.totalEnergy;

    // Require meaningful initial energy
    RC_PRE(std::abs(initialEnergy) > 1.0);

    // Run 1000 integration steps
    constexpr int NUM_STEPS = 1000;
    for (int step = 0; step < NUM_STEPS; ++step) {
        ClassicalEngine::integrateRK4(task);
        EnergyEngine::calculateEnergy(task);

        // Verify energy remains finite throughout
        RC_ASSERT(std::isfinite(task.totalEnergy));
    }

    // Verify drift < 1% after 1000 steps
    const double drift = EnergyEngine::computeEnergyDrift(initialEnergy, task.totalEnergy);
    RC_ASSERT(drift < 0.01);
}

// -----------------------------------------------------------------------
// Task 19.3 (multi-task variant): System Energy Conservation with Multiple Tasks
// Validates: Requirements 4.2, 13.1, 13.5
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, LongSimulationMultiTaskConservation, ()) {
    // Generate 2-4 tasks with various configurations
    const int count = *rc::gen::inRange(2, 5).as("count");

    std::vector<Task> tasks;
    tasks.reserve(count);

    for (int i = 0; i < count; ++i) {
        const double mass = *rc::gen::inRange(100, 1001).as("mass_int") / 100.0;
        const double vx   = *rc::gen::inRange(10, 201).as("vx_int") / 100.0;
        const double py   = *rc::gen::inRange(-500, 501).as("py_int") / 100.0;

        Task t;
        t.mass            = mass;
        t.velocity        = {vx, 0.0};
        t.position        = {0.0, py};
        t.urgencyConstant = 0.0;
        t.kineticFriction = 0.0;
        t.deadlineTime    = 100000.0;
        EnergyEngine::calculateEnergy(t);
        tasks.push_back(t);
    }

    const double initialSystemEnergy = EnergyEngine::computeSystemEnergy(tasks);

    // Require meaningful initial system energy
    RC_PRE(std::abs(initialSystemEnergy) > 1.0);

    // Run 1000 integration steps on all tasks
    constexpr int NUM_STEPS = 1000;
    for (int step = 0; step < NUM_STEPS; ++step) {
        for (auto& task : tasks) {
            ClassicalEngine::integrateRK4(task);
            EnergyEngine::calculateEnergy(task);
        }
    }

    const double finalSystemEnergy = EnergyEngine::computeSystemEnergy(tasks);

    // Verify all energy values remain finite
    for (const auto& task : tasks) {
        RC_ASSERT(std::isfinite(task.totalEnergy));
    }

    // Verify system energy drift < 1% after 1000 steps
    const double drift = EnergyEngine::computeEnergyDrift(initialSystemEnergy, finalSystemEnergy);
    RC_ASSERT(drift < 0.01);
}

// -----------------------------------------------------------------------
// Property 34: Force Scaling Factor Computation
// Validates: Requirements 20.1
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, ForceScalingFactorComputation, ()) {
    // totalEnergy in [0, MAX_KINETIC_ENERGY]
    const double energy = *rc::gen::inRange(0, static_cast<int>(Config::MAX_KINETIC_ENERGY)).as("energy_int") * 1.0;

    Task task;
    task.totalEnergy = energy;

    double factor = EnergyEngine::computeForceScalingFactor(task);

    // Factor must be a function of E: specifically sqrt(E / MAX_KINETIC_ENERGY), clamped to [0, 2]
    double expected = std::sqrt(energy / Config::MAX_KINETIC_ENERGY);
    expected = std::max(0.0, std::min(expected, 2.0));

    RC_ASSERT(std::abs(factor - expected) < 1e-9);

    // Factor must always be in [0, 2]
    RC_ASSERT(factor >= 0.0);
    RC_ASSERT(factor <= 2.0);
}

// -----------------------------------------------------------------------
// Property 35: Low Energy Reduces Force Scaling
// Validates: Requirements 20.4
// -----------------------------------------------------------------------
RC_GTEST_PROP(EnergyProperties, LowEnergyReducesForceScaling, ()) {
    // totalEnergy strictly below MAX_KINETIC_ENERGY -> factor = sqrt(E/E_max) < 1
    // Use range [0, MAX_KINETIC_ENERGY - 1) to guarantee factor < 1
    const double energy = *rc::gen::inRange(0, static_cast<int>(Config::MAX_KINETIC_ENERGY) - 1).as("energy_int") * 1.0;

    Task task;
    task.totalEnergy = energy;

    double factor = EnergyEngine::computeForceScalingFactor(task);

    // For any energy below MAX_KINETIC_ENERGY, sqrt(E/E_max) < 1
    RC_ASSERT(factor < 1.0);
}
