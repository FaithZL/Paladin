//
//  cube.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/7/30.
//  Copyright © 2019 Zero. All rights reserved.
//

#ifndef cube_hpp
#define cube_hpp

#include "shapes/trianglemesh.hpp"

PALADIN_BEGIN

vector<shared_ptr<Shape>> createQuad(shared_ptr<const Transform> o2w,
                                    bool reverseOrientation,
                                    int width, int height = 0);


PALADIN_END

#endif /* cube_hpp */
