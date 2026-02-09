#pragma once

#include "StarConfig.hpp"
#include "StarItem.hpp"
#include "StarSwingableItem.hpp"

namespace Star {

class CodexItem : public Item, public SwingableItem {
public:
  CodexItem(Json const& config, String const& directory, Json const& data);
  auto clone() const -> Ptr<Item> override;

  auto drawables() const -> List<Drawable> override;

  void fireTriggered() override;

  auto iconDrawables() const -> List<Drawable> override;
  auto dropDrawables() const -> List<Drawable> override;

private:
  String m_codexId;
  List<Drawable> m_iconDrawables;
  List<Drawable> m_worldDrawables;
};

}// namespace Star
