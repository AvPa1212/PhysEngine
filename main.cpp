/**
 * @file main.cpp
 * @brief Desktop entry point for the PhysEngine simulation.
 *
 * Creates a SimulationEngine, seeds it with a single test Task, and then runs
 * a fixed-timestep loop for 100 frames.  This executable is primarily used
 * for development and integration testing on the native platform; the
 * production physics engine is compiled to WebAssembly via Emscripten and
 * consumed by the React front-end (momentum-ui).
 *
 * Build: mkdir build && cd build && cmake .. && make
 * Run:   ./PhysEngine   (from the build directory)
 */
#include "core/SimulationEngine.hpp"
#include <chrono>
#include <thread>
#include <iostream>

int main() {
    SimulationEngine engine;

    // Seed an initial task to simulate
    Task initialTask;
    initialTask.deadlineTime = 5.0;   // 5-second deadline before urgency ramps up.
    initialTask.velocity = { 1.0, 0.0 }; // Start moving right at 1 m/s.

    // Simple Hermitian Hamiltonian initialization for testing
    // Only the diagonal elements are set; off-diagonal coupling is zero.
    // Diagonal energy levels: E₀ = 1.0, E₁ = 2.0 (other states remain 0).
    initialTask.hamiltonian.data[0][0] = { 1.0, 0.0 };
    initialTask.hamiltonian.data[1][1] = { 2.0, 0.0 };

    engine.tasks.push_back(initialTask);

    // Use the high-resolution clock to enforce the fixed time-step budget.
    using clock = std::chrono::high_resolution_clock;
    // Convert the floating-point time-step (seconds) to nanoseconds for the
    // sleep_for call, which expects a std::chrono duration.
    const std::chrono::nanoseconds dt(static_cast<long long>(Config::TIME_STEP * 1e9));

    int frameCount = 0;    // Total frames attempted.
    int skippedFrames = 0; // Frames whose compute time exceeded the budget.

    // Main simulation loop — runs exactly 100 frames then exits.
    while (frameCount < 100) {
        auto start_time = clock::now();

        // Advance every task through the full physics pipeline for this frame.
        engine.update();

        // Fixed dt loop constraint with proper error handling
        auto end_time = clock::now();
        auto compute_time = end_time - start_time;

        if (compute_time < dt) {
            // Sleep for the remainder of the frame budget to maintain pacing.
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

    // Report how many frames missed their deadline over the full run.
    if (skippedFrames > 0) {
        std::cerr << "[INFO] Total skipped frames: " << skippedFrames << " / " << frameCount << std::endl;
    }

    return 0;

    return 0;
}