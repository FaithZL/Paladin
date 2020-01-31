# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
set(APP_NAME Paladin)
project(${APP_NAME})

# 使用C++11特性
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# 开启工程虚拟目录
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# 收集所有头文件源文件，存入 ALL_FILES 变量中
file(GLOB_RECURSE ALL_FILES "src/*.h*" "src/*.c*")
file(GLOB MORE_FILES "third/stblib/stb/*.h" "third/stblib/stb/*.c")
foreach(_file IN ITEMS ${MORE_FILES})
    list(APPEND ALL_FILES ${_file})
endforeach()

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

FIND_PACKAGE ( ZLIB )
IF(NOT ZLIB_FOUND)
    # Build zlib
    SET(ZLIB_BUILD_STATIC_LIBS ON CACHE BOOL " " FORCE)
    SET(ZLIB_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
    add_subdirectory(third/zlib)

    SET(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third/zlib")
    SET(ZLIB_LIBRARY zlibstatic)
    SET_PROPERTY(TARGET zlibstatic 
            PROPERTY FOLDER "zlib")
    include_directories(${ZLIB_INCLUDE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/src/ext/zlib")
ENDIF()
include_directories(${ZLIB_INCLUDE_DIRS})

# 添加各个库目录
add_subdirectory(third/lua53)
add_subdirectory(third/glog)
add_subdirectory(third/openexr)
# 设置头文件目录
include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/src/ext")
include_directories ("${PROJECT_SOURCE_DIR}/third/lua53/lua")
include_directories ("${PROJECT_SOURCE_DIR}/third/stblib/stb")
include_directories ("${PROJECT_SOURCE_DIR}/third/glog/src")
include_directories ("${PROJECT_SOURCE_DIR}/third/openexr/IlmBase/Imath")
include_directories ("${PROJECT_SOURCE_DIR}/third/openexr/IlmBase/Half")
include_directories ("${PROJECT_SOURCE_DIR}/third/openexr/IlmBase/Iex")
include_directories ("${PROJECT_SOURCE_DIR}/third/openexr/OpenEXR/IlmImf")
include_directories ("${CMAKE_BINARY_DIR}/third/openexr/IlmBase/config")
include_directories ("${CMAKE_BINARY_DIR}/third/openexr/OpenEXR/config")


# 设置glog库及相关单元测试的虚拟目录
SET_PROPERTY(TARGET glog 
                    logging_unittest 
                    demangle_unittest 
                    utilities_unittest 
                    stl_logging_unittest 
                    symbolize_unittest 
                    stacktrace_unittest 
                    signalhandler_unittest
            PROPERTY FOLDER "glog")
# 设置openexr相关库的虚拟目录
SET_PROPERTY(TARGET IexMath 
                    eLut 
                    toFloat 
                    b44ExpLogTable 
                    dwaLookups 
                    IlmThread 
                    Half 
                    Iex 
                    Imath 
                    IlmImf 
                    CopyIlmBaseLibs 
                    IlmImf-obj
            PROPERTY FOLDER "openexr")

IF(WIN32)
  SET(OPENEXR_LIBS IlmImf Imath Half ${ZLIB_LIBRARY})
ELSE()
  SET(OPENEXR_LIBS IlmImf Imath Half)
ENDIF()

set(EXTRA_LIBS ${EXTRA_LIBS} lua53 glog ${OPENEXR_LIBS})

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
