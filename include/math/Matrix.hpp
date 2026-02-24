#pragma once
#include "Complex.hpp"
#include "core/Config.hpp"
#include <array>

class Matrix {
public:
    Complex data[Config::QUANTUM_DIM][Config::QUANTUM_DIM] = {};

    static Matrix identity() {
        Matrix mat;
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            mat.data[i][i] = { 1.0, 0.0 };
        }
        return mat;
    }

    inline Matrix operator*(const Matrix& other) const {
        Matrix result;
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
                Complex sum = { 0.0, 0.0 };
                for (int k = 0; k < Config::QUANTUM_DIM; ++k) {
                    sum = sum + (data[i][k] * other.data[k][j]);
                }
                result.data[i][j] = sum;
            }
        }
        return result;
    }

    inline Matrix operator*(const Complex& scalar) const {
        Matrix result;
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
                result.data[i][j] = data[i][j] * scalar;
            }
        }
        return result;
    }

    inline Matrix operator-(const Matrix& other) const {
        Matrix result;
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
                result.data[i][j] = data[i][j] - other.data[i][j];
            }
        }
        return result;
    }

    inline std::array<Complex, Config::QUANTUM_DIM> multiplyVector(const std::array<Complex, Config::QUANTUM_DIM>& vec) const {
        std::array<Complex, Config::QUANTUM_DIM> result = {};
        for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
            Complex sum = { 0.0, 0.0 };
            for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
                sum = sum + (data[i][j] * vec[j]);
            }
            result[i] = sum;
        }
        return result;
    }
};