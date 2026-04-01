#include <gtest/gtest.h>
#include "math/Matrix.hpp"
#include "core/Config.hpp"

// ---- identity() ----

TEST(MatrixTest, IdentityHasOneOnDiagonal) {
    Matrix m = Matrix::identity();
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        EXPECT_DOUBLE_EQ(m.data[i][i].real, 1.0);
        EXPECT_DOUBLE_EQ(m.data[i][i].imag, 0.0);
    }
}

TEST(MatrixTest, IdentityHasZeroOffDiagonal) {
    Matrix m = Matrix::identity();
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            if (i != j) {
                EXPECT_DOUBLE_EQ(m.data[i][j].real, 0.0);
                EXPECT_DOUBLE_EQ(m.data[i][j].imag, 0.0);
            }
        }
    }
}

// ---- operator* (matrix × matrix) ----

TEST(MatrixTest, IdentityMultiplicationPreservesMatrix) {
    Matrix I = Matrix::identity();
    Matrix A;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            A.data[i][j] = { static_cast<double>(i + j + 1), 0.0 };
        }
    }
    Matrix result = I * A;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            EXPECT_NEAR(result.data[i][j].real, A.data[i][j].real, 1e-12);
            EXPECT_NEAR(result.data[i][j].imag, A.data[i][j].imag, 1e-12);
        }
    }
}

TEST(MatrixTest, ZeroMatrixMultiplicationYieldsZero) {
    Matrix zero; // default initialised to all zeros
    Matrix I = Matrix::identity();
    Matrix result = zero * I;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            EXPECT_DOUBLE_EQ(result.data[i][j].real, 0.0);
            EXPECT_DOUBLE_EQ(result.data[i][j].imag, 0.0);
        }
    }
}

TEST(MatrixTest, DiagonalMatrixMultiplicationProductCorrect) {
    // A = diag(2,2,2,2), B = diag(3,3,3,3)  →  A*B = diag(6,6,6,6)
    Matrix A, B;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        A.data[i][i] = {2.0, 0.0};
        B.data[i][i] = {3.0, 0.0};
    }
    Matrix result = A * B;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        EXPECT_NEAR(result.data[i][i].real, 6.0, 1e-12);
        EXPECT_NEAR(result.data[i][i].imag, 0.0, 1e-12);
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            if (i != j) {
                EXPECT_NEAR(result.data[i][j].real, 0.0, 1e-12);
                EXPECT_NEAR(result.data[i][j].imag, 0.0, 1e-12);
            }
        }
    }
}

TEST(MatrixTest, IdentitySquaredIsIdentity) {
    Matrix I = Matrix::identity();
    Matrix result = I * I;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            double expected = (i == j) ? 1.0 : 0.0;
            EXPECT_NEAR(result.data[i][j].real, expected, 1e-12);
            EXPECT_NEAR(result.data[i][j].imag, 0.0, 1e-12);
        }
    }
}

// ---- operator* (matrix × Complex scalar) ----

TEST(MatrixTest, ScalarMultiplicationByZeroYieldsZero) {
    Matrix I = Matrix::identity();
    Matrix result = I * Complex{0.0, 0.0};
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            EXPECT_DOUBLE_EQ(result.data[i][j].real, 0.0);
            EXPECT_DOUBLE_EQ(result.data[i][j].imag, 0.0);
        }
    }
}

TEST(MatrixTest, ScalarMultiplicationByRealTwoScalesAllEntries) {
    Matrix I = Matrix::identity();
    Matrix result = I * Complex{2.0, 0.0};
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        EXPECT_NEAR(result.data[i][i].real, 2.0, 1e-12);
        EXPECT_NEAR(result.data[i][i].imag, 0.0, 1e-12);
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            if (i != j) {
                EXPECT_DOUBLE_EQ(result.data[i][j].real, 0.0);
                EXPECT_DOUBLE_EQ(result.data[i][j].imag, 0.0);
            }
        }
    }
}

TEST(MatrixTest, ScalarMultiplicationByImaginaryUnit) {
    // I * i: diagonal entries (1+0i)*(0+1i) = (0+1i)
    Matrix I = Matrix::identity();
    Matrix result = I * Complex{0.0, 1.0};
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        EXPECT_NEAR(result.data[i][i].real, 0.0, 1e-12);
        EXPECT_NEAR(result.data[i][i].imag, 1.0, 1e-12);
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            if (i != j) {
                EXPECT_DOUBLE_EQ(result.data[i][j].real, 0.0);
                EXPECT_DOUBLE_EQ(result.data[i][j].imag, 0.0);
            }
        }
    }
}

// ---- operator- (Matrix subtraction) ----

TEST(MatrixTest, SubtractionSelfIsZero) {
    Matrix I = Matrix::identity();
    Matrix result = I - I;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            EXPECT_DOUBLE_EQ(result.data[i][j].real, 0.0);
            EXPECT_DOUBLE_EQ(result.data[i][j].imag, 0.0);
        }
    }
}

TEST(MatrixTest, SubtractionKnownValues) {
    Matrix A, B;
    A.data[0][0] = {5.0, 3.0};
    B.data[0][0] = {2.0, 1.0};
    Matrix result = A - B;
    EXPECT_NEAR(result.data[0][0].real, 3.0, 1e-12);
    EXPECT_NEAR(result.data[0][0].imag, 2.0, 1e-12);
    // Other entries should be zero (default - default)
    EXPECT_DOUBLE_EQ(result.data[1][1].real, 0.0);
    EXPECT_DOUBLE_EQ(result.data[1][1].imag, 0.0);
}

TEST(MatrixTest, SubtractionIdentityMinusIdentityIsZero) {
    Matrix I = Matrix::identity();
    Matrix result = I - I;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        for (int j = 0; j < Config::QUANTUM_DIM; ++j) {
            EXPECT_DOUBLE_EQ(result.data[i][j].real, 0.0);
            EXPECT_DOUBLE_EQ(result.data[i][j].imag, 0.0);
        }
    }
}

// ---- multiplyVector ----

TEST(MatrixTest, IdentityMultiplyVectorPreservesVector) {
    Matrix I = Matrix::identity();
    std::array<Complex, Config::QUANTUM_DIM> v = {
        Complex{1.0, 0.0}, Complex{0.0, 1.0}, Complex{2.0, -1.0}, Complex{-1.0, 0.5}
    };
    auto result = I.multiplyVector(v);
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        EXPECT_NEAR(result[i].real, v[i].real, 1e-12);
        EXPECT_NEAR(result[i].imag, v[i].imag, 1e-12);
    }
}

TEST(MatrixTest, ZeroMatrixMultiplyVectorIsZero) {
    Matrix zero;
    std::array<Complex, Config::QUANTUM_DIM> v = {
        Complex{1.0, 2.0}, Complex{3.0, 4.0}, Complex{5.0, 6.0}, Complex{7.0, 8.0}
    };
    auto result = zero.multiplyVector(v);
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        EXPECT_DOUBLE_EQ(result[i].real, 0.0);
        EXPECT_DOUBLE_EQ(result[i].imag, 0.0);
    }
}

TEST(MatrixTest, ScalingMatrixDoublesVector) {
    // (2*I) * v = 2*v
    Matrix twoI = Matrix::identity() * Complex{2.0, 0.0};
    std::array<Complex, Config::QUANTUM_DIM> v = {
        Complex{1.0, 0.0}, Complex{0.0, 1.0}, Complex{1.0, 1.0}, Complex{-1.0, 0.0}
    };
    auto result = twoI.multiplyVector(v);
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        EXPECT_NEAR(result[i].real, 2.0 * v[i].real, 1e-12);
        EXPECT_NEAR(result[i].imag, 2.0 * v[i].imag, 1e-12);
    }
}

TEST(MatrixTest, MultiplyVectorWithDiagonalMatrix) {
    // diag(1,2,3,4) * [1,1,1,1] = [1,2,3,4]
    Matrix D;
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        D.data[i][i] = { static_cast<double>(i + 1), 0.0 };
    }
    std::array<Complex, Config::QUANTUM_DIM> v = {
        Complex{1.0, 0.0}, Complex{1.0, 0.0}, Complex{1.0, 0.0}, Complex{1.0, 0.0}
    };
    auto result = D.multiplyVector(v);
    for (int i = 0; i < Config::QUANTUM_DIM; ++i) {
        EXPECT_NEAR(result[i].real, static_cast<double>(i + 1), 1e-12);
        EXPECT_NEAR(result[i].imag, 0.0, 1e-12);
    }
}
