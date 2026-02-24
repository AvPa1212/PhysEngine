#pragma once
#include <cmath>

struct Complex {
    double real = 0.0;
    double imag = 0.0;

    inline Complex operator+(const Complex& other) const {
        return { real + other.real, imag + other.imag };
    }
    inline Complex operator-(const Complex& other) const {
        return { real - other.real, imag - other.imag };
    }
    inline Complex operator*(const Complex& other) const {
        return { real * other.real - imag * other.imag,
                real * other.imag + imag * other.real };
    }
    inline double magnitudeSquared() const {
        return real * real + imag * imag;
    }
};