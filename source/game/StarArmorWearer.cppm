module;

#include "StarGameTypes.hpp"
#include "StarNetElementSystem.hpp"
#include "StarItemDescriptor.hpp"
#include "StarJson.hpp"
#include "StarStrongTypedef.hpp"
#include "StarDataStream.hpp"
#include "StarIdMap.hpp"
import star.status_types;


namespace Star {

class Humanoid;

STAR_CLASS(ArmorItem);
STAR_CLASS(HeadArmor);
STAR_CLASS(ChestArmor);
STAR_CLASS(LegsArmor);
STAR_CLASS(BackArmor);
STAR_CLASS(ToolUserEntity);
STAR_CLASS(Item);
STAR_CLASS(World);
STAR_CLASS(EffectEmitter);

STAR_CLASS(ArmorWearer);

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
