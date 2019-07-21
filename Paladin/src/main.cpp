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
    
//    std::cout << sizeof(uint8_t) << std::endl;
    
    auto p = BlockedArray<int>(8,8);
    
    for (int i = 0; i < 5; ++ i) {
        cout << p.block(i) <<"  b" << i << endl;
        cout << p.offset(i) <<"  o" << i <<endl;
    }
    p.getTotalOffset(6, 6);
    for (int i = 0 ; i < 8; ++ i) {
        for (int j = 0; j < 8; ++j) {
            cout <<"getTotalOffset("<<i<<","<<j<<") = " << p.getTotalOffset(i, j) << endl;
        }
    }
    
    return pld.run(argc, argv);
}

