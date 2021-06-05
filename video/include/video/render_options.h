/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <system/preprocessor_tools.h>

namespace pandora {
  namespace video {
    /// @brief Renderer culling mode
    enum class CullMode : uint32_t {
      none      = 0u, ///< No culling / all polygons
      wireFrame = 1u, ///< No culling / only polygons edges
      cullBack  = 2u, ///< Back-face culling (hide 'back' polygons)
      cullFront = 3u  ///< Front-face culling (hide 'front' polygons)
    };
    /// @brief Buffer transparency mode
    enum class AlphaBlending : uint32_t {
      ignored       = 0u, ///< No transparency
      standard      = 1u, ///< Standard transparency: - rgba(255,0,0,255) == full saturation red, opaque.
                          ///                         - rgba(127,0,0,127) == 50% saturation red, 50% transparent.
                          ///                         - rgba(255,0,0,127) == full saturation red, 50% transparent.
      preMultiplied = 2u  ///< Pre-multiplied transparency: - rgba(255,0,0,255) == full saturation red, opaque.
                          ///                               - rgba(127,0,0,127) == full saturation red, 50% transparent.
                          ///                               - rgba(255,0,0,127) == additive blended red, 50% transparent.
    };
    
    /// @brief Renderer depth-bias settings (depth control, Z-order, clipping)
    struct DepthBias {
      int32_t depthBias = 0;
      float depthBiasClamp = 0.0f;
      float depthBiasSlopeScale = 0.0f;
      bool isClipped = true;
    };
    
    /// @brief Swap-chain render target output mode
    enum class SwapChainTargetMode : uint32_t {
      uniqueOutput      = 0u, ///< Unique display output: must be the only swap-chain of a rendering device (standard usage).
      partialOutput     = 1u  ///< Partial display output: multiple partial swap-chains used (for example, for multi-window rendering).
    };
    /// @brief Advanced swap-chain output params (bit-mask flags)
    /// @warning Most of these features are not supported on old renderers (Direct3D 11.0, OpenGL 4.1...)
    enum class SwapChainOutputFlag : uint32_t {
      none            = 0x0u, ///< No option
      variableRefresh = 0x1u, ///< Required for variable refresh rate display in fullscreen/borderless.
      localOutput     = 0x2u, ///< Restrict to local displays (prevent shared display, remote access, API access...)
      shaderInput     = 0x4u, ///< Allow using swap-chain output as shader input data.
      stereo          = 0x8u  ///< Stereo rendering mode, for 3D devices/glasses (only usable in fullscreen mode)
    };
  }
}
_P_FLAGS_OPERATORS(pandora::video::SwapChainOutputFlag, uint32_t);
