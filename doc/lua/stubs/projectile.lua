---@meta

--- projectile API
---@class projectile
projectile = {}

--- Returns the value for the specified config parameter. If there is no value set, returns the default. ---
---@param parameter string
---@param default Json
---@return Json
function projectile.getParameter(parameter, default) end

--- Destroys the projectile. ---
function projectile.die() end

--- Returns the entity id of the projectile's source entity, or `nil` if no source entity is set. ---
---@return integer
function projectile.sourceEntity() end

--- Returns the projectile's power multiplier. ---
---@return number
function projectile.powerMultiplier() end

--- Returns the projectile's power (damage). ---
---@return number
function projectile.power() end

--- Sets the projectile's power (damage). ---
---@param power number
function projectile.setPower(power) end

--- Returns the projectile's current remaining time to live. ---
---@return number
function projectile.timeToLive() end

--- Sets the projectile's current remaining time to live. Altering the time to live may cause visual disparity between the projectile's master and slave entities. ---
---@param timeToLive number
function projectile.setTimeToLive(timeToLive) end

--- Returns `true` if the projectile has collided and `false` otherwise. ---
---@return boolean
function projectile.collision() end

--- Immediately performs the specified action. Action should be specified in a format identical to a single entry in e.g. actionOnReap in the projectile's configuration. This function will not properly perform rendering actions as they will not be networked. --- #### 'void' projectile.setReferenceVelocity(std::optional<`Vec2F`> velocity) Sets the projectile's reference velocity (a base velocity to which movement is relative)
---@param action Json
function projectile.processAction(action) end
