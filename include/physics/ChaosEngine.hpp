#pragma once
#include "Task.hpp"
#include "core/Config.hpp"

class ChaosEngine {
public:
    static void update(Task& task) {
        double dt = Config::TIME_STEP;
        double sX = task.stressX;
        double sY = task.stressY;
        double sZ = task.stressZ;

        auto dx = [](double x, double y) { return Config::CHAOS_SIGMA * (y - x); };
        auto dy = [](double x, double y, double z) { return x * (Config::CHAOS_RHO - z) - y; };
        auto dz = [](double x, double y, double z) { return x * y - Config::CHAOS_BETA * z; };

        // RK4 Integration for Lorenz
        double k1x = dx(sX, sY);
        double k1y = dy(sX, sY, sZ);
        double k1z = dz(sX, sY, sZ);

        double k2x = dx(sX + k1x * dt / 2.0, sY + k1y * dt / 2.0);
        double k2y = dy(sX + k1x * dt / 2.0, sY + k1y * dt / 2.0, sZ + k1z * dt / 2.0);
        double k2z = dz(sX + k1x * dt / 2.0, sY + k1y * dt / 2.0, sZ + k1z * dt / 2.0);

        double k3x = dx(sX + k2x * dt / 2.0, sY + k2y * dt / 2.0);
        double k3y = dy(sX + k2x * dt / 2.0, sY + k2y * dt / 2.0, sZ + k2z * dt / 2.0);
        double k3z = dz(sX + k2x * dt / 2.0, sY + k2y * dt / 2.0, sZ + k2z * dt / 2.0);

        double k4x = dx(sX + k3x * dt, sY + k3y * dt);
        double k4y = dy(sX + k3x * dt, sY + k3y * dt, sZ + k3z * dt);
        double k4z = dz(sX + k3x * dt, sY + k3y * dt, sZ + k3z * dt);

        task.stressX += (dt / 6.0) * (k1x + 2 * k2x + 2 * k3x + k4x);
        task.stressY += (dt / 6.0) * (k1y + 2 * k2y + 2 * k3y + k4y);
        task.stressZ += (dt / 6.0) * (k1z + 2 * k2z + 2 * k3z + k4z);
    }
};