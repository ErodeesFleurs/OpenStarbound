module;
import std;
import star.json;
import star.string;
import star.json_extra;

// Match client include order for SIMD intrinsics used by xxhash and fast_float.
import star.lua;
import star.lua_converters;
import star.client_application;
import star.image;
import star.bi_map;
import star.ref_ptr;
import star.renderer;

module star.rendering_lua_bindings;

namespace Star {

LuaCallbacks LuaBindings::makeRenderingCallbacks(ClientApplication* app) {
  LuaCallbacks callbacks;
  
  // if the last argument is defined and true, this change will also be saved to starbound.config and read on next game start, use for things such as an interface that does this
  callbacks.registerCallbackWithSignature<unsigned>("framesSkipped", std::bind(std::mem_fn(&ClientApplication::framesSkipped), app));
  callbacks.registerCallbackWithSignature<void, String, bool, std::optional<bool>>("setPostProcessGroupEnabled", std::bind(std::mem_fn(&ClientApplication::setPostProcessGroupEnabled), app, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  callbacks.registerCallbackWithSignature<bool, String>("postProcessGroupEnabled", std::bind(std::mem_fn(&ClientApplication::postProcessGroupEnabled), app, std::placeholders::_1));
  
  
  // not entirely necessary (root.assetJson can achieve the same purpose) but may as well
  callbacks.registerCallbackWithSignature<Json>("postProcessGroups", std::bind(std::mem_fn(&ClientApplication::postProcessGroups), app));
  
  // specifically checks if the effect parameter is an int since Lua prefers converting the values to floats
  callbacks.registerCallback("setEffectParameter", [app](String const& effectName, String const& effectParameter, RenderEffectParameter const& value) {
    auto renderer = app->renderer();
    auto mtype = renderer->getEffectScriptableParameterType(effectName, effectParameter);
    if (mtype) {
      auto type = *mtype;
      if (type == std::uint8_t{1} && std::holds_alternative<float>(value)) {
        renderer->setEffectScriptableParameter(effectName, effectParameter, (int)std::get<float>(value));
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
  callbacks.registerCallbackWithSignature<void, String, unsigned>("setPostProcessLayerPasses", std::bind(std::mem_fn(&ClientApplication::setPostProcessLayerPasses), app, std::placeholders::_1, std::placeholders::_2));

  return callbacks;
}

}
