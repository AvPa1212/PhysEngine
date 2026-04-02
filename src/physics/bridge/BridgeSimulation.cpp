/**
 * @file BridgeSimulation.cpp
 * @brief Simulation-engine bridge functions and task-state extraction helpers.
 */
#include "BridgeInternal.hpp"

#include "core/SimulationEngine.hpp"
#include "physics/Task.hpp"

namespace {
    constexpr std::size_t kInvalidTaskIndex = static_cast<std::size_t>(-1);

    void WriteTaskState(const Task& task, TaskState& outState) {
        outState.posX = task.position.x;
        outState.posY = task.position.y;
        outState.velX = task.velocity.x;
        outState.velY = task.velocity.y;
        outState.accX = task.acceleration.x;
        outState.accY = task.acceleration.y;
    }
}

namespace Bridge {
    SimulationEngine* SimulationCreate() {
        return new SimulationEngine();
    }

    void SimulationDestroy(SimulationEngine* engine) {
        delete engine;
    }

    std::size_t SimulationAddTask(
        SimulationEngine* engine,
        double mass,
        double deadlineTime,
        double urgencyConstant,
        double staticFriction,
        double kineticFriction
    ) {
        if (
            engine == nullptr ||
            mass <= 0.0 ||
            deadlineTime <= 0.0 ||
            staticFriction < 0.0 ||
            kineticFriction < 0.0
        ) {
            return kInvalidTaskIndex;
        }

        engine->addTask(mass, deadlineTime, urgencyConstant, staticFriction, kineticFriction);
        return engine->tasks.empty() ? kInvalidTaskIndex : engine->tasks.size() - 1;
    }

    int SimulationRemoveTask(SimulationEngine* engine, std::size_t index) {
        if (engine == nullptr) {
            return 0;
        }
        return engine->removeTask(index) ? 1 : 0;
    }

    void SimulationUpdate(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->update();
        }
    }

    void SimulationPause(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->pause();
        }
    }

    void SimulationResume(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->resume();
        }
    }

    void SimulationStep(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->singleStep();
        }
    }

    void SimulationSetTimeScale(SimulationEngine* engine, double scale) {
        if (engine != nullptr) {
            engine->setTimeScale(scale);
        }
    }

    double SimulationGetTimeScale(SimulationEngine* engine) {
        return engine != nullptr ? engine->getTimeScale() : 1.0;
    }

    void SimulationSetClassicalEnabled(SimulationEngine* engine, int enabled) {
        if (engine != nullptr) {
            engine->setClassicalEnabled(enabled != 0);
        }
    }

    int SimulationIsClassicalEnabled(SimulationEngine* engine) {
        return (engine != nullptr && engine->isClassicalEnabled()) ? 1 : 0;
    }

    std::size_t SimulationGetTaskCount(SimulationEngine* engine) {
        return engine != nullptr ? engine->tasks.size() : 0u;
    }

    int SimulationGetTaskState(SimulationEngine* engine, std::size_t index, TaskState* outState) {
        if (engine == nullptr || outState == nullptr || index >= engine->tasks.size()) {
            return 0;
        }

        WriteTaskState(engine->tasks[index], *outState);
        return 1;
    }

    std::size_t SimulationGetAllTaskStates(
        SimulationEngine* engine,
        TaskState* outStates,
        std::size_t maxStates
    ) {
        if (engine == nullptr || outStates == nullptr || maxStates == 0) {
            return 0;
        }

        const std::size_t available = engine->tasks.size();
        const std::size_t count = available < maxStates ? available : maxStates;
        for (std::size_t i = 0; i < count; ++i) {
            WriteTaskState(engine->tasks[i], outStates[i]);
        }
        return count;
    }
}
