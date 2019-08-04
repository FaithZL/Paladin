# 查找当前目录下的所有源文件
# 并将名称保存到 ALL_FILES 变量
file(GLOB ALL_FILES "lua/*.h" "lua/*.c")
list(REMOVE_ITEM ALL_FILES "lua.c")

# 生成链接库
add_library (lua53 ${ALL_FILES})
