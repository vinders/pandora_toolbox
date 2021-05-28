/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory>
#include <video/window_resource.h>

namespace scene {
  enum class RenderingApi : uint32_t {
    openGL4 = 0,
    openGLES3 = 1,
    d3d11 = 2
  };
  enum class AntiAliasing : uint32_t {
    none = 0
  };
  enum class VisualEffect : uint32_t {
    none = 0
  };
  enum class Interpolation : uint32_t {
    nearest = 0,
    bilinear = 1,
    bicubic = 2,
    gauss = 3,
    lanczos = 4
  };

  // ---

  struct Options {
    RenderingApi api = RenderingApi::openGL4;
    AntiAliasing aa = AntiAliasing::none;
    VisualEffect fx = VisualEffect::none;
    Interpolation scnFilter = Interpolation::bilinear;
    Interpolation texFilter = Interpolation::bilinear;
    Interpolation sprFilter = Interpolation::bilinear;
    int32_t mouseSensitivity = 1;
  };
}
