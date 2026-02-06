---@meta

--- songbook API
---@class songbook
songbook = {}

--- Returns a string containing the instrumentalist's current band. Defaults to `""`. ---
---@return string
function songbook.band() end

--- Returns the notes of the song currently being played. ---
---@return string
function songbook.song() end

--- TODO: Add description for songbook.keepAlive ---
---@param duration number
function songbook.keepAlive(duration) end

--- Causes the instrumentalist to stop playing. ---
function songbook.stop() end

--- Returns whether the instrumentalist is playing a song. ---
---@return boolean
function songbook.active() end

--- Returns whether the instrumentalist is currently performing part of a song. ---
---@return boolean
function songbook.instrumentPlaying() end

--- TODO: Add description for songbook.play ---
---@param song Json
function songbook.play(song) end

--- Returns the type of instrument last played, as determined by the instrument's `kind` value. Defaults to `""`.
---@return string
function songbook.instrument() end
