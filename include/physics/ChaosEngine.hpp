#pragma once
#include "Task.hpp"
#include "core/Config.hpp"
#include <cmath> // Added for std::abs

class ChaosEngine {
public:
    static void update(Task& task) {
        const double dt = Config::TIME_STEP;
        const double dt_half = dt * 0.5;
        const double dt_sixth = dt / 6.0;
        const double sX = task.stressX;
        const double sY = task.stressY;
        const double sZ = task.stressZ;

        auto dx = [](double x, double y) { return Config::CHAOS_SIGMA * (y - x); };
        auto dy = [](double x, double y, double z) { return x * (Config::CHAOS_RHO - z) - y; };
        auto dz = [](double x, double y, double z) { return x * y - Config::CHAOS_BETA * z; };

        // RK4 Integration for Lorenz
        const double k1x = dx(sX, sY);
        const double k1y = dy(sX, sY, sZ);
        const double k1z = dz(sX, sY, sZ);

        // Precompute k2 midpoint coordinates once (reused for all three k2 evaluations)
        const double m1x = sX + k1x * dt_half;
        const double m1y = sY + k1y * dt_half;
        const double m1z = sZ + k1z * dt_half;
        const double k2x = dx(m1x, m1y);
        const double k2y = dy(m1x, m1y, m1z);
        const double k2z = dz(m1x, m1y, m1z);

        // Precompute k3 midpoint coordinates once (reused for all three k3 evaluations)
        const double m2x = sX + k2x * dt_half;
        const double m2y = sY + k2y * dt_half;
        const double m2z = sZ + k2z * dt_half;
        const double k3x = dx(m2x, m2y);
        const double k3y = dy(m2x, m2y, m2z);
        const double k3z = dz(m2x, m2y, m2z);

        // Precompute k4 endpoint coordinates once (reused for all three k4 evaluations)
        const double m3x = sX + k3x * dt;
        const double m3y = sY + k3y * dt;
        const double m3z = sZ + k3z * dt;
        const double k4x = dx(m3x, m3y);
        const double k4y = dy(m3x, m3y, m3z);
        const double k4z = dz(m3x, m3y, m3z);

        // Final weighted sums
        const double deltaX = dt_sixth * (k1x + 2.0 * k2x + 2.0 * k3x + k4x);
        const double deltaY = dt_sixth * (k1y + 2.0 * k2y + 2.0 * k3y + k4y);
        const double deltaZ = dt_sixth * (k1z + 2.0 * k2z + 2.0 * k3z + k4z);

        task.stressX += deltaX;
        task.stressY += deltaY;
        task.stressZ += deltaZ;

        // --- ENTROPY LOGIC ---
        // Accumulate entropy based on the total movement in phase space.
        // We use a small multiplier (0.05) so it doesn't collapse too instantly.
        task.entropy += (std::abs(deltaX) + std::abs(deltaY) + std::abs(deltaZ)) * 0.001;
    }
};