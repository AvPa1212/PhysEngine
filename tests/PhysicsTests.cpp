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
    // Resetting states to a known sensitive region
    t1.stressX = 1.0; t1.stressY = 1.0; t1.stressZ = 1.0;
    t2 = t1; 
    
    t2.stressX += 0.00001; // Tiny perturbation

    // Increase steps to 3000 to allow exponential divergence to hit the threshold
    for (int i = 0; i < 3000; ++i) {
        ChaosEngine::update(t1);
        ChaosEngine::update(t2);
    }

    double delta = std::abs(t1.stressX - t2.stressX);
    std::cout << "Final Delta: " << delta << std::endl;
    
    assert(delta > 0.1);
    std::cout << "[PASS] Chaos Divergence Test" << std::endl;
}

int main() {
    testRK4Stability();
    testChaosDivergence();
    return 0;
}