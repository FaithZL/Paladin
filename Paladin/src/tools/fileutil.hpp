//
//  fileutil.hpp
//  Paladin
//
//  Created by SATAN_Z on 2019/10/13.
//

#ifndef fileutil_hpp
#define fileutil_hpp

#include "core/header.h"

/**
 * 文件工具
 * 不管了，直接照搬pbrt
 */

PALADIN_BEGIN

bool isAbsolutePath(const std::string &filename);

std::string absolutePath(const std::string &filename);

std::string resolveFilename(const std::string &filename);

std::string directoryContaining(const std::string &filename);

void setSearchDirectory(const std::string &dirname);

inline bool hasExtension(const std::string &value, const std::string &ending) {
    if (ending.size() > value.size()) {
    	return false;
    }
    return std::equal(
                      ending.rbegin(), ending.rend(), value.rbegin(),
                      [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

PALADIN_END

#endif /* fileutil_hpp */
