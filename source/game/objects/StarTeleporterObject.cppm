module;

#include "StarWarpTargetEntity.hpp"
#include "StarObject.hpp"
#include "StarJsonExtra.hpp"

export module star.teleporter_object;

export namespace Star {

class TeleporterObject : public Object, public WarpTargetEntity {
public:
  TeleporterObject(ObjectConfigConstPtr config, Json const& parameters = JsonObject());

  Vec2F footPosition() const override;
};

}

namespace Star {

TeleporterObject::TeleporterObject(ObjectConfigConstPtr config, Json const& parameters) : Object(config, parameters) {
  setUniqueId(configValue("uniqueId", Uuid().hex()).optString());
}

Vec2F TeleporterObject::footPosition() const {
  if (auto footPos = configValue("teleporterFootPosition"))
    return jsonToVec2F(footPos);
  return Vec2F();
}

}
