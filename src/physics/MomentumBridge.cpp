#include "physics/MomentumBridge.h"
#include "physics/Task.hpp"            // Corrected: This contains the actual Task struct
#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include "physics/QuantumEngine.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
using namespace emscripten;
#endif

// --- Internal C++ Logic (Namespaced to ensure Embind sees the C++ types clearly) ---
namespace Bridge {
    // We explicitly return Task* (the concrete type)
    Task* Create() {
        return new Task();
    }

    double GetPositionX(Task* task) {
        return (task != nullptr) ? task->position.x : 0.0;
    }

    double GetEntropy(Task* task) {
        return (task != nullptr) ? task->entropy : 0.0;
    }

    double GetStressX(Task* task) {
        return (task != nullptr) ? task->stressX : 0.0;
    }

    void UpdateChaos(Task* task) {
        if (task) {
            ChaosEngine::update(*task);
        }
    }

    void Collapse(Task* task) {
        if (task) {
            QuantumEngine::collapse(*task);
            // Visual feedback: a sudden velocity change
            task->velocity.y += 5.0;
        }
    }
}

// --- Desktop/Python compatibility layer (extern "C") ---
extern "C" {
    MOMENTUM_API Task* Task_Create() { 
        return Bridge::Create(); 
    }
    
    MOMENTUM_API double Task_GetPositionX(Task* t) { 
        return Bridge::GetPositionX(t); 
    }
    
    MOMENTUM_API double Task_GetEntropy(Task* t) { 
        return Bridge::GetEntropy(t); 
    }
    
    MOMENTUM_API double Task_GetStressX(Task* t) { 
        return Bridge::GetStressX(t); 
    }
    
    MOMENTUM_API void Engine_UpdateChaos(Task* t) { 
        Bridge::UpdateChaos(t); 
    }
    
    MOMENTUM_API void Engine_PerformQuantumCollapse(Task* t) { 
        Bridge::Collapse(t); 
    }
}

// --- WebAssembly Bindings (Emscripten Only) ---
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(momentum_module) {
    // Register the Task class. 
    // Embind now knows that 'Task*' corresponds to this registered name.
    class_<Task>("Task");

    // Bind to the Bridge namespace functions to avoid the 'void*' ambiguity of C symbols
    function("Task_Create", &Bridge::Create, allow_raw_pointers());
    function("Task_GetPositionX", &Bridge::GetPositionX, allow_raw_pointers());
    function("Task_GetEntropy", &Bridge::GetEntropy, allow_raw_pointers());
    function("Task_GetStressX", &Bridge::GetStressX, allow_raw_pointers());
    function("Engine_UpdateChaos", &Bridge::UpdateChaos, allow_raw_pointers());
    function("Engine_PerformQuantumCollapse", &Bridge::Collapse, allow_raw_pointers());
}
#endif