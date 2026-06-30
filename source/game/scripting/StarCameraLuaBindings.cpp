#include "StarCameraLuaBindings.hpp"
#include "StarLuaConverters.hpp"
#include "StarWorldCamera.hpp"
#include "StarRoot.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeCameraCallbacks(WorldCamera* camera) {
  LuaCallbacks callbacks;

  callbacks.registerCallbackWithSignature<Vec2F>("position", [camera]() { return camera->centerWorldPosition(); });
  callbacks.registerCallbackWithSignature<float>("pixelRatio", [camera]() { return camera->pixelRatio(); });
  callbacks.registerCallback("setPixelRatio", [camera](float pixelRatio, Maybe<bool> smooth) {
    if (smooth.value())
      camera->setTargetPixelRatio(pixelRatio);
    else
      camera->setPixelRatio(pixelRatio);
    Root::singleton().configuration()->set("zoomLevel", pixelRatio);
  });

  callbacks.registerCallbackWithSignature<Vec2U>("screenSize", [camera]() { return camera->screenSize(); });
  callbacks.registerCallbackWithSignature<RectF>("worldScreenRect", [camera]() { return camera->worldScreenRect(); });
  callbacks.registerCallbackWithSignature<RectI>("worldTileRect", [camera]() { return camera->worldTileRect(); });
  callbacks.registerCallbackWithSignature<Vec2F>("tileMinScreen", [camera]() { return camera->tileMinScreen(); });
  callbacks.registerCallbackWithSignature<Vec2F, Vec2F>("screenToWorld", [camera](Vec2F const& screenPos) { return camera->screenToWorld(screenPos); });
  callbacks.registerCallbackWithSignature<Vec2F, Vec2F>("worldToScreen", [camera](Vec2F const& worldPos) { return camera->worldToScreen(worldPos); });

  return callbacks;
}

}
