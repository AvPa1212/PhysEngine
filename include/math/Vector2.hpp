#pragma once
#include <cmath>

struct Vector2 {
    double x = 0.0;
    double y = 0.0;

    inline Vector2 operator+(const Vector2& other) const {
        return { x + other.x, y + other.y };
    }
    inline Vector2 operator-(const Vector2& other) const {
        return { x - other.x, y - other.y };
    }
    inline Vector2 operator*(double scalar) const {
        return { x * scalar, y * scalar };
    }
    inline Vector2& operator+=(const Vector2& other) {
        x += other.x; y += other.y; return *this;
    }
};

inline double dot(const Vector2& a, const Vector2& b) {
    return a.x * b.x + a.y * b.y;
}

inline double magnitude(const Vector2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline Vector2 normalize(const Vector2& v) {
    double mag = magnitude(v);
    if (mag == 0.0) return { 0.0, 0.0 };
    return { v.x / mag, v.y / mag };
}