---@meta

--- stagehand API
---@class stagehand
stagehand = {}

--- Returns the stagehand's entity id. Identical to entity.id(), so use that instead. ---
---@return integer
function stagehand.id() end

--- Returns the stagehand's position. This is identical to entity.position(), so use that instead. ---
---@return Vec2F
function stagehand.position() end

--- Moves the stagehand to the specified position. ---
---@param position Vec2F
function stagehand.setPosition(position) end

--- Destroys the stagehand. ---
function stagehand.die() end

--- Returns the stagehand's type name. ---
---@return string
function stagehand.typeName() end

--- Sets the stagehand's unique entity id, or clears it if unspecified.
---@param uniqueId string
function stagehand.setUniqueId(uniqueId) end
