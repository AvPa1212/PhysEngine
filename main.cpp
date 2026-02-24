#include "core/SimulationEngine.hpp"
#include <chrono>
#include <thread>

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
    while (frameCount < 100) {
        engine.update();
        frameCount++;
    }

    //// Print final state of the first task
    //auto& t = engine.tasks[0];
    //printf("Frame 100 Validation:\n");
    //printf("Chaos X: %f\n", t.stressX);
    //printf("Chaos Y: %f\n", t.stressY);
    //printf("Entropy: %f\n", t.entropy);


    while (true) {
        auto start_time = clock::now();

        engine.update();

        // Fixed dt loop constraint
        auto end_time = clock::now();
        auto compute_time = end_time - start_time;

        if (compute_time < dt) {
            std::this_thread::sleep_for(dt - compute_time);
        }
    }

    return 0;
}