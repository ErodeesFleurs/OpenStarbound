module;
import std;
import star.json;

// Keep Uuid's textual definition ahead of the global codex interface on GCC.
import star.data_stream;
import star.casting;
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
class JsonRpcInterface;
class StackWidget;
class ListWidget;
class LabelWidget;
class ButtonWidget;
class ButtonGroupWidget;
class Codex;

class CodexInterface : public Pane {
public:
  CodexInterface(std::shared_ptr<Player> player);

  virtual void show() override;
  virtual void tick(float dt) override;

  void showTitles();
  void showSelectedContents();
  void showContents(String const& codexId);
  void showContents(std::shared_ptr<Codex const> codex);

  void forwardPage();
  void backwardPage();

  bool showNewCodex();

private:
  void updateSpecies();
  void setupPageText();
  void updateCodexList();

  std::shared_ptr<StackWidget> m_stack;

  std::shared_ptr<ListWidget> m_bookList;

  std::shared_ptr<Codex const> m_currentCodex;
  std::size_t m_currentPage;

  std::shared_ptr<ButtonGroupWidget> m_speciesTabs;
  std::shared_ptr<LabelWidget> m_selectLabel;
  std::shared_ptr<LabelWidget> m_titleLabel;
  std::shared_ptr<LabelWidget> m_pageContent;
  std::shared_ptr<LabelWidget> m_pageLabelWidget;
  std::shared_ptr<LabelWidget> m_pageNumberWidget;
  std::shared_ptr<ButtonWidget> m_prevPageButton;
  std::shared_ptr<ButtonWidget> m_nextPageButton;
  std::shared_ptr<ButtonWidget> m_backButton;

  String m_selectText;
  String m_currentSpecies;

  std::shared_ptr<Player> m_player;
  List<PlayerCodexes::CodexEntry> m_codexList;
};

}

export module star.codex_interface;

export namespace Star {
  using ::Star::Player;
  using ::Star::JsonRpcInterface;
  using ::Star::StackWidget;
  using ::Star::ListWidget;
  using ::Star::LabelWidget;
  using ::Star::ButtonWidget;
  using ::Star::ButtonGroupWidget;
  using ::Star::Codex;
  using ::Star::CodexInterface;
}
