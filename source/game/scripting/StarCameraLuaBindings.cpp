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
import star.format;
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

std::expected<LuaCallbacks, StarException> LuaBindings::makeCameraCallbacks(Root& root, WorldCamera* camera) {
  LuaCallbacks callbacks;

  if (auto result = callbacks.registerCallbackWithSignature<Vec2F>("position", std::bind(&WorldCamera::centerWorldPosition, camera)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<float>("pixelRatio", std::bind(&WorldCamera::pixelRatio, camera)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallback("setPixelRatio", [&root, camera](float pixelRatio, std::optional<bool> smooth) -> std::expected<void, StarException> {
    if (smooth.value_or(false))
      camera->setTargetPixelRatio(pixelRatio);
    else
      camera->setPixelRatio(pixelRatio);
    auto configuration = root.configuration();
    if (!configuration)
      return std::unexpected(std::move(configuration.error()));
    return (*configuration)->set("zoomLevel", pixelRatio);
  }); !result)
    return std::unexpected(std::move(result.error()));

  if (auto result = callbacks.registerCallbackWithSignature<Vec2U>("screenSize", std::bind(&WorldCamera::screenSize, camera)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<RectF>("worldScreenRect", std::bind(&WorldCamera::worldScreenRect, camera)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<RectI>("worldTileRect", std::bind(&WorldCamera::worldTileRect, camera)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Vec2F>("tileMinScreen", std::bind(&WorldCamera::tileMinScreen, camera)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Vec2F, Vec2F>("screenToWorld", std::bind(&WorldCamera::screenToWorld, camera, std::placeholders::_1)); !result)
    return std::unexpected(std::move(result.error()));
  if (auto result = callbacks.registerCallbackWithSignature<Vec2F, Vec2F>("worldToScreen", std::bind(&WorldCamera::worldToScreen, camera, std::placeholders::_1)); !result)
    return std::unexpected(std::move(result.error()));

  return callbacks;
}

}
