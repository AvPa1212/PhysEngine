#include "physics/MomentumBridge.h"
#include "physics/Task.hpp"            // Contains the actual Task struct definition
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
    // Constructor / Destructor
    Task* Create() {
        return new Task();
    }

    void Destroy(Task* task) {
        delete task;
    }

    // Getters
    double GetPositionX(Task* task) { return (task != nullptr) ? task->position.x : 0.0; }
    double GetEntropy(Task* task)   { return (task != nullptr) ? task->entropy : 0.0; }
    double GetStressX(Task* task)   { return (task != nullptr) ? task->stressX : 0.0; }
    double GetStressY(Task* task)   { return (task != nullptr) ? task->stressY : 0.0; }
    double GetStressZ(Task* task)   { return (task != nullptr) ? task->stressZ : 0.0; }

    // Setters (For UI Manual Overrides)
    void SetMass(Task* task, double mass) {
        if (task) task->mass = mass;
    }
    
    void SetStress(Task* task, double sx, double sy, double sz) {
        if (task) {
            task->stressX = sx;
            task->stressY = sy;
            task->stressZ = sz;
        }
    }

    // Engine Commands
    void UpdateChaos(Task* task) {
        if (task) {
            ChaosEngine::update(*task);
        }
    }

    void Collapse(Task* task) {
        if (task) {
            QuantumEngine::collapse(*task);
            // Visual "kick": apply upward velocity upon quantum state reduction
            task->velocity.y += 5.0;
        }
    }
}

// --- Desktop/Python compatibility layer (extern "C") ---
// These maintain existing ABI compatibility for your Python scripts
extern "C" {
    MOMENTUM_API Task* Task_Create() { 
        return Bridge::Create(); 
    }

    MOMENTUM_API void Task_Destroy(Task* t) {
        Bridge::Destroy(t);
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
    // 1. Register the Task class
    class_<Task>("Task");

    // 2. Bind Getters
    function("Task_Create", &Bridge::Create, allow_raw_pointers());
    function("Task_Destroy", &Bridge::Destroy, allow_raw_pointers());
    function("Task_GetPositionX", &Bridge::GetPositionX, allow_raw_pointers());
    function("Task_GetEntropy", &Bridge::GetEntropy, allow_raw_pointers());
    function("Task_GetStressX", &Bridge::GetStressX, allow_raw_pointers());
    function("Task_GetStressY", &Bridge::GetStressY, allow_raw_pointers());
    function("Task_GetStressZ", &Bridge::GetStressZ, allow_raw_pointers());
    
    // 3. Bind Setters
    function("Task_SetMass", &Bridge::SetMass, allow_raw_pointers());
    function("Task_SetStress", &Bridge::SetStress, allow_raw_pointers());

    // 4. Bind Engine Actions
    function("Engine_UpdateChaos", &Bridge::UpdateChaos, allow_raw_pointers());
    function("Engine_PerformQuantumCollapse", &Bridge::Collapse, allow_raw_pointers());
}
#endif