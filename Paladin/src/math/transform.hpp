//
//  transform.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef transform_hpp
#define transform_hpp

#include "paladin.hpp"
#include "interaction.hpp"
PALADIN_BEGIN


class Matrix4x4 {
    
public:
    
    Matrix4x4() {
        _m[0][0] = _m[1][1] = _m[2][2] = _m[3][3] = 1.f;
        _m[0][1] = _m[0][2] = _m[0][3] = _m[1][0] = _m[1][2] = _m[1][3] = _m[2][0] =
        _m[2][1] = _m[2][3] = _m[3][0] = _m[3][1] = _m[3][2] = 0.f;
    }
    
    Matrix4x4(const Float p[4][4]) {
        for (int i = 0 ; i < 4; ++ i) {
            for (int j = 0 ; j < 4; ++ j) {
                _m[i][j] = p[i][j];
            }
        }
    }
    
    Matrix4x4(const Float * value) {
        for (int i = 0 ;i< 16 ; ++ i) {
            _a[i] = value[i];
        }
    }
    
    inline void setZero() {
        for (int i = 0 ; i < 16; ++ i) {
            _a[i] = 0;
        }
    }
    
    // 伴随矩阵除以行列式
    Matrix4x4 getInverseMat() const;
    
    Matrix4x4 getTransposeMat() const;
    
    Matrix4x4 getAdjointMat() const;
    
    Float getAdjointElement(Float a1, Float a2, Float a3,
                              Float b1, Float b2, Float b3,
                              Float c1, Float c2, Float c3)const;
    
    Float getDet() const;

    // 返回左上角3x3矩阵的行列式
    inline Float det3x3() const {
        Float det =
            _m[0][0] * (_m[1][1] * _m[2][2] - _m[1][2] * _m[2][1]) -
            _m[0][1] * (_m[1][0] * _m[2][2] - _m[1][2] * _m[2][0]) +
            _m[0][2] * (_m[1][0] * _m[2][1] - _m[1][1] * _m[2][0]);
        return det;
    }
    
    //判断变量中是否包含nan分量
    bool hasNaNs() const {
        for (int i = 0; i < 16; ++i) {
            if (std::isnan(_a[i])) {
                return true;
            }
        }
        return false;
    }
    
    bool operator == (const Matrix4x4 &other) const;
    
    Matrix4x4 operator + (const Matrix4x4 &other) const;
    
    Matrix4x4 operator * (const Matrix4x4 &other) const;
    
    Matrix4x4 operator - (const Matrix4x4 &other) const;
    
    bool operator != (const Matrix4x4 &other) const;
    
    Matrix4x4 operator * (Float num) const;
    
    Matrix4x4 operator / (Float num) const;
    
    bool operator < (const Matrix4x4 &other) const;

    bool isIdentity() const;
    
    static Matrix4x4 identity();

    friend std::ostream &operator<<(std::ostream &os, const Matrix4x4 &mat) {
        // clang-format off
        os << StringPrintf("[ [ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] "
                           "[ %f, %f, %f, %f ] ]",
                           mat._m[0][0], mat._m[0][1], mat._m[0][2], mat._m[0][3],
                           mat._m[1][0], mat._m[1][1], mat._m[1][2], mat._m[1][3],
                           mat._m[2][0], mat._m[2][1], mat._m[2][2], mat._m[2][3],
                           mat._m[3][0], mat._m[3][1], mat._m[3][2], mat._m[3][3]);
        // clang-format on
        return os;
    }
    
private:
    
    union {
        Float _m[4][4];
        Float _a[16];
        struct
        {
            Float _11; Float _12; Float _13; Float _14;
            Float _21; Float _22; Float _23; Float _24;
            Float _31; Float _32; Float _33; Float _34;
            Float _41; Float _42; Float _43; Float _44;
        };
    };

    friend class Transform;
    friend class Quaternion;
    friend class AnimatedTransform;
};


class Transform {
// 参考pbrt设计变换类，包装了矩阵对象，只留变换接口，这样设计的好处在于，代码可读性高，
// 比之前在写OpenGL程序时，需要用齐次坐标来区分点与向量要清晰
public:
    Transform() {

    }

    Transform(const Float mat[4][4]) {
        _mat = Matrix4x4(mat);
        _matInv = _mat.getInverseMat();
    }

    Transform(const Matrix4x4 &mat) : _mat(mat), _matInv(_mat.getInverseMat()) {

    }

    Transform(const Matrix4x4 &mat, const Matrix4x4 &matInv) : _mat(mat), _matInv(matInv) {

    }

    Transform getInverse() const {
        return Transform(_matInv, _mat);
    }

    Transform getTranspose() const {
        return Transform(_mat.getTransposeMat(), _matInv.getTransposeMat());
    }

    bool operator == (const Transform &other) const {
        return other._mat == _mat && other._matInv == _matInv;
    }

    bool operator != (const Transform &other) const {
        return other._mat != _mat || other._matInv != _matInv;
    }

    bool operator < (const Transform &other) const {
        return _mat < other._mat;
    }

    bool isIdentity() const {
        return _mat.isIdentity();
    }

    const Matrix4x4 &getMatrix() const {
        return _mat;
    }

    const Matrix4x4 &getInverseMatrix() const {
        return _matInv;
    }
    
    bool hasScale() const {
        // xyz三个方向的基向量进行变换，只要有一个长度不为1，则含有缩放
        Float la2 = exec(Vector3f(1, 0, 0)).lengthSquared();
        Float lb2 = exec(Vector3f(0, 1, 0)).lengthSquared();
        Float lc2 = exec(Vector3f(0, 0, 1)).lengthSquared();
    #define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
        return (NOT_ONE(la2) || NOT_ONE(lb2) || NOT_ONE(lc2));
    #undef NOT_ONE
    }

    // 对点执行转换
    template<typename T>
    inline Point3<T> exec(const Point3<T> &point) const {
        T x = point.x, y = point.y, z = point.z;
        T xp = _mat._m[0][0] * x + _mat._m[0][1] * y + _mat._m[0][2] * z + _mat._m[0][3];
        T yp = _mat._m[1][0] * x + _mat._m[1][1] * y + _mat._m[1][2] * z + _mat._m[1][3];
        T zp = _mat._m[2][0] * x + _mat._m[2][1] * y + _mat._m[2][2] * z + _mat._m[2][3];
        T wp = _mat._m[3][0] * x + _mat._m[3][1] * y + _mat._m[3][2] * z + _mat._m[3][3];
        CHECK_NE(wp, 0);
        if (wp == 1)
            return Point3<T>(xp, yp, zp);
        else
            return Point3<T>(xp, yp, zp) / wp;
    }

    /*
    计算误差的点的变换
    x′ = ((m0,0 ⊗ x) ⊕ (m0,1 ⊗ y)) ⊕ ((m0,2 ⊗ z) ⊕ m0,3)
        ⊂ m0,0x(1± εm)^3 + m0,1y(1 ± εm)^3 + m0,2z(1± εm)^3 + m0,3(1± εm)^2 
        ⊂(m0,0x+m0,1y+m0,2z+m0,3) + γ3(±m0,0x± m0,1y± m0,2z± m0,3) 
        ⊂(m0,0x+m0,1y+m0,2z+m0,3) ± γ3(|m0,0x|+|m0,1y|+|m0,2z|+|m0,3|).
    以上表达式来自pbrt,自己经过亲自推导之后发现略有不同，如下
    m0,0x(1± εm)^3 + m0,1y(1 ± εm)^3 + m0,2z(1± εm)^2 + m0,3(1± εm)^2 
    更加保守的估计误差为  γ3(|m0,0x|+|m0,1y|+|m0,2z|+|m0,3|)
    以上的推导过程均来自errfloat.h中的基本误差推导公式
    y与z同理
     */
    template <typename T>
    inline Point3<T> exec(const Point3<T> &point, Vector3<T> *absError) const {
        T x = point.x, y = point.y, z = point.z;
        T xp = _mat._m[0][0] * x + _mat._m[0][1] * y + _mat._m[0][2] * z + _mat._m[0][3];
        T yp = _mat._m[1][0] * x + _mat._m[1][1] * y + _mat._m[1][2] * z + _mat._m[1][3];
        T zp = _mat._m[2][0] * x + _mat._m[2][1] * y + _mat._m[2][2] * z + _mat._m[2][3];
        T wp = _mat._m[3][0] * x + _mat._m[3][1] * y + _mat._m[3][2] * z + _mat._m[3][3];
        
        T xErr = (std::abs(_mat._m[0][0] * x) + std::abs(_mat._m[0][1] * y) +
                  std::abs(_mat._m[0][2] * z) + std::abs(_mat._m[0][3]));
        T yErr = (std::abs(_mat._m[1][0] * x) + std::abs(_mat._m[1][1] * y) +
                  std::abs(_mat._m[1][2] * z) + std::abs(_mat._m[1][3]));
        T zErr = (std::abs(_mat._m[2][0] * x) + std::abs(_mat._m[2][1] * y) +
                  std::abs(_mat._m[2][2] * z) + std::abs(_mat._m[2][3]));

        *absError = gamma(3) * Vector3f(xErr, yErr, zErr);
        
        CHECK_NE(wp, 0);
        if (wp == 1)
            return Point3<T>(xp, yp, zp);
        else
            return Point3<T>(xp, yp, zp) / wp;
    }
    
    /*
    上面的转换假设转换点是准确的，但如果被转换的点本身就有误差，则表达式如下
     x′ = (m0,0 ⊗ (x ± δx) ⊕ m0,1 ⊗ (y ± δy)) ⊕ (m0,2 ⊗ (z ± δz) ⊕ m0,3).
     x′ = m0,0(x ± δx)(1± εm)^3 + m0,1(y ± δy)(1± εm)^3 + m0,2(z± δz)(1± εm)^3 + m0,3(1± εm)^2.
     用γ替换高次项，得出
     xError = (γ3 + 1)(|m0,0|δx + |m0,1|δy + |m0,2|δz) 
            + γ3(|m0,0 * x| + |m0,1 * y| + |m0,2 * z| + |m0,3|).
     */
    template <typename T>
    inline Point3<T> exec(const Point3<T> &point, const Vector3<T> &ptError, Vector3<T> *pTransError) const {
        T x = point.x, y = point.y, z = point.z;
        T xp = _mat._m[0][0] * x + _mat._m[0][1] * y + _mat._m[0][2] * z + _mat._m[0][3];
        T yp = _mat._m[1][0] * x + _mat._m[1][1] * y + _mat._m[1][2] * z + _mat._m[1][3];
        T zp = _mat._m[2][0] * x + _mat._m[2][1] * y + _mat._m[2][2] * z + _mat._m[2][3];
        T wp = _mat._m[3][0] * x + _mat._m[3][1] * y + _mat._m[3][2] * z + _mat._m[3][3];

        pTransError->x =
            (gamma(3) + (T)1) *
                (std::abs(_mat._m[0][0]) * ptError.x + std::abs(_mat._m[0][1]) * ptError.y +
                 std::abs(_mat._m[0][2]) * ptError.z) +
            gamma(3) * (std::abs(_mat._m[0][0] * x) + std::abs(_mat._m[0][1] * y) +
                        std::abs(_mat._m[0][2] * z) + std::abs(_mat._m[0][3]));
        pTransError->y =
            (gamma(3) + (T)1) *
                (std::abs(_mat._m[1][0]) * ptError.x + std::abs(_mat._m[1][1]) * ptError.y +
                 std::abs(_mat._m[1][2]) * ptError.z) +
            gamma(3) * (std::abs(_mat._m[1][0] * x) + std::abs(_mat._m[1][1] * y) +
                        std::abs(_mat._m[1][2] * z) + std::abs(_mat._m[1][3]));
        pTransError->z =
            (gamma(3) + (T)1) *
                (std::abs(_mat._m[2][0]) * ptError.x + std::abs(_mat._m[2][1]) * ptError.y +
                 std::abs(_mat._m[2][2]) * ptError.z) +
            gamma(3) * (std::abs(_mat._m[2][0] * x) + std::abs(_mat._m[2][1] * y) +
                        std::abs(_mat._m[2][2] * z) + std::abs(_mat._m[2][3]));
        CHECK_NE(wp, 0);
        if (wp == 1.)
            return Point3<T>(xp, yp, zp);
        else
            return Point3<T>(xp, yp, zp) / wp;
        
    }

    /*
     以下三个函数是对向量执行转换，误差分析方式跟点的转换类似，因此不再赘述
    */
    template<typename T>
    inline Vector3<T> exec(const Vector3<T> &vec) const {
        T x = vec.x, y = vec.y, z = vec.z;
        return Vector3<T>(_mat._m[0][0] * x + _mat._m[0][1] * y + _mat._m[0][2] * z,
                          _mat._m[1][0] * x + _mat._m[1][1] * y + _mat._m[1][2] * z,
                          _mat._m[2][0] * x + _mat._m[2][1] * y + _mat._m[2][2] * z);
    }

    template <typename T>
    inline Vector3<T> exec(const Vector3<T> &v, Vector3<T> *absError) const {
        T x = v.x, y = v.y, z = v.z;
        absError->x =
            gamma(3) * (std::abs(_mat._m[0][0] * v.x) + std::abs(_mat._m[0][1] * v.y) +
                        std::abs(_mat._m[0][2] * v.z));
        absError->y =
            gamma(3) * (std::abs(_mat._m[1][0] * v.x) + std::abs(_mat._m[1][1] * v.y) +
                        std::abs(_mat._m[1][2] * v.z));
        absError->z =
            gamma(3) * (std::abs(_mat._m[2][0] * v.x) + std::abs(_mat._m[2][1] * v.y) +
                        std::abs(_mat._m[2][2] * v.z));
        return Vector3<T>(_mat._m[0][0] * x + _mat._m[0][1] * y + _mat._m[0][2] * z,
                      _mat._m[1][0] * x + _mat._m[1][1] * y + _mat._m[1][2] * z,
                      _mat._m[2][0] * x + _mat._m[2][1] * y + _mat._m[2][2] * z);
    }

    template <typename T>
    inline Vector3<T> exec(const Vector3<T> &v, const Vector3<T> &vError, Vector3<T> *vTransError) const {
        T x = v.x, y = v.y, z = v.z;

        vTransError->x =
            (gamma(3) + (T)1) *
                (std::abs(_mat._m[0][0]) * vError.x + std::abs(_mat._m[0][1]) * vError.y +
                 std::abs(_mat._m[0][2]) * vError.z) +
            gamma(3) * (std::abs(_mat._m[0][0] * v.x) + std::abs(_mat._m[0][1] * v.y) +
                        std::abs(_mat._m[0][2] * v.z));

        vTransError->y =
            (gamma(3) + (T)1) *
                (std::abs(_mat._m[1][0]) * vError.x + std::abs(_mat._m[1][1]) * vError.y +
                 std::abs(_mat._m[1][2]) * vError.z) +
            gamma(3) * (std::abs(_mat._m[1][0] * v.x) + std::abs(_mat._m[1][1] * v.y) +
                        std::abs(_mat._m[1][2] * v.z));

        vTransError->z =
            (gamma(3) + (T)1) *
                (std::abs(_mat._m[2][0]) * vError.x + std::abs(_mat._m[2][1]) * vError.y +
                 std::abs(_mat._m[2][2]) * vError.z) +
            gamma(3) * (std::abs(_mat._m[2][0] * v.x) + std::abs(_mat._m[2][1] * v.y) +
                        std::abs(_mat._m[2][2] * v.z));

        return Vector3<T>(_mat._m[0][0] * x + _mat._m[0][1] * y + _mat._m[0][2] * z,
                      _mat._m[1][0] * x + _mat._m[1][1] * y + _mat._m[1][2] * z,
                      _mat._m[2][0] * x + _mat._m[2][1] * y + _mat._m[2][2] * z);
    }

    template<typename T>
    inline Normal3<T> exec(const Normal3<T> &normal) {
        /*
        法线的转换跟向量的转换所有不同，如果有非比例缩放法线就不能套用向量的转换了
        法线转换矩阵与原转换矩阵的关系如下t为切线向量,S为法线转换矩阵，M为切线转换矩阵
        dot(n, t) = 0
        transpose(n) * t = 0
        transpose(S * n) * (M * t) = 0
        transpose(n) * transpose(S) * M * t = 0 
        只要满足这个表达式的S矩阵，就是我们的目标矩阵
        我们可以假设 transpose(S) * M = I , 可得 S = transpose(inverse(M))
        */
        T x = normal.x, y = normal.y, z = normal.z;
        x = _matInv._m[0][0] * x + _matInv._m[1][0] * y + _matInv._m[2][0] * z;
        y = _matInv._m[0][1] * x + _matInv._m[1][1] * y + _matInv._m[2][1] * z;
        z = _matInv._m[0][2] * x + _matInv._m[1][2] * y + _matInv._m[2][2] * z;
        return Normal3<T>(x, y, z);
    }

    inline Ray exec(const Ray &ray) const {
        Vector3f oError;
        Point3f ori = exec(ray.ori, &oError);
        Vector3f dir = exec(ray.dir);
    }

    inline Ray exec(const Ray &r, Vector3f *oError,
                          Vector3f *dError) const;
    inline Ray exec(const Ray &ray, const Vector3f &oErrorIn,
                          const Vector3f &dErrorIn, Vector3f *oErrorOut,
                          Vector3f *dErrorOut) const;

    SurfaceInteraction exec(const SurfaceInteraction &isect) const;

    inline RayDifferential exec(const RayDifferential &rd) const;

    Bounds3f exec(const Bounds3f &b) const {
        // 对包围盒的8个顶点都进行变换，然后合并，每个顶点都进行了变换，运算量大
       // Bounds3f ret(exec(Point3f(b.pMin.x, b.pMin.y, b.pMin.z)));
       // ret = unionSet(ret, exec(Point3f(b.pMax.x, b.pMin.y, b.pMin.z)));
       // ret = unionSet(ret, exec(Point3f(b.pMin.x, b.pMax.y, b.pMin.z)));
       // ret = unionSet(ret, exec(Point3f(b.pMin.x, b.pMin.y, b.pMax.z)));
       // ret = unionSet(ret, exec(Point3f(b.pMin.x, b.pMax.y, b.pMax.z)));
       // ret = unionSet(ret, exec(Point3f(b.pMax.x, b.pMax.y, b.pMin.z)));
       // ret = unionSet(ret, exec(Point3f(b.pMax.x, b.pMin.y, b.pMax.z)));
       // ret = unionSet(ret, exec(Point3f(b.pMax.x, b.pMax.y, b.pMax.z)));

        /*
        优化思路如下：
        假设矩阵第一行元素为为abcd, 原始点p(x,y,z)
        则 x' = ax + by + cz + d
        x'的最小值为
        min(ax + by + cz + d) = min(ax) + min(by) + min(cz) + min(d)
        x'的最大值为
        max(ax + by + cz + d) = max(ax) + max(by) + max(cz) + max(d)
        立方体xyz三个维度各自只有两个值，最大值跟最小值，组成了8个顶点(2^3 = 8)
        所以ax只有这两个值，最大值跟最小值，以下代码中，遍历到较大值的时候加在max上，较小值加在min上
        y跟z同理
         */
        const Matrix4x4& mat = _mat;
        Point3f minPoint = Point3f(mat._m[0][3], mat._m[1][3], mat._m[2][3]);
        Point3f maxPoint = minPoint;

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3;++j) {
                Float e = mat._m[i][j];
                float p1 = e * b.pMin[j];
                float p2 = e * b.pMax[j];
                if (p1 > p2) {
                    minPoint[i] += p2;
                    maxPoint[i] += p1;
                } else {
                    minPoint[i] += p1;
                    maxPoint[i] += p2;
                }
            }
        }
        Bounds3f ret;
        ret.pMin = minPoint;
        ret.pMax = maxPoint;
        return ret;
    }

    Transform operator * (const Transform &other) const;

    inline bool swapsHandedness() const {
        /*
         如果左上角3x3的矩阵行列式小于零，则说明这个变换是换了手的，至于为何是这样，暂时没有了解太深，搞完主线再说todo
         */
        return _mat.det3x3() < 0;
    }

    static Transform translate(const Vector3f &delta);

    static Transform scale(Float x, Float y, Float z);

    static Transform scale(Float s);

    static Transform rotateX(Float theta, bool bRadian=false);

    static Transform rotateY(Float theta, bool bRadian=false);

    static Transform rotateZ(Float theta, bool bRadian=false);

    static Transform rotate(Float theta, const Vector3f &axis, bool bRadian=false);

    static Transform lookAt(const Point3f &pos, const Point3f &look, const Vector3f &up);

    static Transform orthographic(Float zNear, Float zFar);

    static Transform perspective(Float fov, Float zNear, Float zFar, bool bRadian=false);

private:

    Matrix4x4 _mat;
    Matrix4x4 _matInv;

};

PALADIN_END

#endif /* transform_hpp */
