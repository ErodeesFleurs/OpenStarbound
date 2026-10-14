module;
import std;
import star.list;
import star.maybe;
import star.json;

import star.array;
import star.game_types;
import star.data_stream;
import star.bi_map;
import star.item_descriptor;
#include "StarStrongTypedef.hpp"
import star.id_map;
import star.status_types;


import star.net_element_basic_fields;
import star.net_element_sync_group;

namespace Star {

class Humanoid;

class ArmorItem;
using ArmorItemPtr = std::shared_ptr<ArmorItem>;
using ArmorItemConstPtr = std::shared_ptr<const ArmorItem>;
using ArmorItemWeakPtr = std::weak_ptr<ArmorItem>;
using ArmorItemConstWeakPtr = std::weak_ptr<const ArmorItem>;
using ArmorItemUPtr = std::unique_ptr<ArmorItem>;
using ArmorItemConstUPtr = std::unique_ptr<const ArmorItem>;
class HeadArmor;
using HeadArmorPtr = std::shared_ptr<HeadArmor>;
using HeadArmorConstPtr = std::shared_ptr<const HeadArmor>;
using HeadArmorWeakPtr = std::weak_ptr<HeadArmor>;
using HeadArmorConstWeakPtr = std::weak_ptr<const HeadArmor>;
using HeadArmorUPtr = std::unique_ptr<HeadArmor>;
using HeadArmorConstUPtr = std::unique_ptr<const HeadArmor>;
class ChestArmor;
using ChestArmorPtr = std::shared_ptr<ChestArmor>;
using ChestArmorConstPtr = std::shared_ptr<const ChestArmor>;
using ChestArmorWeakPtr = std::weak_ptr<ChestArmor>;
using ChestArmorConstWeakPtr = std::weak_ptr<const ChestArmor>;
using ChestArmorUPtr = std::unique_ptr<ChestArmor>;
using ChestArmorConstUPtr = std::unique_ptr<const ChestArmor>;
class LegsArmor;
using LegsArmorPtr = std::shared_ptr<LegsArmor>;
using LegsArmorConstPtr = std::shared_ptr<const LegsArmor>;
using LegsArmorWeakPtr = std::weak_ptr<LegsArmor>;
using LegsArmorConstWeakPtr = std::weak_ptr<const LegsArmor>;
using LegsArmorUPtr = std::unique_ptr<LegsArmor>;
using LegsArmorConstUPtr = std::unique_ptr<const LegsArmor>;
class BackArmor;
using BackArmorPtr = std::shared_ptr<BackArmor>;
using BackArmorConstPtr = std::shared_ptr<const BackArmor>;
using BackArmorWeakPtr = std::weak_ptr<BackArmor>;
using BackArmorConstWeakPtr = std::weak_ptr<const BackArmor>;
using BackArmorUPtr = std::unique_ptr<BackArmor>;
using BackArmorConstUPtr = std::unique_ptr<const BackArmor>;
class ToolUserEntity;
using ToolUserEntityPtr = std::shared_ptr<ToolUserEntity>;
using ToolUserEntityConstPtr = std::shared_ptr<const ToolUserEntity>;
using ToolUserEntityWeakPtr = std::weak_ptr<ToolUserEntity>;
using ToolUserEntityConstWeakPtr = std::weak_ptr<const ToolUserEntity>;
using ToolUserEntityUPtr = std::unique_ptr<ToolUserEntity>;
using ToolUserEntityConstUPtr = std::unique_ptr<const ToolUserEntity>;
class Item;
using ItemPtr = std::shared_ptr<Item>;
using ItemConstPtr = std::shared_ptr<const Item>;
using ItemWeakPtr = std::weak_ptr<Item>;
using ItemConstWeakPtr = std::weak_ptr<const Item>;
using ItemUPtr = std::unique_ptr<Item>;
using ItemConstUPtr = std::unique_ptr<const Item>;
class World;
using WorldPtr = std::shared_ptr<World>;
using WorldConstPtr = std::shared_ptr<const World>;
using WorldWeakPtr = std::weak_ptr<World>;
using WorldConstWeakPtr = std::weak_ptr<const World>;
using WorldUPtr = std::unique_ptr<World>;
using WorldConstUPtr = std::unique_ptr<const World>;
class EffectEmitter;
using EffectEmitterPtr = std::shared_ptr<EffectEmitter>;
using EffectEmitterConstPtr = std::shared_ptr<const EffectEmitter>;
using EffectEmitterWeakPtr = std::weak_ptr<EffectEmitter>;
using EffectEmitterConstWeakPtr = std::weak_ptr<const EffectEmitter>;
using EffectEmitterUPtr = std::unique_ptr<EffectEmitter>;
using EffectEmitterConstUPtr = std::unique_ptr<const EffectEmitter>;

class ArmorWearer;
using ArmorWearerPtr = std::shared_ptr<ArmorWearer>;
using ArmorWearerConstPtr = std::shared_ptr<const ArmorWearer>;
using ArmorWearerWeakPtr = std::weak_ptr<ArmorWearer>;
using ArmorWearerConstWeakPtr = std::weak_ptr<const ArmorWearer>;
using ArmorWearerUPtr = std::unique_ptr<ArmorWearer>;
using ArmorWearerConstUPtr = std::unique_ptr<const ArmorWearer>;

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

  bool setItem(std::uint8_t slot, ArmorItemPtr item, bool visible = true);
  void setHeadItem(HeadArmorPtr headItem);
  void setChestItem(ChestArmorPtr chestItem);
  void setLegsItem(LegsArmorPtr legsItem);
  void setBackItem(BackArmorPtr backItem);
  void setHeadCosmeticItem(HeadArmorPtr headCosmeticItem);
  void setChestCosmeticItem(ChestArmorPtr chestCosmeticItem);
  void setLegsCosmeticItem(LegsArmorPtr legsCosmeticItem);
  void setBackCosmeticItem(BackArmorPtr backCosmeticItem);

  ArmorItemPtr item(std::uint8_t slot) const;
  HeadArmorPtr headItem() const;
  ChestArmorPtr chestItem() const;
  LegsArmorPtr legsItem() const;
  BackArmorPtr backItem() const;
  HeadArmorPtr headCosmeticItem() const;
  ChestArmorPtr chestCosmeticItem() const;
  LegsArmorPtr legsCosmeticItem() const;
  BackArmorPtr backCosmeticItem() const;

  ItemDescriptor itemDescriptor(std::uint8_t slot) const;
  ItemDescriptor headItemDescriptor() const;
  ItemDescriptor chestItemDescriptor() const;
  ItemDescriptor legsItemDescriptor() const;
  ItemDescriptor backItemDescriptor() const;
  ItemDescriptor headCosmeticItemDescriptor() const;
  ItemDescriptor chestCosmeticItemDescriptor() const;
  ItemDescriptor legsCosmeticItemDescriptor() const;
  ItemDescriptor backCosmeticItemDescriptor() const;

  // slot is automatically offset
  bool setCosmeticItem(std::uint8_t slot, ArmorItemPtr cosmeticItem);
  ArmorItemPtr cosmeticItem(std::uint8_t slot) const;
  ItemDescriptor cosmeticItemDescriptor(std::uint8_t slot) const;
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
  Array<std::uint8_t, 4> m_wornCosmeticTypes;
  // only works under the assumption that this ArmorWearer
  // will only ever touch one Humanoid (which is true!)
  Maybe<Gender> m_lastGender;
  Maybe<Direction> m_lastDirection;
  bool m_lastNude;
};

}

export module star.armor_wearer;

export namespace Star {
  using ::Star::ArmorItem;
  using ::Star::ArmorItemPtr;
  using ::Star::ArmorItemConstPtr;
  using ::Star::ArmorItemWeakPtr;
  using ::Star::ArmorItemConstWeakPtr;
  using ::Star::ArmorItemUPtr;
  using ::Star::ArmorItemConstUPtr;
  using ::Star::HeadArmor;
  using ::Star::HeadArmorPtr;
  using ::Star::HeadArmorConstPtr;
  using ::Star::HeadArmorWeakPtr;
  using ::Star::HeadArmorConstWeakPtr;
  using ::Star::HeadArmorUPtr;
  using ::Star::HeadArmorConstUPtr;
  using ::Star::ChestArmor;
  using ::Star::ChestArmorPtr;
  using ::Star::ChestArmorConstPtr;
  using ::Star::ChestArmorWeakPtr;
  using ::Star::ChestArmorConstWeakPtr;
  using ::Star::ChestArmorUPtr;
  using ::Star::ChestArmorConstUPtr;
  using ::Star::LegsArmor;
  using ::Star::LegsArmorPtr;
  using ::Star::LegsArmorConstPtr;
  using ::Star::LegsArmorWeakPtr;
  using ::Star::LegsArmorConstWeakPtr;
  using ::Star::LegsArmorUPtr;
  using ::Star::LegsArmorConstUPtr;
  using ::Star::BackArmor;
  using ::Star::BackArmorPtr;
  using ::Star::BackArmorConstPtr;
  using ::Star::BackArmorWeakPtr;
  using ::Star::BackArmorConstWeakPtr;
  using ::Star::BackArmorUPtr;
  using ::Star::BackArmorConstUPtr;
  using ::Star::ToolUserEntity;
  using ::Star::ToolUserEntityPtr;
  using ::Star::ToolUserEntityConstPtr;
  using ::Star::ToolUserEntityWeakPtr;
  using ::Star::ToolUserEntityConstWeakPtr;
  using ::Star::ToolUserEntityUPtr;
  using ::Star::ToolUserEntityConstUPtr;
  using ::Star::Item;
  using ::Star::ItemPtr;
  using ::Star::ItemConstPtr;
  using ::Star::ItemWeakPtr;
  using ::Star::ItemConstWeakPtr;
  using ::Star::ItemUPtr;
  using ::Star::ItemConstUPtr;
  using ::Star::World;
  using ::Star::WorldPtr;
  using ::Star::WorldConstPtr;
  using ::Star::WorldWeakPtr;
  using ::Star::WorldConstWeakPtr;
  using ::Star::WorldUPtr;
  using ::Star::WorldConstUPtr;
  using ::Star::EffectEmitter;
  using ::Star::EffectEmitterPtr;
  using ::Star::EffectEmitterConstPtr;
  using ::Star::EffectEmitterWeakPtr;
  using ::Star::EffectEmitterConstWeakPtr;
  using ::Star::EffectEmitterUPtr;
  using ::Star::EffectEmitterConstUPtr;
  using ::Star::ArmorWearer;
  using ::Star::ArmorWearerPtr;
  using ::Star::ArmorWearerConstPtr;
  using ::Star::ArmorWearerWeakPtr;
  using ::Star::ArmorWearerConstWeakPtr;
  using ::Star::ArmorWearerUPtr;
  using ::Star::ArmorWearerConstUPtr;
}
