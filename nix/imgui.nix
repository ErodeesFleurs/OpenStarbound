{ lib, stdenv, fetchFromGitHub, cmake, ninja, sdl3, freetype, libGL }:

stdenv.mkDerivation {
  pname = "openstarbound-imgui";
  version = "1.92.8";

  # Upstream's vcpkg baseline uses 1.92.8 (source/vcpkg.json); nixpkgs still
  # ships 1.91.4 and marks its FreeType variant broken. Do not substitute the
  # older ABI.
  src = fetchFromGitHub {
    owner = "ocornut";
    repo = "imgui";
    tag = "v1.92.8";
    hash = "sha256-NaSgDE5QEEMrsgYOCAPx5d0XvCIQ9T+ciAKfLFhlmzw=";
  };

  strictDeps = true;
  nativeBuildInputs = [ cmake ninja ];
  propagatedBuildInputs = [ sdl3 freetype ] ++ lib.optional stdenv.hostPlatform.isLinux libGL;

  postPatch = ''
    cp ${./imgui/CMakeLists.txt} CMakeLists.txt
    cp ${./imgui/imgui-config.cmake.in} imgui-config.cmake.in
    # vcpkg makes this visible to consumers that compile without imported targets.
    substituteInPlace imconfig.h \
      --replace-fail $'//#define IMGUI_ENABLE_FREETYPE\n' $'#define IMGUI_ENABLE_FREETYPE\n'
  '';

  meta = {
    description = "ImGui 1.92.8 with SDL3, OpenGL3 and FreeType, as OpenStarbound builds it";
    homepage = "https://github.com/ocornut/imgui";
    license = lib.licenses.mit;
    platforms = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
  };
}
