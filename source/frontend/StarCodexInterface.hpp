#pragma once

#include "StarAssets.hpp"
#include "StarPane.hpp"
#include "StarPlayerCodexes.hpp"

namespace Star {

class Player;
using PlayerPtr = SharedPtr<Player>;
class StackWidget;
using StackWidgetPtr = SharedPtr<StackWidget>;
class ListWidget;
using ListWidgetPtr = SharedPtr<ListWidget>;
class LabelWidget;
using LabelWidgetPtr = SharedPtr<LabelWidget>;
class ButtonWidget;
using ButtonWidgetPtr = SharedPtr<ButtonWidget>;
class ButtonGroupWidget;
using ButtonGroupWidgetPtr = SharedPtr<ButtonGroupWidget>;
class CodexInterface;
using CodexInterfacePtr = SharedPtr<CodexInterface>;
class CodexInterface : public Pane {
public:
  struct Services {
    AssetsConstPtr assets;
    GuiContext& guiContext;
  };

  CodexInterface(PlayerPtr player, Services services);

  void show() override;
  void tick(float dt) override;

  void showTitles();
  void showSelectedContents();
  void showContents(String const& codexId);
  void showContents(CodexConstPtr codex);

  void forwardPage();
  void backwardPage();

  [[nodiscard]] bool showNewCodex();

private:
  void updateSpecies();
  void setupPageText();
  void updateCodexList();

  StackWidgetPtr m_stack;

  CodexConstPtr m_currentCodex;
  size_t m_currentPage = 0;

  WidgetRef<ButtonGroupWidget> m_speciesTabs;
  WidgetRef<LabelWidget> m_selectLabel;
  WidgetRef<LabelWidget> m_titleLabel;
  WidgetRef<ListWidget> m_bookList;
  WidgetRef<LabelWidget> m_pageContent;
  WidgetRef<LabelWidget> m_pageLabelWidget;
  WidgetRef<LabelWidget> m_pageNumberWidget;
  WidgetRef<ButtonWidget> m_prevPageButton;
  WidgetRef<ButtonWidget> m_nextPageButton;
  ButtonWidgetPtr m_backButton;

  String m_selectText;
  String m_currentSpecies;

  PlayerPtr m_player;
  List<PlayerCodexes::CodexEntry> m_codexList;
};

}
