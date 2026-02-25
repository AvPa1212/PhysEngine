#pragma once

// Cross-platform export macros (Works for MSVC on Windows and GCC on Linux)
#ifdef _WIN32
    #define MOMENTUM_API __declspec(dllexport)
#else
    #define MOMENTUM_API __attribute__((visibility("default")))
#endif

extern "C" {
    // --- Task Lifecycle ---
    // Creates a Task in C++ memory and returns an opaque pointer to it
    MOMENTUM_API void* Task_Create();
    MOMENTUM_API void Task_Destroy(void* taskPtr);

    // --- Task Configuration ---
    MOMENTUM_API void Task_SetPosition(void* taskPtr, double x, double y);
    MOMENTUM_API void Task_SetVelocity(void* taskPtr, double vx, double vy);
    MOMENTUM_API void Task_SetMass(void* taskPtr, double mass);
    MOMENTUM_API void Task_SetStress(void* taskPtr, double sx, double sy, double sz);

    // --- Task State Retrieval ---
    MOMENTUM_API double Task_GetPositionX(void* taskPtr);
    MOMENTUM_API double Task_GetPositionY(void* taskPtr);
    MOMENTUM_API double Task_GetStressX(void* taskPtr);
    MOMENTUM_API double Task_GetStressY(void* taskPtr);
    MOMENTUM_API double Task_GetStressZ(void* taskPtr);
    MOMENTUM_API double Task_GetEntropy(void* taskPtr);

    // --- Engine Operations ---
    // Pass the Task pointer into your static engine methods
    MOMENTUM_API void Engine_IntegrateClassical(void* taskPtr);
    MOMENTUM_API void Engine_UpdateChaos(void* taskPtr);

    // --- Quantum Operations ---
    // Returns a probability (0.0 - 1.0) based on the task's current entropy/state
    MOMENTUM_API double Task_GetCollapseProbability(void* taskPtr);

    // Force a state change based on quantum logic
    MOMENTUM_API void Engine_PerformQuantumCollapse(void* taskPtr);
}