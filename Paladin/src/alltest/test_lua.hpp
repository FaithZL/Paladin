#include <stdio.h>
#ifdef _MSC_VER
    #include <direct.h>
    #define GET_CWD _getcwd
#else
    #include <unistd.h>
    #define GET_CWD getcwd
#endif

/*
头文件lua.h定义了Lua提供的基础函数，包括创建Lua环境、调用Lua函数、
读写Lua环境中全局变量，以及注册供Lua调用的新函数等等;

头文件lauxlib.h定义了辅助库提供的辅助函数，它的所有定义都以LuaL_开头。
辅助库是一个使用lua.h中API编写出的一个较高的抽象层。Lua的所有标准库编
写都用到了辅助库；辅助库主要用来解决实际的问题。辅助库并没有直接访问Lua
的内部，它都是用官方的基础API来完成所有工作的；

头文件lualib.h定义了打开标准库的函数。Lua库中没有定义任何全局变量。
它将所有的状态都保存在动态结构lua_State中，所有的C API都要求传入一
个指向该结构的指针。luaL_newstate函数用于创建一个新环境或状态。当
luaL_newstate创建一个新的环境时，新的环境中并没有包含预定义的函
数（eg.print）。为了使Lua保持灵活，小巧，所有的标准库都被组织到了
不同的包中。当我们需要使用哪个标准库时，就可以调用lualib.h中定义的
函数来打开对应的标准库；而辅助函数luaL_openlibs则可以打开所有的标准库。
*/
extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}


static int fmt_fs_cwd(lua_State * L)
{
    char path[1024];
    if(NULL == GET_CWD(path, 1023))
        lua_pushinteger(L, errno);
    else
        lua_pushstring(L, path);
    return 1;
}

void test_lua_main()
{
    printf("\n~~~~~~~~~~LUA_TEST_BEGIN~~~~~~\n");
    //初始化lua虚拟机//
    lua_State *lua_state = luaL_newstate();

    //设置待注册的Lua标准库
    static const luaL_Reg lualibs[]=
    {
        {"base",luaopen_base},
        {NULL,NULL}
    };

    //注册标准库并清空堆栈//
    const luaL_Reg *lib = lualibs;
    for (;lib->func!=NULL;lib++)
    {
        luaL_requiref(lua_state, lib->name, lib->func, 1);
        lua_pop(lua_state, 1);
    }

    lua_pushcfunction(lua_state, fmt_fs_cwd);
    lua_setglobal(lua_state, "LI_GetCWD");

    luaL_openlibs(lua_state);
    //运行lua脚本//
    luaL_dofile(lua_state, "res/scripts/entry.lua");
    //关闭虚拟机//
    lua_close(lua_state);

    printf("\n~~~~~~~~~~LUA_TEST_END~~~~~~\n");
}
