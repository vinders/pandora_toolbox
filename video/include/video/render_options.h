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
    /// @brief Texture addressing mode (out-of-bounds coords management)
    enum class TextureAddressMode : uint32_t {
      border        = 0u, ///< Coordinates outside of the textures are set to the configured border color (set in sampler descriptor or shader).
      clamp         = 1u, ///< Coordinates outside of the textures are set to the edge color.
      repeat        = 2u, ///< Texture is repeated at every junction.
      repeatMirror  = 3u, ///< Texture is repeated and flipped at every junction.
      mirrorClamp   = 4u  ///< Texture is mirrored once horizontally/vertically below '0' coords, then clamped to each side colors.
    };
    /// @brief Texture minification filter type
    enum class MinificationFilter : uint32_t {
      nearest            = 0,  ///< Use nearest point - no mip-map
      linear             = 1u, ///< Bilinear downscaling - no mip-map
      nearest_mipNearest = 2u, ///< Use nearest point - nearest mip-map
      nearest_mipLinear  = 3u, ///< Use nearest point - linear mip-mapping
      linear_mipNearest  = 4u, ///< Bilinear downscaling - nearest mip-map
      linear_mipLinear   = 5u   ///< Bilinear downscaling - linear mip-mapping: trilinear filter
    };
    /// @brief Texture magnification filter type
    enum class MagnificationFilter : uint32_t {
      nearest = 0, ///< Use nearest point during upscaling
      linear = 1u  ///< Bilinear interpolation during upscaling
    };
    /// @brief Depth comparison type for depth/stencil states and shadow samplers
    enum class DepthComparison : uint32_t {
      never        = 0u, ///< always fail
      less         = 1u, ///< success: source < existing-ref
      lessEqual    = 2u, ///< success: source <= existing-ref
      equal        = 3u, ///< success: source == existing-ref
      notEqual     = 4u, ///< success: source != existing-ref
      greaterEqual = 5u, ///< success: source >= existing-ref
      greater      = 6u, ///< success: source > existing-ref
      always       = 7u  ///< always succeed (default value)
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
