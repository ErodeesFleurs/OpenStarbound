---@meta

-- Engine globals (LuaEngine::create) and persistent script context tables.
---@type table
shared = {}
---@type table
storage = {}
---@type table
self = {}

--- Creates a JSON array or attaches the array metatable to an existing table.
---@generic T: table
---@param values? T
---@return T
function jarray(values) end

--- Creates a JSON object or attaches the object metatable to an existing table.
---@generic T: table
---@param values? T
---@return T
function jobject(values) end

---@param container table
---@param key any
function jremove(container, key) end

---@param container table
---@return integer
function jsize(container) end

---@param container table
---@param size integer
function jresize(container, size) end

-- TitleScreen installs widget callbacks under this name.
---@type widget
background = {}

-- /scripts/vec2.lua lives in the player's commercial packed.pak, not this repo.
---@class vec2
vec2 = {}
---@param a Vec2F
---@param b Vec2F|number
---@return Vec2F
function vec2.add(a, b) end
---@param a Vec2F
---@param b Vec2F|number
---@return Vec2F
function vec2.sub(a, b) end
---@param a Vec2F
---@param b Vec2F|number
---@return Vec2F
function vec2.mul(a, b) end

-- StarVoiceLuaBindings.cpp; available in client scripts and script panes.
---@class voice
voice = {}
---@return string[]
function voice.devices() end
---@return JsonObject
function voice.getSettings() end
---@param settings JsonObject
function voice.mergeSettings(settings) end
---@param speakerId integer
---@param muted boolean
function voice.setSpeakerMuted(speakerId, muted) end
---@param speakerId integer
---@return boolean
function voice.speakerMuted(speakerId) end
---@param speakerId integer
---@param volume number
function voice.setSpeakerVolume(speakerId, volume) end
---@param speakerId integer
---@return number
function voice.speakerVolume(speakerId) end
---@param speakerId integer
---@return Vec2F
function voice.speakerPosition(speakerId) end
---@param speakerId? integer
---@return JsonObject
function voice.speaker(speakerId) end
---@param onlyPlaying? boolean
---@return JsonObject[]
function voice.speakers(onlyPlaying) end
