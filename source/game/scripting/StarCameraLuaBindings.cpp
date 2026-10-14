module;
import star.json;
import star.vector;
import star.game_types;
import star.lua;
#include "StarLuaConverters.hpp"
import star.ordered_map;
import star.rect;
import star.bi_map;
import star.thread;
import star.io_device;
import star.list;
import star.image_processing;
import star.hash;
import star.data_stream;
import star.string_view;
import star.directives;
import star.asset_path;
import star.ref_ptr;
import star.logging;
import star.listener;
#include "StarConfig.hpp"
import star.maybe;
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
