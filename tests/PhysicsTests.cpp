#include "physics/ClassicalEngine.hpp"
#include "physics/ChaosEngine.hpp"
#include <iostream>
#include <cassert>

void testRK4Stability() {
    Task task;
    task.position = { 0, 0 };
    task.velocity = { 10, 0 };
    task.mass = 1.0;

    // Run for 100 steps
    for (int i = 0; i < 100; ++i) {
        ClassicalEngine::integrateRK4(task);
    }

    // Assert no NaNs produced
    assert(!std::isnan(task.position.x));
    std::cout << "[PASS] RK4 Stability Test" << std::endl;
}

void testChaosDivergence() {
    Task t1, t2;
    t2.stressX += 0.00001; // Tiny perturbation

    for (int i = 0; i < 1000; ++i) {
        ChaosEngine::update(t1);
        ChaosEngine::update(t2);
    }

    // Lorenz attractor should diverge significantly
    assert(std::abs(t1.stressX - t2.stressX) > 0.1);
    std::cout << "[PASS] Chaos Divergence Test" << std::endl;
}

int main() {
    testRK4Stability();
    testChaosDivergence();
    return 0;
}