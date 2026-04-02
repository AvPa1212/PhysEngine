/**
 * @file EnergyEngine.cpp
 * @brief Energy calculations and redistribution helpers used by the engine.
 */
#include "physics/EnergyEngine.hpp"
#include "physics/Task.hpp"
#include "core/Config.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace {
    double computeVelocityMagnitudeSquared(const Task& task) {
        return task.velocity.x * task.velocity.x + task.velocity.y * task.velocity.y;
    }

    void setVelocityFromTargetKineticEnergy(Task& task, double targetKE) {
        if (targetKE <= 0.0 || task.mass <= 0.0) {
            task.velocity.x = 0.0;
            task.velocity.y = 0.0;
            return;
        }

        const double currentSpeed = std::sqrt(computeVelocityMagnitudeSquared(task));
        const double targetSpeed = std::sqrt(2.0 * targetKE / task.mass);

        if (currentSpeed > 0.001) {
            task.velocity.x = (task.velocity.x / currentSpeed) * targetSpeed;
            task.velocity.y = (task.velocity.y / currentSpeed) * targetSpeed;
        } else {
            // If the task is stationary, seed the injected energy on the X axis.
            task.velocity.x = targetSpeed;
            task.velocity.y = 0.0;
        }
    }
}

double EnergyEngine::computeKineticEnergy(const Task& task) {
    return 0.5 * task.mass * computeVelocityMagnitudeSquared(task);
}

double EnergyEngine::computePotentialEnergy(const Task& task) {
    return task.mass * Config::GRAVITY_CONSTANT * task.position.y;
}

void EnergyEngine::calculateEnergy(Task& task) {
    task.kineticEnergy = std::min(computeKineticEnergy(task), Config::MAX_KINETIC_ENERGY);
    task.potentialEnergy = computePotentialEnergy(task);
    task.totalEnergy = task.kineticEnergy + task.potentialEnergy;
}

void EnergyEngine::injectEnergy(Task& task, double energyAmount) {
    double currentKE = computeKineticEnergy(task);
    double targetKE = currentKE + energyAmount;
    targetKE = std::max(targetKE, 0.0);
    targetKE = std::min(targetKE, Config::MAX_KINETIC_ENERGY);

    setVelocityFromTargetKineticEnergy(task, targetKE);
    calculateEnergy(task);

    // Guard against tiny floating-point overshoot after the round-trip.
    if (task.kineticEnergy > Config::MAX_KINETIC_ENERGY) {
        task.kineticEnergy = Config::MAX_KINETIC_ENERGY;
        task.totalEnergy = task.kineticEnergy + task.potentialEnergy;
    }
}

void EnergyEngine::dissipateEnergy(Task& task, double dampingCoefficient) {
    double currentKE = computeKineticEnergy(task);
    double energyLoss = dampingCoefficient * currentKE * Config::TIME_STEP;
    double targetKE = std::max(currentKE - energyLoss, 0.0);

    setVelocityFromTargetKineticEnergy(task, targetKE);
    calculateEnergy(task);
}

void EnergyEngine::transferEnergy(Task& source, Task& target, double amount) {
    double sourceKE = computeKineticEnergy(source);
    double actualTransfer = std::min(amount, sourceKE);
    actualTransfer = std::max(actualTransfer, 0.0);

    injectEnergy(source, -actualTransfer);
    injectEnergy(target, actualTransfer);
}

double EnergyEngine::computeSystemEnergy(const std::vector<Task>& tasks) {
    double systemEnergy = 0.0;
    for (const auto& task : tasks) {
        systemEnergy += task.totalEnergy;
    }
    return systemEnergy;
}

void EnergyEngine::redistributeEnergy(const Task& completedTask, std::vector<Task>& remainingTasks) {
    double completedEnergy = completedTask.totalEnergy;

    double totalMass = 0.0;
    for (const auto& task : remainingTasks) {
        totalMass += task.mass;
    }

    if (totalMass <= 0.0) {
        return;
    }

    for (auto& task : remainingTasks) {
        double share = (task.mass / totalMass) * completedEnergy;
        injectEnergy(task, share);
    }
}

std::vector<Task*> EnergyEngine::sortByEnergy(std::vector<Task>& tasks) {
    std::vector<Task*> ptrs;
    ptrs.reserve(tasks.size());
    for (auto& task : tasks) {
        ptrs.push_back(&task);
    }

    std::sort(ptrs.begin(), ptrs.end(), [](const Task* a, const Task* b) {
        if (a->totalEnergy != b->totalEnergy) {
            return a->totalEnergy > b->totalEnergy;
        }
        return a->kineticEnergy > b->kineticEnergy;
    });

    return ptrs;
}

double EnergyEngine::computeEnergyDrift(double initialEnergy, double currentEnergy) {
    if (initialEnergy == 0.0) {
        return 0.0;
    }
    return std::abs(currentEnergy - initialEnergy) / initialEnergy;
}

double EnergyEngine::computeMeanEnergy(const std::vector<Task>& tasks) {
    if (tasks.empty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (const auto& task : tasks) {
        sum += task.totalEnergy;
    }
    return sum / static_cast<double>(tasks.size());
}

double EnergyEngine::computeEnergyStdDev(const std::vector<Task>& tasks) {
    if (tasks.empty()) {
        return 0.0;
    }

    double mean = computeMeanEnergy(tasks);
    double variance = 0.0;
    for (const auto& task : tasks) {
        double diff = task.totalEnergy - mean;
        variance += diff * diff;
    }
    variance /= static_cast<double>(tasks.size());
    return std::sqrt(variance);
}

std::vector<Task*> EnergyEngine::identifyHighEnergyTasks(const std::vector<Task>& tasks, double threshold) {
    std::vector<Task*> result;
    for (const auto& task : tasks) {
        if (task.totalEnergy >= threshold) {
            result.push_back(const_cast<Task*>(&task));
        }
    }
    return result;
}

double EnergyEngine::computeForceScalingFactor(const Task& task) {
    double factor = std::sqrt(task.totalEnergy / Config::MAX_KINETIC_ENERGY);
    return std::max(0.0, std::min(factor, 2.0));
}

void EnergyEngine::injectEnergyRateLimited(Task& task, double amount, double& queue, double& lastTime, double maxRate) {
    double elapsed = Config::TIME_STEP;
    double budget = maxRate * elapsed;
    double totalRequested = amount + queue;

    double injectable = std::min(totalRequested, budget);
    injectable = std::max(injectable, 0.0);
    queue = std::max(0.0, totalRequested - injectable);

    if (injectable > 0.0) {
        injectEnergy(task, injectable);
    }

    lastTime += elapsed;
}
