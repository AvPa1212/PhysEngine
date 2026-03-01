#pragma once

// Forward declaration so we don't need to include the full Task definition here
struct Task;

// Cross-platform export macros
#ifdef _WIN32
    #define MOMENTUM_API __declspec(dllexport)
#else
    #define MOMENTUM_API __attribute__((visibility("default")))
#endif

extern "C" {
    // --- Task Lifecycle ---
    // Changed from void* to Task* to satisfy Emscripten Bindings
    MOMENTUM_API Task* Task_Create();
    MOMENTUM_API void Task_Destroy(Task* taskPtr);

    // --- Task Configuration ---
    MOMENTUM_API void Task_SetPosition(Task* taskPtr, double x, double y);
    MOMENTUM_API void Task_SetVelocity(Task* taskPtr, double vx, double vy);
    MOMENTUM_API void Task_SetMass(Task* taskPtr, double mass);
    MOMENTUM_API void Task_SetStress(Task* taskPtr, double sx, double sy, double sz);

    // --- Task State Retrieval ---
    MOMENTUM_API double Task_GetPositionX(Task* taskPtr);
    MOMENTUM_API double Task_GetPositionY(Task* taskPtr);
    MOMENTUM_API double Task_GetStressX(Task* taskPtr);
    MOMENTUM_API double Task_GetStressY(Task* taskPtr);
    MOMENTUM_API double Task_GetStressZ(Task* taskPtr);
    MOMENTUM_API double Task_GetEntropy(Task* taskPtr);

    // --- Engine Operations ---
    MOMENTUM_API void Engine_IntegrateClassical(Task* taskPtr);
    MOMENTUM_API void Engine_UpdateChaos(Task* taskPtr);

    // --- Quantum Operations ---
    MOMENTUM_API double Task_GetCollapseProbability(Task* taskPtr);
    MOMENTUM_API void Engine_PerformQuantumCollapse(Task* taskPtr);
}