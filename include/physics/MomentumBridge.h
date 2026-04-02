#pragma once

#include <stddef.h>

// Forward declaration so we don't need to include the full Task definition here
struct Task;
struct SimulationEngine;

typedef struct TaskState {
    double posX;
    double posY;
    double velX;
    double velY;
    double accX;
    double accY;
} TaskState;

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
    MOMENTUM_API Task* Task_CreateWithParams(double mass, double deadlineTime, double urgencyConstant);
    MOMENTUM_API void Task_Destroy(Task* taskPtr);

    // --- Task Configuration ---
    MOMENTUM_API void Task_SetPosition(Task* taskPtr, double x, double y);
    MOMENTUM_API void Task_SetVelocity(Task* taskPtr, double vx, double vy);
    MOMENTUM_API void Task_SetMass(Task* taskPtr, double mass);
    MOMENTUM_API void Task_SetStress(Task* taskPtr, double sx, double sy, double sz);

    // --- Task State Retrieval ---
    MOMENTUM_API double Task_GetPositionX(Task* taskPtr);
    MOMENTUM_API double Task_GetPositionY(Task* taskPtr);
    MOMENTUM_API double Task_GetVelocityX(Task* taskPtr);
    MOMENTUM_API double Task_GetVelocityY(Task* taskPtr);
    MOMENTUM_API double Task_GetAccelerationX(Task* taskPtr);
    MOMENTUM_API double Task_GetAccelerationY(Task* taskPtr);
    MOMENTUM_API double Task_GetMass(Task* taskPtr);
    MOMENTUM_API double Task_GetStressX(Task* taskPtr);
    MOMENTUM_API double Task_GetStressY(Task* taskPtr);
    MOMENTUM_API double Task_GetStressZ(Task* taskPtr);
    MOMENTUM_API double Task_GetEntropy(Task* taskPtr);
    MOMENTUM_API int Task_GetStepCount(Task* taskPtr);

    // --- Force Application ---
    MOMENTUM_API void Task_ApplyForce(Task* taskPtr, double fx, double fy, double fz);

    // --- Engine Operations ---
    MOMENTUM_API void Engine_IntegrateClassical(Task* taskPtr);
    MOMENTUM_API void Engine_UpdateChaos(Task* taskPtr);

    // --- Simulation Engine Operations ---
    MOMENTUM_API SimulationEngine* Simulation_Create();
    MOMENTUM_API void Simulation_Destroy(SimulationEngine* enginePtr);
    MOMENTUM_API size_t Simulation_AddTask(
        SimulationEngine* enginePtr,
        double mass,
        double deadlineTime,
        double urgencyConstant,
        double staticFriction,
        double kineticFriction
    );
    MOMENTUM_API int Simulation_RemoveTask(SimulationEngine* enginePtr, size_t index);
    MOMENTUM_API void Simulation_Update(SimulationEngine* enginePtr);
    MOMENTUM_API void Simulation_Pause(SimulationEngine* enginePtr);
    MOMENTUM_API void Simulation_Resume(SimulationEngine* enginePtr);
    MOMENTUM_API void Simulation_Step(SimulationEngine* enginePtr);
    MOMENTUM_API void Simulation_SetTimeScale(SimulationEngine* enginePtr, double timeScale);
    MOMENTUM_API double Simulation_GetTimeScale(SimulationEngine* enginePtr);
    MOMENTUM_API void Simulation_SetClassicalEnabled(SimulationEngine* enginePtr, int enabled);
    MOMENTUM_API int Simulation_IsClassicalEnabled(SimulationEngine* enginePtr);
    MOMENTUM_API size_t Simulation_GetTaskCount(SimulationEngine* enginePtr);
    MOMENTUM_API int Simulation_GetTaskState(SimulationEngine* enginePtr, size_t index, TaskState* outState);
    MOMENTUM_API size_t Simulation_GetAllTaskStates(SimulationEngine* enginePtr, TaskState* outStates, size_t maxStates);

    // --- Quantum Operations ---
    MOMENTUM_API double Task_GetCollapseProbability(Task* taskPtr);
    MOMENTUM_API void Engine_PerformQuantumCollapse(Task* taskPtr);

    // --- Energy Query Functions ---
    MOMENTUM_API double Energy_GetKinetic(Task* t);
    MOMENTUM_API double Energy_GetPotential(Task* t);
    MOMENTUM_API double Energy_GetTotal(Task* t);

    // --- Energy Operation Functions ---
    MOMENTUM_API void Energy_Inject(Task* t, double amount);
    MOMENTUM_API void Energy_Dissipate(Task* t, double dampingCoeff);
    MOMENTUM_API void Energy_Transfer(Task* source, Task* target, double amount);

    // --- System-Level Energy Functions ---
    MOMENTUM_API double System_GetTotalEnergy(SimulationEngine* engine);
    MOMENTUM_API void System_RedistributeEnergy(SimulationEngine* engine, int completedTaskIndex);
    MOMENTUM_API void System_EnableDamping(SimulationEngine* engine, double coefficient);
    MOMENTUM_API void System_DisableDamping(SimulationEngine* engine);

    // --- State Serialization ---
    MOMENTUM_API const char* State_Serialize(Task* taskPtr);
    MOMENTUM_API void State_Deserialize(Task* taskPtr, const char* json);

    // --- Energy Analytics ---
    MOMENTUM_API double Analytics_GetMeanEnergy(SimulationEngine* engine);
    MOMENTUM_API double Analytics_GetEnergyStdDev(SimulationEngine* engine);
}