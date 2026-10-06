module;

#include "StarJson.hpp"
#include "StarBiMap.hpp"
#include "StarStrongTypedef.hpp"

namespace Star {

enum class EquipmentSlot : uint8_t {
  Head = 0,
  Chest = 1,
  Legs = 2,
  Back = 3,
  HeadCosmetic = 4,
  ChestCosmetic = 5,
  LegsCosmetic = 6,
  BackCosmetic = 7,
  Cosmetic1,
  Cosmetic2,
  Cosmetic3,
  Cosmetic4,
  Cosmetic5,
  Cosmetic6,
  Cosmetic7,
  Cosmetic8,
  Cosmetic9,
  Cosmetic10,
  Cosmetic11,
  Cosmetic12
};
extern EnumMap<EquipmentSlot> const EquipmentSlotNames;

typedef pair<String, uint8_t> BagSlot;

// Preserve the original strong_typedef expansion so its native wrapper can declare precise stream ADL.
template <typename BaseType> struct TrashSlotWrapper;
  template <typename BaseType>
  struct SwapSlotWrapper : BaseType {
    using BaseType::BaseType;

    SwapSlotWrapper() : BaseType() {}

    SwapSlotWrapper(SwapSlotWrapper const& nt) : BaseType(nt) {}

    SwapSlotWrapper(SwapSlotWrapper&& nt) : BaseType(std::move(nt)) {}

    explicit SwapSlotWrapper(BaseType const& bt) : BaseType(bt) {}

    explicit SwapSlotWrapper(BaseType&& bt) : BaseType(std::move(bt)) {}

    SwapSlotWrapper& operator=(SwapSlotWrapper const& rhs) {
      BaseType::operator=(rhs);
      return *this;
    }

    SwapSlotWrapper& operator=(SwapSlotWrapper&& rhs) {
      BaseType::operator=(std::move(rhs));
      return *this;
    }

    template <class Arg>
    SwapSlotWrapper& operator=(Arg&& other) {
      static_assert(std::is_base_of<BaseType, typename std::decay<Arg>::type>::value == false
              || std::is_same<SwapSlotWrapper, typename std::decay<Arg>::type>::value,
          "" "SwapSlot" " can not implicitly be assigned from " "Empty" "-derived classes or strong " "Empty"
          " typedefs");

      BaseType::operator=(std::forward<Arg>(other));
      return *this;
    }
    friend std::ostream& operator<<(std::ostream& ostream,
        Variant<EquipmentSlot, BagSlot, SwapSlotWrapper<Empty>, TrashSlotWrapper<Empty>> const& slot);
  };
  typedef SwapSlotWrapper<Empty> SwapSlot;
strong_typedef(Empty, TrashSlot);

// Any manageable location in the player inventory can be pointed to by an
// InventorySlot
typedef Variant<EquipmentSlot, BagSlot, SwapSlot, TrashSlot> InventorySlot;

InventorySlot jsonToInventorySlot(Json const& json);
Json jsonFromInventorySlot(InventorySlot const& slot);

std::ostream& operator<<(std::ostream& ostream, InventorySlot const& slot);

// Special items in the player inventory that are not generally manageable
enum class EssentialItem : uint8_t {
  BeamAxe = 0,
  WireTool = 1,
  PaintTool = 2,
  InspectionTool = 3
};
extern EnumMap<EssentialItem> const EssentialItemNames;

// A player's action bar is a collection of custom item shortcuts, and special
// hard coded shortcuts to the essential items.  There is one location selected
// at a time, which is either an entry on the custom bar, or one of the
// essential items, or nothing.
typedef uint8_t CustomBarIndex;
typedef MVariant<CustomBarIndex, EssentialItem> SelectedActionBarLocation;

SelectedActionBarLocation jsonToSelectedActionBarLocation(Json const& json);
Json jsonFromSelectedActionBarLocation(SelectedActionBarLocation const& location);

inline constexpr uint8_t EquipmentSize = 8;
inline constexpr uint8_t EssentialItemCount = 4;

}

template <> struct fmt::formatter<Star::InventorySlot> : ostream_formatter {
  auto format(Star::InventorySlot const& value, fmt::format_context& context) const -> fmt::format_context::iterator;
};

export module star.inventory_types;

export namespace Star {
  using ::Star::EquipmentSlot;
  using ::Star::EquipmentSlotNames;
  using ::Star::BagSlot;
  using ::Star::SwapSlotWrapper;
  using ::Star::SwapSlot;
  using ::Star::TrashSlotWrapper;
  using ::Star::TrashSlot;
  using ::Star::InventorySlot;
  using ::Star::jsonToInventorySlot;
  using ::Star::jsonFromInventorySlot;
  using ::Star::EssentialItem;
  using ::Star::EssentialItemNames;
  using ::Star::CustomBarIndex;
  using ::Star::SelectedActionBarLocation;
  using ::Star::jsonToSelectedActionBarLocation;
  using ::Star::jsonFromSelectedActionBarLocation;
  using ::Star::EquipmentSize;
  using ::Star::EssentialItemCount;
}
