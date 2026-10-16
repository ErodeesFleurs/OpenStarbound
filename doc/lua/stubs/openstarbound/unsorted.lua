---@meta

--- Global functions

--- Compiles the provided **source** and returns it as a callable function. Syntax errors raise an error. - **name** is used for error messages, the default is `loadstring`. - **env** is used as the environment of the returned function, the default is `_ENV`.
---@param source string
---@param name? string
---@param env? LuaTable
---@return LuaFunction
function loadstring(source, name, env) end
