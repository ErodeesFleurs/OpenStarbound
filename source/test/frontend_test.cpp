import std;
import star.chat_bubble_separation;
import star.format;
import star.game_test;
import star.input_event;
import star.json;
import star.key_bindings;
import star.list;
import star.rect;
import star.set;

#include "gtest/gtest.h"

using namespace Star;

namespace {

std::optional<RectF> separateAgainst(List<RectF> const& obstacles, RectF box) {
  BubbleSeparator<int> separator(1.0f);
  int id = 0;
  for (auto const& obstacle : obstacles)
    separator.addBubble(obstacle.center(), obstacle, id++);
  auto prepared = separator.forEach([](BubbleState<int>&, int&) -> std::expected<void, StarException> {
    return {};
  });
  EXPECT_TRUE(prepared);
  separator.addBubble(box.center(), box, -1);
  auto matches = separator.filtered([](BubbleState<int> const&, int id) { return id == -1; });
  EXPECT_EQ(matches.size(), 1u);
  std::optional<RectF> result;
  for (auto const& match : matches)
    result = match.separatedBox;
  return result;
}

TEST(ChatBubbleSeparation, ContainingProjectionsNeedBothEdgeQueries) {
  RectF box{0, 2, 10, 8};
  RectF containing{-100, 0, 100, 10};
  // A nonoverlapping neighbor hides the containing rectangle in one ordering.
  EXPECT_EQ(separateAgainst({containing, {-20, 20, -10, 30}}, box), (RectF{0, 10, 10, 16}));
  EXPECT_EQ(separateAgainst({containing, {20, 20, 30, 30}}, box), (RectF{0, 10, 10, 16}));
}

TEST(ChatBubbleSeparation, EqualSizesRespectVerticalPriorityAndTouchingEdges) {
  RectF box{0, 0, 10, 10};
  EXPECT_EQ(separateAgainst({{0, 5, 10, 15}}, box), box);
  EXPECT_EQ(separateAgainst({{0, -5, 10, 5}}, box), (RectF{0, 10, 10, 20}));
  EXPECT_EQ(separateAgainst({box}, box), (RectF{0, 10, 10, 20}));
  EXPECT_EQ(separateAgainst({{10, 0, 20, 10}, {0, 10, 10, 20}}, box), box);
}

TEST(ChatBubbleSeparation, MixedSizesChainWithoutChangingDimensions) {
  RectF box{0, 0, 10, 10};
  auto result = separateAgainst({{-5, 0, 15, 10}, {-2, 12, 13, 22}, {0, 24, 10, 44}}, RectF{0, 4, 10, 14});
  ASSERT_TRUE(result);
  EXPECT_EQ(*result, (RectF{0, 44, 10, 54}));
  EXPECT_EQ(result->size(), box.size());
  EXPECT_EQ(separateAgainst({{1, -5, 9, 50}}, box), box);
}

const GameTestRegistration keybindingSpecificity("KeyBindings", "MostSpecificModifiersAndEventTypes", [](GameTest& fixture) {
  auto config = Json::parseJson(R"JSON({
    "PlayerLeft": [{"type":"key","value":"A","mods":[]}],
    "PlayerRight": [{"type":"key","value":"A","mods":["LShift"]}],
    "PlayerUp": [{"type":"key","value":"A","mods":["LCtrl"]}],
    "PlayerDown": [{"type":"key","value":"A","mods":["LShift","LCtrl"]}],
    "PlayerJump": [{"type":"key","value":"A","mods":["LShift","LCtrl"]}]
  })JSON");
  ASSERT_TRUE(config) << config.error().what();
  auto bindings = KeyBindings::create(fixture.root(), *config);
  ASSERT_TRUE(bindings) << bindings.error().what();
  EXPECT_EQ(bindings->actions(Key::A), (Set<InterfaceAction>{InterfaceAction::PlayerLeft}));
  EXPECT_EQ(bindings->actions(KeyChord{Key::A, KeyMod::LShift}), (Set<InterfaceAction>{InterfaceAction::PlayerRight}));
  EXPECT_EQ(bindings->actions(KeyChord{Key::A, KeyMod::LCtrl}), (Set<InterfaceAction>{InterfaceAction::PlayerUp}));
  auto both = KeyMod::LShift | KeyMod::LCtrl;
  EXPECT_EQ(bindings->actions(KeyChord{Key::A, both}),
      (Set<InterfaceAction>{InterfaceAction::PlayerDown, InterfaceAction::PlayerJump}));
  EXPECT_EQ(bindings->actions(InputEvent{KeyDownEvent{Key::A, both | KeyMod::LAlt}}),
      (Set<InterfaceAction>{InterfaceAction::PlayerDown, InterfaceAction::PlayerJump}));
  EXPECT_TRUE(bindings->actions(InputEvent{KeyUpEvent{Key::A}}).empty());
  EXPECT_TRUE(bindings->actions(Key::B).empty());
  EXPECT_TRUE(bindings->actionsForKey(Key::B).empty());
  EXPECT_EQ(bindings->actionsForKey(Key::A), (Set<InterfaceAction>{InterfaceAction::PlayerLeft,
      InterfaceAction::PlayerRight, InterfaceAction::PlayerUp, InterfaceAction::PlayerDown, InterfaceAction::PlayerJump}));

  auto tiedConfig = Json::parseJson(R"JSON({
    "PlayerRight": [{"type":"key","value":"A","mods":["LShift"]}],
    "PlayerUp": [{"type":"key","value":"A","mods":["LCtrl"]}]
  })JSON");
  ASSERT_TRUE(tiedConfig) << tiedConfig.error().what();
  auto tied = KeyBindings::create(fixture.root(), *tiedConfig);
  ASSERT_TRUE(tied) << tied.error().what();
  EXPECT_EQ(tied->actions(KeyChord{Key::A, both}),
      (Set<InterfaceAction>{InterfaceAction::PlayerRight, InterfaceAction::PlayerUp}));
});

}
