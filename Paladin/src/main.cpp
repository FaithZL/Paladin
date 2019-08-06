//
//  main.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/6/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#include <iostream>
#include "paladin.hpp"

#include "test_lua.hpp"

USING_PALADIN

USING_STD


int main(int argc, const char * argv[]) {
    // insert code here...
    COUT << "Hello, paladin!\n";
    Paladin pld;
    
    test_lua_main();
    system("pause");
    
    return pld.run(argc, argv);
}

