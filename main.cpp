/**
 * @file main.cpp
 * @brief Desktop entry point for the PhysEngine simulation.
 *
 * Creates a SimulationEngine, seeds it with a single test task, and then runs
 * a fixed-timestep loop for 100 frames. This executable is primarily used for
 * development and integration testing on the native platform; the production
 * physics engine is compiled to WebAssembly via Emscripten and consumed by the
 * React front-end (`momentum-ui`).
 *
 * Build: mkdir build && cd build && cmake .. && make
 * Run:   ./PhysEngine
 */
#include "core/SimulationEngine.hpp"

#include <chrono>
#include <iostream>
#include <thread>

namespace {
    Task createSeedTask() {
        Task task;
        task.deadlineTime = 5.0;
        task.velocity = {1.0, 0.0};

        // Only the diagonal elements are set; off-diagonal coupling is zero.
        task.hamiltonian.data[0][0] = {1.0, 0.0};
        task.hamiltonian.data[1][1] = {2.0, 0.0};
        return task;
    }
}

int main() {
    SimulationEngine engine;
    engine.tasks.push_back(createSeedTask());

    using clock = std::chrono::high_resolution_clock;
    const std::chrono::nanoseconds dt(static_cast<long long>(Config::TIME_STEP * 1e9));

    int frameCount = 0;
    int skippedFrames = 0;

    while (frameCount < 100) {
        auto startTime = clock::now();
        engine.update();

        auto computeTime = clock::now() - startTime;
        if (computeTime < dt) {
            std::this_thread::sleep_for(dt - computeTime);
        } else {
            std::cerr << "[WARNING] Frame " << frameCount
                      << " exceeded time budget: "
                      << std::chrono::duration<double>(computeTime).count()
                      << "s > "
                      << std::chrono::duration<double>(dt).count()
                      << "s" << std::endl;
            skippedFrames += 1;
        }

        frameCount += 1;
    }

    if (skippedFrames > 0) {
        std::cerr << "[INFO] Total skipped frames: " << skippedFrames << " / " << frameCount << std::endl;
    }

    return 0;
}
