#include "core/SimulationEngine.hpp"
#include <chrono>
#include <thread>
#include <iostream>

int main() {
    SimulationEngine engine;

    // Seed an initial task to simulate
    Task initialTask;
    initialTask.deadlineTime = 5.0;
    initialTask.velocity = { 1.0, 0.0 };

    // Simple Hermitian Hamiltonian initialization for testing
    initialTask.hamiltonian.data[0][0] = { 1.0, 0.0 };
    initialTask.hamiltonian.data[1][1] = { 2.0, 0.0 };

    engine.tasks.push_back(initialTask);

    using clock = std::chrono::high_resolution_clock;
    const std::chrono::nanoseconds dt(static_cast<long long>(Config::TIME_STEP * 1e9));

    int frameCount = 0;
    int skippedFrames = 0;
    while (frameCount < 100) {
        auto start_time = clock::now();

        engine.update();

        // Fixed dt loop constraint with proper error handling
        auto end_time = clock::now();
        auto compute_time = end_time - start_time;

        if (compute_time < dt) {
            std::this_thread::sleep_for(dt - compute_time);
        } else {
            // Frame took too long; log warning and skip sleep
            std::cerr << "[WARNING] Frame " << frameCount 
                      << " exceeded time budget: " 
                      << std::chrono::duration<double>(compute_time).count() 
                      << "s > " 
                      << std::chrono::duration<double>(dt).count() 
                      << "s" << std::endl;
            skippedFrames++;
        }
        
        frameCount++;
    }

    if (skippedFrames > 0) {
        std::cerr << "[INFO] Total skipped frames: " << skippedFrames << " / " << frameCount << std::endl;
    }

    return 0;

    return 0;
}