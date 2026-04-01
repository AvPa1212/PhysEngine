/**
 * @file SimulationEngine.hpp
 * @brief Top-level orchestrator that drives all physics sub-systems per frame.
 *
 * SimulationEngine owns a flat list of Task objects and, on each call to
 * update(), advances every task through four pipeline stages:
 *   1. Classical mechanics  — RK4 position/velocity integration
 *   2. Quantum evolution    — approximate Schrödinger wavefunction update
 *   3. Thermodynamics       — Shannon entropy recalculation
 *   4. Chaos dynamics       — Lorenz attractor stress update
 *
 * The engine is deliberately simple (no scene-graph, no spatial indexing)
 * so it compiles cleanly to native desktop code, to WebAssembly, and to
 * any embedded target that supports C++17.
 */
#pragma once
#include <vector>
#include "physics/Task.hpp"
#include "physics/ClassicalEngine.hpp"
#include "physics/QuantumEngine.hpp"
#include "physics/ThermodynamicsEngine.hpp"
#include "physics/ChaosEngine.hpp"

/**
 * @brief Manages a collection of Task objects and steps them forward in time.
 *
 * Typical usage:
 * @code
 *   SimulationEngine engine;
 *   engine.tasks.push_back(myTask);
 *   engine.update(); // call once per frame
 * @endcode
 */
class SimulationEngine {
public:
    /// All tasks currently tracked by the simulation.  Tasks can be added or
    /// removed between frames; the engine processes whatever is present at
    /// the start of each update() call.
    std::vector<Task> tasks;

    /// When true, update() becomes a no-op so task state is frozen.  Toggle
    /// this to implement a pause/resume feature in the UI.
    bool paused = false;

    /**
     * @brief Constructs the engine and pre-allocates task storage.
     *
     * Reserving capacity up-front avoids repeated heap re-allocations as
     * tasks are added during normal operation.  100 is a conservative upper
     * bound for typical usage; increase it if you expect more concurrent tasks.
     */
    SimulationEngine() {
        // Pre-allocate to prevent dynamic allocation during loop
        tasks.reserve(100);
    }

    /**
     * @brief Advances every task by one fixed time-step (Config::TIME_STEP).
     *
     * When the engine is paused this function returns immediately without
     * modifying any task state.  Otherwise each task passes through all
     * four physics sub-systems in order:
     *   - ClassicalEngine::integrateRK4  — updates position, velocity, deadline
     *   - QuantumEngine::evolve          — advances the quantum wavefunction
     *   - ThermodynamicsEngine::updateEntropy — recalculates entropy from |ψ|²
     *   - ChaosEngine::update            — steps the Lorenz stress attractor
     *
     * The order matters: entropy is derived from the wavefunction, so the
     * quantum step must complete before the thermodynamics step runs.
     */
    void update() {
        // When paused, skip integration so task states remain unchanged
        if (paused) return;

        // Advance every task through the full physics pipeline for this frame.
        for (auto& task : tasks) {
            ClassicalEngine::integrateRK4(task);
            QuantumEngine::evolve(task);
            ThermodynamicsEngine::updateEntropy(task);
            ChaosEngine::update(task);
        }
    }
};