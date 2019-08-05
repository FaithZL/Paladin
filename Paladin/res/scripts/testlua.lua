local a
function main()
    print(1 .. a)
end

local function traceback()
    for level = 1, math.huge do
        local info = debug.getinfo(level);
        if not info then break end
        if info.what == "C" then
            print(level,"C function");
        else
            print(string.format("[%s]:%d",info.short_src,info.currentline));
        end
    end
end

xpcall(main, function(msg)
    print("Error: " .. msg .. "\n" .. "Stack: " .. debug.traceback())
end)
