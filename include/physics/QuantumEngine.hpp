#pragma once
#include "Task.hpp"
#include "core/Config.hpp"
#include <cmath>
#include <algorithm> // Added for std::max / std::min

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
            // Multiply by the reciprocal instead of dividing each component,
            // replacing 2*N divisions with one division and 2*N multiplications.
            const double inv_norm = 1.0 / std::sqrt(normSq);
            for (auto& amp : task.psi) {
                amp.real *= inv_norm;
                amp.imag *= inv_norm;
            }
        }
    }

    // --- Added for Momentum Bridge / Blueprint ---
    static double calculateCollapseProbability(const Task& task) {
        // Probability scales with Shannon Entropy.
        // For a 4-state system, max theoretical entropy is ln(4) ≈ 1.386.
        double maxEntropy = 1.38629436;
        double prob = task.entropy / maxEntropy;
        
        // Clamp the probability securely between 0.0 (0%) and 1.0 (100%)
        return std::max(0.0, std::min(1.0, prob));
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
        
        // Project to the pure basis state
        task.psi[maxIdx] = { 1.0, 0.0 };
        
        // A pure state has 0 entropy. Reset it here so the bridge reflects the collapse.
        task.entropy = 0.0; 
    }
};