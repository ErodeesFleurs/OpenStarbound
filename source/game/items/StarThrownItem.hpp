#pragma once

#include "StarConfig.hpp"
#include "StarDrawable.hpp"
#include "StarItem.hpp"
#include "StarPreviewableItem.hpp"
#include "StarSwingableItem.hpp"

namespace Star {

class ThrownItem : public Item, public SwingableItem, public PreviewableItem {
public:
  ThrownItem(Json const& config, String const& directory, Json const& itemParameters = JsonObject());

  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;
  auto preview(Ptr<Player> const& viewer = {}) const -> List<Drawable> override;

protected:
  void fireTriggered() override;

private:
  String m_projectileType;
  Json m_projectileConfig;
  size_t m_ammoUsage;
  List<Drawable> m_drawables;
};

}// namespace Star
