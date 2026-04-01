/**
 * @file ThermodynamicsEngine.hpp
 * @brief Computes thermodynamic entropy from the quantum wavefunction.
 *
 * The entropy here is the quantum Shannon (von Neumann-like) entropy derived
 * directly from the probability distribution defined by the task's wavefunction
 * amplitudes:
 *   S = -k Σᵢ pᵢ ln(pᵢ)   where pᵢ = |ψᵢ|²
 *
 * With Config::ENTROPY_CONSTANT = 1, entropy is reported in nats.  A pure
 * state gives S = 0; a maximally mixed 4-state system gives S = ln(4) ≈ 1.386.
 *
 * Note: ChaosEngine also increments task.entropy based on Lorenz trajectory
 * movement, making the total entropy a combination of quantum and classical
 * contributions.
 */
#pragma once
#include "Task.hpp"
#include "core/Config.hpp"
#include <cmath>

/**
 * @brief Stateless thermodynamics engine that recalculates task entropy each frame.
 */
class ThermodynamicsEngine {
public:
    /**
     * @brief Recalculates and stores the Shannon entropy of the task's quantum state.
     *
     * The entropy formula S = -Σ pᵢ ln(pᵢ) is evaluated over the probability
     * distribution pᵢ = |ψᵢ|² derived from the wavefunction.  The sum naturally
     * skips basis states with negligible probability (p ≤ 1e-12) to avoid
     * evaluating ln(0) which would produce −∞.
     *
     * The result is written into task.entropy, overwriting whatever value was
     * there.  ChaosEngine::update() will then add its own contribution on top
     * of this base value later in the same frame.
     *
     * @param task The task whose entropy is recalculated in place.
     */
    static void updateEntropy(Task& task) {
        double S = 0.0;
        // Iterate over all basis states and accumulate the entropy sum.
        for (const auto& amp : task.psi) {
            double p = amp.magnitudeSquared(); // p = |ψᵢ|²
            if (p > 1e-12) {
                // Contribution: -k·p·ln(p)  (skipped if p ≈ 0 to avoid ln(0))
                S -= Config::ENTROPY_CONSTANT * p * std::log(p);
            }
        }
        task.entropy = S;
    }
};