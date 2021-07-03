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
    vulkan = 2,
    d3d11 = 3
  };
  enum class RenderingMode : uint32_t {
    shaded = 0,
    textured = 1,
    wireframe = 2,
    splitWireframe = 3,
    splitNoFx = 4
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
  enum class MipMap : uint32_t {
    none = 0,
    nearest = 1,
    linear = 2,
    anisotropic = 3
  };
  enum class Mapping : uint32_t {
    none = 0,
    tangent = 1,
    obj = 2,
    world = 3
  };
  enum class LightMode : uint32_t {
    none = 0,
    lights = 1,
    ao = 2,
    lightsAo = 3
  };
  enum class ColorBlending : uint32_t {
    none = 0,
    gray = 1,
    red = 2,
    green = 3,
    blue = 4
  };
  enum class FogFx : uint32_t {
    none = 0,
    light = 1,
    dark = 2
  };
  enum class HdrFx : uint32_t {
    none = 0,
    bloom = 1,
    luma = 2,
    bloomLuma = 3
  };
  enum class SharpFx : uint32_t {
    none = 0,
    gauss = 1,
    median = 2,
    unsharpMasking = 3
  };
  enum class ArtFx : uint32_t {
    none = 0,
    pencil1 = 1,
    pencil2 = 2,
    cel1 = 3,
    cel2 = 4,
    sobel = 5,
    prewitt = 6,
    storyBook = 7,
    brokenGlass = 8,
    crt = 9
  };
  enum class NoiseFx : uint32_t {
    none = 0,
    photo = 1,
    tv = 2
  };

  // ---

  struct Options {
    RenderingApi api = RenderingApi::openGL4;
    RenderingMode renderMode = RenderingMode::textured;
    AntiAliasing aa = AntiAliasing::none;
    Interpolation texFilter = Interpolation::bilinear;
    Interpolation sprFilter = Interpolation::bilinear;
    Upscaling texUpscale = Upscaling::none;
    MipMap texMip = MipMap::linear;
    Mapping texMapping = Mapping::none;
    int32_t mouseSensitivity = 1;
    LightMode light = LightMode::lights;
    bool useVsync = false;
    bool useMiniMap = false;
    bool useParticles = false;
    bool useRefraction = false;
    bool useMotionBlur = false;
    int32_t gamma = 0;
    ColorBlending colorBlend = ColorBlending::none;
    FogFx fog = FogFx::none;
    HdrFx hdrFx = HdrFx::none;
    SharpFx sharpFx = SharpFx::none;
    ArtFx artFx = ArtFx::none;
    NoiseFx noise = NoiseFx::none;
  };
}
