#include <gtest/gtest.h>
#include "math/Vector2.hpp"

// ---- operator+ ----

TEST(Vector2Test, AdditionWithKnownValues) {
    Vector2 a{1.0, 2.0};
    Vector2 b{3.0, 4.0};
    Vector2 result = a + b;
    EXPECT_DOUBLE_EQ(result.x, 4.0);
    EXPECT_DOUBLE_EQ(result.y, 6.0);
}

TEST(Vector2Test, AdditionWithNegativeValues) {
    Vector2 a{-1.0, 5.0};
    Vector2 b{3.0, -2.0};
    Vector2 result = a + b;
    EXPECT_DOUBLE_EQ(result.x, 2.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
}

// ---- operator- ----

TEST(Vector2Test, SubtractionWithKnownValues) {
    Vector2 a{5.0, 7.0};
    Vector2 b{2.0, 3.0};
    Vector2 result = a - b;
    EXPECT_DOUBLE_EQ(result.x, 3.0);
    EXPECT_DOUBLE_EQ(result.y, 4.0);
}

TEST(Vector2Test, SubtractionResultingInNegative) {
    Vector2 a{1.0, 2.0};
    Vector2 b{4.0, 6.0};
    Vector2 result = a - b;
    EXPECT_DOUBLE_EQ(result.x, -3.0);
    EXPECT_DOUBLE_EQ(result.y, -4.0);
}

// ---- operator* (scalar) ----

TEST(Vector2Test, ScalarMultiplicationWithKnownValues) {
    Vector2 v{3.0, 4.0};
    Vector2 result = v * 2.0;
    EXPECT_DOUBLE_EQ(result.x, 6.0);
    EXPECT_DOUBLE_EQ(result.y, 8.0);
}

TEST(Vector2Test, ScalarMultiplicationByZero) {
    Vector2 v{3.0, 4.0};
    Vector2 result = v * 0.0;
    EXPECT_DOUBLE_EQ(result.x, 0.0);
    EXPECT_DOUBLE_EQ(result.y, 0.0);
}

TEST(Vector2Test, ScalarMultiplicationByNegative) {
    Vector2 v{2.0, -3.0};
    Vector2 result = v * -1.0;
    EXPECT_DOUBLE_EQ(result.x, -2.0);
    EXPECT_DOUBLE_EQ(result.y, 3.0);
}

// ---- operator+= ----

TEST(Vector2Test, AddAssignWithKnownValues) {
    Vector2 a{1.0, 2.0};
    Vector2 b{3.0, 4.0};
    a += b;
    EXPECT_DOUBLE_EQ(a.x, 4.0);
    EXPECT_DOUBLE_EQ(a.y, 6.0);
}

TEST(Vector2Test, AddAssignAccumulatesCorrectly) {
    Vector2 a{0.0, 0.0};
    Vector2 step{1.5, 2.5};
    a += step;
    a += step;
    EXPECT_DOUBLE_EQ(a.x, 3.0);
    EXPECT_DOUBLE_EQ(a.y, 5.0);
}

// ---- dot() ----

TEST(Vector2Test, DotProductWithKnownValues) {
    Vector2 a{1.0, 2.0};
    Vector2 b{3.0, 4.0};
    // 1*3 + 2*4 = 11
    EXPECT_DOUBLE_EQ(dot(a, b), 11.0);
}

TEST(Vector2Test, DotProductOfPerpendicularVectorsIsZero) {
    Vector2 a{1.0, 0.0};
    Vector2 b{0.0, 1.0};
    EXPECT_DOUBLE_EQ(dot(a, b), 0.0);
}

TEST(Vector2Test, DotProductOfParallelVectors) {
    Vector2 a{3.0, 4.0};
    Vector2 b{3.0, 4.0};
    // 9 + 16 = 25
    EXPECT_DOUBLE_EQ(dot(a, b), 25.0);
}

// ---- magnitude() ----

TEST(Vector2Test, MagnitudeOf3_4Is5) {
    Vector2 v{3.0, 4.0};
    EXPECT_NEAR(magnitude(v), 5.0, 1e-10);
}

TEST(Vector2Test, MagnitudeOfUnitVectorIsOne) {
    Vector2 v{1.0, 0.0};
    EXPECT_NEAR(magnitude(v), 1.0, 1e-10);
}

TEST(Vector2Test, MagnitudeOfZeroVectorIsZero) {
    Vector2 v{0.0, 0.0};
    EXPECT_NEAR(magnitude(v), 0.0, 1e-10);
}

TEST(Vector2Test, MagnitudeWithNegativeComponents) {
    Vector2 v{-3.0, -4.0};
    EXPECT_NEAR(magnitude(v), 5.0, 1e-10);
}

// ---- normalize() ----

TEST(Vector2Test, NormalizeProducesUnitVector) {
    Vector2 v{3.0, 4.0};
    Vector2 n = normalize(v);
    EXPECT_NEAR(n.x, 0.6, 1e-10);
    EXPECT_NEAR(n.y, 0.8, 1e-10);
    // Verify magnitude of result is 1
    EXPECT_NEAR(magnitude(n), 1.0, 1e-10);
}

TEST(Vector2Test, NormalizeAxisAlignedVector) {
    Vector2 v{0.0, 5.0};
    Vector2 n = normalize(v);
    EXPECT_NEAR(n.x, 0.0, 1e-10);
    EXPECT_NEAR(n.y, 1.0, 1e-10);
}

TEST(Vector2Test, NormalizeZeroVectorReturnsZero) {
    // Edge case: zero vector should return (0, 0) without crashing
    Vector2 v{0.0, 0.0};
    Vector2 n = normalize(v);
    EXPECT_DOUBLE_EQ(n.x, 0.0);
    EXPECT_DOUBLE_EQ(n.y, 0.0);
}

TEST(Vector2Test, NormalizeNegativeVector) {
    Vector2 v{-3.0, -4.0};
    Vector2 n = normalize(v);
    EXPECT_NEAR(n.x, -0.6, 1e-10);
    EXPECT_NEAR(n.y, -0.8, 1e-10);
    EXPECT_NEAR(magnitude(n), 1.0, 1e-10);
}
