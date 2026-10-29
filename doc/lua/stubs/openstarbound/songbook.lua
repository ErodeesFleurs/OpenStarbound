---@meta

--- songbook API
---@class songbook
songbook = {}

--- Returns the native playback time-source identifier; empty before playback. ---
---@return string
function songbook.band() end

--- Returns the submitted song JSON, normally {resource, abc}, or null after stop. ---
---@return Json
function songbook.song() end

--- Updates instrument kind/position and refreshes native playback keepalive. Instrument scripts normally maintain this. ---
---@param instrument string
---@param position Vec2F
function songbook.keepAlive(instrument, position) end

--- Causes the instrumentalist to stop playing. ---
function songbook.stop() end

--- Returns whether the instrumentalist is playing a song. ---
---@return boolean
function songbook.active() end

--- Returns whether the instrumentalist is currently performing part of a song. ---
---@return boolean
function songbook.instrumentPlaying() end

--- Starts native playback of song JSON, normally {resource, abc}. A nonempty band uses a shared time source; "" selects a local one. ---
---@param song Json
---@param band string
function songbook.play(song, band) end

--- Returns the type of instrument last played, as determined by the instrument's `kind` value. Defaults to `""`.
---@return string
function songbook.instrument() end
