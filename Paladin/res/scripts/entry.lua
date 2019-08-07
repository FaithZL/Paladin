package.path = LI_GetCWD() .. "/?.lua;" .. package.path

local function main()
    local path = package.path
    
    while true
    do
        local index1 = string.find(path, ";")
        if index1 then
            print(string.sub(path, 1, index1))
            path = string.sub(path, index1 + 1)
        else
        	print(path)
            break
        end
    end
    
    local luatest = require("res/scripts/testlua")
    luatest.who()
end

xpcall(main, function(msg)
    print("Error: " .. msg .. "\n" .. "Stack: " .. debug.traceback())
end)

