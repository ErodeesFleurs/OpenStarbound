# Interface

The `interface` table contains bindings which allow scripts to display a message at the bottom of the screen, among other things.

---

#### `void` interface.queueMessage(`String` message, [`float` cooldown, [`float` springState]])

Queues a message popup at the bottom of the screen with an optional **cooldown** and **springState**.

---

#### `void` interface.setHudVisible(`bool` visible)

Sets the HUD's visibility.

---

#### `bool` interface.hudVisible()

Returns the HUD's visibility.

---

#### `PaneId` interface.bindRegisteredPane(`String` paneName)
Binds a native registered pane or a dynamically created string-ID pane to its pane/widget callbacks. Returns `nil` if no instance exists. Native identifiers are listed below; the bundled Lua panes use the dynamic identifiers `"chat"` and `"status"`.
<details><summary><b>Panes</b></summary>
EscapeDialog<br>
Inventory<br>
Codex<br>
Cockpit<br>
Tech<br>
Songbook<br>
Ai<br>
Popup<br>
Confirmation<br>
JoinRequest<br>
Options<br>
QuestLog<br>
ActionBar<br>
TeamBar<br>
WireInterface<br>
PlanetText<br>
RadioMessagePopup<br>
CraftingPlain<br>
QuestTracker<br>
MmUpgrade<br>
Collections<br>
</details>

---

#### `void` interface.displayRegisteredPane(`String` paneName)

Displays an existing instance. For a Lua registration without an instance, requests opening through the Lua interface manager.

---

#### `CanvasWidget` interface.bindCanvas(`String` name, [`bool` ignoreInterfaceScale = false])

Binds the canvas widget on the main interface with the specified name as userdata for easy access. The `CanvasWidget` has the same methods as described in `widget.md`.

- **ignoreInterfaceScale** is used to ignore the current interface scaling and bind the canvas with the screen size.

---

#### `float` interface.scale()

Returns the scale used for interfaces.

## Lua-managed interface instances

The session controller loads `/interface.config:panes` using the shared Lua
manager. Registrations, opening/closing, session/world scope, event dispatch and
semantic action mapping are Lua policy; `BaseScriptPane` has no HUD-specific
configuration or state. See [script pane runtime](scriptpane.md).

Load the public Lua helper with:

```lua
require "/interface/opensb/ui/api.lua"

ui.register("example", {
  config = "/interface/example/example.config",
  layer = "Window",
  scope = "session",
  open = false
})
ui.open("example")
ui.send("example", "refresh", {reason = "externalChange"})
ui.close("example")
```

These helpers enqueue requests; they do not mutate the pane stack inside the
calling script. Native IDs and `"$ui"` are reserved. `ui.send` dispatches through
the registration's `events` map, or `onEvent(name, payload)` when unmapped.
Messages sent by handlers are deferred to a subsequent delivery boundary.
World-scoped events and instances are discarded at world changes; session
instances retain standard `storage` through capability rebinding.

### Bundled status-effect HUD

The `"status"` session-scoped `"Hud"` registration loads
`/interface/opensb/status/status.config` and its `status.lua` script. Lua owns
icon filtering, ordering, layout, duration masks and tooltip hit testing, using
the shared `BaseScriptPane` and Canvas widgets. Status application and lifetime
calculation remain in the native `status` service.

The script reads the existing `/interface.config` settings `statusIconPos`,
`statusIconShift` and `statusIconDarkenImage`. Iconless effects do not occupy
layout slots; mixed image sizes and overlapping icons are supported. Top-left
anchoring follows window resizing and interface scaling without a separate
screen-size tracker. All presentation state is derived from the current
`status.activeUniqueStatusEffectSummary()` and rebuilt on script initialization.
Patch the registered pane configuration or script to customize this HUD.

### Bundled breath HUD

The `"breath"` session instance loads `/interface/opensb/breath/breath.config`.
Lua owns the ten-block display, rounded visibility threshold and zero-breath
warning flash; resource regeneration, depletion and damage remain native.
The existing `breathPos` and `breathBarPos` settings remain supported.

The generic `"Background"` pane layer renders before notifications and other
native HUD elements, below the later World/Hud/window stack. This preserves
the original breath ordering without a dedicated native breath host.
The pane uses `interactive: false` so its screen-sized Canvas does not capture
input. Its physical-pixel Canvas preserves the original centering and integer
icon-step behavior at fractional interface scales.

### Low-level instance bridge

* `interface.createPane(id, config, layer)` constructs an undisplayed script
  pane from resolved JSON. `id` must be unused and not a reserved native ID.
  `layer` uses native names such as `"Hud"`, `"Window"` or `"ModalWindow"`.
* `interface.destroyPane(id)` dismisses and releases a dynamic instance.
  It does not destroy native registered panes. Instance storage ends here.
* `interface.callPane(id, method, arguments?)` synchronously invokes a Lua
  method on an existing script instance; JSON array arguments are spread
  positionally. Returns its JSON result or `nil` for a missing method.
  It neither opens nor creates instances.
* `interface.sendPaneEvent(id, event, payload)` enqueues a manager event.
  Prefer `ui.send` and the other helpers to constructing controller requests.
* `interface.paneIdAvailable(id)` checks whether an ID is permitted for Lua ownership (nonempty and not reserved/native); an existing dynamic instance may still use that ID.
* `interface.paneInputTargets()` returns dynamic IDs in current native input
  priority, including capture, layers, click-to-raise and modal barriers.
  The Lua manager uses this order rather than registration order for actions.
* `interface.session()` returns `hasPlayer`, `inWorld`, `playerGeneration` and
  `worldGeneration`. Generations identify changes, not world or player IDs.
  Use `hasPlayer`/`inWorld` to test availability: unavailable session service
  groups are explicitly empty tables, not inherited old callbacks or `nil`.

Use the Lua manager for normal registration/lifetime changes; the low-level
bridge exists for allocation, native stack access and synchronous queries.
