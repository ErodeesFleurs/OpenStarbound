# Widget

The `widget` table now contains extra bindings.

---

#### `AudioInstance` widget.playSound(`String` audio, [`int` loops = 0], [`float` volume = 1.0f])

An existing callback, now returns the played sound instance.

---

## Widget specific callbacks

These callbacks only work for some widget types.

---

### TextBoxWidget helpers

#### `String` widget.getHint(`String` widgetName)

Gets the hint text of a TextBoxWidget.

---

#### `void` widget.setHint(`String` widgetName, `String` hint)

Sets the hint text of a TextBoxWidget.

---

#### `Int` widget.getCursorPosition(`String` widgetName)

Gets the zero-based Unicode codepoint cursor position of a TextBoxWidget (not a UTF-8 byte offset or grapheme index).

---

#### `void` widget.setCursorPosition(`String` widgetName, `int` cursorPosition)

Sets the cursor position of a TextBoxWidget, clamped to its text length, and clears its selection.

---
#### `void` widget.setTextEditorRendering(`String` widgetName, `bool` enabled)

Controls native text, selection, caret and border drawing without changing visibility, focus or input handling. Set `false` to paint the entire field using Canvas. Do not hide the widget: hidden widgets no longer request text-input capture.

#### `Json` widget.getTextEditorState(`String` widgetName)

Returns `{text, displayText, cursor, anchor, selectionStart, selectionEnd, cursorOffset}`. All indices are zero-based Unicode codepoint offsets; the ordered selection is `[selectionStart, selectionEnd)`, with `anchor` retaining its direction. `displayText` uses the engine's password masking/newline markers. `cursorOffset` is the horizontal caret offset from the text anchor in interface units, before overfill scrolling. Use `widget.setTextStyle` with the same style used by Canvas.

#### `void` widget.setTextSelection(`String` widgetName, `int` anchor, `int` cursor)

Sets the selection and caret, clamping both to the text length. Equal indices clear the selection.

#### `void` widget.setTextStyle(`String` widgetName, `Json` style)

Sets the TextBoxWidget's complete text style: `fontSize`, `font`, `color`, `shadow`, `lineSpacing`, `directives` and `backDirectives`. This also controls caret measurement when native drawing is disabled.

The off-draw editor is still a real `textbox` child with a registered callback. Existing `widget.focus`, `blur`, `setText` and textbox callback/`enterKey`/`escapeKey` configuration apply. Callbacks retain their existing event semantics (including key release); scripts can also read the state while rendering. The containing pane must forward raw focused-widget events and report its keyboard capture mode/area. Set the widget's position and size to the actual input field so the platform receives the candidate-window area.

Editing includes Unicode-codepoint navigation, engine word-boundary navigation with Ctrl/Alt, Home/End, key repeat, Shift selection, Ctrl+A, clipboard copy/cut/paste and committed `TextInputEvent` insertion/replacement. Copy/cut without a selection keeps the engine's whole-field behavior. Selection replacement respects the textbox regex and width constraints. This remains a single-line field: Shift+Enter stores a newline rendered as `↵`; it is not a multiline layout editor. Combining marks/grapheme clusters are not treated as one caret step.

The SDL backend starts/stops platform text input and passes the input area/caret offset to SDL. It forwards committed text, but the engine has no composition/preedit event representation or Lua preedit API; composition display and candidate behavior remain platform/SDL-dependent. This API does not add shaping, grapheme editing, or custom preedit rendering.

---


### SliderBarWidget helpers

#### `void` widget.setSliderRange(`String` widgetName, `int` minimum, `int` maximum, [`int` step])

Sets the allowed range for a slider, optionally overriding the step used when the slider value is changed. The step defaults to
`1` when omitted.

---

### ImageStretchWidget helpers

#### `void` widget.setImageStretchSet(`String` widgetName, `Json` imageStretchSet)

Sets the full image set of a ImageStretchWidget.
```lua
{
  ["begin"] = "image.png",
  ["inner"] = "image.png",
  ["end"] = "image.png"
}
```

---

### FlowLayout helpers

#### `void` widget.addFlowImage(`String` widgetName, `String` childName, `String` imagePath)

Appends a new ImageWidget to the specified FlowLayout widget, using `childName` as the new widget's identifier within the
layout.

---

### ScrollArea helpers

#### `Maybe<Vec2I>` widget.getScrollOffset(`String` widgetName)

Gets the current scroll offset of a ScrollArea widget. Returns `nil` if the widget is not a ScrollArea.

---

#### `void` widget.setScrollOffset(`String` widgetName, `Vec2I` offset)

Sets the current scroll offset of a ScrollArea widget.

---

#### `Vec2I` widget.getMaxScrollPosition(`String` widgetName)

Gets the maximum scroll position of a ScrollArea widget. This is the maximum offset that can be scrolled to.

---

### ListWidget helpers

#### `void` widget.clearListSelected(`String` widgetName)

Clears the selected item in a ListWidget, if any.
