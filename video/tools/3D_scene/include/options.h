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
    none = 0,
    fxaa = 1,
    smaa2x = 2,
    smaa4x = 3,
    smaa8x = 4,
    msaa2x = 5,
    msaa4x = 6,
    msaa8x = 7
  };
  enum class VisualEffect : uint32_t {
    none = 0,
    colorFilter = 1,
    crtVision = 2,
    fog = 3,
    bloom = 4,
    lumaSharpen = 5,
    brokenGlass = 6,
    storyBook = 7,
    pencil = 8,
    neon = 9,
    cartoon = 10,
    median = 11,
    unsharpMasking = 12
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
