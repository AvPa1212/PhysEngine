/**
 * @file BridgeState.cpp
 * @brief Lightweight JSON serialization helpers for bridge callers.
 *
 * The bridge intentionally avoids adding a JSON dependency. The parser below
 * only supports the flat numeric payloads produced by Serialize().
 */
#include "BridgeInternal.hpp"

#include "physics/EnergyEngine.hpp"
#include "physics/Task.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace {
    bool TryParseJsonDouble(const std::string& json, const std::string& key, double& out) {
        const std::string search = "\"" + key + "\":";
        std::size_t pos = json.find(search);
        if (pos == std::string::npos) {
            return false;
        }

        pos += search.length();
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
            pos += 1;
        }

        try {
            out = std::stod(json.substr(pos));
            return true;
        } catch (...) {
            return false;
        }
    }

    bool TryParseJsonInt(const std::string& json, const std::string& key, int& out) {
        const std::string search = "\"" + key + "\":";
        std::size_t pos = json.find(search);
        if (pos == std::string::npos) {
            return false;
        }

        pos += search.length();
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
            pos += 1;
        }

        try {
            out = std::stoi(json.substr(pos));
            return true;
        } catch (...) {
            return false;
        }
    }
}

namespace Bridge {
    std::string Serialize(Task* task) {
        if (task == nullptr) {
            return "{}";
        }

        std::ostringstream stream;
        stream << std::setprecision(17)
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
        return stream.str();
    }

    void Deserialize(Task* task, const std::string& json) {
        if (task == nullptr || json.empty()) {
            return;
        }

        double doubleValue = 0.0;
        int intValue = 0;

        if (TryParseJsonDouble(json, "posX", doubleValue)) {
            task->position.x = doubleValue;
        }
        if (TryParseJsonDouble(json, "posY", doubleValue)) {
            task->position.y = doubleValue;
        }
        if (TryParseJsonDouble(json, "velX", doubleValue)) {
            task->velocity.x = doubleValue;
        }
        if (TryParseJsonDouble(json, "velY", doubleValue)) {
            task->velocity.y = doubleValue;
        }
        if (TryParseJsonDouble(json, "mass", doubleValue)) {
            task->mass = doubleValue;
        }
        if (TryParseJsonDouble(json, "stressX", doubleValue)) {
            task->stressX = doubleValue;
        }
        if (TryParseJsonDouble(json, "stressY", doubleValue)) {
            task->stressY = doubleValue;
        }
        if (TryParseJsonDouble(json, "stressZ", doubleValue)) {
            task->stressZ = doubleValue;
        }
        if (TryParseJsonDouble(json, "entropy", doubleValue)) {
            task->entropy = doubleValue;
        }
        if (TryParseJsonInt(json, "stepCount", intValue)) {
            task->stepCount = intValue;
        }

        const bool hasKinetic = TryParseJsonDouble(json, "kineticEnergy", doubleValue);
        if (hasKinetic) {
            task->kineticEnergy = doubleValue;
        }
        const bool hasPotential = TryParseJsonDouble(json, "potentialEnergy", doubleValue);
        if (hasPotential) {
            task->potentialEnergy = doubleValue;
        }
        const bool hasTotal = TryParseJsonDouble(json, "totalEnergy", doubleValue);
        if (hasTotal) {
            task->totalEnergy = doubleValue;
        }

        if (hasKinetic && hasPotential && hasTotal) {
            const double expected = task->kineticEnergy + task->potentialEnergy;
            if (std::abs(task->totalEnergy - expected) > 1e-9) {
                EnergyEngine::calculateEnergy(*task);
            }
        }
    }
}
