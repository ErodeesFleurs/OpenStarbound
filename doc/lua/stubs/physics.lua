---@meta

--- physics API
---@class physics
physics = {}

--- Enables or disables the specified physics force region. ---
---@param force string
---@param enabled boolean
function physics.setForceEnabled(force, enabled) end

--- Moves the specified physics collision region to the specified position. ---
---@param collision string
---@param position Vec2F
function physics.setCollisionPosition(collision, position) end

--- Enables or disables the specified physics collision region.
---@param collision string
---@param enabled boolean
function physics.setCollisionEnabled(collision, enabled) end
