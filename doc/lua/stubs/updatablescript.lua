---@meta

--- script API
---@class script
script = {}

--- Sets the script's update delta. ---
---@param dt integer
function script.setUpdateDelta(dt) end

--- Returns the duration in seconds between periodic updates to the script.
---@return number
function script.updateDt() end
