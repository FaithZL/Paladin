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

#include "lowdiscrepancy.hpp"

USING_PALADIN

USING_STD


int main(int argc, const char * argv[]) {
    // insert code here...
    COUT << "Hello, paladin!\n";
    Paladin pld;
    
//    test_lua_main();
    cout << InverseRadicalInverse<2>(16, 7);

#ifdef _MSC_VER
    //让VS运行debug时不至于黑屏一闪而过
    system("pause");
#endif    
    return pld.run(argc, argv);
}

