/**
 * @file QuantumEngine.hpp
 * @brief Quantum wavefunction evolution and collapse for Task objects.
 *
 * Implements a second-order approximate Schrödinger time-evolution scheme:
 *   U ≈ I - iHΔt - ½H²Δt²
 *
 * This is the Taylor expansion of the exact unitary operator
 * U = exp(-iHΔt/ħ) truncated at second order.  With ħ = 1 and the small
 * time-steps used here (1/60 s), the approximation remains accurate enough
 * for the visualisation goals of the engine.
 *
 * Wavefunction normalisation is enforced after each evolution step to
 * prevent amplitude drift accumulating over thousands of frames.
 */
#pragma once
#include "Task.hpp"
#include "core/Config.hpp"
#include <cmath>
#include <algorithm> // Added for std::max / std::min

/**
 * @brief Stateless quantum-mechanics engine operating on Task instances.
 */
class QuantumEngine {
public:
    /**
     * @brief Advances the task's wavefunction by one time-step.
     *
     * Applies the approximate evolution operator
     *   U ≈ I - iHΔt - ½H²Δt²
     * to the current state vector task.psi, producing the new wavefunction
     * ψ(t+Δt) = U·ψ(t).
     *
     * Steps:
     *  1. Build scaled matrices: H_dt_i = H·(iΔt) and H2_half = H²·(½Δt²).
     *  2. Assemble U = I - H_dt_i - H2_half.
     *  3. Multiply U by the current psi column vector.
     *  4. Re-normalise to correct accumulated floating-point drift.
     *
     * @param task The task whose quantum state (psi, hamiltonian) is evolved
     *             in place.
     */
    static void evolve(Task& task) {
        double dt = Config::TIME_STEP;
        Matrix I = Matrix::identity();
        Matrix H = task.hamiltonian;

        // Complex factors for the two expansion terms.
        // i_dt = iΔt  (pure imaginary — shifts the phase of each state)
        // half_dt_sq = ½Δt² (real — provides the second-order correction)
        Complex i_dt = { 0.0, dt };
        Complex half_dt_sq = { 0.5 * dt * dt, 0.0 };

        // U ≈ I - iHdt - 0.5H^2 dt^2
        Matrix H_dt_i = H * i_dt;           // First-order term:  -iHΔt
        Matrix H2_half = (H * H) * half_dt_sq; // Second-order term: -½H²Δt²

        // Assemble the full evolution operator.
        Matrix U = I - H_dt_i - H2_half;

        // Apply U to the wavefunction: ψ(t+Δt) = U·ψ(t)
        task.psi = U.multiplyVector(task.psi);

        // Re-normalise so probabilities continue to sum to 1.
        normalize(task);
    }

    /**
     * @brief Ensures the wavefunction amplitudes represent valid probabilities.
     *
     * The sum of squared magnitudes Σ|ψᵢ|² must equal 1 for the
     * probabilities to be well-defined.  Numerical operations accumulate
     * small errors that grow over time, so this function rescales each
     * amplitude by 1/√(Σ|ψᵢ|²) after every evolution step.
     *
     * A reciprocal is computed once and then multiplied into both the real
     * and imaginary parts of each amplitude, replacing 2·N divisions with
     * one division and 2·N multiplications.
     *
     * @param task The task whose psi vector is normalised in place.
     */
    static void normalize(Task& task) {
        double normSq = 0.0;
        // First pass: accumulate the total probability (squared norm).
        for (const auto& amp : task.psi) {
            normSq += amp.magnitudeSquared();
        }

        if (normSq > 0.0) {
            // Multiply by the reciprocal instead of dividing each component,
            // replacing 2*N divisions with one division and 2*N multiplications.
            const double inv_norm = 1.0 / std::sqrt(normSq);
            // Second pass: rescale every amplitude to restore unit norm.
            for (auto& amp : task.psi) {
                amp.real *= inv_norm;
                amp.imag *= inv_norm;
            }
        }
    }

    /**
     * @brief Computes the probability that the task will undergo wavefunction collapse.
     *
     * The collapse probability is proportional to the task's Shannon entropy,
     * normalised by the theoretical maximum entropy of a 4-state system:
     *   ln(4) ≈ 1.386.
     *
     * A task in a pure state (all probability concentrated in one basis state)
     * has entropy ≈ 0 and collapse probability ≈ 0 (nothing to collapse).
     * A maximally mixed state has entropy = ln(4) and collapse probability = 1.
     *
     * The result is clamped to [0, 1] to guard against slight numerical
     * overshoots.
     *
     * @param task The task to evaluate.
     * @return Collapse probability in [0.0, 1.0].
     */
    // --- Added for Momentum Bridge / Blueprint ---
    static double calculateCollapseProbability(const Task& task) {
        // Probability scales with Shannon Entropy.
        // For a 4-state system, max theoretical entropy is ln(4) ≈ 1.386.
        double maxEntropy = 1.38629436;
        double prob = task.entropy / maxEntropy;
        
        // Clamp the probability securely between 0.0 (0%) and 1.0 (100%)
        return std::max(0.0, std::min(1.0, prob));
    }

    /**
     * @brief Deterministically collapses the wavefunction to its dominant state.
     *
     * Simulates a projective measurement by selecting the basis state with the
     * highest probability |ψᵢ|² as the post-measurement state.  All other
     * amplitudes are zeroed and the chosen state is set to |ψᵢ| = 1.
     *
     * After collapse:
     *  - task.psi is a pure basis state (one component = 1, rest = 0).
     *  - task.entropy is reset to 0 because a pure state has zero entropy.
     *
     * Note: This is a deterministic (rather than random) implementation;
     * the highest-probability state is always chosen.  A probabilistic
     * measurement would require a random-number generator.
     *
     * @param task The task whose wavefunction is collapsed in place.
     */
    static void collapse(Task& task) {
        // Deterministic collapse implies projecting onto highest probability state for Phase 1
        int maxIdx = 0;
        double maxProb = -1.0;

        // Find the basis state with the largest probability.
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            double p = task.psi[i].magnitudeSquared();
            if (p > maxProb) {
                maxProb = p;
                maxIdx = i;
            }
            // Zero every state; the winner is restored below.
            task.psi[i] = { 0.0, 0.0 };
        }
        
        // Project to the pure basis state
        task.psi[maxIdx] = { 1.0, 0.0 };
        
        // A pure state has 0 entropy. Reset it here so the bridge reflects the collapse.
        task.entropy = 0.0; 
    }
};