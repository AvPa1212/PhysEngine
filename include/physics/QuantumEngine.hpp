#pragma once
#include "Task.hpp"
#include "core/Config.hpp"
#include <cmath>

class QuantumEngine {
public:
    static void evolve(Task& task) {
        double dt = Config::TIME_STEP;
        Matrix I = Matrix::identity();
        Matrix H = task.hamiltonian;

        Complex i_dt = { 0.0, dt };
        Complex half_dt_sq = { 0.5 * dt * dt, 0.0 };

        // U ≈ I - iHdt - 0.5H^2 dt^2
        Matrix H_dt_i = H * i_dt;
        Matrix H2_half = (H * H) * half_dt_sq;

        Matrix U = I - H_dt_i - H2_half;

        task.psi = U.multiplyVector(task.psi);
        normalize(task);
    }

    static void normalize(Task& task) {
        double normSq = 0.0;
        for (const auto& amp : task.psi) {
            normSq += amp.magnitudeSquared();
        }

        if (normSq > 0.0) {
            double norm = std::sqrt(normSq);
            for (auto& amp : task.psi) {
                amp.real /= norm;
                amp.imag /= norm;
            }
        }
    }

    static void collapse(Task& task) {
        // Deterministic collapse implies projecting onto highest probability state for Phase 1
        int maxIdx = 0;
        double maxProb = -1.0;

        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            double p = task.psi[i].magnitudeSquared();
            if (p > maxProb) {
                maxProb = p;
                maxIdx = i;
            }
            task.psi[i] = { 0.0, 0.0 };
        }
        task.psi[maxIdx] = { 1.0, 0.0 };
    }
};