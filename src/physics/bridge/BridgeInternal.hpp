/**
 * @file BridgeInternal.hpp
 * @brief Internal declarations for the public task/simulation bridge.
 *
 * The public API remains declared in `physics/MomentumBridge.h`. This header
 * only exists so the bridge implementation can be split into focused
 * translation units without exposing extra internal structure.
 */
#pragma once

#include "physics/MomentumBridge.h"
#include <cstddef>
#include <string>

namespace Bridge {
    Task* Create();
    Task* CreateWithParams(double mass, double deadlineTime, double urgencyConstant);
    void Destroy(Task* task);

    double GetPositionX(Task* task);
    double GetPositionY(Task* task);
    double GetVelocityX(Task* task);
    double GetVelocityY(Task* task);
    double GetAccelerationX(Task* task);
    double GetAccelerationY(Task* task);
    double GetMass(Task* task);
    double GetEntropy(Task* task);
    double GetStressX(Task* task);
    double GetStressY(Task* task);
    double GetStressZ(Task* task);
    int GetStepCount(Task* task);
    double GetCollapseProbability(Task* task);

    void SetPosition(Task* task, double x, double y);
    void SetVelocity(Task* task, double vx, double vy);
    void SetMass(Task* task, double mass);
    void SetStress(Task* task, double sx, double sy, double sz);

    void IntegrateClassical(Task* task);
    void UpdateChaos(Task* task);
    void Collapse(Task* task);
    void ApplyForce(Task* task, double fx, double fy, double fz);

    SimulationEngine* SimulationCreate();
    void SimulationDestroy(SimulationEngine* engine);
    std::size_t SimulationAddTask(
        SimulationEngine* engine,
        double mass,
        double deadlineTime,
        double urgencyConstant,
        double staticFriction,
        double kineticFriction
    );
    int SimulationRemoveTask(SimulationEngine* engine, std::size_t index);
    void SimulationUpdate(SimulationEngine* engine);
    void SimulationPause(SimulationEngine* engine);
    void SimulationResume(SimulationEngine* engine);
    void SimulationStep(SimulationEngine* engine);
    void SimulationSetTimeScale(SimulationEngine* engine, double scale);
    double SimulationGetTimeScale(SimulationEngine* engine);
    void SimulationSetClassicalEnabled(SimulationEngine* engine, int enabled);
    int SimulationIsClassicalEnabled(SimulationEngine* engine);
    std::size_t SimulationGetTaskCount(SimulationEngine* engine);
    int SimulationGetTaskState(SimulationEngine* engine, std::size_t index, TaskState* outState);
    std::size_t SimulationGetAllTaskStates(SimulationEngine* engine, TaskState* outStates, std::size_t maxStates);

    std::string Serialize(Task* task);
    void Deserialize(Task* task, const std::string& json);

    double GetKineticEnergy(Task* task);
    double GetPotentialEnergy(Task* task);
    double GetTotalEnergy(Task* task);
    void InjectEnergy(Task* task, double amount);
    void DissipateEnergy(Task* task, double dampingCoeff);
    void TransferEnergy(Task* source, Task* target, double amount);
    double GetSystemTotalEnergy(SimulationEngine* engine);
    void RedistributeEnergy(SimulationEngine* engine, int completedTaskIndex);
    void EnableDamping(SimulationEngine* engine, double coefficient);
    void DisableDamping(SimulationEngine* engine);
    double GetMeanEnergy(SimulationEngine* engine);
    double GetEnergyStdDev(SimulationEngine* engine);
}
