/**
 * @file MomentumBridge.cpp
 * @brief Implementation of the public C/WASM bridge for the physics engine.
 *
 * Provides three binding layers:
 *
 *  1. **Bridge namespace** — C++ helper functions that implement the actual
 *     logic, perform null-pointer safety checks, and forward to the engine
 *     classes.  Keeping the real logic here means it is testable from C++
 *     without going through the C ABI.
 *
 *  2. **extern "C" block** — thin wrappers that satisfy the C ABI declared in
 *     MomentumBridge.h.  Python ctypes and any other C-compatible caller uses
 *     these entry points.
 *
 *  3. **EMSCRIPTEN_BINDINGS block** — only compiled when targeting WebAssembly
 *     via Emscripten.  Registers every Bridge function with Embind so that
 *     JavaScript can call them as regular module methods (e.g.
 *     Module.Task_Create()).
 *
 * When adding a new public API function, add it in all three places so that
 * desktop/Python and browser/WASM callers both gain access.
 */
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
    // -----------------------------------------------------------------------
    // Constructor / Destructor
    // -----------------------------------------------------------------------

    /**
     * @brief Allocates a new default-initialised Task on the heap.
     * @return Raw owning pointer; the caller must eventually call Destroy().
     */
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

    /**
     * @brief Frees the Task pointed to by @p task.
     * @param task Pointer returned by Create().  nullptr is handled gracefully
     *             by the delete operator.
     */
    void Destroy(Task* task) {
        delete task;
    }

    // -----------------------------------------------------------------------
    // Getters — all return 0 / 0.0 when task is nullptr for safety
    // -----------------------------------------------------------------------

    /** @brief Returns position.x or 0.0 if task is null. */
    double GetPositionX(Task* task) { return (task != nullptr) ? task->position.x : 0.0; }

    /** @brief Returns position.y or 0.0 if task is null. */
    double GetPositionY(Task* task) { return (task != nullptr) ? task->position.y : 0.0; }

    /** @brief Returns velocity.x or 0.0 if task is null. */
    double GetVelocityX(Task* task) { return (task != nullptr) ? task->velocity.x : 0.0; }

    /** @brief Returns velocity.y or 0.0 if task is null. */
    double GetVelocityY(Task* task) { return (task != nullptr) ? task->velocity.y : 0.0; }
    double GetAccelerationX(Task* task) { return (task != nullptr) ? task->acceleration.x : 0.0; }
    double GetAccelerationY(Task* task) { return (task != nullptr) ? task->acceleration.y : 0.0; }

    /** @brief Returns mass or 0.0 if task is null. */
    double GetMass(Task* task)      { return (task != nullptr) ? task->mass : 0.0; }

    /** @brief Returns entropy or 0.0 if task is null. */
    double GetEntropy(Task* task)   { return (task != nullptr) ? task->entropy : 0.0; }

    /** @brief Returns stressX or 0.0 if task is null. */
    double GetStressX(Task* task)   { return (task != nullptr) ? task->stressX : 0.0; }

    /** @brief Returns stressY or 0.0 if task is null. */
    double GetStressY(Task* task)   { return (task != nullptr) ? task->stressY : 0.0; }

    /** @brief Returns stressZ or 0.0 if task is null. */
    double GetStressZ(Task* task)   { return (task != nullptr) ? task->stressZ : 0.0; }

    /** @brief Returns stepCount or 0 if task is null. */
    int GetStepCount(Task* task)    { return (task != nullptr) ? task->stepCount : 0; }

    /**
     * @brief Returns the wavefunction collapse probability in [0, 1].
     *
     * Delegates to QuantumEngine::calculateCollapseProbability.
     * Returns 0.0 if task is null.
     */
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

    // -----------------------------------------------------------------------
    // Setters — For UI Manual Overrides; all guard against null task pointer
    // -----------------------------------------------------------------------

    /**
     * @brief Directly sets the task's 2-D position.
     * @param task  Target task pointer (no-op if null).
     * @param x     New X coordinate.
     * @param y     New Y coordinate.
     */
    void SetPosition(Task* task, double x, double y) {
        if (task) { task->position.x = x; task->position.y = y; }
    }

    /**
     * @brief Directly sets the task's 2-D velocity.
     * @param task  Target task pointer (no-op if null).
     * @param vx    New X velocity.
     * @param vy    New Y velocity.
     */
    void SetVelocity(Task* task, double vx, double vy) {
        if (task) { task->velocity.x = vx; task->velocity.y = vy; }
    }

    /**
     * @brief Directly sets the task's mass.
     * @param task  Target task pointer (no-op if null).
     * @param mass  New mass value (should be > 0).
     */
    void SetMass(Task* task, double mass) {
        if (task) task->mass = mass;
    }

    /**
     * @brief Directly sets all three Lorenz stress components.
     * @param task  Target task pointer (no-op if null).
     * @param sx    New X stress value.
     * @param sy    New Y stress value.
     * @param sz    New Z stress value.
     */
    void SetStress(Task* task, double sx, double sy, double sz) {
        if (task) {
            task->stressX = sx;
            task->stressY = sy;
            task->stressZ = sz;
        }
    }

    // -----------------------------------------------------------------------
    // Engine Commands — single-step physics operations
    // -----------------------------------------------------------------------

    /**
     * @brief Runs one RK4 classical integration step on the task.
     * @param task  Target task (no-op if null).
     */
    void IntegrateClassical(Task* task) {
        if (task) {
            ClassicalEngine::integrateRK4(*task);
        }
    }

    /**
     * @brief Advances the Lorenz chaos state and increments the step counter.
     *
     * The extra stepCount increment here accounts for the fact that ChaosEngine
     * itself does not increment the counter; when called independently of the
     * full SimulationEngine loop the counter must still advance.
     *
     * @param task  Target task (no-op if null).
     */
    void UpdateChaos(Task* task) {
        if (task) {
            ChaosEngine::update(*task);
            task->stepCount++;
        }
    }

    /**
     * @brief Collapses the wavefunction and applies a visual velocity kick.
     *
     * After calling QuantumEngine::collapse() (which zeros entropy and sets a
     * pure basis state), a small upward velocity is added so the task visually
     * "bounces" in the Three.js viewport to confirm the collapse event.
     *
     * @param task  Target task (no-op if null).
     */
    void Collapse(Task* task) {
        if (task) {
            QuantumEngine::collapse(*task);
            // Visual "kick": apply upward velocity upon quantum state reduction
            task->velocity.y += 5.0;
        }
    }

    // -----------------------------------------------------------------------
    // Force Application
    // -----------------------------------------------------------------------

    /**
     * @brief Applies an instantaneous force impulse to the task.
     *
     * Velocity is 2D; z-force is ignored to keep Δv = F/m behavior consistent.
     * The impulse is Δv = F/m, applied directly to the velocity components.
     *
     * @param task   Target task (no-op if null or zero mass).
     * @param fx     Force X component.
     * @param fy     Force Y component.
     * @param fz     Force Z component (accepted for API symmetry, unused).
     */
    void ApplyForce(Task* task, double fx, double fy, double /*fz*/) {
        if (task && task->mass > 0.0) {
            task->velocity.x += fx / task->mass;
            task->velocity.y += fy / task->mass;
        }
    }

    // -----------------------------------------------------------------------
    // JSON Serialization Helpers (internal linkage via static)
    // -----------------------------------------------------------------------

    /**
     * @brief Searches @p json for @p key and parses its value as a double.
     *
     * Uses simple string-find rather than a full JSON parser for zero-dependency
     * portability.  Stops parsing at the first non-numeric character after the
     * value, relying on std::stod's natural stopping behaviour.
     *
     * Returns true and sets `out` if `key` is found; returns false otherwise.
     *
     * @param json  Full JSON string to search.
     * @param key   Key name (without quotes).
     * @param out   Receives the parsed double on success.
     * @return true if the key was found and parsed successfully; false otherwise.
     */
    static bool tryParseJsonDouble(const std::string& json, const std::string& key, double& out) {
        std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos) return false;
        pos += search.length();
        // Skip optional whitespace between the colon and the value.
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        try {
            out = std::stod(json.substr(pos));
            return true;
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Searches @p json for @p key and parses its value as an int.
     *
     * Mirrors tryParseJsonDouble but calls std::stoi for integer values.
     *
     * @param json  Full JSON string to search.
     * @param key   Key name (without quotes).
     * @param out   Receives the parsed int on success.
     * @return true if the key was found and parsed successfully; false otherwise.
     */
    static bool tryParseJsonInt(const std::string& json, const std::string& key, int& out) {
        std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos) return false;
        pos += search.length();
        // Skip optional whitespace between the colon and the value.
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        try {
            out = std::stoi(json.substr(pos));
            return true;
        } catch (...) {
            return false;
        }
    }

    // -----------------------------------------------------------------------
    // State Serialization
    // -----------------------------------------------------------------------

    /**
     * @brief Serialises the task's core simulation state to a JSON string.
     *
     * Uses std::setprecision(17) to ensure every double can be round-tripped
     * through Deserialize without losing precision (17 significant digits is
     * the minimum needed for IEEE 754 doubles).
     *
     * Returns "{}" for a null task pointer.
     *
     * @param task  Task to serialise (may be nullptr).
     * @return JSON string with keys: posX, posY, velX, velY, mass, stressX,
     *         stressY, stressZ, entropy, stepCount.
     */
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

    /**
     * @brief Restores task fields from a JSON string produced by Serialize.
     *
     * Only updates fields whose keys are present in the JSON, leaving the
     * rest of the task untouched.  This prevents partial JSON (or `{}`)
     * from silently zeroing existing state.
     *
     * @param task  Task to update (no-op if null or json is empty).
     * @param json  JSON string; missing or unparseable keys are silently skipped.
     */
    void Deserialize(Task* task, const std::string& json) {
        if (!task || json.empty()) return;
        double dval;
        int ival;
        // Each tryParse call updates the field only if the key is present.
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
    /** @brief Creates a new Task. @see Bridge::Create */
    MOMENTUM_API Task* Task_Create() { 
        return Bridge::Create(); 
    }

    MOMENTUM_API Task* Task_CreateWithParams(double mass, double deadlineTime, double urgencyConstant) {
        return Bridge::CreateWithParams(mass, deadlineTime, urgencyConstant);
    }

    /** @brief Destroys a Task. @see Bridge::Destroy */
    MOMENTUM_API void Task_Destroy(Task* t) {
        Bridge::Destroy(t);
    }

    /** @brief Sets task position. @see Bridge::SetPosition */
    MOMENTUM_API void Task_SetPosition(Task* t, double x, double y) {
        Bridge::SetPosition(t, x, y);
    }

    /** @brief Sets task velocity. @see Bridge::SetVelocity */
    MOMENTUM_API void Task_SetVelocity(Task* t, double vx, double vy) {
        Bridge::SetVelocity(t, vx, vy);
    }

    /** @brief Sets task mass. @see Bridge::SetMass */
    MOMENTUM_API void Task_SetMass(Task* t, double mass) {
        Bridge::SetMass(t, mass);
    }

    /** @brief Sets Lorenz stress components. @see Bridge::SetStress */
    MOMENTUM_API void Task_SetStress(Task* t, double sx, double sy, double sz) {
        Bridge::SetStress(t, sx, sy, sz);
    }

    /** @brief Returns task position X. @see Bridge::GetPositionX */
    MOMENTUM_API double Task_GetPositionX(Task* t) { 
        return Bridge::GetPositionX(t); 
    }

    /** @brief Returns task position Y. @see Bridge::GetPositionY */
    MOMENTUM_API double Task_GetPositionY(Task* t) {
        return Bridge::GetPositionY(t);
    }

    /** @brief Returns task entropy. @see Bridge::GetEntropy */
    MOMENTUM_API double Task_GetEntropy(Task* t) { 
        return Bridge::GetEntropy(t); 
    }

    /** @brief Returns Lorenz stress X. @see Bridge::GetStressX */
    MOMENTUM_API double Task_GetStressX(Task* t) { 
        return Bridge::GetStressX(t); 
    }

    /** @brief Returns Lorenz stress Y. @see Bridge::GetStressY */
    MOMENTUM_API double Task_GetStressY(Task* t) {
        return Bridge::GetStressY(t);
    }

    /** @brief Returns Lorenz stress Z. @see Bridge::GetStressZ */
    MOMENTUM_API double Task_GetStressZ(Task* t) {
        return Bridge::GetStressZ(t);
    }

    /** @brief Returns wavefunction collapse probability. @see Bridge::GetCollapseProbability */
    MOMENTUM_API double Task_GetCollapseProbability(Task* t) {
        return Bridge::GetCollapseProbability(t);
    }

    /** @brief Returns task velocity X. @see Bridge::GetVelocityX */
    MOMENTUM_API double Task_GetVelocityX(Task* t) {
        return Bridge::GetVelocityX(t);
    }

    /** @brief Returns task velocity Y. @see Bridge::GetVelocityY */
    MOMENTUM_API double Task_GetVelocityY(Task* t) {
        return Bridge::GetVelocityY(t);
    }

    MOMENTUM_API double Task_GetAccelerationX(Task* t) {
        return Bridge::GetAccelerationX(t);
    }

    MOMENTUM_API double Task_GetAccelerationY(Task* t) {
        return Bridge::GetAccelerationY(t);
    }

    /** @brief Returns task mass. @see Bridge::GetMass */
    MOMENTUM_API double Task_GetMass(Task* t) {
        return Bridge::GetMass(t);
    }

    /** @brief Returns integration step count. @see Bridge::GetStepCount */
    MOMENTUM_API int Task_GetStepCount(Task* t) {
        return Bridge::GetStepCount(t);
    }

    /** @brief Applies a force impulse to the task. @see Bridge::ApplyForce */
    MOMENTUM_API void Task_ApplyForce(Task* t, double fx, double fy, double fz) {
        Bridge::ApplyForce(t, fx, fy, fz);
    }

    /** @brief Runs one RK4 classical integration step. @see Bridge::IntegrateClassical */
    MOMENTUM_API void Engine_IntegrateClassical(Task* t) {
        Bridge::IntegrateClassical(t);
    }

    /** @brief Advances the Lorenz chaos state. @see Bridge::UpdateChaos */
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

    /** @brief Collapses the quantum wavefunction. @see Bridge::Collapse */
    MOMENTUM_API void Engine_PerformQuantumCollapse(Task* t) { 
        Bridge::Collapse(t);    
    }

    // Thread-local buffer to keep returned JSON alive until the next call on this thread
    static thread_local std::string g_serializeBuffer;

    /**
     * @brief Serialises the task to JSON; the pointer is valid until the next call on this thread.
     * @see Bridge::Serialize
     */
    MOMENTUM_API const char* State_Serialize(Task* t) {
        g_serializeBuffer = Bridge::Serialize(t);
        return g_serializeBuffer.c_str();
    }

    /**
     * @brief Deserialises a JSON string into the task.
     * @see Bridge::Deserialize
     */
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
// Registers every Bridge function as a JavaScript-callable module method.
// The allow_raw_pointers() policy is required because Task* is an unmanaged
// raw pointer; Embind would otherwise refuse to bind it.
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(momentum_module) {
    // 1. Register the Task class so Embind knows the Task* type.
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