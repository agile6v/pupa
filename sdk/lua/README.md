

#### Example

```lua
pupa = require 'pupa_lua'

err = pupa.init("pupa.store", 10, 2)
if err then
    print("error: ", err)
    return
end

local key = "Hello"
local value = "pupa"

-- pupa.set
local err = pupa.set(key, value)
if err then
    print("error: ", err)
    return
end

-- pupa.stats
local ret, err = pupa.stats()
if err then
    print("error: ", err)
    return
end

print(ret)

-- pupa.get
local ret, err = pupa.get(key)
if err then
    print("error: ", err)
    return
end

print("** Got ", key, " : ", ret)


-- pupa.delete
local err = pupa.delete(key)
if err then
    print("error: ", err)
    return
end

-- pupa.stats
local ret, err = pupa.stats()
if err then
    print("error: ", err)
    return
end

print(ret)

-- pupa.fini
err = pupa.fini()
if err then
    print("error: ", err)
    return
end
```