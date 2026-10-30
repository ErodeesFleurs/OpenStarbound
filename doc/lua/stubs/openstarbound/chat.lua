---@meta

--- chat API
---@class chat
chat = {}

--- Sends a new message in the chat. Optionally specifies the modeName: - `"Broadcast"`: Global chat. Default value. - `"Local"`: Chat within the current planet. - `"Party"`: Chat within the current party. If `speak` is false, the chat bubble will not appear above the player. If `data` is provided, it will be sent as a JSON object with the message. This can be used to send custom data with the message. ---
---@param message string
---@param modeName? string
---@param speak? boolean
---@param data? JsonObject
function chat.send(message, modeName, speak, data) end

--- Executes a command and returns synchronous strings without displaying them. Extra arguments cannot grant clipboard authorization.
---@param command string
---@return string[]
function chat.command(command) end

--- Adds a local display/log message, never sends a packet or authenticates the sender.
--- fromConnection overrides local origin metadata (default client ID; 0 for local radio server-origin records).
---@param text string
---@param config? {mode?:string, channelName?:string, fromNick?:string, portrait?:string, showPane?:boolean, fromConnection?:integer}
function chat.addMessage(text, config) end

--- Returns the current chat input text. ---
---@return string
function chat.input() end

--- Sets the current chat input text. If `moveCursor` is true, the cursor will be moved to the end of the text. Returns true if the input was set successfully, false otherwise. ---
---@param text string
---@param moveCursor? boolean
---@return boolean
function chat.setInput(text, moveCursor) end

--- Clears received messages, not input; removes the newest count messages or all when omitted.
---@param count? unsigned
function chat.clear(count) end

---@param args string
---@return Json ...
function chat.parseArguments(args) end
