#include "physics/MomentumBridge.h"
#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include "physics/QuantumEngine.hpp" // Added for Quantum operations

// We cast the void* back to a Task* internally so the engines know what to do with it.
extern "C" {

    void* Task_Create() {
        return static_cast<void*>(new Task());
    }

    void Task_Destroy(void* taskPtr) {
        if (taskPtr) {
            delete static_cast<Task*>(taskPtr);
        }
    }

    void Task_SetPosition(void* taskPtr, double x, double y) {
        if (taskPtr) {
            auto* task = static_cast<Task*>(taskPtr);
            task->position.x = x;
            task->position.y = y;
        }
    }

    void Task_SetVelocity(void* taskPtr, double vx, double vy) {
        if (taskPtr) {
            auto* task = static_cast<Task*>(taskPtr);
            task->velocity.x = vx;
            task->velocity.y = vy;
        }
    }

    void Task_SetMass(void* taskPtr, double mass) {
        if (taskPtr) {
            static_cast<Task*>(taskPtr)->mass = mass;
        }
    }

    void Task_SetStress(void* taskPtr, double sx, double sy, double sz) {
        if (taskPtr) {
            auto* task = static_cast<Task*>(taskPtr);
            task->stressX = sx;
            task->stressY = sy;
            task->stressZ = sz;
        }
    }

    double Task_GetPositionX(void* taskPtr) {
        return taskPtr ? static_cast<Task*>(taskPtr)->position.x : 0.0;
    }

    double Task_GetPositionY(void* taskPtr) {
        return taskPtr ? static_cast<Task*>(taskPtr)->position.y : 0.0;
    }

    double Task_GetStressX(void* taskPtr) {
        return taskPtr ? static_cast<Task*>(taskPtr)->stressX : 0.0;
    }

    double Task_GetStressY(void* taskPtr) {
        return taskPtr ? static_cast<Task*>(taskPtr)->stressY : 0.0;
    }

    double Task_GetStressZ(void* taskPtr) {
        return taskPtr ? static_cast<Task*>(taskPtr)->stressZ : 0.0;
    }

    double Task_GetEntropy(void* taskPtr) {
        // Assuming you have an entropy field based on your earlier Validation phase
        return taskPtr ? static_cast<Task*>(taskPtr)->entropy : 0.0;
    }

    void Engine_IntegrateClassical(void* taskPtr) {
        if (taskPtr) {
            ClassicalEngine::integrateRK4(*static_cast<Task*>(taskPtr));
        }
    }

    void Engine_UpdateChaos(void* taskPtr) {
        if (taskPtr) {
            ChaosEngine::update(*static_cast<Task*>(taskPtr));
        }
    }

    // --- New Quantum Operations ---

    double Task_GetCollapseProbability(void* taskPtr) {
        if (!taskPtr) return 0.0;
        auto* task = static_cast<Task*>(taskPtr);
        
        // Example logic: Probability increases with entropy, capped at 1.0 (100%)
        return (task->entropy > 1.0) ? 1.0 : task->entropy;
    }

    void Engine_PerformQuantumCollapse(void* taskPtr) {
        if (taskPtr) {
            auto* task = static_cast<Task*>(taskPtr);
            
            // Note: If you have a specific static method in QuantumEngine, 
            // you can call it here like: QuantumEngine::collapse(*task);
            
            // Otherwise, here is the manual fallback logic:
            task->stressX = (task->stressX > 0) ? 1.0 : -1.0;
            task->entropy = 0.0; // Reset entropy after the collapse event
        }
    }
}