#include <gtest/gtest.h>
#include "math/Complex.hpp"

// ---- Default construction ----

TEST(ComplexTest, DefaultConstructionIsZero) {
    Complex c;
    EXPECT_DOUBLE_EQ(c.real, 0.0);
    EXPECT_DOUBLE_EQ(c.imag, 0.0);
}

// ---- operator+ ----

TEST(ComplexTest, AdditionWithKnownValues) {
    Complex a{1.0, 2.0};
    Complex b{3.0, 4.0};
    Complex r = a + b;
    EXPECT_DOUBLE_EQ(r.real, 4.0);
    EXPECT_DOUBLE_EQ(r.imag, 6.0);
}

TEST(ComplexTest, AdditionWithNegativeValues) {
    Complex a{-1.0, 5.0};
    Complex b{3.0, -2.0};
    Complex r = a + b;
    EXPECT_DOUBLE_EQ(r.real, 2.0);
    EXPECT_DOUBLE_EQ(r.imag, 3.0);
}

TEST(ComplexTest, AdditionByZeroIsIdentity) {
    Complex a{3.0, -4.0};
    Complex zero{0.0, 0.0};
    Complex r = a + zero;
    EXPECT_DOUBLE_EQ(r.real, a.real);
    EXPECT_DOUBLE_EQ(r.imag, a.imag);
}

// ---- operator- ----

TEST(ComplexTest, SubtractionWithKnownValues) {
    Complex a{5.0, 7.0};
    Complex b{2.0, 3.0};
    Complex r = a - b;
    EXPECT_DOUBLE_EQ(r.real, 3.0);
    EXPECT_DOUBLE_EQ(r.imag, 4.0);
}

TEST(ComplexTest, SubtractionResultingInNegative) {
    Complex a{1.0, 2.0};
    Complex b{4.0, 6.0};
    Complex r = a - b;
    EXPECT_DOUBLE_EQ(r.real, -3.0);
    EXPECT_DOUBLE_EQ(r.imag, -4.0);
}

TEST(ComplexTest, SubtractionSelfIsZero) {
    Complex a{7.0, -3.0};
    Complex r = a - a;
    EXPECT_DOUBLE_EQ(r.real, 0.0);
    EXPECT_DOUBLE_EQ(r.imag, 0.0);
}

// ---- operator* ----

TEST(ComplexTest, MultiplicationWithKnownValues) {
    // (1+2i)(3+4i) = (3-8) + (4+6)i = -5 + 10i
    Complex a{1.0, 2.0};
    Complex b{3.0, 4.0};
    Complex r = a * b;
    EXPECT_DOUBLE_EQ(r.real, -5.0);
    EXPECT_DOUBLE_EQ(r.imag, 10.0);
}

TEST(ComplexTest, MultiplicationByOne) {
    Complex a{3.0, -4.0};
    Complex one{1.0, 0.0};
    Complex r = a * one;
    EXPECT_DOUBLE_EQ(r.real, a.real);
    EXPECT_DOUBLE_EQ(r.imag, a.imag);
}

TEST(ComplexTest, MultiplicationPureImaginarySquaredIsMinusOne) {
    // i * i = -1 + 0i
    Complex i{0.0, 1.0};
    Complex r = i * i;
    EXPECT_DOUBLE_EQ(r.real, -1.0);
    EXPECT_DOUBLE_EQ(r.imag, 0.0);
}

TEST(ComplexTest, MultiplicationByZero) {
    Complex a{5.0, 3.0};
    Complex zero{0.0, 0.0};
    Complex r = a * zero;
    EXPECT_DOUBLE_EQ(r.real, 0.0);
    EXPECT_DOUBLE_EQ(r.imag, 0.0);
}

TEST(ComplexTest, MultiplicationIsCommutative) {
    Complex a{2.0, 3.0};
    Complex b{-1.0, 4.0};
    Complex ab = a * b;
    Complex ba = b * a;
    EXPECT_DOUBLE_EQ(ab.real, ba.real);
    EXPECT_DOUBLE_EQ(ab.imag, ba.imag);
}

TEST(ComplexTest, MultiplicationRealNumbers) {
    // (2+0i)(3+0i) = 6+0i
    Complex a{2.0, 0.0};
    Complex b{3.0, 0.0};
    Complex r = a * b;
    EXPECT_DOUBLE_EQ(r.real, 6.0);
    EXPECT_DOUBLE_EQ(r.imag, 0.0);
}

// ---- magnitudeSquared ----

TEST(ComplexTest, MagnitudeSquaredKnownValues) {
    // |3 + 4i|^2 = 9 + 16 = 25
    Complex c{3.0, 4.0};
    EXPECT_DOUBLE_EQ(c.magnitudeSquared(), 25.0);
}

TEST(ComplexTest, MagnitudeSquaredRealOnly) {
    Complex c{5.0, 0.0};
    EXPECT_DOUBLE_EQ(c.magnitudeSquared(), 25.0);
}

TEST(ComplexTest, MagnitudeSquaredImaginaryOnly) {
    Complex c{0.0, 3.0};
    EXPECT_DOUBLE_EQ(c.magnitudeSquared(), 9.0);
}

TEST(ComplexTest, MagnitudeSquaredOfZeroIsZero) {
    Complex c{0.0, 0.0};
    EXPECT_DOUBLE_EQ(c.magnitudeSquared(), 0.0);
}

TEST(ComplexTest, MagnitudeSquaredWithNegativeComponents) {
    Complex c{-3.0, -4.0};
    EXPECT_DOUBLE_EQ(c.magnitudeSquared(), 25.0);
}

TEST(ComplexTest, MagnitudeSquaredOfUnitComplexIsOne) {
    Complex c{1.0, 0.0};
    EXPECT_DOUBLE_EQ(c.magnitudeSquared(), 1.0);
}

TEST(ComplexTest, MagnitudeSquaredEqualsRealSquaredPlusImagSquared) {
    Complex c{-2.0, 5.0};
    double expected = c.real * c.real + c.imag * c.imag;
    EXPECT_DOUBLE_EQ(c.magnitudeSquared(), expected);
}
