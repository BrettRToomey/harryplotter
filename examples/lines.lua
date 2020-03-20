local hp = require "harryplotter"

local x = {}
local y = {}

for i=1, 100 do
    x[i] = i
    y[i] = i * 20
end

local plot = hp.new {
    title = "Example plot",
    xaxis = "Time",
    yaxis = "Rate",
}

plot:line(x, y)
plot:line(y, x)
plot:line(y, y)

plot:show()
