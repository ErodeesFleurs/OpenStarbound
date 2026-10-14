module;
import std;
import star.json;

// Keep Uuid's textual definition ahead of the global codex interface on GCC.
import star.data_stream;
#include "StarCasting.hpp"
import star.input_event;
import star.color;
import star.list;
import star.image_processing;
import star.hash;
import star.string_view;
import star.thread;
import star.directives;
import star.bi_map;
import star.string;
import star.text;
import star.asset_path;
import star.maybe;
import star.listener;
import star.game_types;
import star.set;
import star.audio;
import star.map;


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

class Player;
using PlayerPtr = std::shared_ptr<Player>;
using PlayerConstPtr = std::shared_ptr<Player const>;
using PlayerWeakPtr = std::weak_ptr<Player>;
using PlayerConstWeakPtr = std::weak_ptr<Player const>;
using PlayerUPtr = std::unique_ptr<Player>;
using PlayerConstUPtr = std::unique_ptr<Player const>;
class JsonRpcInterface;
using JsonRpcInterfacePtr = std::shared_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstPtr = std::shared_ptr<JsonRpcInterface const>;
using JsonRpcInterfaceWeakPtr = std::weak_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstWeakPtr = std::weak_ptr<JsonRpcInterface const>;
using JsonRpcInterfaceUPtr = std::unique_ptr<JsonRpcInterface>;
using JsonRpcInterfaceConstUPtr = std::unique_ptr<JsonRpcInterface const>;
class StackWidget;
using StackWidgetPtr = std::shared_ptr<StackWidget>;
using StackWidgetConstPtr = std::shared_ptr<StackWidget const>;
using StackWidgetWeakPtr = std::weak_ptr<StackWidget>;
using StackWidgetConstWeakPtr = std::weak_ptr<StackWidget const>;
using StackWidgetUPtr = std::unique_ptr<StackWidget>;
using StackWidgetConstUPtr = std::unique_ptr<StackWidget const>;
class ListWidget;
using ListWidgetPtr = std::shared_ptr<ListWidget>;
using ListWidgetConstPtr = std::shared_ptr<ListWidget const>;
using ListWidgetWeakPtr = std::weak_ptr<ListWidget>;
using ListWidgetConstWeakPtr = std::weak_ptr<ListWidget const>;
using ListWidgetUPtr = std::unique_ptr<ListWidget>;
using ListWidgetConstUPtr = std::unique_ptr<ListWidget const>;
class LabelWidget;
using LabelWidgetPtr = std::shared_ptr<LabelWidget>;
using LabelWidgetConstPtr = std::shared_ptr<LabelWidget const>;
using LabelWidgetWeakPtr = std::weak_ptr<LabelWidget>;
using LabelWidgetConstWeakPtr = std::weak_ptr<LabelWidget const>;
using LabelWidgetUPtr = std::unique_ptr<LabelWidget>;
using LabelWidgetConstUPtr = std::unique_ptr<LabelWidget const>;
class ButtonWidget;
using ButtonWidgetPtr = std::shared_ptr<ButtonWidget>;
using ButtonWidgetConstPtr = std::shared_ptr<ButtonWidget const>;
using ButtonWidgetWeakPtr = std::weak_ptr<ButtonWidget>;
using ButtonWidgetConstWeakPtr = std::weak_ptr<ButtonWidget const>;
using ButtonWidgetUPtr = std::unique_ptr<ButtonWidget>;
using ButtonWidgetConstUPtr = std::unique_ptr<ButtonWidget const>;
class ButtonGroupWidget;
using ButtonGroupWidgetPtr = std::shared_ptr<ButtonGroupWidget>;
using ButtonGroupWidgetConstPtr = std::shared_ptr<ButtonGroupWidget const>;
using ButtonGroupWidgetWeakPtr = std::weak_ptr<ButtonGroupWidget>;
using ButtonGroupWidgetConstWeakPtr = std::weak_ptr<ButtonGroupWidget const>;
using ButtonGroupWidgetUPtr = std::unique_ptr<ButtonGroupWidget>;
using ButtonGroupWidgetConstUPtr = std::unique_ptr<ButtonGroupWidget const>;
class Codex;
using CodexPtr = std::shared_ptr<Codex>;
using CodexConstPtr = std::shared_ptr<Codex const>;
using CodexWeakPtr = std::weak_ptr<Codex>;
using CodexConstWeakPtr = std::weak_ptr<Codex const>;
using CodexUPtr = std::unique_ptr<Codex>;
using CodexConstUPtr = std::unique_ptr<Codex const>;

class CodexInterface;
using CodexInterfacePtr = std::shared_ptr<CodexInterface>;
using CodexInterfaceConstPtr = std::shared_ptr<CodexInterface const>;
using CodexInterfaceWeakPtr = std::weak_ptr<CodexInterface>;
using CodexInterfaceConstWeakPtr = std::weak_ptr<CodexInterface const>;
using CodexInterfaceUPtr = std::unique_ptr<CodexInterface>;
using CodexInterfaceConstUPtr = std::unique_ptr<CodexInterface const>;
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
  std::size_t m_currentPage;

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
