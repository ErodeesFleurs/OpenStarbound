module;
import star.json;
import star.vector;
import star.game_types;
import star.lua;
import star.lua_converters;
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
import std;
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

LuaCallbacks LuaBindings::makeCameraCallbacks(Root& root, WorldCamera* camera) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<Vec2F>("position", std::bind(&WorldCamera::centerWorldPosition, camera));
  callbacks.registerCallbackWithSignature<float>("pixelRatio", std::bind(&WorldCamera::pixelRatio, camera));
  callbacks.registerCallback("setPixelRatio", [&root, camera](float pixelRatio, std::optional<bool> smooth) {
    if (smooth.value_or(false))
      camera->setTargetPixelRatio(pixelRatio);
    else
      camera->setPixelRatio(pixelRatio);
    root.configuration()->set("zoomLevel", pixelRatio);
  });

  callbacks.registerCallbackWithSignature<Vec2U>("screenSize", std::bind(&WorldCamera::screenSize, camera));
  callbacks.registerCallbackWithSignature<RectF>("worldScreenRect", std::bind(&WorldCamera::worldScreenRect, camera));
  callbacks.registerCallbackWithSignature<RectI>("worldTileRect", std::bind(&WorldCamera::worldTileRect, camera));
  callbacks.registerCallbackWithSignature<Vec2F>("tileMinScreen", std::bind(&WorldCamera::tileMinScreen, camera));
  callbacks.registerCallbackWithSignature<Vec2F, Vec2F>("screenToWorld", std::bind(&WorldCamera::screenToWorld, camera, std::placeholders::_1));
  callbacks.registerCallbackWithSignature<Vec2F, Vec2F>("worldToScreen", std::bind(&WorldCamera::worldToScreen, camera, std::placeholders::_1));

  return callbacks;
}

}
