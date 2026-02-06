---@meta

--- universe API
---@class universe
universe = {}

--- Returns the UUID for the given client connection, or `nil` if the client is unknown. ---
---@param clientId integer
---@return Optional<string>
function universe.uuidForClient(clientId) end

--- Returns a list of all currently connected client IDs. ---
---@return integer[]
function universe.clientIds() end

--- Returns the number of connected clients. ---
---@return integer
function universe.numberOfClients() end

--- Returns whether the specified client ID is currently connected. ---
---@param clientId integer
---@return boolean
function universe.isConnectedClient(clientId) end

--- Returns the nickname associated with the client. ---
---@param clientId integer
---@return string
function universe.clientNick(clientId) end

--- Returns the client ID for the supplied nickname if that player is connected. ---
---@param nick string
---@return Optional<integer>
function universe.findNick(nick) end

--- Sends a server broadcast message to every connected client. ---
---@param message string
function universe.adminBroadcast(message) end

--- Sends a direct whisper to the specified client. ---
---@param clientId integer
---@param message string
function universe.adminWhisper(clientId, message) end

--- Returns `true` if the client is flagged as a server administrator. ---
---@param clientId integer
---@return boolean
function universe.isAdmin(clientId) end

--- Returns whether the client currently has PVP enabled. ---
---@param clientId integer
---@return boolean
function universe.isPvp(clientId) end

--- Sets or clears the PVP flag for the client. When `enabled` is omitted the flag defaults to `true`. ---
---@param clientId integer
---@param enabled boolean
function universe.setPvp(clientId, enabled) end

--- Returns `true` if the world identified by the string world ID is currently loaded on the server. ---
---@param worldId string
---@return boolean
function universe.isWorldActive(worldId) end

--- Returns a list of world IDs for every active world. ---
---@return string[]
function universe.activeWorlds() end

--- Queues a remote message to the specified world. `messageName` is dispatched to scripts running in that world and any additional arguments are forwarded as JSON values. ---
---@param worldId string
---@param messageName string
---@param args Json
---@return RpcPromise<Json>
function universe.sendWorldMessage(worldId, messageName, args) end

--- Sends a raw network packet to the specified client. See `world.sendPacket` for details on constructing packet payloads. ---
---@param clientId integer
---@param packetType string
---@param packetData Json
---@return boolean
function universe.sendPacket(clientId, packetType, packetData) end

--- Returns the world ID for the world that the client currently occupies. ---
---@param clientId integer
---@return string
function universe.clientWorld(clientId) end

--- Disconnects the client, optionally providing a disconnect reason. ---
---@param clientId integer
---@param reason string
function universe.disconnectClient(clientId, reason) end

--- Bans a client by connection ID. The `banIp` and `banUuid` parameters control which identifiers are recorded; `timeout` (in seconds) performs a temporary ban when provided.
---@param clientId integer
---@param reason string
---@param banIp boolean
---@param banUuid boolean
---@param timeout number
function universe.banClient(clientId, reason, banIp, banUuid, timeout) end
