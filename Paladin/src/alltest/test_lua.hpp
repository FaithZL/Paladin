#include <stdio.h>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
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

    luaL_openlibs(lua_state);
    //运行lua脚本//
    luaL_dofile(lua_state, "../res/scripts/testlua.lua");
    //关闭虚拟机//
    lua_close(lua_state);

    printf("\n~~~~~~~~~~LUA_TEST_END~~~~~~\n");
}