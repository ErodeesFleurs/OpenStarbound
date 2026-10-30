---@meta

--- widget API

--- Sets an additional native drawing/screen-geometry offset without changing the base getPosition value.
---@param widgetName string
---@param offset Vec2I
function widget.setDrawingOffset(widgetName, offset) end

--- Controls clipping to this widget's bounds; ancestor clipping and input policy remain unchanged.
---@param widgetName string
---@param enabled boolean
function widget.setScissoringEnabled(widgetName, enabled) end

--- Runs a native Layout update with zero elapsed time after geometry changes.
---@param widgetName string
function widget.refreshLayout(widgetName) end

--- Replaces native ImageWidget drawables; an empty array clears them.
---@param widgetName string
---@param drawables Json[]
---@param scale? Vec2F Optional transform of all drawable geometry.
function widget.setImageDrawables(widgetName, drawables, scale) end

--- Assigns a native display ItemBag; stacking rules apply without changing inventory.
---@param widgetName string
---@param items Json[]
---@param capacity? integer Defaults to visible slot count, not an implicit larger bag.
function widget.setItemGridItems(widgetName, items, capacity) end

--- Shares the actual native Item with an ItemSlotWidget; nil clears, no inventory move or JSON reconstruction.
---@param widgetName string
---@param item? NativeItemHandle
function widget.setItemSlotHandle(widgetName, item) end

--- Updates supplied display fields only; unspecified settings stay unchanged.
---@param widgetName string
---@param display {durability?:boolean, count?:boolean, secondaryIcon?:boolean, highlight?:boolean, countPosition?:Json}
function widget.setItemSlotDisplay(widgetName, display) end

--- Shares actual native PortraitEntity for live animation; nil clears.
---@param widgetName string
---@param entity? NativePortraitHandle
function widget.setPortraitEntity(widgetName, entity) end

--- Native Color mixing/rounding; amount defaults to 0 when mix is supplied.
---@param widgetName string
---@param color Color
---@param mix? Color
---@param amount? number
function widget.setProgressColor(widgetName, color, mix, amount) end

--- Sets ProgressWidget overlay image; empty string clears.
---@param widgetName string
---@param image string
function widget.setProgressOverlay(widgetName, image) end

--- Label rendered Unicode-codepoint limit, excluding escape commands; nil removes, stored text unchanged.
---@param widgetName string
---@param limit? integer
function widget.setTextCharLimit(widgetName, limit) end

--- Gets the hint text of a TextBoxWidget. ---
---@param widgetName string
---@return string
function widget.getHint(widgetName) end

--- Sets the hint text of a TextBoxWidget. ---
---@param widgetName string
---@param hint string
function widget.setHint(widgetName, hint) end

--- Gets the cursor position of a TextBoxWidget. ---
---@param widgetName string
---@return Int
function widget.getCursorPosition(widgetName) end

--- Sets the cursor position of a TextBoxWidget. --- ### SliderBarWidget helpers
---@param widgetName string
---@param cursorPosition number
function widget.setCursorPosition(widgetName, cursorPosition) end

--- Disable native drawing only, retaining text input and focus.
---@param widgetName string
---@param enabled boolean
function widget.setTextEditorRendering(widgetName, enabled) end

---@class TextEditorState
---@field text string
---@field displayText string
---@field cursor integer Zero-based Unicode codepoint index.
---@field anchor integer Zero-based Unicode codepoint index.
---@field selectionStart integer Inclusive codepoint index.
---@field selectionEnd integer Exclusive codepoint index.
---@field cursorOffset number Interface units relative to the text anchor, before scrolling.

---@param widgetName string
---@return TextEditorState?
function widget.getTextEditorState(widgetName) end

---@param widgetName string
---@param anchor integer
---@param cursor integer
function widget.setTextSelection(widgetName, anchor, cursor) end

--- Applies full native TextStyle to LabelWidget or TextBoxWidget.
---@param widgetName string
---@param style TextStyle
function widget.setTextStyle(widgetName, style) end

--- Sets the allowed range for a slider, optionally overriding the step used when the slider value is changed. The step defaults to `1` when omitted. --- ### ImageStretchWidget helpers
---@param widgetName string
---@param minimum number
---@param maximum number
---@param step number
function widget.setSliderRange(widgetName, minimum, maximum, step) end

--- Sets the full image set of a ImageStretchWidget. ```lua { ["begin"] = "image.png", ["inner"] = "image.png", ["end"] = "image.png" } ``` --- ### FlowLayout helpers
---@param widgetName string
---@param imageStretchSet Json
function widget.setImageStretchSet(widgetName, imageStretchSet) end

--- Appends a new ImageWidget to the specified FlowLayout widget, using `childName` as the new widget's identifier within the layout.
---@param widgetName string
---@param childName string
---@param imagePath string
function widget.addFlowImage(widgetName, childName, imagePath) end

--- Clears the selection of a ListWidget.
---@param widgetName string
function widget.clearListSelected(widgetName) end