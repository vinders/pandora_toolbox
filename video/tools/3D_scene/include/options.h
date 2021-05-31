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
    none = 0,
    openGL4 = 1,
    openGLES3 = 2,
    d3d11 = 3
  };
  enum class RenderingMode : uint32_t {
    normal = 0,
    wireframe = 1,
    ambientOcclusion = 2
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
    gauss = 2,
    bessel = 3,
    lanczos = 4,
    spline16 = 5
  };
  enum class Upscaling : uint32_t {
    none = 0
  };
  enum class Mapping : uint32_t {
    none = 0
  };

  // ---

  struct Options {
    RenderingApi api = RenderingApi::openGL4;
    RenderingMode renderMode = RenderingMode::normal;
    AntiAliasing aa = AntiAliasing::none;
    VisualEffect fx = VisualEffect::none;
    Interpolation scnFilter = Interpolation::bilinear;
    Interpolation texFilter = Interpolation::bilinear;
    Interpolation sprFilter = Interpolation::bilinear;
    Upscaling texUpscale = Upscaling::none;
    Upscaling sprUpscale = Upscaling::none;
    Mapping texMapping = Mapping::none;
    int32_t mouseSensitivity = 1;
    bool useVsync = false;
    bool splitScreen = false;
  };
}
