module;

// Keep Uuid's textual definition ahead of the global codex interface on GCC.
#include "StarArray.hpp"
#include "StarDataStream.hpp"
#include "StarVector.hpp"
#include "StarCasting.hpp"
#include "StarInputEvent.hpp"
#include "StarColor.hpp"
#include "StarList.hpp"
#include "StarRect.hpp"
#include "StarJson.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarStringView.hpp"
#include "StarThread.hpp"
import star.directives;
#include "StarBiMap.hpp"
#include "StarStringView.hpp"
#include "StarString.hpp"
import star.text;
#include "StarString.hpp"
#include "StarPoly.hpp"
#include "StarJson.hpp"
import star.asset_path;
#include "StarMaybe.hpp"
import star.listener;
#include "StarThread.hpp"
#include "StarGameTypes.hpp"
#include "StarSet.hpp"
#include "StarAudio.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"


import star.uuid;
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
import star.player_codexes;

namespace Star {

STAR_CLASS(Player);
STAR_CLASS(JsonRpcInterface);
STAR_CLASS(StackWidget);
STAR_CLASS(ListWidget);
STAR_CLASS(LabelWidget);
STAR_CLASS(ButtonWidget);
STAR_CLASS(ButtonGroupWidget);
STAR_CLASS(Codex);

STAR_CLASS(CodexInterface);
class CodexInterface : public Pane {
public:
  CodexInterface(PlayerPtr player);

  virtual void show() override;
  virtual void tick(float dt) override;

  void showTitles();
  void showSelectedContents();
  void showContents(String const& codexId);
  void showContents(CodexConstPtr codex);

  void forwardPage();
  void backwardPage();

  bool showNewCodex();

private:
  void updateSpecies();
  void setupPageText();
  void updateCodexList();

  StackWidgetPtr m_stack;

  ListWidgetPtr m_bookList;

  CodexConstPtr m_currentCodex;
  size_t m_currentPage;

  ButtonGroupWidgetPtr m_speciesTabs;
  LabelWidgetPtr m_selectLabel;
  LabelWidgetPtr m_titleLabel;
  LabelWidgetPtr m_pageContent;
  LabelWidgetPtr m_pageLabelWidget;
  LabelWidgetPtr m_pageNumberWidget;
  ButtonWidgetPtr m_prevPageButton;
  ButtonWidgetPtr m_nextPageButton;
  ButtonWidgetPtr m_backButton;

  String m_selectText;
  String m_currentSpecies;

  PlayerPtr m_player;
  List<PlayerCodexes::CodexEntry> m_codexList;
};

}

export module star.codex_interface;

export namespace Star {
  using ::Star::Player;
  using ::Star::PlayerPtr;
  using ::Star::PlayerConstPtr;
  using ::Star::PlayerWeakPtr;
  using ::Star::PlayerConstWeakPtr;
  using ::Star::PlayerUPtr;
  using ::Star::PlayerConstUPtr;
  using ::Star::JsonRpcInterface;
  using ::Star::JsonRpcInterfacePtr;
  using ::Star::JsonRpcInterfaceConstPtr;
  using ::Star::JsonRpcInterfaceWeakPtr;
  using ::Star::JsonRpcInterfaceConstWeakPtr;
  using ::Star::JsonRpcInterfaceUPtr;
  using ::Star::JsonRpcInterfaceConstUPtr;
  using ::Star::StackWidget;
  using ::Star::StackWidgetPtr;
  using ::Star::StackWidgetConstPtr;
  using ::Star::StackWidgetWeakPtr;
  using ::Star::StackWidgetConstWeakPtr;
  using ::Star::StackWidgetUPtr;
  using ::Star::StackWidgetConstUPtr;
  using ::Star::ListWidget;
  using ::Star::ListWidgetPtr;
  using ::Star::ListWidgetConstPtr;
  using ::Star::ListWidgetWeakPtr;
  using ::Star::ListWidgetConstWeakPtr;
  using ::Star::ListWidgetUPtr;
  using ::Star::ListWidgetConstUPtr;
  using ::Star::LabelWidget;
  using ::Star::LabelWidgetPtr;
  using ::Star::LabelWidgetConstPtr;
  using ::Star::LabelWidgetWeakPtr;
  using ::Star::LabelWidgetConstWeakPtr;
  using ::Star::LabelWidgetUPtr;
  using ::Star::LabelWidgetConstUPtr;
  using ::Star::ButtonWidget;
  using ::Star::ButtonWidgetPtr;
  using ::Star::ButtonWidgetConstPtr;
  using ::Star::ButtonWidgetWeakPtr;
  using ::Star::ButtonWidgetConstWeakPtr;
  using ::Star::ButtonWidgetUPtr;
  using ::Star::ButtonWidgetConstUPtr;
  using ::Star::ButtonGroupWidget;
  using ::Star::ButtonGroupWidgetPtr;
  using ::Star::ButtonGroupWidgetConstPtr;
  using ::Star::ButtonGroupWidgetWeakPtr;
  using ::Star::ButtonGroupWidgetConstWeakPtr;
  using ::Star::ButtonGroupWidgetUPtr;
  using ::Star::ButtonGroupWidgetConstUPtr;
  using ::Star::Codex;
  using ::Star::CodexPtr;
  using ::Star::CodexConstPtr;
  using ::Star::CodexWeakPtr;
  using ::Star::CodexConstWeakPtr;
  using ::Star::CodexUPtr;
  using ::Star::CodexConstUPtr;
  using ::Star::CodexInterface;
  using ::Star::CodexInterfacePtr;
  using ::Star::CodexInterfaceConstPtr;
  using ::Star::CodexInterfaceWeakPtr;
  using ::Star::CodexInterfaceConstWeakPtr;
  using ::Star::CodexInterfaceUPtr;
  using ::Star::CodexInterfaceConstUPtr;
}
