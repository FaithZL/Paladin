# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (Paladin)

# 使用C++11特性
add_definitions (-std=c++11)

# 收集所有头文件源文件，存入ALL_FILES变量中
file(GLOB_RECURSE ALL_FILES "*.h" "*.h??" "*.cpp")

# 定义按目录结构归类的方法
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)


foreach(_source IN ITEMS ${ALL_FILES})
    assign_source_group(${_source})
endforeach()

# 设置头文件目录
include_directories ("${PROJECT_SOURCE_DIR}/ext")
include_directories ("${PROJECT_SOURCE_DIR}/src/accelerators")
include_directories ("${PROJECT_SOURCE_DIR}/src/cameras")
include_directories ("${PROJECT_SOURCE_DIR}/src/core")
include_directories ("${PROJECT_SOURCE_DIR}/src/filters")
include_directories ("${PROJECT_SOURCE_DIR}/src/integrators")
include_directories ("${PROJECT_SOURCE_DIR}/src/lights")
include_directories ("${PROJECT_SOURCE_DIR}/src/materials")
include_directories ("${PROJECT_SOURCE_DIR}/src/math")
include_directories ("${PROJECT_SOURCE_DIR}/src/samplers")
include_directories ("${PROJECT_SOURCE_DIR}/src/shapes")
include_directories ("${PROJECT_SOURCE_DIR}/src/textures")
include_directories ("${PROJECT_SOURCE_DIR}/src/tools")

# 指定生成目标
add_executable (Paladin ${ALL_FILES})

