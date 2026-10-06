

#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarApplicationController.hpp"
#include "StarColor.hpp"
#include "StarFont.hpp"
#include "StarRenderer.hpp"
#include "StarDirectives.hpp"
#include "StarBiMap.hpp"
#include "StarRoot.hpp"
#include "StarStringView.hpp"
#include "StarText.hpp"
#include "StarString.hpp"
#include "StarDataStream.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
#include "StarAssetPath.hpp"
#include "StarMaybe.hpp"
#include "StarListener.hpp"
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
import star.font_texture_group;
import star.anchor_types;
import star.text_painter;
import star.drawable;
import star.asset_texture_group;
import star.drawable_painter;
import star.gui_types;
import star.key_bindings;
import star.mixer;
import star.gui_context;
import star.widget;
import star.pane;
#include "StarLuaComponents.hpp"
import star.widget_parsing;
import star.gui_reader;


import star.base_script_pane;


import star.bindings_menu;

import star.input_lua_bindings;

namespace Star {

BindingsMenu::BindingsMenu(Json const& config) : BaseScriptPane(config) {
  m_script.setLuaRoot(make_shared<LuaRoot>());
  m_script.addCallbacks("input", LuaBindings::makeInputCallbacks());
}

void BindingsMenu::show() {
  BaseScriptPane::show();
}

void BindingsMenu::displayed() {
  BaseScriptPane::displayed();
}

void BindingsMenu::dismissed() {
  BaseScriptPane::dismissed();
}

}