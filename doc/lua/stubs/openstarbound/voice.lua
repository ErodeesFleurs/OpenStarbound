---@meta

--- voice API
---@class voice
voice = {}

--- @return string[]
function voice.devices() end

--- @return table<string, any>
function voice.getSettings() end

--- @param settings table<string, any>
function voice.mergeSettings(settings) end

--- @param speakerId integer
--- @param muted boolean
function voice.setSpeakerMuted(speakerId, muted) end

--- @param speakerId integer
--- @return boolean
function voice.speakerMuted(speakerId) end

--- @param speakerId integer
--- @param volume number
function voice.setSpeakerVolume(speakerId, volume) end

--- @param speakerId integer
--- @return number
function voice.speakerVolume(speakerId) end

--- @param speakerId integer
--- @return Vec2F
function voice.speakerPosition(speakerId) end

--- @param speakerId integer?
--- @return JsonObject
function voice.speaker(speakerId) end

--- @param onlyPlaying boolean?
--- @return table<string, any>[]
function voice.speakers(onlyPlaying) end
