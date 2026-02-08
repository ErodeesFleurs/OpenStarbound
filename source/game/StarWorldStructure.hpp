#pragma once

#include "StarException.hpp"
#include "StarGameTypes.hpp"
#include "StarJson.hpp"
#include "StarMaterialTypes.hpp"
#include "StarRect.hpp"

namespace Star {

using WorldStructureException = ExceptionDerived<"WorldStructureException">;

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

  WorldStructure();
  WorldStructure(String const& configPath);
  WorldStructure(Json const& store);

  [[nodiscard]] auto configValue(String const& name) const -> Json;

  [[nodiscard]] auto backgroundOverlays() const -> List<Overlay> const&;
  [[nodiscard]] auto foregroundOverlays() const -> List<Overlay> const&;

  [[nodiscard]] auto backgroundBlocks() const -> List<Block> const&;
  [[nodiscard]] auto foregroundBlocks() const -> List<Block> const&;

  [[nodiscard]] auto objects() const -> List<Object> const&;

  [[nodiscard]] auto flaggedBlocks(String const& flag) const -> List<Vec2I>;

  [[nodiscard]] auto region() const -> RectI;
  [[nodiscard]] auto anchorPosition() const -> Vec2I;

  void setAnchorPosition(Vec2I const& anchorPosition);
  void translate(Vec2I const& distance);

  [[nodiscard]] auto store() const -> Json;

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

}// namespace Star
