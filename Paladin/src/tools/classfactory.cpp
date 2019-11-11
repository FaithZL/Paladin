//
//  classfactory.cpp
//  Paladin
//
//  Created by SATAN_Z on 2019/11/11.
//

#include "classfactory.hpp"

PALADIN_BEGIN


ClassFactory * ClassFactory::_instance = nullptr;

ClassFactory * ClassFactory::getInstance() {
    if (_instance == nullptr) {
        _instance = new ClassFactory();
    }
    return _instance;
}

void ClassFactory::registClass(const std::string& className, createObject func) {
    _classMap.insert(std::make_pair(className, func));
}

createObject ClassFactory::getCreatorByName(const std::string& className) {
    auto iter = _classMap.find(className);
    if (iter == _classMap.end()) {
        return nullptr;
    }
    return iter->second;
}

PALADIN_END
