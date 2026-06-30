#pragma once

#include "StarAssets.hpp"
#include "StarJson.hpp"
#include "StarLuaRoot.hpp"
#include "StarVehicle.hpp"

namespace Star {

class Rebuilder;
using RebuilderPtr = SharedPtr<Rebuilder>;
class ParticleDatabase;
using ParticleDatabaseConstPtr = SharedPtr<ParticleDatabase const>;
class ImageMetadataDatabase;
using ImageMetadataDatabaseConstPtr = SharedPtr<ImageMetadataDatabase const>;

struct VehicleDatabaseExceptionTag {
  static constexpr char const* typeName = "VehicleDatabaseException";
};
using VehicleDatabaseException = TypedException<StarException, VehicleDatabaseExceptionTag>;

class VehicleDatabase {
public:
  VehicleDatabase(AssetsConstPtr assets, ParticleDatabaseConstPtr particleDatabase, ImageMetadataDatabaseConstPtr imageMetadataDatabase, LuaRootServices luaRootServices);

  VehiclePtr create(String const& vehicleName, Json const& extraConfig = Json()) const;

  ByteArray netStore(VehiclePtr const& vehicle, NetCompatibilityRules rules) const;
  VehiclePtr netLoad(ByteArray const& netStore, NetCompatibilityRules rules) const;

  Json diskStore(VehiclePtr const& vehicle) const;
  VehiclePtr diskLoad(Json const& diskStore) const;

private:
  AssetsConstPtr m_assets;
  ParticleDatabaseConstPtr m_particleDatabase;
  ImageMetadataDatabaseConstPtr m_imageMetadataDatabase;
  StringMap<pair<String, Json>> m_vehicles;

  mutable RecursiveMutex m_luaMutex;
  RebuilderPtr m_rebuilder;
};

}// namespace Star
