#pragma once

#include "StarJson.hpp"
#include "StarRect.hpp"
#include "StarGameTypes.hpp"
#include "StarAssets.hpp"
#include "StarImageMetadataDatabase.hpp"
#include "StarMaterialDatabase.hpp"

namespace Star {

struct WorldStructureExceptionTag { static constexpr char const* typeName = "WorldStructureException"; };
using WorldStructureException = TypedException<StarException, WorldStructureExceptionTag>;

class WorldStructure;

class WorldStructure {
public:
  struct Overlay {
    Vec2F min;
    String image;
    bool fullbright;
  };

  struct Block {
    Vec2I position;
    MaterialId materialId;
    // If the material here should not be removed on upgrade, this flag will be
    // set to true.
    bool residual;
    MaterialColorVariant materialColor;
    MaterialHue materialHue;
    ModId materialMod;
  };

  struct Object {
    Vec2I position;
    String name;
    Direction direction;
    Json parameters;
    // If an object is not designed to be removed on upgrade, this flag will be
    // set to true.
    bool residual;
  };

  WorldStructure() = default;
  WorldStructure(AssetsConstPtr assets, MaterialDatabaseConstPtr materialDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, String const& configPath);
  WorldStructure(Json const& store);

  [[nodiscard]] Json configValue(String const& name) const;

  [[nodiscard]] List<Overlay> const& backgroundOverlays() const;
  [[nodiscard]] List<Overlay> const& foregroundOverlays() const;

  [[nodiscard]] List<Block> const& backgroundBlocks() const;
  [[nodiscard]] List<Block> const& foregroundBlocks() const;

  [[nodiscard]] List<Object> const& objects() const;

  [[nodiscard]] List<Vec2I> flaggedBlocks(String const& flag) const;

  [[nodiscard]] RectI region() const;
  [[nodiscard]] Vec2I anchorPosition() const;

  void setAnchorPosition(Vec2I const& anchorPosition);
  void translate(Vec2I const& distance);

  [[nodiscard]] Json store() const;

private:
  struct BlockKey {
    bool anchor;
    bool foregroundBlock;
    MaterialId foregroundMat;
    bool foregroundResidual;
    bool backgroundBlock;
    MaterialId backgroundMat;
    bool backgroundResidual;
    String object;
    Direction objectDirection;
    Json objectParameters;
    bool objectResidual;
    StringList flags;
    MaterialColorVariant foregroundMatColor;
    MaterialColorVariant backgroundMatColor;
    MaterialHue foregroundMatHue;
    MaterialHue backgroundMatHue;
    ModId foregroundMatMod;
    ModId backgroundMatMod;
  };

  RectI m_region;
  Vec2I m_anchorPosition;
  Json m_config;

  List<Overlay> m_backgroundOverlays;
  List<Overlay> m_foregroundOverlays;

  List<Block> m_backgroundBlocks;
  List<Block> m_foregroundBlocks;

  List<Object> m_objects;
  StringMap<List<Vec2I>> m_flaggedBlocks;
};

}
