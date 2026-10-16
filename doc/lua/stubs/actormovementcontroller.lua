---@meta

--- mcontroller API

--- Returns a rect containing the entire collision of the movement controller, in local coordinates. ---
---@return RectF
function mcontroller.boundBox() end

--- Anchors the movement controller to an anchorable entity at the given anchor index. ---
---@param anchorableEntity EntityId
function mcontroller.setAnchorState(anchorableEntity) end

--- Reset the anchor state. ---
function mcontroller.resetAnchorState() end

--- Returns the base movement parameters. ---
---@return ActorMovementParameters
function mcontroller.baseParameters() end

--- Returns whether the actor movement controller is currently walking. ---
---@return boolean
function mcontroller.walking() end

--- Returns whether the actor movement controller is currently running. ---
---@return boolean
function mcontroller.running() end

--- Returns the direction that the actor movement controller is currently moving in. -1 for left, 1 for right. ---
---@return number
function mcontroller.movingDirection() end

--- Returns the facing direction. -1 for left, 1 for right. ---
---@return number
function mcontroller.facingDirection() end

--- Returns whether the controller is currently crouching. ---
---@return boolean
function mcontroller.crouching() end

--- Returns whether the controller is currently flying. ---
---@return boolean
function mcontroller.flying() end

--- Returns whether the controller is currently falling. ---
---@return boolean
function mcontroller.falling() end

--- Returns whether the controller can currently jump. ---
---@return boolean
function mcontroller.canJump() end

--- Returns whether the controller is currently jumping. ---
---@return boolean
function mcontroller.jumping() end

--- Returns whether the controller is currently in a ground movement state. Movement controllers can be in ground movement even when onGround returns false. ---
---@return boolean
function mcontroller.groundMovement() end

--- Returns whether the controller is currently in liquid movement mode. --- ## controls The actor movement controller has a set of controls. Controls can be set anywhere and are accumulated and evaluated after all scripts are run. Controls will either override previously set controls, or combine them. Controls are either cleared before every script update, or can be set to be manually cleared. ---
---@return boolean
function mcontroller.liquidMovement() end

--- Rotates the controller. Each control adds to the previous one. ---
---@param rotation number
function mcontroller.controlRotation(rotation) end

--- Controls acceleration. Each control adds to the previous one. ---
---@param acceleration Vec2F
function mcontroller.controlAcceleration(acceleration) end

--- Controls force. Each control adds to the previous one. ---
---@param force Vec2F
function mcontroller.controlForce(force) end

--- Approaches the targetVelocity using the force provided. If the current velocity is higher than the provided targetVelocity, the targetVelocity will still be approached, effectively slowing down the entity. Each control overrides the previous one. ---
---@param targetVelocity Vec2F
---@param maxControlForce number
function mcontroller.controlApproachVelocity(targetVelocity, maxControlForce) end

--- Approaches the targetVelocity but only along the provided angle, not affecting velocity in the perpendicular axis. If positiveOnly, then it will not slow down the movementController if it is already moving faster than targetVelocity. Each control overrides the previous one. ---
---@param angle number
---@param targetVelocity number
---@param maxControlForce number
---@param positiveOnly boolean
function mcontroller.controlApproachVelocityAlongAngle(angle, targetVelocity, maxControlForce, positiveOnly) end

--- Approaches an X velocity. Same as using approachVelocityAlongAngle with angle 0. Each control overrides the previous one. ---
---@param targetVelocity number
---@param maxControlForce number
function mcontroller.controlApproachXVelocity(targetVelocity, maxControlForce) end

--- Approaches a Y velocity. Same as using approachVelocityAlongAngle with angle (Pi / 2). Each control overrides the previous one. ---
---@param targetVelocity number
---@param maxControlForce number
function mcontroller.controlApproachYVelocity(targetVelocity, maxControlForce) end

--- Changes movement parameters. Parameters are merged into the base parameters. Each control is merged into the previous one. ---
---@param parameters ActorMovementParameters
function mcontroller.controlParameters(parameters) end

--- Changes movement modifiers. Modifiers are merged into the base modifiers. Each control is merged into the previous one. ---
---@param modifiers ActorMovementModifiers
function mcontroller.controlModifiers(modifiers) end

--- Controls movement in a direction. Each control replaces the previous one. ---
---@param direction number
---@param run boolean
function mcontroller.controlMove(direction, run) end

--- Controls the facing direction. Each control replaces the previous one. ---
---@param direction number
function mcontroller.controlFace(direction) end

--- Controls dropping through platforms. ---
function mcontroller.controlDown() end

--- Controls crouching. ---
function mcontroller.controlCrouch() end

--- Controls starting a jump. Only has an effect if canJump is true. ---
---@param jump boolean
function mcontroller.controlJump(jump) end

--- Keeps holding jump. Will not trigger a new jump, and can be held in the air. ---
function mcontroller.controlHoldJump() end

--- Controls flying in the specified direction (or {0, 0} to stop) with the configured flightSpeed parameter. Each control overrides the previous one. ---
---@param direction Vec2F
function mcontroller.controlFly(direction) end

--- Returns whether the controller is currently set to auto clear controls before each script update. ---
---@return boolean
function mcontroller.autoClearControls() end

--- Set whether to automatically clear controls before each script update. ---
---@param enabled boolean
function mcontroller.setAutoClearControls(enabled) end

--- Manually clear all controls.
function mcontroller.clearControls() end
