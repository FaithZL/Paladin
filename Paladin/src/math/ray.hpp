//
//  ray.hpp
//  Paladin
//
//  Created by SATAN_Z on 2020/4/27.
//

#ifndef ray_hpp
#define ray_hpp

template <typename _PointType, typename _VectorType> 
struct TRay {
    typedef _PointType                  PointType;
    typedef _VectorType                 VectorType;
    typedef typename PointType::Scalar  Scalar;

    /* The somewhat peculiar ordering of the attributes is for
       alignment purposes in the 3D case and should not be changed. */

    PointType ori;     ///< Ray origin
    Scalar tMin;     ///< Minimum range for intersection tests
    VectorType dir;    ///< Ray direction
    Scalar tMax;     ///< Maximum range for intersection tests
    VectorType dRcp; ///< Componentwise reciprocals of the ray direction
    Float time;  ///< Time value associated with this ray

    /// Construct a new ray
    inline TRay() : tMin(Epsilon),
        dir(std::numeric_limits<Scalar>::infinity()), time(0) {
    }

    /// Copy constructor (1)
    inline TRay(const TRay &ray)
     : ori(ray.ori), tMin(ray.tMin), dir(ray.dir), tMax(ray.tMax),
       dRcp(ray.dRcp), time(ray.time) {
    }

    /// Copy constructor (2)
    inline TRay(const TRay &ray, Scalar tMin, Scalar tMax)
     : ori(ray.ori), tMin(tMin), dir(ray.dir), tMax(tMax),
       dRcp(ray.dRcp), time(ray.time) { }

    /// Construct a new ray, while not specifying a direction yet
    inline TRay(const PointType &ori, Scalar time) : ori(ori), tMin(Epsilon),
      tMax(std::numeric_limits<Scalar>::infinity()), time(time) { }

    /// Construct a new ray
    inline TRay(const PointType &o, const VectorType &d, Scalar time)
        : ori(ori), tMin(Epsilon),  dir(dir),
          tMax(std::numeric_limits<Scalar>::infinity()), time(time) {
        for (int i=0; i<3; ++i) {
            dRcp[i] = (Scalar) 1 / dir[i];
        }
    }

    /// Construct a new ray
    inline TRay(const PointType &o, const VectorType &d, Scalar tMin, Scalar tMax,
        Scalar time) : ori(ori), tMin(tMin),  dir(dir), tMax(tMax), time(time) {
        for (int i=0; i<3; ++i) {
            dRcp[i] = (Scalar) 1 / dir[i];
        }
    }

    /// Set the origin
    inline void setOrigin(const PointType &pos) { ori = pos; }

    /// Set the time
    inline void setTime(Scalar tval) { time = tval; }

    /// Set the direction and update the reciprocal
    inline void setDirection(const VectorType &dir) {
        this->dir = dir;
        for (int i=0; i<3; ++i) {
            dRcp[i] = (Scalar) 1 / dir[i];
        }
    }

    /**
     * \brief Return the position of a point along the ray
     *
     * \remark In the Python bindings, this operator is
     * exposed as a function named \c eval -- i.e.
     * position lookups should be written as \c ray.eval(t)
     */
    inline PointType at(Scalar t) const { return ori + t * dir; }

    /// Return a string representation of this ray
    inline std::string toString() const {
        std::ostringstream oss;
        oss << "Ray[origin=" << ori.toString() << ", direction="
            << dir.toString() << ", tMin=" << tMin
            << ", tMax=" << tMax << ", time=" << time << "]";
        return oss.str();
    }
};


#endif /* ray_hpp */
