/**
 * @file BridgeEnergy.cpp
 * @brief Energy-specific bridge functions for task and simulation callers.
 */
#include "BridgeInternal.hpp"

#include "core/SimulationEngine.hpp"
#include "physics/EnergyEngine.hpp"
#include "physics/Task.hpp"

namespace Bridge {
    double GetKineticEnergy(Task* task) {
        return task != nullptr ? task->kineticEnergy : 0.0;
    }

    double GetPotentialEnergy(Task* task) {
        return task != nullptr ? task->potentialEnergy : 0.0;
    }

    double GetTotalEnergy(Task* task) {
        return task != nullptr ? task->totalEnergy : 0.0;
    }

    void InjectEnergy(Task* task, double amount) {
        if (task != nullptr) {
            EnergyEngine::injectEnergy(*task, amount);
        }
    }

    void DissipateEnergy(Task* task, double dampingCoeff) {
        if (task != nullptr) {
            EnergyEngine::dissipateEnergy(*task, dampingCoeff);
        }
    }

    void TransferEnergy(Task* source, Task* target, double amount) {
        if (source != nullptr && target != nullptr) {
            EnergyEngine::transferEnergy(*source, *target, amount);
        }
    }

    double GetSystemTotalEnergy(SimulationEngine* engine) {
        return engine != nullptr ? engine->getSystemEnergy() : 0.0;
    }

    void RedistributeEnergy(SimulationEngine* engine, int completedTaskIndex) {
        if (engine == nullptr || completedTaskIndex < 0) {
            return;
        }

        const std::size_t index = static_cast<std::size_t>(completedTaskIndex);
        if (index >= engine->tasks.size()) {
            return;
        }

        Task completedTask = engine->tasks[index];
        engine->tasks.erase(engine->tasks.begin() + static_cast<std::ptrdiff_t>(index));
        EnergyEngine::redistributeEnergy(completedTask, engine->tasks);
    }

    void EnableDamping(SimulationEngine* engine, double coefficient) {
        if (engine != nullptr) {
            engine->enableDamping(coefficient);
        }
    }

    void DisableDamping(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->disableDamping();
        }
    }

    double GetMeanEnergy(SimulationEngine* engine) {
        return engine != nullptr ? EnergyEngine::computeMeanEnergy(engine->tasks) : 0.0;
    }

    double GetEnergyStdDev(SimulationEngine* engine) {
        return engine != nullptr ? EnergyEngine::computeEnergyStdDev(engine->tasks) : 0.0;
    }
}
