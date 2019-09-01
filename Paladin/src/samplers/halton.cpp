//
//  halton.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/8/29.
//

#include "halton.hpp"

PALADIN_BEGIN

static CONSTEXPR int kMaxResolution = 128;

static void extendedGCD(uint64_t a, uint64_t b, int64_t *x, int64_t *y) {
    if (b == 0) {
        *x = 1;
        *y = 0;
        return;
    }
    int64_t d = a / b, xp, yp;
    extendedGCD(b, a % b, &xp, &yp);
    *x = yp;
    *y = xp - (d * yp);
}

static uint64_t multiplicativeInverse(int64_t a, int64_t n) {
    int64_t x, y;
    extendedGCD(a, n, &x, &y);
    return Mod(x, n);
}



PALADIN_END
