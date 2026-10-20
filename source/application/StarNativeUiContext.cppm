module;
#include "SDL3/SDL.h"
#include "imgui.h"

import std;
import star.format;

namespace Star {

// Null targets capture the current state while a new native context is created.
class NativeUiContextScope {
public:
  NativeUiContextScope(SDL_Window* window, SDL_GLContext glContext, ImGuiContext* imguiContext)
    : m_previousWindow(SDL_GL_GetCurrentWindow()),
      m_previousGlContext(SDL_GL_GetCurrentContext()),
      m_previousImguiContext(ImGui::GetCurrentContext()) {
    if (glContext && (window != m_previousWindow || glContext != m_previousGlContext)) {
      if (!SDL_GL_MakeCurrent(window, glContext))
        fatalError(strf("Could not activate native OpenGL context: {}", SDL_GetError()).c_str(), true);
    }
    if (imguiContext)
      ImGui::SetCurrentContext(imguiContext);
  }

  ~NativeUiContextScope() {
    ImGui::SetCurrentContext(m_previousImguiContext);
    if (SDL_GL_GetCurrentWindow() != m_previousWindow || SDL_GL_GetCurrentContext() != m_previousGlContext) {
      if (!SDL_GL_MakeCurrent(m_previousWindow, m_previousGlContext))
        fatalError(strf("Could not restore native OpenGL context: {}", SDL_GetError()).c_str(), true);
    }
  }

  NativeUiContextScope(NativeUiContextScope const&) = delete;
  NativeUiContextScope& operator=(NativeUiContextScope const&) = delete;
  NativeUiContextScope(NativeUiContextScope&&) = delete;
  NativeUiContextScope& operator=(NativeUiContextScope&&) = delete;

  void discardImguiContext(ImGuiContext* context) {
    if (m_previousImguiContext == context)
      m_previousImguiContext = nullptr;
  }

  void discardGlContext(SDL_GLContext context) {
    if (m_previousGlContext == context) {
      m_previousGlContext = nullptr;
      m_previousWindow = nullptr;
    }
  }

private:
  SDL_Window* m_previousWindow;
  SDL_GLContext m_previousGlContext;
  ImGuiContext* m_previousImguiContext;
};

}

export module star.native_ui_context;

export namespace Star {
  using ::Star::NativeUiContextScope;
}
