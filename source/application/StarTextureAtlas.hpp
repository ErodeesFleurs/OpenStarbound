#pragma once

#include "StarCasting.hpp"
#include "StarException.hpp"
#include "StarImage.hpp"
#include "StarRect.hpp"

import std;

namespace Star {

using TextureAtlasException = ExceptionDerived<"TextureAtlasException">;

// Implements a set of "texture atlases" or, sets of smaller textures grouped
// as a larger texture.
template <typename AtlasTextureHandle>
class TextureAtlasSet {
public:
  struct Texture {
    [[nodiscard]] virtual auto imageSize() const -> Vec2U = 0;

    virtual auto atlasTexture() const -> AtlasTextureHandle const& = 0;
    [[nodiscard]] virtual auto atlasTextureCoordinates() const -> RectU = 0;

    // A locked texture will never be moved during compression, so its
    // atlasTexture and textureCoordinates will not change.
    virtual void setLocked(bool locked) = 0;

    // Returns true if this texture has been freed or the parent
    // TextureAtlasSet has been destructed.
    [[nodiscard]] virtual auto expired() const -> bool = 0;
  };

  using TextureHandle = std::shared_ptr<Texture>;

  TextureAtlasSet(unsigned cellSize, unsigned atlasNumCells);

  virtual ~TextureAtlasSet() = default;

  // The constant square size of all atlas textures
  [[nodiscard]] auto atlasTextureSize() const -> Vec2U;

  // Removes all existing textures and destroys all texture atlases.
  void reset();

  // Adds texture to some TextureAtlas.  Texture must fit in a single atlas
  // texture, otherwise an exception is thrown.  Returns a pointer to the new
  // texture.  If borderPixels is true, then fills a 1px border around the
  // given image in the atlas with the nearest color value, to prevent
  // bleeding.
  auto addTexture(Image const& image, bool borderPixels = true) -> TextureHandle;

  // Removes the given texture from the TextureAtlasSet and invalidates the
  // pointer.
  void freeTexture(TextureHandle const& texture);

  [[nodiscard]] auto totalAtlases() const -> unsigned;
  [[nodiscard]] auto totalTextures() const -> unsigned;
  [[nodiscard]] auto averageFillLevel() const -> float;

  // Takes images from sparsely filled atlases and moves them to less sparsely
  // filled atlases in an effort to free up room.  This method tages the atlas
  // with the lowest fill level and picks a texture from it, removes it, and
  // re-adds it to the AtlasSet.  It does this up to textureCount textures,
  // until it finds a texture where re-adding it to the texture atlas simply
  // moves the texture into the same atlas, at which point it stops.
  void compressionPass(std::size_t textureCount = std::numeric_limits<std::size_t>::max());

  // The number of atlases that the AtlasSet will attempt to fit a texture in
  // before giving up and creating a new atlas.  Tries in order of least full
  // to most full.  Defaults to 3.
  [[nodiscard]] auto textureFitTries() const -> unsigned;
  void setTextureFitTries(unsigned textureFitTries);

protected:
  virtual auto createAtlasTexture(Vec2U const& size, PixelFormat pixelFormat) -> AtlasTextureHandle = 0;
  virtual void destroyAtlasTexture(AtlasTextureHandle const& atlasTexture) = 0;
  virtual void copyAtlasPixels(AtlasTextureHandle const& atlasTexture, Vec2U const& bottomLeft, Image const& image) = 0;

private:
  struct TextureAtlas {
    AtlasTextureHandle atlasTexture;
    std::unique_ptr<bool[]> usedCells;
    unsigned usedCellCount;
  };

  struct AtlasPlacement {
    TextureAtlas* atlas;
    bool borderPixels = false;
    RectU occupiedCells;
    RectU textureCoords;
  };

  struct TextureEntry : Texture {
    virtual ~TextureEntry() = default;

    [[nodiscard]] auto imageSize() const -> Vec2U override;

    auto atlasTexture() const -> AtlasTextureHandle const& override;
    [[nodiscard]] auto atlasTextureCoordinates() const -> RectU override;

    // A locked texture will never be moved during compression, so its
    // atlasTexture and textureCoordinates will not change.
    void setLocked(bool locked) override;

    [[nodiscard]] auto expired() const -> bool override;

    Image textureImage;
    AtlasPlacement atlasPlacement;
    bool placementLocked = false;
    bool textureExpired = false;
  };

  void setAtlasRegionUsed(TextureAtlas* extureAtlas, RectU const& region, bool used) const;

  auto addTextureToAtlas(TextureAtlas* atlas, Image const& image, bool borderPixels) -> std::optional<AtlasPlacement>;
  void sortAtlases();

  unsigned m_atlasCellSize;
  unsigned m_atlasNumCells;
  unsigned m_textureFitTries{};

  List<std::shared_ptr<TextureAtlas>> m_atlases;
  HashSet<std::shared_ptr<TextureEntry>> m_textures;
};

template <typename AtlasTextureHandle>
TextureAtlasSet<AtlasTextureHandle>::TextureAtlasSet(unsigned cellSize, unsigned atlasNumCells)
    : m_atlasCellSize(cellSize), m_atlasNumCells(atlasNumCells) {}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::atlasTextureSize() const -> Vec2U {
  return Vec2U::filled(m_atlasCellSize * m_atlasNumCells);
}

template <typename AtlasTextureHandle>
void TextureAtlasSet<AtlasTextureHandle>::reset() {
  for (auto const& texture : m_textures)
    texture->textureExpired = true;

  for (auto const& atlas : m_atlases)
    destroyAtlasTexture(atlas->atlasTexture);

  m_atlases.clear();
  m_textures.clear();
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::addTexture(Image const& image, bool borderPixels) -> TextureHandle {
  if (image.empty())
    throw TextureAtlasException("Empty image given in TextureAtlasSet::addTexture");

  Image finalImage;
  if (borderPixels) {
    Vec2U imageSize = image.size();
    Vec2U finalImageSize = imageSize + Vec2U(2, 2);
    finalImage = Image(finalImageSize, PixelFormat::RGBA32);

    // Fill 1px border on all sides of the image
    for (unsigned y = 0; y < finalImageSize[1]; ++y) {
      for (unsigned x = 0; x < finalImageSize[0]; ++x) {
        unsigned xind = clamp<unsigned>(x, 1, imageSize[0]) - 1;
        unsigned yind = clamp<unsigned>(y, 1, imageSize[1]) - 1;
        finalImage.set32(x, y, image.getrgb(xind, yind));
      }
    }
  } else {
    finalImage = image;
  }

  auto tryAtlas = [&](TextureAtlas* atlas) -> TextureHandle {
    auto placement = addTextureToAtlas(atlas, finalImage, borderPixels);
    if (!placement)
      return nullptr;

    auto textureEntry = std::make_shared<TextureEntry>();
    textureEntry->textureImage = std::move(finalImage);
    textureEntry->atlasPlacement = *placement;

    m_textures.add(textureEntry);
    sortAtlases();
    return textureEntry;
  };

  // Try the first 'm_textureFitTries' atlases to see if we can fit a given
  // texture in an existing atlas.  Do this from the most full to the least
  // full atlas to maximize compression.
  std::size_t startAtlas = m_atlases.size() - std::min<std::size_t>(m_atlases.size(), m_textureFitTries);
  for (std::size_t i = startAtlas; i < m_atlases.size(); ++i) {
    if (auto texturePtr = tryAtlas(m_atlases[i].get()))
      return texturePtr;
  }

  // If we have not found an existing atlas to put the texture, need to create
  // a new atlas
  m_atlases.append(std::make_shared<TextureAtlas>(TextureAtlas{
    createAtlasTexture(Vec2U::filled(m_atlasCellSize * m_atlasNumCells), PixelFormat::RGBA32),
    std::unique_ptr<bool[]>(new bool[m_atlasNumCells * m_atlasNumCells]()), 0}));

  if (auto texturePtr = tryAtlas(m_atlases.last().get()))
    return texturePtr;

  // If it can't fit in a brand new empty atlas, it will not fit in any atlas
  destroyAtlasTexture(m_atlases.last()->atlasTexture);
  m_atlases.removeLast();
  throw TextureAtlasException("Could not add texture to new atlas in TextureAtlasSet::addTexture, too large");
}

template <typename AtlasTextureHandle>
void TextureAtlasSet<AtlasTextureHandle>::freeTexture(TextureHandle const& texture) {
  auto textureEntry = convert<TextureEntry>(texture);

  setAtlasRegionUsed(textureEntry->atlasPlacement.atlas, textureEntry->atlasPlacement.occupiedCells, false);
  sortAtlases();

  textureEntry->textureExpired = true;
  m_textures.remove(textureEntry);
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::totalAtlases() const -> unsigned {
  return m_atlases.size();
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::totalTextures() const -> unsigned {
  return m_textures.size();
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::averageFillLevel() const -> float {
  if (m_atlases.empty())
    return 0.0f;

  float atlasFillLevelSum = 0.0f;
  for (auto const& atlas : m_atlases)
    atlasFillLevelSum += atlas->usedCellCount / (float)square(m_atlasNumCells);
  return atlasFillLevelSum / m_atlases.size();
}

template <typename AtlasTextureHandle>
void TextureAtlasSet<AtlasTextureHandle>::compressionPass(std::size_t textureCount) {
  while (m_atlases.size() > 1 && textureCount > 0) {
    // Find the least full atlas, If it is empty, remove it and start at the
    // next atlas.
    auto const& smallestAtlas = m_atlases.last();
    if (smallestAtlas->usedCellCount == 0) {
      destroyAtlasTexture(smallestAtlas->atlasTexture);
      m_atlases.removeLast();
      continue;
    }

    // Loop over the currently loaded textures to find the smallest texture in
    // the smallest atlas that is not locked.
    TextureEntry* smallestTexture = nullptr;
    for (auto const& texture : m_textures) {
      if (texture->atlasPlacement.atlas == m_atlases.last().get()) {
        if (!texture->placementLocked) {
          if (!smallestTexture || texture->atlasPlacement.occupiedCells.volume() < smallestTexture->atlasPlacement.occupiedCells.volume())
            smallestTexture = texture.get();
        }
      }
    }

    // If we were not able to find a smallest texture because the texture is
    // locked, then simply stop.  TODO: This could be done better, this will
    // prevent compressing textures that are not from the smallest atlas if the
    // smallest atlas has only locked textures.
    if (!smallestTexture)
      break;

    // Try to add the texture to any atlas that isn't the last (most empty) one
    std::size_t startAtlas = m_atlases.size() - 1 - std::min<std::size_t>(m_atlases.size() - 1, m_textureFitTries);
    for (std::size_t i = startAtlas; i < m_atlases.size() - 1; ++i) {
      if (auto placement = addTextureToAtlas(m_atlases[i].get(), smallestTexture->textureImage, smallestTexture->atlasPlacement.borderPixels)) {
        setAtlasRegionUsed(smallestTexture->atlasPlacement.atlas, smallestTexture->atlasPlacement.occupiedCells, false);
        smallestTexture->atlasPlacement = *placement;
        smallestTexture = nullptr;
        sortAtlases();
        break;
      }
    }

    // If we have not managed to move the smallest texture into any other
    // atlas, assume the atlas set is compressed enough and quit.
    if (smallestTexture)
      break;

    --textureCount;
  }
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::textureFitTries() const -> unsigned {
  return m_textureFitTries;
}

template <typename AtlasTextureHandle>
void TextureAtlasSet<AtlasTextureHandle>::setTextureFitTries(unsigned textureFitTries) {
  m_textureFitTries = textureFitTries;
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::TextureEntry::imageSize() const -> Vec2U {
  if (atlasPlacement.borderPixels)
    return textureImage.size() - Vec2U(2, 2);
  else
    return textureImage.size();
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::TextureEntry::atlasTexture() const -> AtlasTextureHandle const& {
  return atlasPlacement.atlas->atlasTexture;
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::TextureEntry::atlasTextureCoordinates() const -> RectU {
  return atlasPlacement.textureCoords;
}

template <typename AtlasTextureHandle>
void TextureAtlasSet<AtlasTextureHandle>::TextureEntry::setLocked(bool locked) {
  placementLocked = locked;
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::TextureEntry::expired() const -> bool {
  return textureExpired;
}

template <typename AtlasTextureHandle>
void TextureAtlasSet<AtlasTextureHandle>::setAtlasRegionUsed(TextureAtlas* textureAtlas, RectU const& region, bool used) const {
  for (unsigned y = region.yMin(); y < region.yMax(); ++y) {
    for (unsigned x = region.xMin(); x < region.xMax(); ++x) {
      auto& val = textureAtlas->usedCells[y * m_atlasNumCells + x];
      bool oldVal = val;
      val = used;
      if (oldVal && !val) {
        textureAtlas->usedCellCount -= 1;
      } else if (!oldVal && used) {
        textureAtlas->usedCellCount += 1;
      }
    }
  }
}

template <typename AtlasTextureHandle>
void TextureAtlasSet<AtlasTextureHandle>::sortAtlases() {
  sort(m_atlases, [](auto const& a1, auto const& a2) -> auto {
    return a1->usedCellCount > a2->usedCellCount;
  });
}

template <typename AtlasTextureHandle>
auto TextureAtlasSet<AtlasTextureHandle>::addTextureToAtlas(TextureAtlas* atlas, Image const& image, bool borderPixels) -> std::optional<AtlasPlacement> {
  bool found = false;
  // Minimum cell indexes where this texture fits in this atlas.
  unsigned fitCellX = 0;
  unsigned fitCellY = 0;

  Vec2U imageSize = image.size();

  // Number of cells this image will take.
  std::size_t numCellsX = (imageSize[0] + m_atlasCellSize - 1) / m_atlasCellSize;
  std::size_t numCellsY = (imageSize[1] + m_atlasCellSize - 1) / m_atlasCellSize;

  if (numCellsX > m_atlasNumCells || numCellsY > m_atlasNumCells)
    return {};

  for (std::size_t cellY = 0; cellY <= m_atlasNumCells - numCellsY; ++cellY) {
    for (std::size_t cellX = 0; cellX <= m_atlasNumCells - numCellsX; ++cellX) {
      // Check this box of numCellsX x numCellsY for fit.
      found = true;
      std::size_t fx;
      std::size_t fy;
      for (fy = cellY; fy < cellY + numCellsY; ++fy) {
        for (fx = cellX; fx < cellX + numCellsX; ++fx) {
          if (atlas->usedCells[fy * m_atlasNumCells + fx]) {
            found = false;
            break;
          }
        }
        if (!found)
          break;
      }
      if (!found) {
        // If it does not fit, then we can skip to the block past the first
        // horizontal used block;
        cellX = fx;
      } else {
        fitCellX = cellX;
        fitCellY = cellY;
        break;
      }
    }
    if (found)
      break;
  }

  if (!found)
    return {};

  setAtlasRegionUsed(atlas, RectU::withSize({fitCellX, fitCellY}, {(unsigned)numCellsX, (unsigned)numCellsY}), true);

  copyAtlasPixels(atlas->atlasTexture, Vec2U(fitCellX * m_atlasCellSize, fitCellY * m_atlasCellSize), image);

  AtlasPlacement atlasPlacement;
  atlasPlacement.atlas = atlas;
  atlasPlacement.borderPixels = borderPixels;
  atlasPlacement.occupiedCells = RectU::withSize(Vec2U(fitCellX, fitCellY), Vec2U(numCellsX, numCellsY));
  if (borderPixels)
    atlasPlacement.textureCoords = RectU::withSize(Vec2U(fitCellX * m_atlasCellSize + 1, fitCellY * m_atlasCellSize + 1), imageSize - Vec2U(2, 2));
  else
    atlasPlacement.textureCoords = RectU::withSize(Vec2U(fitCellX * m_atlasCellSize, fitCellY * m_atlasCellSize), imageSize);
  return atlasPlacement;
}

}// namespace Star
