#pragma once

#include "StarHumanoid.hpp"
#include "StarNetElementSystem.hpp"
#include "StarEffectEmitter.hpp"
#include "StarItemDescriptor.hpp"
#include "StarStatusTypes.hpp"
#include "StarLightSource.hpp"
#include "StarDamage.hpp"
#include "StarItemDatabase.hpp"

namespace Star {

class ObjectItem;
class ArmorItem;
using ArmorItemPtr = SharedPtr<ArmorItem>;
class HeadArmor;
using HeadArmorPtr = SharedPtr<HeadArmor>;
class ChestArmor;
using ChestArmorPtr = SharedPtr<ChestArmor>;
class LegsArmor;
using LegsArmorPtr = SharedPtr<LegsArmor>;
class BackArmor;
using BackArmorPtr = SharedPtr<BackArmor>;
class ToolUserEntity;
class Item;
class World;

class ArmorWearer;
using ArmorWearerPtr = SharedPtr<ArmorWearer>;

class ArmorWearer : public NetElementSyncGroup {
public:
  ArmorWearer(ItemDatabaseConstPtr itemDatabase);

  // returns true if movement parameters changed
  [[nodiscard]] bool setupHumanoid(Humanoid& humanoid, bool forceNude);
  void effects(EffectEmitter& effectEmitter);
  [[nodiscard]] List<PersistentStatusEffect> statusEffects(bool cosmeticOnly = false) const;

  void reset();

  [[nodiscard]] Json diskStore() const;
  void diskLoad(Json const& diskStore);

  [[nodiscard]] bool setItem(uint8_t slot, ArmorItemPtr item, bool visible = true);
  void setHeadItem(HeadArmorPtr headItem);
  void setChestItem(ChestArmorPtr chestItem);
  void setLegsItem(LegsArmorPtr legsItem);
  void setBackItem(BackArmorPtr backItem);
  void setHeadCosmeticItem(HeadArmorPtr headCosmeticItem);
  void setChestCosmeticItem(ChestArmorPtr chestCosmeticItem);
  void setLegsCosmeticItem(LegsArmorPtr legsCosmeticItem);
  void setBackCosmeticItem(BackArmorPtr backCosmeticItem);

  [[nodiscard]] ArmorItemPtr item(uint8_t slot) const;
  [[nodiscard]] HeadArmorPtr headItem() const;
  [[nodiscard]] ChestArmorPtr chestItem() const;
  [[nodiscard]] LegsArmorPtr legsItem() const;
  [[nodiscard]] BackArmorPtr backItem() const;
  [[nodiscard]] HeadArmorPtr headCosmeticItem() const;
  [[nodiscard]] ChestArmorPtr chestCosmeticItem() const;
  [[nodiscard]] LegsArmorPtr legsCosmeticItem() const;
  [[nodiscard]] BackArmorPtr backCosmeticItem() const;

  [[nodiscard]] ItemDescriptor itemDescriptor(uint8_t slot) const;
  [[nodiscard]] ItemDescriptor headItemDescriptor() const;
  [[nodiscard]] ItemDescriptor chestItemDescriptor() const;
  [[nodiscard]] ItemDescriptor legsItemDescriptor() const;
  [[nodiscard]] ItemDescriptor backItemDescriptor() const;
  [[nodiscard]] ItemDescriptor headCosmeticItemDescriptor() const;
  [[nodiscard]] ItemDescriptor chestCosmeticItemDescriptor() const;
  [[nodiscard]] ItemDescriptor legsCosmeticItemDescriptor() const;
  [[nodiscard]] ItemDescriptor backCosmeticItemDescriptor() const;

  // slot is automatically offset
  [[nodiscard]] bool setCosmeticItem(uint8_t slot, ArmorItemPtr cosmeticItem);
  [[nodiscard]] ArmorItemPtr cosmeticItem(uint8_t slot) const;
  [[nodiscard]] ItemDescriptor cosmeticItemDescriptor(uint8_t slot) const;
private:
  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  struct Armor {
    ArmorItemPtr item;
    bool visible = true;
    bool needsSync = true;
    bool needsStore = true;
    bool isCosmetic = false;
    bool isCurrentlyVisible = false;
    NetElementData<ItemDescriptor> netState;
  };

  Array<Armor, 20> m_armors;
  Array<uint8_t, 4> m_wornCosmeticTypes;
  ItemDatabaseConstPtr m_itemDatabase;
  // only works under the assumption that this ArmorWearer
  // will only ever touch one Humanoid (which is true!)
  Maybe<Gender> m_lastGender;
  Maybe<Direction> m_lastDirection;
  bool m_lastNude;
};

}
