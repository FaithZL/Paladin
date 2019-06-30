//
//  point.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef point_hpp
#define point_hpp

#include "header.hpp"

namespace zero {
    
    template <typename T>
    class Point2 {
        
    public:
//        explicit Point2(const Point3<T> &p) : x(p.x), y(p.y) {
//            
//        }
        
        Point2(): x(0),y(0) {
            
        }
        
        template <typename U>
        explicit Point2(const Point2<U> &p) {
            x = (T)p.x;
            y = (T)p.y;
        }
        
        Point2(const Point2<T> &p) {
            x = p.x;
            y = p.y;
        }
        
        Point2<T> &operator=(const Point2<T> &p) {
            x = p.x;
            y = p.y;
            return *this;
        }
        
        Point2<T> operator+(const Vector2<T> &v) const {
            return Point2<T>(x + v.x, y + v.y);
        }
        
        Point2<T> &operator+=(const Vector2<T> &v) {
            x += v.x;
            y += v.y;
            return *this;
        }
        
        Vector2<T> operator-(const Point2<T> &p) const {
            return Vector2<T>(x - p.x, y - p.y);
        }
        
        T x;
        T y;
    }
    
}

#endif /* point_h */




