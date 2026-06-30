#include "StarSimpleTooltip.hpp"
#include "StarAlgorithm.hpp"
#include "StarAssets.hpp"
#include "StarGuiReader.hpp"
#include "StarPane.hpp"

namespace Star {

UniquePtr<Pane> SimpleTooltipBuilder::buildTooltip(String const& text, SimpleTooltipServices services) {
  auto assets = requireServiceValueAs<StarException>(std::move(services.assets), "SimpleTooltipBuilder", "assets");

  auto tooltip = make_unique<Pane>(services.guiContext);
  tooltip->removeAllChildren();
  GuiReader reader(services.guiContext);
  reader.construct(assets->json("/interface/tooltips/simpletooltip.tooltip"), tooltip.get());
  tooltip->setLabel("contentLabel", text);

  auto stretchBackground = tooltip->fetchChild<Widget>("stretchBackground");
  stretchBackground->setSize(Vec2I{tooltip->fetchChild<Widget>("contentLabel")->size()[0] + 8, stretchBackground->size()[1]});
  tooltip->setSize(stretchBackground->size());

  return tooltip;
}

}
