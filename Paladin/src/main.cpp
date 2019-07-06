//
//  main.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include <iostream>
#include "paladin.hpp"

USING_PALADIN

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, paladin!\n";
    Paladin paladin;
    Point3f p;
    Vector3f v;
    RayDifferential(p, v);
    return paladin.run(argc, argv);
}

