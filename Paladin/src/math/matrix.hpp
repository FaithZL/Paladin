//
//  matrix.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/4/28.
//

#ifndef matrix_hpp
#define matrix_hpp

#include "core/header.h"

PALADIN_BEGIN

template <int M, int N, typename T> 
struct Matrix {
public:
    T m[M][N];

    /**
     * \brief Construct a new MxN matrix without initializing it.
     *
     * This constructor is useful when the matrix will either not
     * be used at all (it might be part of a larger data structure)
     * or initialized at a later point in time. Always make sure
     * that one of the two is the case! Otherwise your program will do
     * computations involving uninitialized memory, which will probably
     * lead to a difficult-to-find bug.
     */
    Matrix() {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] = T(0);
            }
        }
    }

    /// Initialize the matrix with constant entries
    explicit inline Matrix(T value) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] = value;
            }
        }
    }

    /// Initialize the matrix from a given MxN array
    explicit inline Matrix(const T _m[M][N]) {
        memcpy(m, _m, sizeof(T) * M * N);
    }

    /// Initialize the matrix from a given (flat) MxN array in row-major order
    explicit inline Matrix(const T _m[M*N]) {
        memcpy(m, _m, sizeof(T) * M * N);
    }

    /// Copy constructor
    inline Matrix(const Matrix &mtx) {
        memcpy(m, mtx.m, sizeof(T) * M * N);
    }

    /// Initialize with the identity matrix
    void setIdentity() {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }

    /// Initialize with zeroes
    void setZero() {
        memset(m, 0, sizeof(T) * M * N);
    }


    /// Indexing operator
    inline T &operator()(int i, int j) { return m[i][j]; }

    /// Indexing operator (const verions)
    inline const T & operator()(int i, int j) const { return m[i][j]; }

    /// Equality operator
    inline bool operator==(const Matrix &mat) const {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                if (m[i][j] != mat.m[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    /// Inequality operator
    inline bool operator!=(const Matrix &mat) const {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                if (m[i][j] != mat.m[i][j]) {
                    return true;
                }
            }
        }
        return false;
    }

    /// Assignment operator
    inline Matrix &operator=(const Matrix &mat) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] = mat.m[i][j];
            }
        }
        return *this;
    }

    /// Matrix addition (returns a temporary)
    inline Matrix operator+(const Matrix &mat) const {
        Matrix result;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                result.m[i][j] = m[i][j] + mat.m[i][j];
            }
        }
        return result;
    }

    /// Matrix-scalar addition (returns a temporary)
    inline Matrix operator+(T value) const {
        Matrix result;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                result.m[i][j] = m[i][j] + value;
            }
        }
        return result;
    }

    /// Matrix addition
    inline const Matrix &operator+=(const Matrix &mat) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] += mat.m[i][j];
            }
        }
        return *this;
    }

    /// Matrix-scalar addition
    inline const Matrix &operator+=(T value) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] += value;
            }
        }
        return *this;
    }

    /// Matrix subtraction (returns a temporary)
    inline Matrix operator-(const Matrix &mat) const {
        Matrix result;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                result.m[i][j] = m[i][j] - mat.m[i][j];
            }
        }
        return result;
    }

    /// Matrix-scalar subtraction (returns a temporary)
    inline Matrix operator-(T value) const {
        Matrix result;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                result.m[i][j] = m[i][j] - value;
            }
        }
        return result;
    }

    /// Matrix subtraction
    inline const Matrix &operator-=(const Matrix &mat) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] -= mat.m[i][j];
            }
        }
        return *this;
    }

    /// Matrix-scalar subtraction
    inline const Matrix &operator-(T value) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] -= value;
            }
        }
        return *this;
    }

    /// Matrix-scalar addition
    inline const Matrix &operator-=(T value) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] -= value;
            }
        }
        return *this;
    }

    /// Component-wise negation
    inline Matrix operator-() const {
        Matrix result;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                result.m[i][j] = -m[i][j];
            }
        }
        return result;
    }

    /// Scalar multiplication (creates a temporary)
    inline Matrix operator*(T value) const {
        Matrix result;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                result.m[i][j] = m[i][j]*value;
            }
        }
        return result;
    }

    /// Scalar multiplication
    inline const Matrix& operator*=(T value) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] *= value;
            }
        }
        return *this;
    }

    /// Scalar division (creates a temporary)
    inline Matrix operator/(T value) const {
        Matrix result;
        DCHECK(value != 0);
        Float recip = 1/value;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                result.m[i][j] = m[i][j]*recip;
            }
        }
        return result;
    }

    /// Scalar division
    inline const Matrix& operator/=(T value) {
        DCHECK(value != 0);
        Float recip = 1/value;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                m[i][j] *= recip;
            }
        }
        return *this;
    }

    /// Matrix multiplication (for square matrices)
    inline const Matrix &operator*=(const Matrix &mat) {
        DCHECK(M == N);
        Matrix temp = *this * mat;
        *this = temp;
        return *this;
    }

    /// Compute the trace of a square matrix
    inline Float trace() const {
        DCHECK(M == N);
        Float sum = 0;
        for (int i = 0; i < M; ++i) {
            sum += m[i][i];
        }
        return sum;
    }

    /// Compute the Frobenius norm
    inline Float frob() const {
        Float val = 0;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                val += m[i][j] * m[i][j];
            }
        }
        return std::sqrt(val);
    }

    /// Check if the matrix is identically zero
    inline bool isZero() const {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                if (m[i][j] != 0) {
                    return false;
                }
            }
        }
        return true;
    }

    /// Test if this is the identity matrix
    inline bool isIdentity() const {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                if (m[i][j] != ((i == j) ? 1 : 0)) {
                    return false;
                }
            }
        }
        return true;
    }

    /// Compute the inverse of a square matrix using the Gauss-Jordan algorithm
    bool invert(Matrix &target) const {
        DCHECK(M == N);

        int indxc[N], indxr[N];
        int ipiv[N];
        memset(ipiv, 0, sizeof(int)*N);
        memcpy(target.m, m, M*N*sizeof(T));

        for (int i = 0; i < N; i++) {
            int irow = -1, icol = -1;
            T big = 0;
            for (int j = 0; j < N; j++) {
                if (ipiv[j] != 1) {
                    for (int k = 0; k < N; k++) {
                        if (ipiv[k] == 0) {
                            if (std::abs(target.m[j][k]) >= big) {
                                big = std::abs(target.m[j][k]);
                                irow = j;
                                icol = k;
                            }
                        } else if (ipiv[k] > 1) {
                            return false;
                        }
                    }
                }
            }
            ++ipiv[icol];
            if (irow != icol) {
                for (int k = 0; k < N; ++k) {
                    std::swap(target.m[irow][k], target.m[icol][k]);
                }
            }
            indxr[i] = irow;
            indxc[i] = icol;
            if (target.m[icol][icol] == 0) {
                return false;
            }
            T pivinv = 1.f / target.m[icol][icol];
            target.m[icol][icol] = 1.f;
            for (int j = 0; j < N; j++) {
                target.m[icol][j] *= pivinv;
            }
            for (int j = 0; j < N; j++) {
                if (j != icol) {
                    T save = target.m[j][icol];
                    target.m[j][icol] = 0;
                    for (int k = 0; k < N; k++) {
                        target.m[j][k] -= target.m[icol][k]*save;
                    }
                }
            }
        }
        for (int j = N-1; j >= 0; j--) {
            if (indxr[j] != indxc[j]) {
                for (int k = 0; k < N; k++) {
                    std::swap(target.m[k][indxr[j]], target.m[k][indxc[j]]);
                }
            }
        }
        return true;
    }

    /// Compute the transpose of this matrix
    inline void transpose(Matrix<N, M, T> &target) const {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                target.m[i][j] = m[j][i];
            }
        }
    }

    /// Return a string representation
    std::string toString() const {
        std::ostringstream oss;
        oss << "Matrix" << M << "x" << N << "["<< std::endl;
        for (int i = 0; i < M; ++i) {
            oss << "  ";
            for (int j = 0; j < N; ++j) {
                oss << m[i][j];
                if (j != N-1) {
                    oss << ", ";
                }
            }
            if (i != M-1) {
                oss << ";";
            }
            oss << std::endl;
        }
        oss << "]";
        return oss.str();
    }
protected:

};


PALADIN_END

#endif /* matrix_hpp */
