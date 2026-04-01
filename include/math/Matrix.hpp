/**
 * @file Matrix.hpp
 * @brief Square complex matrix for quantum Hamiltonian operations.
 *
 * The matrix dimension is fixed at compile time via Config::QUANTUM_DIM
 * (currently 4), which allows the data to live on the stack and avoids
 * heap allocations in the inner simulation loop.
 *
 * The primary use-case is building and applying the approximate
 * time-evolution operator U ≈ I - iHΔt - ½H²Δt² to the wavefunction
 * stored in Task::psi each simulation step.
 */
#pragma once
#include "Complex.hpp"
#include "core/Config.hpp"
#include <array>

/**
 * @brief Fixed-size QUANTUM_DIM × QUANTUM_DIM matrix of Complex entries.
 *
 * All elements are zero-initialised by the default constructor, so a
 * freshly constructed Matrix represents the zero matrix.  Use
 * Matrix::identity() to obtain the identity matrix.
 */
class Matrix {
public:
    /// 2-D array of Complex entries; row-major indexing: data[row][col].
    Complex data[Config::QUANTUM_DIM][Config::QUANTUM_DIM] = {};

    /**
     * @brief Constructs the multiplicative identity matrix.
     *
     * Sets the diagonal entries to (1, 0) and leaves all off-diagonal
     * entries at (0, 0).  This is used as the starting point when
     * building the time-evolution operator U.
     *
     * @return A new Matrix with 1s on the main diagonal and 0s elsewhere.
     */
    static Matrix identity() {
        Matrix mat;
        // Walk the diagonal and set each diagonal element to the real unit 1.
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            mat.data[i][i] = { 1.0, 0.0 };
        }
        return mat;
    }

    /**
     * @brief Matrix–matrix product (standard linear-algebra definition).
     *
     * Each output entry (i,j) is the dot product of row i of *this with
     * column j of @p other:
     *   result[i][j] = Σₖ data[i][k] * other.data[k][j]
     *
     * Used to compute H² when building the second-order expansion of U.
     *
     * @param other The right-hand matrix operand.
     * @return Product matrix (QUANTUM_DIM × QUANTUM_DIM).
     */
    inline Matrix operator*(const Matrix& other) const {
        Matrix result;
        // Outer loop: iterate over each row of the result.
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            // Middle loop: iterate over each column of the result.
            for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
                Complex sum = { 0.0, 0.0 };
                // Inner loop: accumulate the dot product for result[i][j].
                for (int k = 0; k < Config::QUANTUM_DIM; ++k) {
                    sum = sum + (data[i][k] * other.data[k][j]);
                }
                result.data[i][j] = sum;
            }
        }
        return result;
    }

    /**
     * @brief Scalar multiplication by a complex number.
     *
     * Each entry of the returned matrix equals data[i][j] * @p scalar.
     * This is used to scale H by the complex factor iΔt or by ½Δt²
     * when computing the terms of the evolution operator U.
     *
     * @param scalar Complex scale factor.
     * @return A new matrix with every entry multiplied by @p scalar.
     */
    inline Matrix operator*(const Complex& scalar) const {
        Matrix result;
        // Iterate over every element and scale it individually.
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
                result.data[i][j] = data[i][j] * scalar;
            }
        }
        return result;
    }

    /**
     * @brief Component-wise matrix subtraction.
     *
     * result[i][j] = data[i][j] - other.data[i][j] for all (i, j).
     * Used to assemble U = I - H_dt_i - H2_half.
     *
     * @param other The matrix to subtract from *this.
     * @return Element-wise difference matrix.
     */
    inline Matrix operator-(const Matrix& other) const {
        Matrix result;
        // Iterate over every element and subtract element-wise.
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
                result.data[i][j] = data[i][j] - other.data[i][j];
            }
        }
        return result;
    }

    /**
     * @brief Matrix–vector product: result[i] = Σⱼ data[i][j] * vec[j].
     *
     * Applies this matrix to a column vector of complex amplitudes.
     * This is the core operation that advances the wavefunction by one
     * time step: ψ(t+Δt) = U · ψ(t).
     *
     * @param vec Input column vector of QUANTUM_DIM complex amplitudes.
     * @return Transformed output vector of QUANTUM_DIM complex values.
     */
    inline std::array<Complex, Config::QUANTUM_DIM> multiplyVector(const std::array<Complex, Config::QUANTUM_DIM>& vec) const {
        std::array<Complex, Config::QUANTUM_DIM> result = {};
        // Outer loop: compute one output amplitude per basis state.
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            Complex sum = { 0.0, 0.0 };
            // Inner loop: sum contributions from all input amplitudes.
            for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
                sum = sum + (data[i][j] * vec[j]);
            }
            result[i] = sum;
        }
        return result;
    }
};