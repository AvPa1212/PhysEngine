#include "physics/MomentumBridge.h"
#include "physics/Task.hpp"            // Contains the actual Task struct definition
#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include "physics/QuantumEngine.hpp"
#include "physics/EnergyEngine.hpp"
#include "core/SimulationEngine.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cmath>

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

    Task* CreateWithParams(double mass, double deadlineTime, double urgencyConstant) {
        Task* task = new Task();
        if (mass > 0.0) {
            task->mass = mass;
        }
        if (deadlineTime > 0.0) {
            task->deadlineTime = deadlineTime;
        }
        task->urgencyConstant = urgencyConstant;
        return task;
    }

    void Destroy(Task* task) {
        delete task;
    }

    // Getters
    double GetPositionX(Task* task) { return (task != nullptr) ? task->position.x : 0.0; }
    double GetPositionY(Task* task) { return (task != nullptr) ? task->position.y : 0.0; }
    double GetVelocityX(Task* task) { return (task != nullptr) ? task->velocity.x : 0.0; }
    double GetVelocityY(Task* task) { return (task != nullptr) ? task->velocity.y : 0.0; }
    double GetAccelerationX(Task* task) { return (task != nullptr) ? task->acceleration.x : 0.0; }
    double GetAccelerationY(Task* task) { return (task != nullptr) ? task->acceleration.y : 0.0; }
    double GetMass(Task* task)      { return (task != nullptr) ? task->mass : 0.0; }
    double GetEntropy(Task* task)   { return (task != nullptr) ? task->entropy : 0.0; }
    double GetStressX(Task* task)   { return (task != nullptr) ? task->stressX : 0.0; }
    double GetStressY(Task* task)   { return (task != nullptr) ? task->stressY : 0.0; }
    double GetStressZ(Task* task)   { return (task != nullptr) ? task->stressZ : 0.0; }
    int GetStepCount(Task* task)    { return (task != nullptr) ? task->stepCount : 0; }
    double GetCollapseProbability(Task* task) {
        return (task != nullptr) ? QuantumEngine::calculateCollapseProbability(*task) : 0.0;
    }

    SimulationEngine* SimulationCreate() {
        return new SimulationEngine();
    }

    void SimulationDestroy(SimulationEngine* engine) {
        delete engine;
    }

    std::size_t SimulationAddTask(
        SimulationEngine* engine,
        double mass,
        double deadlineTime,
        double urgencyConstant,
        double staticFriction,
        double kineticFriction
    ) {
        if (engine == nullptr) {
            return static_cast<std::size_t>(-1);
        }
        if (mass <= 0.0 || deadlineTime <= 0.0 || staticFriction < 0.0 || kineticFriction < 0.0) {
            return static_cast<std::size_t>(-1);
        }

        engine->addTask(mass, deadlineTime, urgencyConstant, staticFriction, kineticFriction);
        return engine->tasks.empty() ? static_cast<std::size_t>(-1) : (engine->tasks.size() - 1);
    }

    int SimulationRemoveTask(SimulationEngine* engine, std::size_t index) {
        if (engine == nullptr) {
            return 0;
        }
        return engine->removeTask(index) ? 1 : 0;
    }

    void SimulationUpdate(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->update();
        }
    }

    void SimulationPause(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->pause();
        }
    }

    void SimulationResume(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->resume();
        }
    }

    void SimulationStep(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->singleStep();
        }
    }

    void SimulationSetTimeScale(SimulationEngine* engine, double scale) {
        if (engine != nullptr) {
            engine->setTimeScale(scale);
        }
    }

    double SimulationGetTimeScale(SimulationEngine* engine) {
        return (engine != nullptr) ? engine->getTimeScale() : 1.0;
    }

    void SimulationSetClassicalEnabled(SimulationEngine* engine, int enabled) {
        if (engine != nullptr) {
            engine->setClassicalEnabled(enabled != 0);
        }
    }

    int SimulationIsClassicalEnabled(SimulationEngine* engine) {
        return (engine != nullptr && engine->isClassicalEnabled()) ? 1 : 0;
    }

    std::size_t SimulationGetTaskCount(SimulationEngine* engine) {
        return (engine != nullptr) ? engine->tasks.size() : 0u;
    }

    int SimulationGetTaskState(SimulationEngine* engine, std::size_t index, TaskState* outState) {
        if (engine == nullptr || outState == nullptr || index >= engine->tasks.size()) {
            return 0;
        }

        const Task& task = engine->tasks[index];
        outState->posX = task.position.x;
        outState->posY = task.position.y;
        outState->velX = task.velocity.x;
        outState->velY = task.velocity.y;
        outState->accX = task.acceleration.x;
        outState->accY = task.acceleration.y;
        return 1;
    }

    std::size_t SimulationGetAllTaskStates(SimulationEngine* engine, TaskState* outStates, std::size_t maxStates) {
        if (engine == nullptr || outStates == nullptr || maxStates == 0) {
            return 0;
        }

        const std::size_t available = engine->tasks.size();
        const std::size_t count = (available < maxStates) ? available : maxStates;
        for (std::size_t i = 0; i < count; ++i) {
            const Task& task = engine->tasks[i];
            outStates[i].posX = task.position.x;
            outStates[i].posY = task.position.y;
            outStates[i].velX = task.velocity.x;
            outStates[i].velY = task.velocity.y;
            outStates[i].accX = task.acceleration.x;
            outStates[i].accY = task.acceleration.y;
        }
        return count;
    }

    // Setters (For UI Manual Overrides)
    void SetPosition(Task* task, double x, double y) {
        if (task) { task->position.x = x; task->position.y = y; }
    }

    void SetVelocity(Task* task, double vx, double vy) {
        if (task) { task->velocity.x = vx; task->velocity.y = vy; }
    }

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
    void IntegrateClassical(Task* task) {
        if (task) {
            ClassicalEngine::integrateRK4(*task);
        }
    }

    void UpdateChaos(Task* task) {
        if (task) {
            ChaosEngine::update(*task);
            task->stepCount++;
        }
    }

    void Collapse(Task* task) {
        if (task) {
            QuantumEngine::collapse(*task);
            // Visual "kick": apply upward velocity upon quantum state reduction
            task->velocity.y += 5.0;
        }
    }

    // --- Force Application ---
    // Velocity is 2D; z-force is ignored to keep Δv = F/m behavior consistent.
    void ApplyForce(Task* task, double fx, double fy, double /*fz*/) {
        if (task && task->mass > 0.0) {
            task->velocity.x += fx / task->mass;
            task->velocity.y += fy / task->mass;
        }
    }

    // --- JSON Serialization Helpers ---
    // Returns true and sets `out` if `key` is found; returns false otherwise.
    static bool tryParseJsonDouble(const std::string& json, const std::string& key, double& out) {
        std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos) return false;
        pos += search.length();
        // Skip whitespace
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        try {
            out = std::stod(json.substr(pos));
            return true;
        } catch (...) {
            return false;
        }
    }

    static bool tryParseJsonInt(const std::string& json, const std::string& key, int& out) {
        std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos) return false;
        pos += search.length();
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        try {
            out = std::stoi(json.substr(pos));
            return true;
        } catch (...) {
            return false;
        }
    }

    // --- State Serialization ---
    std::string Serialize(Task* task) {
        if (!task) return "{}";
        std::ostringstream oss;
        oss << std::setprecision(17)
            << "{\"posX\":" << task->position.x
            << ",\"posY\":" << task->position.y
            << ",\"velX\":" << task->velocity.x
            << ",\"velY\":" << task->velocity.y
            << ",\"mass\":" << task->mass
            << ",\"stressX\":" << task->stressX
            << ",\"stressY\":" << task->stressY
            << ",\"stressZ\":" << task->stressZ
            << ",\"entropy\":" << task->entropy
            << ",\"stepCount\":" << task->stepCount
            << ",\"kineticEnergy\":" << task->kineticEnergy
            << ",\"potentialEnergy\":" << task->potentialEnergy
            << ",\"totalEnergy\":" << task->totalEnergy
            << "}";
        return oss.str();
    }

    // Only updates fields whose keys are present in the JSON, leaving the
    // rest of the task untouched.  This prevents partial JSON (or `{}`)
    // from silently zeroing existing state.
    void Deserialize(Task* task, const std::string& json) {
        if (!task || json.empty()) return;
        double dval;
        int ival;
        if (tryParseJsonDouble(json, "posX", dval))    task->position.x = dval;
        if (tryParseJsonDouble(json, "posY", dval))    task->position.y = dval;
        if (tryParseJsonDouble(json, "velX", dval))    task->velocity.x = dval;
        if (tryParseJsonDouble(json, "velY", dval))    task->velocity.y = dval;
        if (tryParseJsonDouble(json, "mass", dval))    task->mass = dval;
        if (tryParseJsonDouble(json, "stressX", dval)) task->stressX = dval;
        if (tryParseJsonDouble(json, "stressY", dval)) task->stressY = dval;
        if (tryParseJsonDouble(json, "stressZ", dval)) task->stressZ = dval;
        if (tryParseJsonDouble(json, "entropy", dval)) task->entropy = dval;
        if (tryParseJsonInt(json, "stepCount", ival))   task->stepCount = ival;

        // Restore energy fields
        bool hasKE = tryParseJsonDouble(json, "kineticEnergy", dval);
        if (hasKE) task->kineticEnergy = dval;
        bool hasPE = tryParseJsonDouble(json, "potentialEnergy", dval);
        if (hasPE) task->potentialEnergy = dval;
        bool hasTE = tryParseJsonDouble(json, "totalEnergy", dval);
        if (hasTE) task->totalEnergy = dval;

        // Validate consistency: if all three fields were present, check totalEnergy == KE + PE
        if (hasKE && hasPE && hasTE) {
            double expected = task->kineticEnergy + task->potentialEnergy;
            if (std::abs(task->totalEnergy - expected) > 1e-9) {
                // Inconsistent: recalculate from position and velocity
                EnergyEngine::calculateEnergy(*task);
            }
        }
    }
    // --- Energy Queries ---
    double GetKineticEnergy(Task* task)   { return (task != nullptr) ? task->kineticEnergy : 0.0; }
    double GetPotentialEnergy(Task* task) { return (task != nullptr) ? task->potentialEnergy : 0.0; }
    double GetTotalEnergy(Task* task)     { return (task != nullptr) ? task->totalEnergy : 0.0; }

    // --- Energy Operations ---
    void InjectEnergy(Task* task, double amount) {
        if (task != nullptr) {
            EnergyEngine::injectEnergy(*task, amount);
        }
    }

    void DissipateEnergy(Task* task, double dampingCoeff) {
        if (task != nullptr) {
            EnergyEngine::dissipateEnergy(*task, dampingCoeff);
        }
    }

    void TransferEnergy(Task* source, Task* target, double amount) {
        if (source != nullptr && target != nullptr) {
            EnergyEngine::transferEnergy(*source, *target, amount);
        }
    }

    // --- System-Level Energy ---
    double GetSystemTotalEnergy(SimulationEngine* engine) {
        return (engine != nullptr) ? engine->getSystemEnergy() : 0.0;
    }

    void RedistributeEnergy(SimulationEngine* engine, int completedTaskIndex) {
        if (engine == nullptr) return;
        const std::size_t idx = static_cast<std::size_t>(completedTaskIndex);
        if (completedTaskIndex < 0 || idx >= engine->tasks.size()) return;
        Task completedTask = engine->tasks[idx];
        engine->tasks.erase(engine->tasks.begin() + static_cast<std::ptrdiff_t>(idx));
        EnergyEngine::redistributeEnergy(completedTask, engine->tasks);
    }

    void EnableDamping(SimulationEngine* engine, double coefficient) {
        if (engine != nullptr) {
            engine->enableDamping(coefficient);
        }
    }

    void DisableDamping(SimulationEngine* engine) {
        if (engine != nullptr) {
            engine->disableDamping();
        }
    }

    double GetMeanEnergy(SimulationEngine* engine) {
        if (engine == nullptr) return 0.0;
        return EnergyEngine::computeMeanEnergy(engine->tasks);
    }

    double GetEnergyStdDev(SimulationEngine* engine) {
        if (engine == nullptr) return 0.0;
        return EnergyEngine::computeEnergyStdDev(engine->tasks);
    }
}

// --- Desktop/Python compatibility layer (extern "C") ---
// These maintain existing ABI compatibility for your Python scripts
extern "C" {
    MOMENTUM_API Task* Task_Create() { 
        return Bridge::Create(); 
    }

    MOMENTUM_API Task* Task_CreateWithParams(double mass, double deadlineTime, double urgencyConstant) {
        return Bridge::CreateWithParams(mass, deadlineTime, urgencyConstant);
    }

    MOMENTUM_API void Task_Destroy(Task* t) {
        Bridge::Destroy(t);
    }
    
    MOMENTUM_API void Task_SetPosition(Task* t, double x, double y) {
        Bridge::SetPosition(t, x, y);
    }

    MOMENTUM_API void Task_SetVelocity(Task* t, double vx, double vy) {
        Bridge::SetVelocity(t, vx, vy);
    }

    MOMENTUM_API void Task_SetMass(Task* t, double mass) {
        Bridge::SetMass(t, mass);
    }

    MOMENTUM_API void Task_SetStress(Task* t, double sx, double sy, double sz) {
        Bridge::SetStress(t, sx, sy, sz);
    }

    MOMENTUM_API double Task_GetPositionX(Task* t) { 
        return Bridge::GetPositionX(t); 
    }

    MOMENTUM_API double Task_GetPositionY(Task* t) {
        return Bridge::GetPositionY(t);
    }

    MOMENTUM_API double Task_GetEntropy(Task* t) { 
        return Bridge::GetEntropy(t); 
    }
    
    MOMENTUM_API double Task_GetStressX(Task* t) { 
        return Bridge::GetStressX(t); 
    }

    MOMENTUM_API double Task_GetStressY(Task* t) {
        return Bridge::GetStressY(t);
    }

    MOMENTUM_API double Task_GetStressZ(Task* t) {
        return Bridge::GetStressZ(t);
    }

    MOMENTUM_API double Task_GetCollapseProbability(Task* t) {
        return Bridge::GetCollapseProbability(t);
    }

    MOMENTUM_API double Task_GetVelocityX(Task* t) {
        return Bridge::GetVelocityX(t);
    }

    MOMENTUM_API double Task_GetVelocityY(Task* t) {
        return Bridge::GetVelocityY(t);
    }

    MOMENTUM_API double Task_GetAccelerationX(Task* t) {
        return Bridge::GetAccelerationX(t);
    }

    MOMENTUM_API double Task_GetAccelerationY(Task* t) {
        return Bridge::GetAccelerationY(t);
    }

    MOMENTUM_API double Task_GetMass(Task* t) {
        return Bridge::GetMass(t);
    }

    MOMENTUM_API int Task_GetStepCount(Task* t) {
        return Bridge::GetStepCount(t);
    }

    MOMENTUM_API void Task_ApplyForce(Task* t, double fx, double fy, double fz) {
        Bridge::ApplyForce(t, fx, fy, fz);
    }

    MOMENTUM_API void Engine_IntegrateClassical(Task* t) {
        Bridge::IntegrateClassical(t);
    }

    MOMENTUM_API void Engine_UpdateChaos(Task* t) { 
        Bridge::UpdateChaos(t); 
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
        return Bridge::SimulationAddTask(engine, mass, deadlineTime, urgencyConstant, staticFriction, kineticFriction);
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

    MOMENTUM_API size_t Simulation_GetAllTaskStates(SimulationEngine* engine, TaskState* outStates, size_t maxStates) {
        return Bridge::SimulationGetAllTaskStates(engine, outStates, maxStates);
    }
    
    MOMENTUM_API void Engine_PerformQuantumCollapse(Task* t) { 
        Bridge::Collapse(t);    
    }

    // Thread-local buffer to keep returned JSON alive until the next call on this thread
    static thread_local std::string g_serializeBuffer;

    MOMENTUM_API const char* State_Serialize(Task* t) {
        g_serializeBuffer = Bridge::Serialize(t);
        return g_serializeBuffer.c_str();
    }

    MOMENTUM_API void State_Deserialize(Task* t, const char* json) {
        if (json) Bridge::Deserialize(t, std::string(json));
    }

    // --- Energy Query Functions ---
    MOMENTUM_API double Energy_GetKinetic(Task* t) {
        return Bridge::GetKineticEnergy(t);
    }

    MOMENTUM_API double Energy_GetPotential(Task* t) {
        return Bridge::GetPotentialEnergy(t);
    }

    MOMENTUM_API double Energy_GetTotal(Task* t) {
        return Bridge::GetTotalEnergy(t);
    }

    // --- Energy Operation Functions ---
    MOMENTUM_API void Energy_Inject(Task* t, double amount) {
        Bridge::InjectEnergy(t, amount);
    }

    MOMENTUM_API void Energy_Dissipate(Task* t, double dampingCoeff) {
        Bridge::DissipateEnergy(t, dampingCoeff);
    }

    MOMENTUM_API void Energy_Transfer(Task* source, Task* target, double amount) {
        Bridge::TransferEnergy(source, target, amount);
    }

    // --- System-Level Energy Functions ---
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

// --- WebAssembly Bindings (Emscripten Only) ---
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(momentum_module) {
    // 1. Register the Task class
    class_<Task>("Task");
    class_<SimulationEngine>("SimulationEngine");

    // 2. Bind Getters
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
    
    // 3. Bind Setters
    function("Task_SetPosition", &Bridge::SetPosition, allow_raw_pointers());
    function("Task_SetVelocity", &Bridge::SetVelocity, allow_raw_pointers());
    function("Task_SetMass", &Bridge::SetMass, allow_raw_pointers());
    function("Task_SetStress", &Bridge::SetStress, allow_raw_pointers());

    // 4. Bind Force Application
    function("Task_ApplyForce", &Bridge::ApplyForce, allow_raw_pointers());

    // 5. Bind Engine Actions
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

    // 6. Bind State Serialization
    function("State_Serialize", &Bridge::Serialize, allow_raw_pointers());
    function("State_Deserialize", &Bridge::Deserialize, allow_raw_pointers());

    // 7. Bind Energy Query Functions
    function("Energy_GetKinetic", &Bridge::GetKineticEnergy, allow_raw_pointers());
    function("Energy_GetPotential", &Bridge::GetPotentialEnergy, allow_raw_pointers());
    function("Energy_GetTotal", &Bridge::GetTotalEnergy, allow_raw_pointers());

    // 8. Bind Energy Operation Functions
    function("Energy_Inject", &Bridge::InjectEnergy, allow_raw_pointers());
    function("Energy_Dissipate", &Bridge::DissipateEnergy, allow_raw_pointers());
    function("Energy_Transfer", &Bridge::TransferEnergy, allow_raw_pointers());

    // 9. Bind System-Level Energy Functions
    function("System_GetTotalEnergy", &Bridge::GetSystemTotalEnergy, allow_raw_pointers());
    function("System_RedistributeEnergy", &Bridge::RedistributeEnergy, allow_raw_pointers());
    function("System_EnableDamping", &Bridge::EnableDamping, allow_raw_pointers());
    function("System_DisableDamping", &Bridge::DisableDamping, allow_raw_pointers());

    // 10. Bind Energy Analytics Functions
    function("Analytics_GetMeanEnergy", &Bridge::GetMeanEnergy, allow_raw_pointers());
    function("Analytics_GetEnergyStdDev", &Bridge::GetEnergyStdDev, allow_raw_pointers());
}
#endif