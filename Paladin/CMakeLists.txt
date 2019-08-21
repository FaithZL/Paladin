# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
set(APP_NAME Paladin)
project(${APP_NAME})

# 使用C++11特性
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# 收集所有头文件源文件，存入 ALL_FILES 变量中
file(GLOB_RECURSE ALL_FILES "src/*.h*" "src/*.cpp")

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

# 添加各个库目录
add_subdirectory(third/lua53)
add_subdirectory(third/tinyobjloader)
set(EXTRA_LIBS ${EXTRA_LIBS} lua53 tinyobjloader)

# 设置头文件目录
include_directories ("${PROJECT_SOURCE_DIR}/src/alltest")
include_directories ("${PROJECT_SOURCE_DIR}/src/accelerators")
include_directories ("${PROJECT_SOURCE_DIR}/src/parallel")
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
include_directories ("${PROJECT_SOURCE_DIR}/third/lua53/lua")
include_directories ("${PROJECT_SOURCE_DIR}/third/tinyobjloader")

# 指定生成目标
add_executable (${APP_NAME} ${ALL_FILES})

# 添加链接库
target_link_libraries(${APP_NAME} ${EXTRA_LIBS})

# 设置输出路径
MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    #set_target_properties(${APP_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${APP_NAME}>)
    set_target_properties(${APP_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif()

add_custom_command(TARGET ${APP_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/res $<TARGET_FILE_DIR:${APP_NAME}>/res)
