module;

#include "StarItem.hpp"
#include "StarPointableItem.hpp"
#include "StarToolUserItem.hpp"
#include "StarEntityRendering.hpp"
#include "StarJsonExtra.hpp"
#include "StarAssets.hpp"
#include "StarWorld.hpp"

export module star.flashlight;

export namespace Star {

STAR_CLASS(Flashlight);

class Flashlight : public Item, public PointableItem, public ToolUserItem {
public:
  Flashlight(Json const& config, String const& directory, Json const& parameters = JsonObject());

  ItemPtr clone() const override;

  List<Drawable> drawables() const override;

  List<LightSource> lightSources() const;

private:
  String m_image;
  Vec2F m_handPosition;
  Vec2F m_lightPosition;
  Color m_lightColor;
  float m_beamWidth;
  float m_ambientFactor;
};

}

namespace Star {

Flashlight::Flashlight(Json const& config, String const& directory, Json const& parameters)
  : Item(config, directory, parameters) {
  m_image = AssetPath::relativeTo(directory, instanceValue("image").toString());
  m_handPosition = jsonToVec2F(instanceValue("handPosition"));
  m_lightPosition = jsonToVec2F(instanceValue("lightPosition"));
  m_lightColor = jsonToColor(instanceValue("lightColor"));
  m_beamWidth = instanceValue("beamLevel").toFloat();
  m_ambientFactor = instanceValue("beamAmbience").toFloat();
}

ItemPtr Flashlight::clone() const {
  return make_shared<Flashlight>(*this);
}

List<Drawable> Flashlight::drawables() const {
  return {Drawable::makeImage(m_image, 1.0f / TilePixels, true, -m_handPosition / TilePixels)};
}

List<LightSource> Flashlight::lightSources() const {
  if (!initialized())
    return {};

  float angle = world()->geometry().diff(owner()->aimPosition(), owner()->position()).angle();
  LightSource lightSource;
  lightSource.type = LightType::Point;
  lightSource.position = owner()->position() + owner()->handPosition(hand(), (m_lightPosition - m_handPosition) / TilePixels);
  lightSource.color = m_lightColor.toRgbF();
  lightSource.pointBeam = m_beamWidth;
  lightSource.beamAngle = angle;
  lightSource.beamAmbience = m_ambientFactor;
  return {std::move(lightSource)};
}

}
