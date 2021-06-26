/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <system/preprocessor_tools.h>

namespace pandora {
  namespace video {
    // -- rasterizing / merge --
    
    /// @brief Renderer culling mode
    enum class CullMode : uint32_t {
      none      = 0u, ///< No culling / all polygons
      wireFrame = 1u, ///< No culling / only polygons edges
      cullBack  = 2u, ///< Back-face culling (hide 'back' polygons)
      cullFront = 3u  ///< Front-face culling (hide 'front' polygons)
    };
    /// @brief Renderer depth-bias settings (depth control, Z-order, clipping)
    struct DepthBias {
      int32_t depthBias = 0;
      float depthBiasClamp = 0.0f;
      float depthBiasSlopeScale = 0.0f;
      bool isClipped = true;
    };
    
    /// @brief Color/alpha blend factor
    enum class BlendFactor : uint32_t {
      zero            = 0u, ///< All zero        (0,0,0,0)             -> GL_ZERO / D3D11_BLEND_ZERO
      one             = 1u, ///< All one         (1,1,1,1)             -> GL_ONE / D3D11_BLEND_ONE
      sourceColor     = 2u, ///< Source color    (sR,sG,sB,sA)         -> GL_SRC_COLOR / D3D11_BLEND_SRC_COLOR
      sourceInvColor  = 3u, ///< Source opposite (1-sR,1-sG,1-sB,1-sA) -> GL_ONE_MINUS_SRC_COLOR / D3D11_BLEND_INV_SRC_COLOR
      sourceAlpha     = 4u, ///< Source alpha    (sA,sA,sA,sA)         -> GL_SRC_ALPHA / D3D11_BLEND_SRC_ALPHA
      sourceInvAlpha  = 5u, ///< Src. alpha opp. (1-sA,1-sA,1-sA,1-sA) -> GL_ONE_MINUS_SRC_ALPHA / D3D11_BLEND_INV_SRC_ALPHA
      destColor       = 6u, ///< Dest. color     (dR,dG,dB,dA)         -> GL_DST_COLOR / D3D11_BLEND_DEST_COLOR
      destInvColor    = 7u, ///< Dest. opposite  (1-dR,1-dG,1-dB,1-dA) -> GL_ONE_MINUS_DST_COLOR / D3D11_BLEND_INV_DEST_COLOR
      destAlpha       = 8u, ///< Dest. alpha     (dA,dA,dA,dA)         -> GL_DST_ALPHA / D3D11_BLEND_DEST_ALPHA
      destInvAlpha    = 9u, ///< Dest. alpha opp.(1-dA,1-dA,1-dA,1-dA) -> GL_ONE_MINUS_DST_ALPHA / D3D11_BLEND_INV_DEST_ALPHA
      sourceAlphaSat  = 10u,///< Alpha saturation clamp (f,f,f,1) with f = min(sA,1-dA) -> GL_SRC_ALPHA_SATURATE / D3D11_BLEND_SRC_ALPHA_SAT
      dualSrcColor    = 11u,///< Dual-source color                     -> GL_SRC1_COLOR / D3D11_BLEND_SRC1_COLOR
      dualSrcInvColor = 12u,///< Dual-source opposite                  -> GL_ONE_MINUS_SRC1_COLOR / D3D11_BLEND_INV_SRC1_COLOR
      dualSrcAlpha    = 13u,///< Dual-source alpha                     -> GL_SRC1_ALPHA / D3D11_BLEND_SRC1_ALPHA
      dualSrcInvAlpha = 14u,///< Dual-source alpha opposite            -> GL_ONE_MINUS_SRC1_ALPHA / D3D11_BLEND_INV_SRC1_ALPHA
      constantColor   = 15u,///< Constant (cR,cG,cB,cA)                -> GL_CONSTANT_COLOR / D3D11_BLEND_BLEND_FACTOR
                            ///< Uses constant color provided at binding (requires 'GL_ARB_imaging' extension with OpenGL)
      constantInvColor= 16u ///< Constant (1-cR,1-cG,1-cB,1-cA)        -> GL_ONE_MINUS_CONSTANT_COLOR / D3D11_BLEND_INV_BLEND_FACTOR (requires 'GL_ARB_imaging' extension with OpenGL)
                            ///< Opposite of constant color provided at binding (requires 'GL_ARB_imaging' extension with OpenGL)
    };
    /// @brief Color/alpha blend operator
    enum class BlendOperator : uint32_t {
      none        = 0u, ///< Disable all blending for render target (if color and/or alpha operator is 'none', no blending occurs)
      add         = 1u, ///< Source RGBA + dest. RGBA    -> GL_FUNC_ADD / D3D11_BLEND_OP_ADD
      subtract    = 2u, ///< Source RGBA - dest. RGBA    -> GL_FUNC_SUBTRACT / D3D11_BLEND_OP_SUBTRACT
      revSubtract = 3u, ///< Dest. RGBA - source RGBA    -> GL_FUNC_REVERSE_SUBTRACT / D3D11_BLEND_OP_REV_SUBTRACT
      minimum     = 4u, ///< min(source RGBA, dest RGBA) -> GL_MIN / D3D11_BLEND_OP_MIN
      maximum     = 5u  ///< max(source RGBA, dest RGBA) -> GL_MAX / D3D11_BLEND_OP_MAX
    };
    
    /// @brief Blending settings for a render-target - grouped color/alpha
    struct TargetBlendingParams final {
      BlendFactor sourceFactor = BlendFactor::one;
      BlendFactor destFactor = BlendFactor::zero;
      BlendOperator op   = BlendOperator::add;
    };
    /// @brief Blending settings for a render-target - separate color/alpha
    struct TargetBlendingSplitParams final {
      BlendFactor sourceColorFactor = BlendFactor::one;
      BlendFactor destColorFactor   = BlendFactor::zero;
      BlendOperator colorOp         = BlendOperator::add;
      BlendFactor sourceAlphaFactor = BlendFactor::one;
      BlendFactor destAlphaFactor   = BlendFactor::zero;
      BlendOperator alphaOp         = BlendOperator::add;
    };
    
    
    // -- textures / filtering --
    
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
    
    
    // -- depth / stencil tests --
    
    /// @brief Value comparison type for depth/stencil tests and shadow samplers
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
    /// @brief Depth/stencil operation to perform on pixel/fragment
    enum class DepthStencilOperation : uint32_t {
      keep           = 0u, ///< Keep existing stencil value
      setZero        = 1u, ///< Set stencil value to 0
      replace        = 2u, ///< Replace stencil value with reference value
      invert         = 3u, ///< Invert stencil value
      incrementClamp = 4u, ///< Increment stencil value (clamp result)
      decrementClamp = 5u, ///< Decrement stencil value (clamp result)
      incrementWrap  = 6u, ///< Increment stencil value (wrap result)
      decrementWrap  = 7u  ///< Decrement stencil value (wrap result)
    };
    
    /// @brief Operations to perform after a depth-test (no stencil-test)
    /// @remarks Usually one instance per polygon orientation (facing/back)
    struct DepthOperationGroup {
      DepthStencilOperation failureOp; ///< Operation to perform on pixel when depth-test fails (example: incrementWrap (front) / decrementWrap (back))
      DepthStencilOperation passOp;    ///< Operation to perform on pixel when depth-test passes (example: keep)
    };
    /// @brief Operations to perform after a depth/stencil-test + stencil-test type
    /// @remarks Usually one instance per polygon orientation (facing/back)
    struct DepthStencilOperationGroup {
      DepthStencilOperation failureOp;     ///< Operation to perform on pixel when stencil-test fails
      DepthStencilOperation depthFailureOp;///< Operation to perform on pixel when stencil-test passes / depth-test fails
      DepthStencilOperation passOp;        ///< Operation to perform on pixel when all tests pass
      DepthComparison stencilTest;         ///< Stencil-test comparison to perform
    };
    
    
    // -- swap-chain behavior --
    
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
