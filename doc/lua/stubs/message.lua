---@meta

--- message API
---@class message
message = {}

--- Messages of the specified message type received by this script context will call the specified function. For a string message name, the first two handler arguments are the message name and a boolean indicating a local sender, followed by the message arguments. A configuration table can set `localOnly` and `passName`; `passName` defaults to false for the table form. Passing nil as the handler removes it.
---@param messageName string|{name: string, passName?: boolean, localOnly?: boolean}
---@param handler? LuaFunction
function message.setHandler(messageName, handler) end
