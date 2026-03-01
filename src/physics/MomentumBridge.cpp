#include "physics/MomentumBridge.h"
#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include "physics/QuantumEngine.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
using namespace emscripten;
#endif

// We use Task* instead of void* so Emscripten can track the type
extern "C" {

    // --- Task Lifecycle ---
    Task* Task_Create() {
        return new Task();
    }

    void Task_Destroy(Task* task) {
        if (task) {
            delete task;
        }
    }

    // --- Classical Mechanics ---
    void Task_SetPosition(Task* task, double x, double y) {
        if (task) {
            task->position.x = x;
            task->position.y = y;
        }
    }

    void Task_SetVelocity(Task* task, double vx, double vy) {
        if (task) {
            task->velocity.x = vx;
            task->velocity.y = vy;
        }
    }

    void Task_SetMass(Task* task, double mass) {
        if (task) {
            task->mass = mass;
        }
    }

    // --- Chaotic Subsystem (Lorenz) ---
    void Task_SetStress(Task* task, double sx, double sy, double sz) {
        if (task) {
            task->stressX = sx;
            task->stressY = sy;
            task->stressZ = sz;
        }
    }

    // --- Getters ---
    double Task_GetPositionX(Task* task) {
        return task ? task->position.x : 0.0;
    }

    double Task_GetPositionY(Task* task) {
        return task ? task->position.y : 0.0;
    }

    double Task_GetStressX(Task* task) {
        return task ? task->stressX : 0.0;
    }

    double Task_GetStressY(Task* task) {
        return task ? task->stressY : 0.0;
    }

    double Task_GetStressZ(Task* task) {
        return task ? task->stressZ : 0.0;
    }

    double Task_GetEntropy(Task* task) {
        return task ? task->entropy : 0.0;
    }

    // --- Execution Engines ---
    void Engine_IntegrateClassical(Task* task) {
        if (task) {
            ClassicalEngine::integrateRK4(*task);
        }
    }

    void Engine_UpdateChaos(Task* task) {
        if (task) {
            ChaosEngine::update(*task);
        }
    }

    // --- Quantum Dynamics Engine ---
    double Task_GetCollapseProbability(Task* task) {
        if (!task) return 0.0;
        return QuantumEngine::calculateCollapseProbability(*task);
    }

    void Engine_PerformQuantumCollapse(Task* task) {
        if (task) {
            QuantumEngine::collapse(*task);
            task->velocity.y += 5.0; 
        }
    }
}

// --- WebAssembly Bindings (Emscripten Only) ---
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(momentum_web_bridge) {
    
    // 1. REGISTER THE CLASS
    // This is the "Magic Fix" for the 'Pv' error. 
    // It tells Emscripten that Task* is a valid handle for JS.
    class_<Task>("Task");

    // 2. REGISTER THE FUNCTIONS
    // We explicitly use allow_raw_pointers() for Task* interaction.
    function("Task_Create", &Task_Create, allow_raw_pointers());
    function("Task_Destroy", &Task_Destroy, allow_raw_pointers());
    function("Task_SetPosition", &Task_SetPosition, allow_raw_pointers());
    function("Task_GetPositionX", &Task_GetPositionX, allow_raw_pointers());
    function("Task_GetStressX", &Task_GetStressX, allow_raw_pointers()); // Added for the UI
    function("Task_GetEntropy", &Task_GetEntropy, allow_raw_pointers());
    function("Engine_UpdateChaos", &Engine_UpdateChaos, allow_raw_pointers());
    function("Engine_PerformQuantumCollapse", &Engine_PerformQuantumCollapse, allow_raw_pointers());
}
#endif