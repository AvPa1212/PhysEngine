#pragma once
#include "Task.hpp"
#include "core/Config.hpp"
#include <cmath>

class ThermodynamicsEngine {
public:
    static void updateEntropy(Task& task) {
        double S = 0.0;
        for (const auto& amp : task.psi) {
            double p = amp.magnitudeSquared();
            if (p > 1e-12) {
                S -= Config::ENTROPY_CONSTANT * p * std::log(p);
            }
        }
        task.entropy = S;
    }
};