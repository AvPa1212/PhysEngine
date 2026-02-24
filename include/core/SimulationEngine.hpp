#pragma once
#include <vector>
#include "physics/Task.hpp"
#include "physics/ClassicalEngine.hpp"
#include "physics/QuantumEngine.hpp"
#include "physics/ThermodynamicsEngine.hpp"
#include "physics/ChaosEngine.hpp"

class SimulationEngine {
public:
    std::vector<Task> tasks;

    SimulationEngine() {
        // Pre-allocate to prevent dynamic allocation during loop
        tasks.reserve(100);
    }

    void update() {
        for (auto& task : tasks) {
            ClassicalEngine::integrateRK4(task);
            QuantumEngine::evolve(task);
            ThermodynamicsEngine::updateEntropy(task);
            ChaosEngine::update(task);
        }
    }
};