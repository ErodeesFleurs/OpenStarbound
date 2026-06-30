#pragma once

#include "StarItem.hpp"
#include "StarFireableItem.hpp"
#include "StarBeamItem.hpp"
#include "StarAssets.hpp"
#include "StarObjectDatabase.hpp"

namespace Star {

class ObjectItem;
using ObjectItemPtr = SharedPtr<ObjectItem>;

class ObjectItem : public Item, public FireableItem, public BeamItem {
public:
  ObjectItem(AssetsConstPtr assets, ImageMetadataDatabaseConstPtr imageMetadataDatabase, Json const& config, String const& directory, Json const& objectParameters, ObjectDatabaseConstPtr objectDatabase);
  virtual ~ObjectItem() = default;

  [[nodiscard]] ItemPtr clone() const override;

  void init(ToolUserEntity& owner, ToolHand hand) override;
  void update(float dt, FireMode fireMode, bool shifting, HashSet<MoveControlType> const& moves) override;

  [[nodiscard]] List<Drawable> nonRotatedDrawables() const override;

  [[nodiscard]] float cooldownTime() const override;
  void fire(FireMode mode, bool shifting, bool edgeTriggered) override;

  [[nodiscard]] String objectName() const;
  [[nodiscard]] Json objectParameters() const;

  [[nodiscard]] bool placeInWorld(FireMode mode, bool shifting);
  [[nodiscard]] bool canPlace(bool shifting) const;

private:
  ObjectDatabaseConstPtr m_objectDatabase;
  bool m_shifting;
};

}
