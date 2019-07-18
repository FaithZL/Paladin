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
    Paladin pld;
    Point3f p;
    Vector3f v;
    
    std::cout << Log2(8) << std::endl;
    
    return pld.run(argc, argv);
}

