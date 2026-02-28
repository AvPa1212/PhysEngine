#include "physics/MomentumBridge.h"
#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include "physics/QuantumEngine.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
using namespace emscripten;
#endif

extern "C" {

    // --- Task Lifecycle ---
    void* Task_Create() {
        return static_cast<void*>(new Task());
    }

    void Task_Destroy(void* taskPtr) {
        if (taskPtr) {
            delete static_cast<Task*>(taskPtr);
        }
    }

    // --- Classical Mechanics ---
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

    // --- Chaotic Subsystem (Lorenz) ---
    void Task_SetStress(void* taskPtr, double sx, double sy, double sz) {
        if (taskPtr) {
            auto* task = static_cast<Task*>(taskPtr);
            task->stressX = sx;
            task->stressY = sy;
            task->stressZ = sz;
        }
    }

    // --- Getters ---
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
        return taskPtr ? static_cast<Task*>(taskPtr)->entropy : 0.0;
    }

    // --- Execution Engines ---
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

    // --- Quantum Dynamics Engine ---
    double Task_GetCollapseProbability(void* taskPtr) {
        if (!taskPtr) return 0.0;
        auto* task = static_cast<Task*>(taskPtr);
        
        // Momentum Blueprint: Probability is tied to current quantum state norm
        // Often correlated with entropy in chaotic regimes
        return QuantumEngine::calculateCollapseProbability(*task);
    }

    void Engine_PerformQuantumCollapse(void* taskPtr) {
        if (taskPtr) {
            auto* task = static_cast<Task*>(taskPtr);
            
            // Forces state vector to a pure basis state (Entropy -> 0)
            QuantumEngine::collapse(*task);
            
            // Classical reaction to collapse (Momentum Blueprint)
            // A collapse typically causes a sudden "nudge" in classical velocity
            task->velocity.y += 5.0; 
        }
    }
}

// --- WebAssembly Bindings (Emscripten Only) ---
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(momentum_web_bridge) {
    function("Task_Create", &Task_Create, allow_raw_pointers());
    function("Task_Destroy", &Task_Destroy, allow_raw_pointers());
    function("Task_SetPosition", &Task_SetPosition, allow_raw_pointers());
    function("Task_GetPositionX", &Task_GetPositionX, allow_raw_pointers());
    function("Task_GetEntropy", &Task_GetEntropy, allow_raw_pointers());
    function("Engine_UpdateChaos", &Engine_UpdateChaos, allow_raw_pointers());
    function("Engine_PerformQuantumCollapse", &Engine_PerformQuantumCollapse, allow_raw_pointers());
}
#endif