#pragma once

#include "StarHumanoid.hpp"
#include "StarNetElementSystem.hpp"
#include "StarEffectEmitter.hpp"
#include "StarItemDescriptor.hpp"
#include "StarStatusTypes.hpp"
#include "StarLightSource.hpp"
#include "StarDamage.hpp"

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
  ArmorWearer();

  // returns true if movement parameters changed
  bool setupHumanoid(Humanoid& humanoid, bool forceNude);
  void effects(EffectEmitter& effectEmitter);
  List<PersistentStatusEffect> statusEffects(bool cosmeticOnly = false) const;

  void reset();

  Json diskStore() const;
  void diskLoad(Json const& diskStore);

  bool setItem(uint8_t slot, ArmorItemPtr item, bool visible = true);
  void setHeadItem(HeadArmorPtr headItem);
  void setChestItem(ChestArmorPtr chestItem);
  void setLegsItem(LegsArmorPtr legsItem);
  void setBackItem(BackArmorPtr backItem);
  void setHeadCosmeticItem(HeadArmorPtr headCosmeticItem);
  void setChestCosmeticItem(ChestArmorPtr chestCosmeticItem);
  void setLegsCosmeticItem(LegsArmorPtr legsCosmeticItem);
  void setBackCosmeticItem(BackArmorPtr backCosmeticItem);

  ArmorItemPtr item(uint8_t slot) const;
  HeadArmorPtr headItem() const;
  ChestArmorPtr chestItem() const;
  LegsArmorPtr legsItem() const;
  BackArmorPtr backItem() const;
  HeadArmorPtr headCosmeticItem() const;
  ChestArmorPtr chestCosmeticItem() const;
  LegsArmorPtr legsCosmeticItem() const;
  BackArmorPtr backCosmeticItem() const;

  ItemDescriptor itemDescriptor(uint8_t slot) const;
  ItemDescriptor headItemDescriptor() const;
  ItemDescriptor chestItemDescriptor() const;
  ItemDescriptor legsItemDescriptor() const;
  ItemDescriptor backItemDescriptor() const;
  ItemDescriptor headCosmeticItemDescriptor() const;
  ItemDescriptor chestCosmeticItemDescriptor() const;
  ItemDescriptor legsCosmeticItemDescriptor() const;
  ItemDescriptor backCosmeticItemDescriptor() const;

  // slot is automatically offset
  bool setCosmeticItem(uint8_t slot, ArmorItemPtr cosmeticItem);
  ArmorItemPtr cosmeticItem(uint8_t slot) const;
  ItemDescriptor cosmeticItemDescriptor(uint8_t slot) const;
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
  // only works under the assumption that this ArmorWearer
  // will only ever touch one Humanoid (which is true!)
  Maybe<Gender> m_lastGender;
  Maybe<Direction> m_lastDirection;
  bool m_lastNude;
};

}
