#pragma once
#include <vector>
#include <cstddef>
#include <algorithm>
#include "core/Config.hpp"
#include "physics/Task.hpp"
#include "physics/ClassicalEngine.hpp"
#include "physics/EnergyEngine.hpp"
#include "physics/QuantumEngine.hpp"
#include "physics/ThermodynamicsEngine.hpp"
#include "physics/ChaosEngine.hpp"

class SimulationEngine {
public:
    std::vector<Task> tasks;
    bool paused = false;
    bool classicalEnabled = true;
    double timeScale = 1.0;

    // Energy system configuration
    bool dampingEnabled = false;
    double dampingCoefficient = Config::DEFAULT_DAMPING_COEFFICIENT;
    double maxEnergyInjectionRate = Config::MAX_ENERGY_INJECTION_RATE;

    // Energy conservation tracking
    double initialSystemEnergy = 0.0;

    // Rate limiting state (Task 16.1)
    double energyInjectionQueue = 0.0;
    double lastInjectionTime = 0.0;

    SimulationEngine() {
        // Pre-allocate to prevent dynamic allocation during loop
        tasks.reserve(100);
    }

    void update() {
        // When paused, skip integration so task states remain unchanged
        if (paused) return;

        const double scaledDt = effectiveTimeStep();

        for (auto& task : tasks) {
            if (classicalEnabled) {
                ClassicalEngine::integrateRK4(task, scaledDt);
            }
            // Energy calculations occur right after classical integration
            EnergyEngine::calculateEnergy(task);
            if (dampingEnabled) {
                EnergyEngine::dissipateEnergy(task, dampingCoefficient);
            }
            QuantumEngine::evolve(task);
            ThermodynamicsEngine::updateEntropy(task);
            ChaosEngine::update(task);
        }
    }

    Task& addTask(
        double mass,
        double deadlineTime,
        double urgencyConstant = 100.0,
        double staticFriction = 0.5,
        double kineticFriction = 0.3
    ) {
        Task task;
        task.position = {0.0, 0.0};
        task.velocity = {0.0, 0.0};
        task.acceleration = {0.0, 0.0};
        task.mass = mass;
        task.deadlineTime = deadlineTime;
        task.urgencyConstant = urgencyConstant;
        task.staticFriction = staticFriction;
        task.kineticFriction = kineticFriction;
        tasks.push_back(task);
        return tasks.back();
    }

    bool removeTask(std::size_t index) {
        if (index >= tasks.size()) {
            return false;
        }
        tasks.erase(tasks.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    }

    void pause() {
        paused = true;
    }

    void resume() {
        paused = false;
    }

    void step() {
        if (paused) {
            singleStep();
            return;
        }
        update();
    }

    void singleStep() {
        const bool wasPaused = paused;
        paused = false;
        update();
        paused = wasPaused;
    }

    void setTimeScale(double scale) {
        if (scale > 0.0) {
            timeScale = scale;
        }
    }

    double getTimeScale() const {
        return timeScale;
    }

    void setClassicalEnabled(bool enabled) {
        classicalEnabled = enabled;
    }

    bool isClassicalEnabled() const {
        return classicalEnabled;
    }

    // Task 6.3: Enable damping with clamped coefficient
    void enableDamping(double coefficient) {
        dampingEnabled = true;
        dampingCoefficient = std::max(0.0, std::min(1.0, coefficient));
    }

    // Task 6.4: Disable damping
    void disableDamping() {
        dampingEnabled = false;
    }

    // Task 6.5: Get total system energy
    double getSystemEnergy() const {
        return EnergyEngine::computeSystemEnergy(tasks);
    }

    // Task 6.6: Get energy drift relative to initial system energy
    double getEnergyDrift() const {
        double currentEnergy = getSystemEnergy();
        return EnergyEngine::computeEnergyDrift(initialSystemEnergy, currentEnergy);
    }

    // Task 16.3: Inject energy with rate limiting (delegates to EnergyEngine)
    void injectEnergyRateLimited(Task& task, double amount) {
        EnergyEngine::injectEnergyRateLimited(task, amount, energyInjectionQueue, lastInjectionTime, maxEnergyInjectionRate);
    }

    // Task 16.3: Process queued energy injection for a given task
    void processInjectionQueue(Task& task) {
        if (energyInjectionQueue > 0.0) {
            EnergyEngine::injectEnergyRateLimited(task, 0.0, energyInjectionQueue, lastInjectionTime, maxEnergyInjectionRate);
        }
    }

private:
    double effectiveTimeStep() const {
        return timeScale * Config::TIME_STEP;
    }
};