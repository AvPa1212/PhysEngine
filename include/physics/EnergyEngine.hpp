#pragma once

#include "math/Vector2.hpp"
#include "core/Config.hpp"
#include <vector>

struct Task;

class EnergyEngine {
public:
    // Core energy calculations
    static void calculateEnergy(Task& task);
    static double computeKineticEnergy(const Task& task);
    static double computePotentialEnergy(const Task& task);

    // Energy operations
    static void injectEnergy(Task& task, double energyAmount);
    static void dissipateEnergy(Task& task, double dampingCoefficient);
    static void transferEnergy(Task& source, Task& target, double amount);

    // System-level operations
    static double computeSystemEnergy(const std::vector<Task>& tasks);
    static void redistributeEnergy(const Task& completedTask, std::vector<Task>& remainingTasks);

    // Analytics
    static std::vector<Task*> sortByEnergy(std::vector<Task>& tasks);
    static double computeEnergyDrift(double initialEnergy, double currentEnergy);
    static double computeMeanEnergy(const std::vector<Task>& tasks);
    static double computeEnergyStdDev(const std::vector<Task>& tasks);
    static std::vector<Task*> identifyHighEnergyTasks(const std::vector<Task>& tasks, double threshold);

    // Force modulation
    static double computeForceScalingFactor(const Task& task);

    // Rate-limited energy injection
    static void injectEnergyRateLimited(Task& task, double amount, double& queue, double& lastTime, double maxRate);
};
