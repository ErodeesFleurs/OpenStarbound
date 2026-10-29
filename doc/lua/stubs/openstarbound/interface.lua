---@meta

--- interface API
---@class interface
interface = {}

--- Queues a message popup at the bottom of the screen with an optional **cooldown** and **springState**. ---
---@param message string
---@param cooldown? number
---@param springState? number
function interface.queueMessage(message, cooldown, springState) end

--- Sets the HUD's visibility. ---
---@param visible boolean
function interface.setHudVisible(visible) end

--- Returns the HUD's visibility. ---
---@return boolean
function interface.hudVisible() end

--- Binds native registered IDs or dynamic Lua pane IDs; returns nil without an instance.
---@param paneName string
---@return LuaCallbacks?
function interface.bindRegisteredPane(paneName) end

--- Displays a registered pane. ---
---@param paneName string
function interface.displayRegisteredPane(paneName) end

--- Binds the canvas widget on the main interface with the specified name as userdata for easy access. The `CanvasWidget` has the same methods as described in `widget.md`. - **ignoreInterfaceScale** is used to ignore the current interface scaling and bind the canvas with the screen size. ---
---@param name string
---@param ignoreInterfaceScale? boolean
---@return CanvasWidget
function interface.bindCanvas(name, ignoreInterfaceScale) end

--- Returns the scale used for interfaces.
---@return number
function interface.scale() end

--- Internal native item-tooltip fallback service; uses the current UI player.
--- Engine-produced opaque handles only, not item descriptors or createTooltip hook results.
---@param item userdata
---@return userdata
function interface.itemTooltip(item) end

--- Immediately loads a resolved client UI cinematic; native errors propagate, unlike deferred player playback.
---@param definition Json
function interface.playCinematic(definition) end

--- Whether the client cinematic overlay has not completed; independent of player/world attachment.
---@return boolean
function interface.cinematicActive() end

--- Constructs an undisplayed generic script pane; Lua manages its lifetime.
---@param id string
---@param config Json
---@param layer string
function interface.createPane(id, config, layer) end

---@param id string
function interface.destroyPane(id) end

--- Calls an existing script pane; does not open it.
---@param id string
---@param method string
---@param arguments? Json[]
---@return Json?
function interface.callPane(id, method, arguments) end

--- Enqueues an event; prefer the shared ui Lua helpers for instance policy.
---@param id string
---@param event string
---@param payload Json
function interface.sendPaneEvent(id, event, payload) end

--- Checks reserved/native naming, not whether a dynamic instance already exists.
---@param id string
---@return boolean
function interface.paneIdAvailable(id) end

--- Returns dynamic pane IDs in native capture/layer/stack/modal input priority.
---@return string[]
function interface.paneInputTargets() end

--- Reports a top pane on the named native layer, including native windows; invalid layer names raise an error.
---@param layer string
---@return boolean
function interface.paneLayerOccupied(layer) end

---@return {hasPlayer:boolean, inWorld:boolean, playerGeneration:integer, worldGeneration:integer}
function interface.session() end

--- Available to client UI scripts; metadata is distinct from instance storage.
---@class client
client = {}

---@class ClientMetadata
---@field get fun(fallback?:Json):Json
---@field set fun(value:Json)

---@param namespace string
---@return ClientMetadata
function client.storage(namespace) end

--- Loaded by require "/interface/opensb/ui/api.lua"; requests are asynchronous.
---@class ui
ui = {}

---@param id string
---@param definition {config:Json|string, layer?:string, scope?:string, open?:boolean, retain?:boolean, order?:number, actions?:table<string,string>, globalActions?:table<string,string>, events?:table<string,string>}
function ui.register(id, definition) end

---@param id string
function ui.open(id) end

--- Ordinary dismissal when displayed (honoring retain), otherwise opens the registered instance.
---@param id string
function ui.toggle(id) end

---@param id string
function ui.close(id) end

---@param id string
---@param event string
---@param payload Json
function ui.send(id, event, payload) end
