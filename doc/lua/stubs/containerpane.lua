---@meta

--- pane API
---@class pane
pane = {}

--- Returns the entity id of the container that this pane is connected to. ---
---@return integer
function pane.containerEntityId() end

--- Returns the entity id of the player that opened this pane. ---
---@return integer
function pane.playerEntityId() end

--- Closes the pane.
function pane.dismiss() end
