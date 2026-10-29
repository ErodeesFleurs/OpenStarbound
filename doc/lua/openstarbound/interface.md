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
Binds a native registered pane or a dynamically created string-ID pane to its pane/widget callbacks. Returns `nil` if no instance exists. Native identifiers are listed below; bundled Lua registrations use their configured string identifiers, such as `"chat"` and `"songbook"`.
<details><summary><b>Panes</b></summary>
EscapeDialog<br>
Inventory<br>
Cockpit<br>
Tech<br>
Ai<br>
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
World-scoped events and instances are discarded at world or player changes.
Player-scoped instances survive world changes but are discarded when their
player changes or becomes unavailable; session instances retain standard
`storage` through capability rebinding.

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

### Bundled bookmark editor

The `"bookmarkEditor"` world-scoped `"ModalWindow"` registration loads
`/interface/opensb/bookmarks/bookmarkeditor.config`. Its `edit` event accepts
`{bookmark, sourceEntityId, sourceUniqueId?}`; bookmark data uses the canonical
JSON returned by `player.teleportBookmarks()`. Native flag/teleporter
interactions and teleport-list edit buttons deliver this event through the
generic interface bridge, without a native editor class.

Lua owns new/edit mode, stored-name lookup, icon/target labels, focused name
editing, empty-name fallback, confirmation, deletion and cancellation.
Identity is the bookmark's target, never its name or current list index.
Persistence uses the existing player bookmark APIs. The original asset's
textbox naming constraints remain in effect.

The editor preserves its draft across script reload, but closes when its
interaction source disappears or its player/world context changes. World-scoped
instances and queued events are discarded on either player or world generation
changes. Teleport eligibility, reachability and warp execution remain native.

### Bundled ordinary popup

The `"popup"` registration uses the ordinary `"Window"` layer and `"player"`
scope. Its `show` event accepts `{message, title?, subtitle?, sound?}` through
the generic event bridge; native `ShowPopup` interactions use this same path.
Lua uses the original `/interface/windowconfig/popup.config` layout and sound
list, and the existing `pane.setTitle(title, subtitle)` for native title drawing.

Missing `sound` chooses a configured random default, `""` is silent, and a
nonempty value overrides the default. Each show replaces the current content
and plays its sound once; script reload restores content without replaying it.
Updating an already displayed popup does not raise it; closing and reopening
uses normal window stacking. World transitions retain the popup, while player
changes and interface teardown discard it.

This registration does not handle confirmation, join requests or HTTP trust.

### Bundled songbook

The `"songbook"` registration loads
`/interface/opensb/songbook/songbook.config` on the `"Window"` layer with
`"player"` scope and `"retain": true`. Native instrument interactions open it
through the generic interface bridge. It reuses the original songbook widget
layout; no native songbook window class is involved.

Lua owns the `/songs/` ABC catalog, ASCII-case-insensitive sorting, literal
search and highlighting of every match. Rows carry original resource paths,
not filtered indices. Play submits `{resource, abc}` with the original asset
bytes and the entered band to the current player's `songbook.play`; no selection
leaves the window open. Closing the window never stops playback.

Ordinary dismissal retains the actual widgets, including search, band and
selection. Live reload rescans and clears selection without replaying a song.
World transitions retain the instance; player changes and interface shutdown
release it. Instrument keepalive, ABC playback, audio and network synchronization
remain native services.

Native `/swap` may reuse the current Songbook and continue its playback;
discarding player-scoped UI does not add an implicit `songbook.stop()`.

### Bundled codex

The `"codex"` registration loads `/interface/opensb/codex/codex.config` as a
player-scoped, retained `"Window"`, reusing the original species tabs, list,
labels and paging buttons. Lua owns filtering, selection and paging; rows carry
stable book IDs, including when identical titles or read-driven sorting change
their positions. Empty books have no enabled page navigation.

The main-bar button uses the generic manager toggle. The
`globalActions.InterfaceCodex` registration opens or toggles the window even
before an instance exists, without overriding native keyboard capture or modal
barriers. Ordinary dismissal preserves species, book and page; world changes
retain the window, while player changes and interface teardown destroy it.

`/interface/opensb/codex/unread.lua` owns automatic unread opening. It waits while
a modal window is present, selects the native first unread ID, and acknowledges
it through `player.markCodexRead`. Reload does not replay already acknowledged
books. Content parsing, unlocks, read flags and ordering remain native player
domain services; known content follows the player's held Codex objects.

### Bundled quest tracker

The canonical `"QuestTracker"` dynamic registration loads
`/interface/opensb/questtracker/questtracker.config` as an automatically opened,
session-scoped `"Hud"`. The public identifier keeps its original spelling, but
there is no native tracker window class or native registration.

Lua reads one `player.currentQuestView()` per update and owns objective
formatting, collapsed first-incomplete selection, expanded layout, progress
frames and compass animation. Expanded mode lists every objective; when all
are complete, collapsed mode shows the first. The compass preserves the native
float32 spring and advances per update, not by `dt`.

The original named widgets and negative-coordinate layout remain available.
Only a left click on the active background toggles expansion; progress and
compass extensions do not consume unrelated pointer input. Generic drawing
offset and scissoring helpers allow these extensions outside the zero-sized
anchored root, without HUD-specific host fields.

With no effective quest the pane and its children are hidden, not destroyed.
The instance keeps ticking, so a new eligible quest reappears automatically.
Expansion and compass state survive reload and player/world transitions;
current player services are centrally rebound. Cached pane/widget handles
remain usable through these transitions. `isDisplayed()` reports retained
display membership; use `isVisible()` or child activity to inspect visibility.
Interface teardown ends the instance and its handles.

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
* `interface.paneLayerOccupied(layer)` reports whether the native layer has
  a top pane, including native windows. Layer names use the same spelling as
  `createPane`; invalid names raise an error. It does not create an instance.
* `interface.session()` returns `hasPlayer`, `inWorld`, `playerGeneration` and
  `worldGeneration`. Generations identify changes, not world or player IDs.
  Use `hasPlayer`/`inWorld` to test availability: unavailable session service
  groups are explicitly empty tables, not inherited old callbacks or `nil`.

Use the Lua manager for normal registration/lifetime changes; the low-level
bridge exists for allocation, native stack access and synchronous queries.
