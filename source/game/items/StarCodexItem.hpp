#pragma once

#include "StarItem.hpp"
#include "StarPlayerCodexes.hpp"
#include "StarSwingableItem.hpp"
#include "StarAssets.hpp"

namespace Star {

class CodexItem : public Item, public SwingableItem {
public:
  CodexItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& data);
  [[nodiscard]] ItemPtr clone() const override;

  [[nodiscard]] List<Drawable> drawables() const override;

  void fireTriggered() override;

  [[nodiscard]] List<Drawable> iconDrawables() const override;
  [[nodiscard]] List<Drawable> dropDrawables() const override;

private:
  AssetsConstPtr m_assets;
  String m_codexId;
  List<Drawable> m_iconDrawables;
  List<Drawable> m_worldDrawables;
};

}
