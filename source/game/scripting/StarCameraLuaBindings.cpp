module;
#include "StarJson.hpp"
#include "StarPoly.hpp"
#include "StarGameTypes.hpp"
#include "StarInterpolation.hpp"
#include "StarLua.hpp"
#include "StarLuaConverters.hpp"
#include "StarOrderedMap.hpp"
#include "StarRect.hpp"
#include "StarBiMap.hpp"
#include "StarThread.hpp"
#include "StarIODevice.hpp"
#include "StarList.hpp"
import star.image_processing;
#include "StarHash.hpp"
#include "StarDataStream.hpp"
#include "StarStringView.hpp"
import star.directives;
import star.asset_path;
#include "StarRefPtr.hpp"
#include "StarLogging.hpp"
import star.listener;
#include "StarConfig.hpp"
import star.version;

import star.world_geometry;

import star.asset_source;
import star.assets;
import star.root_base;
import star.configuration;
import star.root;

import star.world_camera;

module star.camera_lua_bindings;

namespace Star {

LuaCallbacks LuaBindings::makeCameraCallbacks(WorldCamera* camera) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<Vec2F>("position", bind(&WorldCamera::centerWorldPosition, camera));
  callbacks.registerCallbackWithSignature<float>("pixelRatio", bind(&WorldCamera::pixelRatio, camera));
  callbacks.registerCallback("setPixelRatio", [camera](float pixelRatio, Maybe<bool> smooth) {
    if (smooth.value())
      camera->setTargetPixelRatio(pixelRatio);
    else
      camera->setPixelRatio(pixelRatio);
    Root::singleton().configuration()->set("zoomLevel", pixelRatio);
  });

  callbacks.registerCallbackWithSignature<Vec2U>("screenSize", bind(&WorldCamera::screenSize, camera));
  callbacks.registerCallbackWithSignature<RectF>("worldScreenRect", bind(&WorldCamera::worldScreenRect, camera));
  callbacks.registerCallbackWithSignature<RectI>("worldTileRect", bind(&WorldCamera::worldTileRect, camera));
  callbacks.registerCallbackWithSignature<Vec2F>("tileMinScreen", bind(&WorldCamera::tileMinScreen, camera));
  callbacks.registerCallbackWithSignature<Vec2F, Vec2F>("screenToWorld", bind(&WorldCamera::screenToWorld, camera, _1));
  callbacks.registerCallbackWithSignature<Vec2F, Vec2F>("worldToScreen", bind(&WorldCamera::worldToScreen, camera, _1));

  return callbacks;
}

}
