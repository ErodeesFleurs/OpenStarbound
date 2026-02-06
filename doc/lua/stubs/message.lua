---@meta

--- message API
---@class message
message = {}

--- Messages of the specified message type received by this script context will call the specified function. The first two arguments passed to the handler function will be the `String` messageName and a `bool` indicating whether the message is from a local entity, followed by any arguments sent with the message.
---@param messageName string|table
---@param handler function
function message.setHandler(messageName, handler) end
