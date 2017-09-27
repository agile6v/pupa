
local http = require "resty.http"

local _M = {}

local function heartbeat(host, i, interval)
    while (true)
    do
        ngx.log(ngx.INFO, ">>> thread ", i, ", master : ", host)
        local httpc = http.new()
        local res, err = httpc:request_uri(host, {
            method = "POST",
            body = "a=1&b=2",
            headers = {["Content-Type"] = "application/x-www-form-urlencoded"}
        })

        if not res then
            ngx.log(ngx.INFO, "failed to request: ", err)
            return
        end

        ngx.log(ngx.INFO, "heartbeat response: ", res.status, ", body: ", res.body)

        ngx.sleep(interval)
    end
end

function _M.start(hosts, interval)
    local handler
    handler = function (premature)
        if premature then
            return
        end

        for i, host in pairs(hosts) do
            -- start heartbeart thread
            ngx.log(ngx.INFO, "start thread ", i, " : connect to master - ", host)
            ngx.thread.spawn(heartbeat, host, i, interval)
        end

        ngx.log(ngx.INFO, "start all threads to complete")
    end

    if 0 == ngx.worker.id() then
        local ok, err = ngx.timer.at(5, handler)
        if not ok then
            ngx.log(ngx.ERR, "failed to create the timer: ", err)
            return
        end
    end
end

return _M
