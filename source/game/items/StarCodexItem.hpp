#pragma once

#include "StarItem.hpp"
#include "StarPlayerCodexes.hpp"
#include "StarSwingableItem.hpp"
#include "StarIAssets.hpp"

namespace Star {

class CodexItem : public Item, public SwingableItem {
public:
  CodexItem(IAssetsConstPtr assets, Json const& config, String const& directory, Json const& data);
  virtual ItemPtr clone() const override;

  virtual List<Drawable> drawables() const override;

  virtual void fireTriggered() override;

  virtual List<Drawable> iconDrawables() const override;
  virtual List<Drawable> dropDrawables() const override;

private:
  IAssetsConstPtr m_assets;
  String m_codexId;
  List<Drawable> m_iconDrawables;
  List<Drawable> m_worldDrawables;
};

}
