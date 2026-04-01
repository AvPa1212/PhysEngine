/**
 * @file Vector2.hpp
 * @brief 2-D Euclidean vector used throughout the physics engine.
 *
 * Provides a plain-old-data struct plus a small set of free functions that
 * cover the arithmetic needed for classical mechanics (force accumulation,
 * velocity integration, friction direction, etc.).  All operations are
 * inlined so the compiler can eliminate function-call overhead in
 * tight simulation loops.
 */
#pragma once
#include <cmath>

/**
 * @brief Two-dimensional vector with double-precision components.
 *
 * Represents a point or direction in 2-D space.  The struct is
 * value-initialised to (0, 0) by default so that Task members are
 * always in a known state before the first simulation step.
 */
struct Vector2 {
    double x = 0.0; ///< Horizontal component (positive → right).
    double y = 0.0; ///< Vertical   component (positive → up).

    /**
     * @brief Component-wise addition.
     * @param other The vector to add to this one.
     * @return A new Vector2 whose components are the element-wise sums.
     */
    inline Vector2 operator+(const Vector2& other) const {
        return { x + other.x, y + other.y };
    }

    /**
     * @brief Component-wise subtraction.
     * @param other The vector to subtract from this one.
     * @return A new Vector2 whose components are the element-wise differences.
     */
    inline Vector2 operator-(const Vector2& other) const {
        return { x - other.x, y - other.y };
    }

    /**
     * @brief Scalar multiplication (vector × scalar).
     * @param scalar The real-valued scale factor.
     * @return A new Vector2 with each component multiplied by @p scalar.
     */
    inline Vector2 operator*(double scalar) const {
        return { x * scalar, y * scalar };
    }

    /**
     * @brief In-place component-wise addition.
     * @param other The vector to add into this one.
     * @return Reference to *this after the update.
     */
    inline Vector2& operator+=(const Vector2& other) {
        x += other.x; y += other.y; return *this;
    }
};

/**
 * @brief Computes the dot product of two vectors.
 *
 * The dot product is defined as a·b = aₓbₓ + a_yb_y.  It equals
 * |a||b|cos(θ), where θ is the angle between the vectors, making it
 * useful for projecting one vector onto another and for collision
 * response calculations.
 *
 * @param a First vector.
 * @param b Second vector.
 * @return The scalar dot product.
 */
inline double dot(const Vector2& a, const Vector2& b) {
    return a.x * b.x + a.y * b.y;
}

/**
 * @brief Computes the Euclidean (L2) length of a vector.
 *
 * magnitude(v) = √(vₓ² + v_y²).  Used both directly and as a
 * denominator in normalise() and friction calculations.
 *
 * @param v The vector whose length is required.
 * @return Non-negative scalar length.
 */
inline double magnitude(const Vector2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

/**
 * @brief Returns a unit-length vector in the same direction as @p v.
 *
 * If @p v is the zero vector (magnitude == 0) the function returns
 * {0, 0} rather than producing a divide-by-zero.  This safe-guard is
 * important for friction force calculations, which call normalise on
 * the current velocity; a perfectly stationary object has zero velocity
 * and must therefore produce zero friction.
 *
 * @param v The vector to normalise.
 * @return Unit vector parallel to @p v, or {0, 0} if @p v is zero.
 */
inline Vector2 normalize(const Vector2& v) {
    double mag = magnitude(v);
    if (mag == 0.0) return { 0.0, 0.0 };
    return { v.x / mag, v.y / mag };
}