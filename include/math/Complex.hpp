/**
 * @file Complex.hpp
 * @brief Lightweight complex-number type used for quantum state amplitudes.
 *
 * The standard library's std::complex<double> is not used here so that
 * the engine can control memory layout and keep the type trivially
 * copyable for WASM/JS interop without pulling in additional headers.
 */
#pragma once
#include <cmath>

/**
 * @brief A complex number with double-precision real and imaginary parts.
 *
 * Used to represent quantum probability amplitudes (ψᵢ) stored in the
 * Task::psi array and entries of the Hamiltonian matrix.  All operators
 * follow standard complex-number arithmetic rules.
 */
struct Complex {
    double real = 0.0; ///< Real part of the complex number.
    double imag = 0.0; ///< Imaginary part of the complex number.

    /**
     * @brief Component-wise addition: (a+bi) + (c+di) = (a+c) + (b+d)i.
     * @param other Right-hand operand.
     * @return Sum as a new Complex.
     */
    inline Complex operator+(const Complex& other) const {
        return { real + other.real, imag + other.imag };
    }

    /**
     * @brief Component-wise subtraction: (a+bi) - (c+di) = (a-c) + (b-d)i.
     * @param other Right-hand operand.
     * @return Difference as a new Complex.
     */
    inline Complex operator-(const Complex& other) const {
        return { real - other.real, imag - other.imag };
    }

    /**
     * @brief Complex multiplication: (a+bi)(c+di) = (ac-bd) + (ad+bc)i.
     *
     * This is the standard rule derived from the distributive law and
     * the identity i² = -1.  It is used when applying the time-evolution
     * operator U to the wavefunction.
     *
     * @param other Right-hand operand.
     * @return Product as a new Complex.
     */
    inline Complex operator*(const Complex& other) const {
        return { real * other.real - imag * other.imag,
                real * other.imag + imag * other.real };
    }

    /**
     * @brief Returns the squared magnitude (modulus²) |z|² = a² + b².
     *
     * Squaring avoids a sqrt() call.  In quantum mechanics |ψᵢ|² is
     * the probability of finding the system in state i, so this method
     * is called frequently during entropy calculations and wavefunction
     * normalisation.
     *
     * @return Non-negative real value equal to real² + imag².
     */
    inline double magnitudeSquared() const {
        return real * real + imag * imag;
    }
};