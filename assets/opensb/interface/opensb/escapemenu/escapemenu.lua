function init()
  local build = sb.buildInfo()
  widget.setText("lblversion", "OpenStarbound - " .. build.version .. " (" .. build.architecture .. ")")
end

function returnToGame()
  pane.dismiss()
end

function showOptions()
  pane.dismiss()
  interface.displayRegisteredPane("Options")
end

function saveAndQuit()
  interface.returnToTitle()
  pane.dismiss()
end

function onAction(action, pressed)
  if action ~= "close" or not pressed then return false end
  pane.dismiss()
  return true
end
