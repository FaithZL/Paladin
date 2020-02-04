//
//  assignment.h
//  Paladin
//
//  Created by SATAN_Z on 2020/2/4.
//

#ifndef assignment_h
#define assignment_h

#include "core/header.h"

PALADIN_BEGIN

// 用于作用域内临时修改一个变量，作用域之后还原修改
template<typename T>
class ScopedAssignment {
    
public:
    
    ScopedAssignment(T * target = nullptr, T value = T())
    : _target(target) {
        if (_target) {
            _backup = * target;
            *target = value;
        }
    }
    
    ~ScopedAssignment() {
        if (_target) {
            *_target = _backup;
        }
    }
    
    ScopedAssignment &operator=(const ScopedAssignment &) = delete;
    
    ScopedAssignment &operator=(ScopedAssignment &&other) {
        if (_target) {
            *_target = _backup;
        }
        _target = other._target;
        _backup = other._backup;
        other._target = nullptr;
        return *this;
    }
    
private:
    T * _target;
    T _backup;
};

PALADIN_END

#endif /* assignment_h */
