#include "physics/MomentumBridge.h"
#include "physics/Task.hpp"            // Contains the actual Task struct definition
#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include "physics/QuantumEngine.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

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
    double GetPositionY(Task* task) { return (task != nullptr) ? task->position.y : 0.0; }
    double GetVelocityX(Task* task) { return (task != nullptr) ? task->velocity.x : 0.0; }
    double GetVelocityY(Task* task) { return (task != nullptr) ? task->velocity.y : 0.0; }
    double GetMass(Task* task)      { return (task != nullptr) ? task->mass : 0.0; }
    double GetEntropy(Task* task)   { return (task != nullptr) ? task->entropy : 0.0; }
    double GetStressX(Task* task)   { return (task != nullptr) ? task->stressX : 0.0; }
    double GetStressY(Task* task)   { return (task != nullptr) ? task->stressY : 0.0; }
    double GetStressZ(Task* task)   { return (task != nullptr) ? task->stressZ : 0.0; }
    int GetStepCount(Task* task)    { return (task != nullptr) ? task->stepCount : 0; }
    double GetCollapseProbability(Task* task) {
        return (task != nullptr) ? QuantumEngine::calculateCollapseProbability(*task) : 0.0;
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
    void ApplyForce(Task* task, double fx, double fy, double fz) {
        if (task && task->mass > 0.0) {
            task->velocity.x += fx / task->mass;
            task->velocity.y += fy / task->mass;
            task->stressX += fz * 0.01;
        }
    }

    // --- JSON Serialization Helpers ---
    static double parseJsonDouble(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos) return 0.0;
        pos += search.length();
        // Skip whitespace
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        try {
            return std::stod(json.substr(pos));
        } catch (...) {
            return 0.0;
        }
    }

    static int parseJsonInt(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos) return 0;
        pos += search.length();
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        try {
            return std::stoi(json.substr(pos));
        } catch (...) {
            return 0;
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
            << "}";
        return oss.str();
    }

    void Deserialize(Task* task, const std::string& json) {
        if (!task || json.empty()) return;
        task->position.x = parseJsonDouble(json, "posX");
        task->position.y = parseJsonDouble(json, "posY");
        task->velocity.x = parseJsonDouble(json, "velX");
        task->velocity.y = parseJsonDouble(json, "velY");
        task->mass = parseJsonDouble(json, "mass");
        task->stressX = parseJsonDouble(json, "stressX");
        task->stressY = parseJsonDouble(json, "stressY");
        task->stressZ = parseJsonDouble(json, "stressZ");
        task->entropy = parseJsonDouble(json, "entropy");
        task->stepCount = parseJsonInt(json, "stepCount");
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
    
    MOMENTUM_API void Engine_PerformQuantumCollapse(Task* t) { 
        Bridge::Collapse(t);    
    }

    // Static buffer to keep returned JSON alive until the next call
    static std::string g_serializeBuffer;

    MOMENTUM_API const char* State_Serialize(Task* t) {
        g_serializeBuffer = Bridge::Serialize(t);
        return g_serializeBuffer.c_str();
    }

    MOMENTUM_API void State_Deserialize(Task* t, const char* json) {
        if (json) Bridge::Deserialize(t, std::string(json));
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
    function("Task_GetPositionY", &Bridge::GetPositionY, allow_raw_pointers());
    function("Task_GetVelocityX", &Bridge::GetVelocityX, allow_raw_pointers());
    function("Task_GetVelocityY", &Bridge::GetVelocityY, allow_raw_pointers());
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

    // 6. Bind State Serialization
    function("State_Serialize", &Bridge::Serialize, allow_raw_pointers());
    function("State_Deserialize", &Bridge::Deserialize, allow_raw_pointers());
}
#endif