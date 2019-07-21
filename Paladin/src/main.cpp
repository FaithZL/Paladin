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

USING_STD

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, paladin!\n";
    Paladin pld;
    Vector3f v;
    uint32_t a = 1;
    uint32_t b = 4294967295;
    b = floatToBits(MaxFloat);
    cout << MaxFloat << endl;
    cout << bitsToFloat(b) << endl;
    
    return pld.run(argc, argv);
}

