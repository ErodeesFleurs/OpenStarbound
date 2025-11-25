-- Liquid Melt Tech
-- Press the activate key to melt into the ground like a liquid and reform at the mouse cursor position

function init()
  self.meltDuration = config.getParameter("meltDuration", 0.5)
  self.reformDuration = config.getParameter("reformDuration", 0.5)
  self.cooldown = config.getParameter("cooldown", 2.0)
  self.maxTeleportDistance = config.getParameter("maxTeleportDistance", 30.0)
  self.energyCost = config.getParameter("energyCost", 50.0)
  
  self.state = "idle"
  self.timer = 0
  self.cooldownTimer = 0
  self.startPosition = nil
  self.targetPosition = nil
  self.originalDirectives = nil
end

function uninit()
  -- Reset player state when tech is unloaded
  tech.setParentHidden(false)
  tech.setParentDirectives()
  tech.setToolUsageSuppressed(false)
end

function update(args)
  local dt = args.dt
  
  -- Handle cooldown
  if self.cooldownTimer > 0 then
    self.cooldownTimer = math.max(0, self.cooldownTimer - dt)
  end
  
  if self.state == "idle" then
    -- Check for activation (special1 key bound to tech activation)
    if args.moves["special1"] and self.cooldownTimer <= 0 then
      -- Check if player has enough energy
      if status.overConsumeResource("energy", self.energyCost) then
        startMelt(args)
      end
    end
  elseif self.state == "melting" then
    updateMelting(args, dt)
  elseif self.state == "reforming" then
    updateReforming(args, dt)
  end
end

function startMelt(args)
  self.state = "melting"
  self.timer = 0
  self.startPosition = mcontroller.position()
  
  -- Calculate target position based on mouse cursor
  local aimPos = tech.aimPosition()
  self.targetPosition = findNearestGround(aimPos)
  
  -- If no valid ground found or target too far, use current position
  if not self.targetPosition then
    self.targetPosition = self.startPosition
  else
    -- Limit teleport distance
    local distanceVec = world.distance(self.targetPosition, self.startPosition)
    local distance = vec2.mag(distanceVec)
    if distance > self.maxTeleportDistance then
      local direction = world.distance(self.targetPosition, self.startPosition)
      direction = vec2.norm(direction)
      self.targetPosition = vec2.add(self.startPosition, vec2.mul(direction, self.maxTeleportDistance))
      self.targetPosition = findNearestGround(self.targetPosition) or self.startPosition
    end
  end
  
  -- Suppress tool usage during teleport
  tech.setToolUsageSuppressed(true)
  
  -- Stop movement
  mcontroller.setVelocity({0, 0})
  
  -- Play melt animation
  animator.setAnimationState("meltState", "melting")
  animator.playSound("meltSound")
end

function updateMelting(args, dt)
  self.timer = self.timer + dt
  local progress = math.min(self.timer / self.meltDuration, 1.0)
  
  -- Keep player stationary during melt
  mcontroller.setVelocity({0, 0})
  
  -- Apply liquid-like visual effect - squash and fade down
  local squashAmount = 1.0 - progress * 0.8  -- Squash vertically
  local stretchAmount = 1.0 + progress * 0.5  -- Stretch horizontally
  local fadeAmount = 1.0 - progress * 0.7  -- Fade out slightly
  
  -- Apply visual directives for melting effect
  local fadeValue = math.floor(fadeAmount * 255)
  local hexFade = string.format("%02x", fadeValue)
  tech.setParentDirectives("?multiply=6699ff" .. hexFade .. "?scalenearest=" .. stretchAmount .. ";" .. squashAmount)
  
  -- Move player downward as they "melt"
  local yOffset = -progress * 1.5
  tech.setParentOffset({0, yOffset})
  
  -- Spawn melting particles
  spawnMeltParticles(progress)
  
  if progress >= 1.0 then
    -- Melt complete - teleport to target
    tech.setParentHidden(true)
    mcontroller.setPosition(self.targetPosition)
    
    -- Start reforming
    self.state = "reforming"
    self.timer = 0
    animator.setAnimationState("meltState", "reforming")
    animator.playSound("reformSound")
  end
end

function updateReforming(args, dt)
  self.timer = self.timer + dt
  local progress = math.min(self.timer / self.reformDuration, 1.0)
  
  -- Keep player stationary during reform
  mcontroller.setVelocity({0, 0})
  
  if progress < 0.2 then
    -- Still hidden at start
    tech.setParentHidden(true)
  else
    -- Start showing player
    tech.setParentHidden(false)
    
    -- Apply visual effect - unsquash and fade in
    local adjustedProgress = (progress - 0.2) / 0.8
    local squashAmount = 0.2 + adjustedProgress * 0.8  -- Unsquash from flat
    local stretchAmount = 1.5 - adjustedProgress * 0.5  -- Unstretch
    local fadeAmount = 0.3 + adjustedProgress * 0.7  -- Fade in
    
    local fadeValue = math.floor(fadeAmount * 255)
    local hexFade = string.format("%02x", fadeValue)
    tech.setParentDirectives("?multiply=6699ff" .. hexFade .. "?scalenearest=" .. stretchAmount .. ";" .. squashAmount)
    
    -- Rise from ground
    local yOffset = -(1.0 - adjustedProgress) * 1.5
    tech.setParentOffset({0, yOffset})
    
    -- Spawn reforming particles
    spawnReformParticles(adjustedProgress)
  end
  
  if progress >= 1.0 then
    -- Reform complete
    completeReform()
  end
end

function completeReform()
  self.state = "idle"
  self.timer = 0
  self.cooldownTimer = self.cooldown
  
  -- Reset visual effects
  tech.setParentHidden(false)
  tech.setParentDirectives()
  tech.setParentOffset({0, 0})
  tech.setToolUsageSuppressed(false)
  
  animator.setAnimationState("meltState", "idle")
end

function findNearestGround(position)
  -- Search downward for ground
  local searchDistance = 50
  local startPoint = {position[1], position[2] + 2}  -- Start slightly above
  local endPoint = {position[1], position[2] - searchDistance}
  
  local collisionPoint = world.lineTileCollisionPoint(startPoint, endPoint, {"Block", "Dynamic", "Platform"})
  
  if collisionPoint then
    -- Return position slightly above the ground
    return {collisionPoint[1][1], collisionPoint[1][2] + 2.5}
  end
  
  -- If no ground below, try to find ground at the exact position
  if not world.pointTileCollision(position, {"Block", "Dynamic"}) then
    return {position[1], position[2]}
  end
  
  -- Try to find an open spot nearby
  for yOff = 0, 10 do
    local testPos = {position[1], position[2] + yOff}
    if not world.pointTileCollision(testPos, {"Block", "Dynamic"}) then
      return testPos
    end
  end
  
  return nil
end

function spawnMeltParticles(progress)
  if math.random() < 0.4 then
    animator.burstParticleEmitter("meltParticles")
  end
end

function spawnReformParticles(progress)
  if math.random() < 0.5 then
    animator.burstParticleEmitter("reformParticles")
  end
end

-- Vector utility functions
vec2 = {}

function vec2.add(a, b)
  return {a[1] + b[1], a[2] + b[2]}
end

function vec2.sub(a, b)
  return {a[1] - b[1], a[2] - b[2]}
end

function vec2.mul(v, s)
  return {v[1] * s, v[2] * s}
end

function vec2.mag(v)
  return math.sqrt(v[1] * v[1] + v[2] * v[2])
end

function vec2.norm(v)
  local m = vec2.mag(v)
  if m > 0 then
    return {v[1] / m, v[2] / m}
  end
  return {0, 0}
end
