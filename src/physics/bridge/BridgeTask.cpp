/**
 * @file BridgeTask.cpp
 * @brief Task-oriented bridge functions exposed to C, Python, and WASM callers.
 */
#include "BridgeInternal.hpp"

#include "physics/Task.hpp"
#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include "physics/QuantumEngine.hpp"

namespace {
    constexpr double kCollapseVelocityKickY = 5.0;
}

namespace Bridge {
    Task* Create() {
        return new Task();
    }

    Task* CreateWithParams(double mass, double deadlineTime, double urgencyConstant) {
        Task* task = new Task();
        if (mass > 0.0) {
            task->mass = mass;
        }
        if (deadlineTime > 0.0) {
            task->deadlineTime = deadlineTime;
        }
        task->urgencyConstant = urgencyConstant;
        return task;
    }

    void Destroy(Task* task) {
        delete task;
    }

    double GetPositionX(Task* task) {
        return task != nullptr ? task->position.x : 0.0;
    }

    double GetPositionY(Task* task) {
        return task != nullptr ? task->position.y : 0.0;
    }

    double GetVelocityX(Task* task) {
        return task != nullptr ? task->velocity.x : 0.0;
    }

    double GetVelocityY(Task* task) {
        return task != nullptr ? task->velocity.y : 0.0;
    }

    double GetAccelerationX(Task* task) {
        return task != nullptr ? task->acceleration.x : 0.0;
    }

    double GetAccelerationY(Task* task) {
        return task != nullptr ? task->acceleration.y : 0.0;
    }

    double GetMass(Task* task) {
        return task != nullptr ? task->mass : 0.0;
    }

    double GetEntropy(Task* task) {
        return task != nullptr ? task->entropy : 0.0;
    }

    double GetStressX(Task* task) {
        return task != nullptr ? task->stressX : 0.0;
    }

    double GetStressY(Task* task) {
        return task != nullptr ? task->stressY : 0.0;
    }

    double GetStressZ(Task* task) {
        return task != nullptr ? task->stressZ : 0.0;
    }

    int GetStepCount(Task* task) {
        return task != nullptr ? task->stepCount : 0;
    }

    double GetCollapseProbability(Task* task) {
        return task != nullptr ? QuantumEngine::calculateCollapseProbability(*task) : 0.0;
    }

    void SetPosition(Task* task, double x, double y) {
        if (task == nullptr) {
            return;
        }
        task->position.x = x;
        task->position.y = y;
    }

    void SetVelocity(Task* task, double vx, double vy) {
        if (task == nullptr) {
            return;
        }
        task->velocity.x = vx;
        task->velocity.y = vy;
    }

    void SetMass(Task* task, double mass) {
        if (task != nullptr) {
            task->mass = mass;
        }
    }

    void SetStress(Task* task, double sx, double sy, double sz) {
        if (task == nullptr) {
            return;
        }
        task->stressX = sx;
        task->stressY = sy;
        task->stressZ = sz;
    }

    void IntegrateClassical(Task* task) {
        if (task != nullptr) {
            ClassicalEngine::integrateRK4(*task);
        }
    }

    void UpdateChaos(Task* task) {
        if (task == nullptr) {
            return;
        }
        ChaosEngine::update(*task);
        task->stepCount += 1;
    }

    void Collapse(Task* task) {
        if (task == nullptr) {
            return;
        }
        QuantumEngine::collapse(*task);
        task->velocity.y += kCollapseVelocityKickY;
    }

    void ApplyForce(Task* task, double fx, double fy, double /*fz*/) {
        if (task == nullptr || task->mass <= 0.0) {
            return;
        }
        task->velocity.x += fx / task->mass;
        task->velocity.y += fy / task->mass;
    }
}
