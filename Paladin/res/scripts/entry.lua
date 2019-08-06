
local function main()
    local path = package.path
    
    while path
    do
        local index1 = string.find(path, ";")
        if index1 then
            print(string.sub(path, 1, index1))
            path = string.sub(path, index1 + 1)
        else
            break
        end
    end
    
    local info = debug.getinfo(1, "S") -- 第二个参数 "S" 表示仅返回 source,short_src等字段， 其他还可以 "n", "f", "I", "L"等 返回不同的字段信息
    local path = info.source
    path = string.sub(path, 2, -1) -- 去掉开头的"@"
    path = string.match(path, "^.*/")
    print("cur path = ", path)
    -- local luatest = require("luatest")
    --    luatest.who()

    print("LI_GetCWD() = ", LI_GetCWD())
end

xpcall(main, function(msg)
    print("Error: " .. msg .. "\n" .. "Stack: " .. debug.traceback())
end)

   