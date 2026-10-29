require "/interface/opensb/ui/manager.lua"
require "/interface/opensb/codex/unread.lua"
require "/interface/opensb/questdialogs/pending.lua"

local manager

function init()
  manager = UiManager.new(storage, root.assetJson("/interface.config:panes"), interface.session())
  manager:restore()
end

function enqueue(id, event, payload)
  manager:enqueue(id, event, payload)
end

function dispatch(id, method, arguments)
  return manager:dispatch(id, method, arguments)
end

function open(id)
  manager:flush()
  return manager:open(id)
end

function toggle(id)
  manager:flush()
  return manager:toggle(id)
end

function input(event)
  return manager:input(event)
end
function flush()
  manager:flush()
end


function update(dt)
  manager:update()
  QuestDialogs.update(manager)
  CodexUnread.update(manager)
end

function suspend(session)
  manager:suspend(session)
end

function shutdown()
  if manager then manager:shutdown() end
end
