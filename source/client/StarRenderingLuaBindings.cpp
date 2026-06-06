#include "StarRenderingLuaBindings.hpp"
#include "StarJsonExtra.hpp"
#include "StarLuaConverters.hpp"
#include "StarClientApplication.hpp"
#include "StarRenderer.hpp"

namespace Star {

LuaCallbacks LuaBindings::makeRenderingCallbacks(ClientApplication* app) {
  LuaCallbacks callbacks;
  
  // if the last argument is defined and true, this change will also be saved to starbound.config and read on next game start, use for things such as an interface that does this
  callbacks.registerCallbackWithSignature<unsigned>("framesSkipped", [app]() { return app->framesSkipped(); });
  callbacks.registerCallbackWithSignature<void, String, bool, Maybe<bool>>("setPostProcessGroupEnabled", [app](String const& group, bool const& enabled, Maybe<bool> const& save) { app->setPostProcessGroupEnabled(group, enabled, save); });
  callbacks.registerCallbackWithSignature<bool, String>("postProcessGroupEnabled", [app](String const& group) { return app->postProcessGroupEnabled(group); });


  // not entirely necessary (root.assetJson can achieve the same purpose) but may as well
  callbacks.registerCallbackWithSignature<Json>("postProcessGroups", [app]() { return app->postProcessGroups(); });
  
  // typedef Variant<float, int, Vec4F, Vec3F, Vec2F, bool> RenderEffectParameter;
  // TODO: maybe we should be checking the effect's type and converting lua based on that instead of converting to a Variant and relying on the Variant's ordering
  // specifically checks if the effect parameter is an int since Lua prefers converting the values to floats
  callbacks.registerCallback("setEffectParameter", [app](String const& effectName, String const& effectParameter, RenderEffectParameter const& value) {
    auto renderer = app->renderer();
    auto mtype = renderer->getEffectScriptableParameterType(effectName, effectParameter);
    if (mtype) {
      auto type = mtype.value();
      if (type == 1 && value.is<float>()) {
        renderer->setEffectScriptableParameter(effectName, effectParameter, static_cast<int>(value.get<float>()));
      } else {
        renderer->setEffectScriptableParameter(effectName, effectParameter, value);
      }
    }
  });
  
  callbacks.registerCallback("getEffectParameter", [app](String const& effectName, String const& effectParameter) {
    auto renderer = app->renderer();
    return renderer->getEffectScriptableParameter(effectName, effectParameter);
  });
  
  // not saved; should be loaded by Lua again
  callbacks.registerCallbackWithSignature<void, String, unsigned>("setPostProcessLayerPasses", [app](String const& layer, unsigned const& passes) { app->setPostProcessLayerPasses(layer, passes); });

  return callbacks;
}


}
