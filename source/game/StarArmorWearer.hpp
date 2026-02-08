#pragma once

#include "StarConfig.hpp"
#include "StarEffectEmitter.hpp"
#include "StarHumanoid.hpp"
#include "StarItemDescriptor.hpp"
#include "StarStatusTypes.hpp"

import std;

namespace Star {

class ArmorItem;
class HeadArmor;
class ChestArmor;
class LegsArmor;
class BackArmor;

class ArmorWearer : public NetElementSyncGroup {
public:
  ArmorWearer();

  // returns true if movement parameters changed
  auto setupHumanoid(Humanoid& humanoid, bool forceNude) -> bool;
  void effects(EffectEmitter& effectEmitter);
  auto statusEffects(bool cosmeticOnly = false) const -> List<PersistentStatusEffect>;

  void reset();

  auto diskStore() const -> Json;
  void diskLoad(Json const& diskStore);

  auto setItem(std::uint8_t slot, Ptr<ArmorItem> item, bool visible = true) -> bool;
  void setHeadItem(Ptr<HeadArmor> headItem);
  void setChestItem(Ptr<ChestArmor> chestItem);
  void setLegsItem(Ptr<LegsArmor> legsItem);
  void setBackItem(Ptr<BackArmor> backItem);
  void setHeadCosmeticItem(Ptr<HeadArmor> headCosmeticItem);
  void setChestCosmeticItem(Ptr<ChestArmor> chestCosmeticItem);
  void setLegsCosmeticItem(Ptr<LegsArmor> legsCosmeticItem);
  void setBackCosmeticItem(Ptr<BackArmor> backCosmeticItem);

  auto item(std::uint8_t slot) const -> Ptr<ArmorItem>;
  auto headItem() const -> Ptr<HeadArmor>;
  auto chestItem() const -> Ptr<ChestArmor>;
  auto legsItem() const -> Ptr<LegsArmor>;
  auto backItem() const -> Ptr<BackArmor>;
  auto headCosmeticItem() const -> Ptr<HeadArmor>;
  auto chestCosmeticItem() const -> Ptr<ChestArmor>;
  auto legsCosmeticItem() const -> Ptr<LegsArmor>;
  auto backCosmeticItem() const -> Ptr<BackArmor>;

  auto itemDescriptor(std::uint8_t slot) const -> ItemDescriptor;
  auto headItemDescriptor() const -> ItemDescriptor;
  auto chestItemDescriptor() const -> ItemDescriptor;
  auto legsItemDescriptor() const -> ItemDescriptor;
  auto backItemDescriptor() const -> ItemDescriptor;
  auto headCosmeticItemDescriptor() const -> ItemDescriptor;
  auto chestCosmeticItemDescriptor() const -> ItemDescriptor;
  auto legsCosmeticItemDescriptor() const -> ItemDescriptor;
  auto backCosmeticItemDescriptor() const -> ItemDescriptor;

  // slot is automatically offset
  auto setCosmeticItem(std::uint8_t slot, Ptr<ArmorItem> cosmeticItem) -> bool;
  auto cosmeticItem(std::uint8_t slot) const -> Ptr<ArmorItem>;
  auto cosmeticItemDescriptor(std::uint8_t slot) const -> ItemDescriptor;

private:
  void netElementsNeedLoad(bool full) override;
  void netElementsNeedStore() override;

  struct Armor {
    Ptr<ArmorItem> item;
    bool visible = true;
    bool needsSync = true;
    bool needsStore = true;
    bool isCosmetic = false;
    bool isCurrentlyVisible = false;
    NetElementData<ItemDescriptor> netState;
  };

  Array<Armor, 20> m_armors;
  Array<std::uint8_t, 4> m_wornCosmeticTypes;
  // only works under the assumption that this ArmorWearer
  // will only ever touch one Humanoid (which is true!)
  std::optional<Gender> m_lastGender;
  std::optional<Direction> m_lastDirection;
  bool m_lastNude;
};

}// namespace Star
