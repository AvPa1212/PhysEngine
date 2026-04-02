/**
 * @file MomentumBridgeExports.cpp
 * @brief Public C ABI wrappers and optional Emscripten bindings.
 */
#include "BridgeInternal.hpp"

#include "core/SimulationEngine.hpp"
#include "physics/Task.hpp"

#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
using namespace emscripten;
#endif

namespace {
    thread_local std::string gSerializeBuffer;
}

extern "C" {
    MOMENTUM_API Task* Task_Create() {
        return Bridge::Create();
    }

    MOMENTUM_API Task* Task_CreateWithParams(double mass, double deadlineTime, double urgencyConstant) {
        return Bridge::CreateWithParams(mass, deadlineTime, urgencyConstant);
    }

    MOMENTUM_API void Task_Destroy(Task* task) {
        Bridge::Destroy(task);
    }

    MOMENTUM_API void Task_SetPosition(Task* task, double x, double y) {
        Bridge::SetPosition(task, x, y);
    }

    MOMENTUM_API void Task_SetVelocity(Task* task, double vx, double vy) {
        Bridge::SetVelocity(task, vx, vy);
    }

    MOMENTUM_API void Task_SetMass(Task* task, double mass) {
        Bridge::SetMass(task, mass);
    }

    MOMENTUM_API void Task_SetStress(Task* task, double sx, double sy, double sz) {
        Bridge::SetStress(task, sx, sy, sz);
    }

    MOMENTUM_API double Task_GetPositionX(Task* task) {
        return Bridge::GetPositionX(task);
    }

    MOMENTUM_API double Task_GetPositionY(Task* task) {
        return Bridge::GetPositionY(task);
    }

    MOMENTUM_API double Task_GetVelocityX(Task* task) {
        return Bridge::GetVelocityX(task);
    }

    MOMENTUM_API double Task_GetVelocityY(Task* task) {
        return Bridge::GetVelocityY(task);
    }

    MOMENTUM_API double Task_GetAccelerationX(Task* task) {
        return Bridge::GetAccelerationX(task);
    }

    MOMENTUM_API double Task_GetAccelerationY(Task* task) {
        return Bridge::GetAccelerationY(task);
    }

    MOMENTUM_API double Task_GetMass(Task* task) {
        return Bridge::GetMass(task);
    }

    MOMENTUM_API double Task_GetStressX(Task* task) {
        return Bridge::GetStressX(task);
    }

    MOMENTUM_API double Task_GetStressY(Task* task) {
        return Bridge::GetStressY(task);
    }

    MOMENTUM_API double Task_GetStressZ(Task* task) {
        return Bridge::GetStressZ(task);
    }

    MOMENTUM_API double Task_GetEntropy(Task* task) {
        return Bridge::GetEntropy(task);
    }

    MOMENTUM_API int Task_GetStepCount(Task* task) {
        return Bridge::GetStepCount(task);
    }

    MOMENTUM_API void Task_ApplyForce(Task* task, double fx, double fy, double fz) {
        Bridge::ApplyForce(task, fx, fy, fz);
    }

    MOMENTUM_API void Engine_IntegrateClassical(Task* task) {
        Bridge::IntegrateClassical(task);
    }

    MOMENTUM_API void Engine_UpdateChaos(Task* task) {
        Bridge::UpdateChaos(task);
    }

    MOMENTUM_API double Task_GetCollapseProbability(Task* task) {
        return Bridge::GetCollapseProbability(task);
    }

    MOMENTUM_API void Engine_PerformQuantumCollapse(Task* task) {
        Bridge::Collapse(task);
    }

    MOMENTUM_API SimulationEngine* Simulation_Create() {
        return Bridge::SimulationCreate();
    }

    MOMENTUM_API void Simulation_Destroy(SimulationEngine* engine) {
        Bridge::SimulationDestroy(engine);
    }

    MOMENTUM_API size_t Simulation_AddTask(
        SimulationEngine* engine,
        double mass,
        double deadlineTime,
        double urgencyConstant,
        double staticFriction,
        double kineticFriction
    ) {
        return Bridge::SimulationAddTask(
            engine,
            mass,
            deadlineTime,
            urgencyConstant,
            staticFriction,
            kineticFriction
        );
    }

    MOMENTUM_API int Simulation_RemoveTask(SimulationEngine* engine, size_t index) {
        return Bridge::SimulationRemoveTask(engine, index);
    }

    MOMENTUM_API void Simulation_Update(SimulationEngine* engine) {
        Bridge::SimulationUpdate(engine);
    }

    MOMENTUM_API void Simulation_Pause(SimulationEngine* engine) {
        Bridge::SimulationPause(engine);
    }

    MOMENTUM_API void Simulation_Resume(SimulationEngine* engine) {
        Bridge::SimulationResume(engine);
    }

    MOMENTUM_API void Simulation_Step(SimulationEngine* engine) {
        Bridge::SimulationStep(engine);
    }

    MOMENTUM_API void Simulation_SetTimeScale(SimulationEngine* engine, double timeScale) {
        Bridge::SimulationSetTimeScale(engine, timeScale);
    }

    MOMENTUM_API double Simulation_GetTimeScale(SimulationEngine* engine) {
        return Bridge::SimulationGetTimeScale(engine);
    }

    MOMENTUM_API void Simulation_SetClassicalEnabled(SimulationEngine* engine, int enabled) {
        Bridge::SimulationSetClassicalEnabled(engine, enabled);
    }

    MOMENTUM_API int Simulation_IsClassicalEnabled(SimulationEngine* engine) {
        return Bridge::SimulationIsClassicalEnabled(engine);
    }

    MOMENTUM_API size_t Simulation_GetTaskCount(SimulationEngine* engine) {
        return Bridge::SimulationGetTaskCount(engine);
    }

    MOMENTUM_API int Simulation_GetTaskState(SimulationEngine* engine, size_t index, TaskState* outState) {
        return Bridge::SimulationGetTaskState(engine, index, outState);
    }

    MOMENTUM_API size_t Simulation_GetAllTaskStates(
        SimulationEngine* engine,
        TaskState* outStates,
        size_t maxStates
    ) {
        return Bridge::SimulationGetAllTaskStates(engine, outStates, maxStates);
    }

    MOMENTUM_API const char* State_Serialize(Task* task) {
        gSerializeBuffer = Bridge::Serialize(task);
        return gSerializeBuffer.c_str();
    }

    MOMENTUM_API void State_Deserialize(Task* task, const char* json) {
        if (json != nullptr) {
            Bridge::Deserialize(task, std::string(json));
        }
    }

    MOMENTUM_API double Energy_GetKinetic(Task* task) {
        return Bridge::GetKineticEnergy(task);
    }

    MOMENTUM_API double Energy_GetPotential(Task* task) {
        return Bridge::GetPotentialEnergy(task);
    }

    MOMENTUM_API double Energy_GetTotal(Task* task) {
        return Bridge::GetTotalEnergy(task);
    }

    MOMENTUM_API void Energy_Inject(Task* task, double amount) {
        Bridge::InjectEnergy(task, amount);
    }

    MOMENTUM_API void Energy_Dissipate(Task* task, double dampingCoeff) {
        Bridge::DissipateEnergy(task, dampingCoeff);
    }

    MOMENTUM_API void Energy_Transfer(Task* source, Task* target, double amount) {
        Bridge::TransferEnergy(source, target, amount);
    }

    MOMENTUM_API double System_GetTotalEnergy(SimulationEngine* engine) {
        return Bridge::GetSystemTotalEnergy(engine);
    }

    MOMENTUM_API void System_RedistributeEnergy(SimulationEngine* engine, int completedTaskIndex) {
        Bridge::RedistributeEnergy(engine, completedTaskIndex);
    }

    MOMENTUM_API void System_EnableDamping(SimulationEngine* engine, double coefficient) {
        Bridge::EnableDamping(engine, coefficient);
    }

    MOMENTUM_API void System_DisableDamping(SimulationEngine* engine) {
        Bridge::DisableDamping(engine);
    }

    MOMENTUM_API double Analytics_GetMeanEnergy(SimulationEngine* engine) {
        return Bridge::GetMeanEnergy(engine);
    }

    MOMENTUM_API double Analytics_GetEnergyStdDev(SimulationEngine* engine) {
        return Bridge::GetEnergyStdDev(engine);
    }
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(momentum_module) {
    class_<Task>("Task");
    class_<SimulationEngine>("SimulationEngine");

    function("Task_Create", &Bridge::Create, allow_raw_pointers());
    function("Task_CreateWithParams", &Bridge::CreateWithParams, allow_raw_pointers());
    function("Task_Destroy", &Bridge::Destroy, allow_raw_pointers());
    function("Task_GetPositionX", &Bridge::GetPositionX, allow_raw_pointers());
    function("Task_GetPositionY", &Bridge::GetPositionY, allow_raw_pointers());
    function("Task_GetVelocityX", &Bridge::GetVelocityX, allow_raw_pointers());
    function("Task_GetVelocityY", &Bridge::GetVelocityY, allow_raw_pointers());
    function("Task_GetAccelerationX", &Bridge::GetAccelerationX, allow_raw_pointers());
    function("Task_GetAccelerationY", &Bridge::GetAccelerationY, allow_raw_pointers());
    function("Task_GetMass", &Bridge::GetMass, allow_raw_pointers());
    function("Task_GetEntropy", &Bridge::GetEntropy, allow_raw_pointers());
    function("Task_GetStressX", &Bridge::GetStressX, allow_raw_pointers());
    function("Task_GetStressY", &Bridge::GetStressY, allow_raw_pointers());
    function("Task_GetStressZ", &Bridge::GetStressZ, allow_raw_pointers());
    function("Task_GetStepCount", &Bridge::GetStepCount, allow_raw_pointers());
    function("Task_GetCollapseProbability", &Bridge::GetCollapseProbability, allow_raw_pointers());

    function("Task_SetPosition", &Bridge::SetPosition, allow_raw_pointers());
    function("Task_SetVelocity", &Bridge::SetVelocity, allow_raw_pointers());
    function("Task_SetMass", &Bridge::SetMass, allow_raw_pointers());
    function("Task_SetStress", &Bridge::SetStress, allow_raw_pointers());

    function("Task_ApplyForce", &Bridge::ApplyForce, allow_raw_pointers());

    function("Engine_IntegrateClassical", &Bridge::IntegrateClassical, allow_raw_pointers());
    function("Engine_UpdateChaos", &Bridge::UpdateChaos, allow_raw_pointers());
    function("Engine_PerformQuantumCollapse", &Bridge::Collapse, allow_raw_pointers());

    function("Simulation_Create", &Bridge::SimulationCreate, allow_raw_pointers());
    function("Simulation_Destroy", &Bridge::SimulationDestroy, allow_raw_pointers());
    function("Simulation_AddTask", &Bridge::SimulationAddTask, allow_raw_pointers());
    function("Simulation_RemoveTask", &Bridge::SimulationRemoveTask, allow_raw_pointers());
    function("Simulation_Update", &Bridge::SimulationUpdate, allow_raw_pointers());
    function("Simulation_Pause", &Bridge::SimulationPause, allow_raw_pointers());
    function("Simulation_Resume", &Bridge::SimulationResume, allow_raw_pointers());
    function("Simulation_Step", &Bridge::SimulationStep, allow_raw_pointers());
    function("Simulation_SetTimeScale", &Bridge::SimulationSetTimeScale, allow_raw_pointers());
    function("Simulation_GetTimeScale", &Bridge::SimulationGetTimeScale, allow_raw_pointers());
    function("Simulation_SetClassicalEnabled", &Bridge::SimulationSetClassicalEnabled, allow_raw_pointers());
    function("Simulation_IsClassicalEnabled", &Bridge::SimulationIsClassicalEnabled, allow_raw_pointers());
    function("Simulation_GetTaskCount", &Bridge::SimulationGetTaskCount, allow_raw_pointers());

    function("State_Serialize", &Bridge::Serialize, allow_raw_pointers());
    function("State_Deserialize", &Bridge::Deserialize, allow_raw_pointers());

    function("Energy_GetKinetic", &Bridge::GetKineticEnergy, allow_raw_pointers());
    function("Energy_GetPotential", &Bridge::GetPotentialEnergy, allow_raw_pointers());
    function("Energy_GetTotal", &Bridge::GetTotalEnergy, allow_raw_pointers());

    function("Energy_Inject", &Bridge::InjectEnergy, allow_raw_pointers());
    function("Energy_Dissipate", &Bridge::DissipateEnergy, allow_raw_pointers());
    function("Energy_Transfer", &Bridge::TransferEnergy, allow_raw_pointers());

    function("System_GetTotalEnergy", &Bridge::GetSystemTotalEnergy, allow_raw_pointers());
    function("System_RedistributeEnergy", &Bridge::RedistributeEnergy, allow_raw_pointers());
    function("System_EnableDamping", &Bridge::EnableDamping, allow_raw_pointers());
    function("System_DisableDamping", &Bridge::DisableDamping, allow_raw_pointers());

    function("Analytics_GetMeanEnergy", &Bridge::GetMeanEnergy, allow_raw_pointers());
    function("Analytics_GetEnergyStdDev", &Bridge::GetEnergyStdDev, allow_raw_pointers());
}
#endif
