module;

#include "StarJson.hpp"
#include "StarColor.hpp"
#include "StarDrawable.hpp"
#include "StarGameTypes.hpp"

namespace Star {

typedef uint32_t EntityRenderLayer;

inline constexpr unsigned RenderLayerUpperBits = 5;
inline constexpr unsigned RenderLayerLowerBits = 32 - RenderLayerUpperBits;
inline constexpr EntityRenderLayer RenderLayerLowerMask = (EntityRenderLayer)-1 >> RenderLayerUpperBits;

inline constexpr EntityRenderLayer RenderLayerBackgroundOverlay = 1 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerBackgroundTile = 2 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerPlatform = 3 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerPlant = 4 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerPlantDrop = 5 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerObject = 6 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerPreviewObject = 7 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerBackParticle = 8 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerVehicle = 9 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerEffect = 10 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerProjectile = 11 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerMonster = 12 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerNpc = 13 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerPlayer = 14 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerItemDrop = 15 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerLiquid = 16 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerMiddleParticle = 17 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerForegroundTile = 18 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerForegroundEntity = 19 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerForegroundOverlay = 20 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerFrontParticle = 21 << RenderLayerLowerBits;
inline constexpr EntityRenderLayer RenderLayerOverlay = 22 << RenderLayerLowerBits;

EntityRenderLayer parseRenderLayer(String renderLayer);

struct PreviewTile {
  PreviewTile();
  PreviewTile(Vec2I const& position, bool foreground, MaterialId matId, MaterialHue hueShift, bool updateMatId);
  PreviewTile(Vec2I const& position, bool foreground, Vec3B const& light, bool updateLight);
  PreviewTile(Vec2I const& position, bool foreground, MaterialId matId, MaterialHue hueShift, bool updateMatId, Vec3B const& light, bool updateLight, MaterialColorVariant colorVariant);
  PreviewTile(Vec2I const& position, LiquidId liqId);

  Vec2I position;
  bool foreground;

  LiquidId liqId;
  MaterialId matId;
  MaterialHue hueShift;
  bool updateMatId;
  MaterialColorVariant colorVariant;
  Vec3B light;
  bool updateLight;
};

struct OverheadBar {
  OverheadBar();
  OverheadBar(Json const& json);
  OverheadBar(Maybe<String> icon, float percentage, Color color, bool detailOnly);

  Vec2F entityPosition;
  Maybe<String> icon;
  float percentage;
  Color color;
  bool detailOnly;
};

enum class EntityHighlightEffectType {
  None,
  Interactive,
  Inspectable,
  Interesting,
  Inspected
};
extern EnumMap<EntityHighlightEffectType> const EntityHighlightEffectTypeNames;

struct EntityHighlightEffect {
  EntityHighlightEffectType type = EntityHighlightEffectType::None;
  float level = 0.0f;
};

}

export module star.entity_rendering_types;

export namespace Star {
  using ::Star::EntityRenderLayer;
  using ::Star::parseRenderLayer;
  using ::Star::PreviewTile;
  using ::Star::OverheadBar;
  using ::Star::EntityHighlightEffectType;
  using ::Star::EntityHighlightEffectTypeNames;
  using ::Star::EntityHighlightEffect;
  using ::Star::RenderLayerUpperBits;
  using ::Star::RenderLayerLowerBits;
  using ::Star::RenderLayerLowerMask;
  using ::Star::RenderLayerBackgroundOverlay;
  using ::Star::RenderLayerBackgroundTile;
  using ::Star::RenderLayerPlatform;
  using ::Star::RenderLayerPlant;
  using ::Star::RenderLayerPlantDrop;
  using ::Star::RenderLayerObject;
  using ::Star::RenderLayerPreviewObject;
  using ::Star::RenderLayerBackParticle;
  using ::Star::RenderLayerVehicle;
  using ::Star::RenderLayerEffect;
  using ::Star::RenderLayerProjectile;
  using ::Star::RenderLayerMonster;
  using ::Star::RenderLayerNpc;
  using ::Star::RenderLayerPlayer;
  using ::Star::RenderLayerItemDrop;
  using ::Star::RenderLayerLiquid;
  using ::Star::RenderLayerMiddleParticle;
  using ::Star::RenderLayerForegroundTile;
  using ::Star::RenderLayerForegroundEntity;
  using ::Star::RenderLayerForegroundOverlay;
  using ::Star::RenderLayerFrontParticle;
  using ::Star::RenderLayerOverlay;
}
