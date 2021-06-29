/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Direct3D11 - bindings with native types (same labels/values as other renderers: only namespace differs)
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
# include "./d3d_11.h"
# include <system/_private/_enum_flags.h>

  namespace pandora {
    namespace video {
      namespace d3d11 {
        // -- rasterizer settings --
    
        /// @brief Rasterizer culling mode
        enum class CullMode : int/*D3D11_CULL_MODE*/ {
          none      = D3D11_CULL_NONE, ///< No culling / all polygons
          cullBack  = D3D11_CULL_BACK, ///< Back-face culling (hide back-facing polygons)
          cullFront = D3D11_CULL_FRONT ///< Front-face culling (hide front-facing polygons)
        };

        /// @brief Texture addressing mode (out-of-bounds coord management)
        enum class TextureWrap : int/*D3D11_TEXTURE_ADDRESS_MODE*/ {
          clampToBorder = D3D11_TEXTURE_ADDRESS_BORDER,     ///< Coordinates outside of the textures are set to the configured border color (set in sampler descriptor or shader).
          clampToEdge   = D3D11_TEXTURE_ADDRESS_CLAMP,      ///< Coordinates outside of the textures are set to the edge color.
          repeat        = D3D11_TEXTURE_ADDRESS_WRAP,       ///< Texture is repeated at every junction.
          repeatMirror  = D3D11_TEXTURE_ADDRESS_MIRROR,     ///< Texture is repeated and flipped at every junction.
          mirrorOnce    = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE ///< Texture is mirrored once horizontally/vertically below '0' coords, then clamped to each edge colors.
        };
        /// @brief Texture minify/magnify/mip-map filter type
        /// @remarks Values such as D3D11_FILTER_MIN_MAG_MIP_LINEAR are obtained by combining 
        ///          separate minify/magnify/mip-map filters in RendererStateFactory.
        enum class TextureFilter : int {
          nearest = 0, ///< Use nearest point
          linear  = 1  ///< Linear interpolation
        };
        
        
        // -- component data formats --
        
        /// @brief RGBA color component - bit-mask flags
        enum class ColorComponentFlag : int/*D3D11_COLOR_WRITE_ENABLE*/ {
          none  = 0,
          red   = D3D11_COLOR_WRITE_ENABLE_RED,
          green = D3D11_COLOR_WRITE_ENABLE_GREEN,
          blue  = D3D11_COLOR_WRITE_ENABLE_BLUE,
          alpha = D3D11_COLOR_WRITE_ENABLE_ALPHA,
          all   = D3D11_COLOR_WRITE_ENABLE_ALL
        };
        
        /// @brief Color/normal/depth/stencil data formats (vertex array buffers, depth/stencil buffers, textures...)
        /// @remarks - HDR rendering / color space: RG(BA) components between 10 and 32 bits ('rgba16_f_scRGB' or 'rgb10a2_unorm_hdr10' recommended).
        ///          - SDR rendering / color space: other component types ('rgba8_sRGB' or 'rgba8_unorm' recommended).
        ///          - HDR rendering on SDR devices: shaders will need to convert color values (or they'll be clipped when displayed).
        enum class DataFormat : int/*DXGI_FORMAT*/ {
          unknown = DXGI_FORMAT_UNKNOWN, ///< Unknown/empty format
          
          // HDR / 32 bit
          rgba32_f = DXGI_FORMAT_R32G32B32A32_FLOAT,///< R32G32B32A32 float
          rgb32_f = DXGI_FORMAT_R32G32B32_FLOAT,    ///< R32G32B32 float
          rg32_f = DXGI_FORMAT_R32G32_FLOAT,        ///< R32G32 float
          r32_f = DXGI_FORMAT_R32_FLOAT,            ///< R32 float
          
          rgba32_ui = DXGI_FORMAT_R32G32B32A32_UINT,///< R32G32B32A32 unsigned int
          rgb32_ui = DXGI_FORMAT_R32G32B32_UINT,    ///< R32G32B32 unsigned int
          rg32_ui = DXGI_FORMAT_R32G32_UINT,        ///< R32G32 unsigned int
          r32_ui = DXGI_FORMAT_R32_UINT,            ///< R32 unsigned int

          rgba32_i = DXGI_FORMAT_R32G32B32A32_SINT,///< R32G32B32A32 int
          rgb32_i = DXGI_FORMAT_R32G32B32_SINT,    ///< R32G32B32 int
          rg32_i = DXGI_FORMAT_R32G32_SINT,        ///< R32G32 int
          r32_i = DXGI_FORMAT_R32_SINT,            ///< R32 int
          
          // HDR / 16 bit
          rgba16_f_scRGB = DXGI_FORMAT_R16G16B16A16_FLOAT, ///< R16G16B16A16 float
          rg16_f = DXGI_FORMAT_R16G16_FLOAT,               ///< R16G16 float
          r16_f = DXGI_FORMAT_R16_FLOAT,                   ///< R16 float
          
          rgba16_ui = DXGI_FORMAT_R16G16B16A16_UINT,///< R16G16B16A16 unsigned int
          rg16_ui = DXGI_FORMAT_R16G16_UINT,        ///< R16G16 unsigned int
          r16_ui = DXGI_FORMAT_R16_UINT,            ///< R16 unsigned int
          
          rgba16_i = DXGI_FORMAT_R16G16B16A16_SINT,///< R16G16B16A16 int
          rg16_i = DXGI_FORMAT_R16G16_SINT,        ///< R16G16 int
          r16_i = DXGI_FORMAT_R16_SINT,            ///< R16 int
          
          rgba16_unorm = DXGI_FORMAT_R16G16B16A16_UNORM,///< R16G16B16A16 normalized unsigned int
          rg16_unorm = DXGI_FORMAT_R16G16_UNORM,        ///< R16G16 normalized unsigned int
          r16_unorm = DXGI_FORMAT_R16_UNORM,            ///< R16 normalized unsigned int
          
          rgba16_snorm = DXGI_FORMAT_R16G16B16A16_SNORM,///< R16G16B16A16 normalized int
          rg16_snorm = DXGI_FORMAT_R16G16_SNORM,        ///< R16G16 normalized int
          r16_snorm = DXGI_FORMAT_R16_SNORM,            ///< R16 normalized int
          
          // HDR / 10-11 bit
          rgb10a2_unorm_hdr10 = DXGI_FORMAT_R10G10B10A2_UNORM, ///< HDR-10/BT.2100: R10G10B10A2 normalized unsigned int
          rgb10a2_ui = DXGI_FORMAT_R10G10B10A2_UINT, ///< R10G10B10A2 unsigned int
          rg11b10_f = DXGI_FORMAT_R11G11B10_FLOAT,   ///< R11G11B10 float
          
          // 9 bit
          rgb9e5_uf = DXGI_FORMAT_R9G9B9E5_SHAREDEXP, ///< R9G9B9E5 float (shared exponent)
          
          // 8 bit
          rgba8_ui = DXGI_FORMAT_R8G8B8A8_UINT,///< R8G8B8A8 unsigned int
          rg8_ui = DXGI_FORMAT_R8G8_UINT,      ///< R8G8 unsigned int
          r8_ui = DXGI_FORMAT_R8_UINT,         ///< R8 unsigned int
          
          rgba8_i = DXGI_FORMAT_R8G8B8A8_SINT,///< R8G8B8A8 int
          rg8_i = DXGI_FORMAT_R8G8_SINT,      ///< R8G8 int
          r8_i = DXGI_FORMAT_R8_SINT,         ///< R8 int
          
          rgba8_sRGB = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, ///< R8G8B8A8 normalized unsigned int (sRGB)
          bgra8_sRGB = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, ///< B8G8R8A8 normalized unsigned int (sRGB)
          rgba8_unorm = DXGI_FORMAT_R8G8B8A8_UNORM,///< R8G8B8A8 normalized unsigned int
          bgra8_unorm = DXGI_FORMAT_B8G8R8A8_UNORM,///< B8G8R8A8 normalized unsigned int
          rg8_unorm = DXGI_FORMAT_R8G8_UNORM,      ///< R8G8 normalized unsigned int
          r8_unorm = DXGI_FORMAT_R8_UNORM,         ///< R8 normalized unsigned int
          a8_unorm = DXGI_FORMAT_A8_UNORM,         ///< A8 normalized unsigned int
          
          rgba8_snorm = DXGI_FORMAT_R8G8B8A8_SNORM,///< R8G8B8A8 normalized int
          rg8_snorm = DXGI_FORMAT_R8G8_SNORM,      ///< R8G8 normalized int
          r8_snorm = DXGI_FORMAT_R8_SNORM,         ///< R8 normalized int
          
          // 4-6 bit
          rgb5a1_unorm = DXGI_FORMAT_B5G5R5A1_UNORM, ///< B5G5R5A1 normalized unsigned int
          r5g6b5_unorm = DXGI_FORMAT_B5G6R5_UNORM,   ///< B5G6R5 normalized unsigned int
          rgba4_unorm = DXGI_FORMAT_B4G4R4A4_UNORM,  ///< B4G4R4A4 normalized unsigned int
          
          // depth/stencil
          d32_f = DXGI_FORMAT_D32_FLOAT,                  ///< D32 float
          d16_unorm = DXGI_FORMAT_D16_UNORM,              ///< D16 normalized unsigned int
          d32_f_s8_ui = DXGI_FORMAT_D32_FLOAT_S8X24_UINT, ///< D32 float / S8 unsigned int
          d24_unorm_s8_ui = DXGI_FORMAT_D24_UNORM_S8_UINT,///< D24 normalized unsigned int / S8 unsigned int

          // block-compression (not supported in OpenGL)
          bc6h_uf = DXGI_FORMAT_BC6H_UF16, ///< BC6H unsigned float
          bc6h_f = DXGI_FORMAT_BC6H_SF16,  ///< BC6H float
          
          bc7_sRGB  = DXGI_FORMAT_BC7_UNORM_SRGB, ///< BC7 normalized unsigned int (sRGB)
          bc7_unorm = DXGI_FORMAT_BC7_UNORM       ///< BC7 normalized unsigned int
        };
      
        /// @brief Primitive topology - vertex interpretation mode (tessellation patches excluded)
        /// @remarks <...>Adj = topology with adjacency: generation of additional adjacent vertices (based on buffer vertices):
        ///                     - those vertices are only visible in the vertex shader and geometry shader;
        ///                     - if some culling is configured before geometry shader stage, adjacent vertices may disappear;
        ///                     - can't be used with tessellation shaders;
        ///                     - has no effect with 'points' topology.
        enum class VertexTopology : int/*D3D11_PRIMITIVE_TOPOLOGY*/ {
          points           = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,        ///< List of points
          lines            = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,         ///< List of separate line segments
          linesAdj         = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,     ///< List of separate line segments - with adjacency
          lineStrips       = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,        ///< Poly-line - line between each vertex and the previous one
          lineStripsAdj    = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,    ///< Poly-line - line between each vertex and the previous one - with adjacency
          triangles        = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,     ///< List of separate triangles
          trianglesAdj     = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ, ///< List of separate triangles - with adjacency
          triangleStrip    = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,    ///< Triangle-strip - triangle for each vertex and the previous two
          triangleStripAdj = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ ///< Triangle-strip - triangle for each vertex and the previous two - with adjacency
        };
        
        /// @brief Index data formats (vertex index buffers)
        enum class VertexIndexFormat : int/*DXGI_FORMAT*/ {
          r32_ui = DXGI_FORMAT_R32_UINT, ///< 32-bit unsigned integers (recommended)
          r16_ui = DXGI_FORMAT_R16_UINT  ///< 16-bit unsigned integers
        };
        /// @brief Depth/stencil data formats (depth/stencil buffers)
        enum class DepthStencilFormat : int/*DXGI_FORMAT*/ {
          d32_f = DXGI_FORMAT_D32_FLOAT,                  ///< 32-bit float depth (recommended)
          d32_f_s8_ui = DXGI_FORMAT_D32_FLOAT_S8X24_UINT, ///< 32-bit float depth / 8-bit stencil
          d24_unorm_s8_ui = DXGI_FORMAT_D24_UNORM_S8_UINT,///< 24-bit float depth / 8-bit stencil (recommended)
          d16_unorm = DXGI_FORMAT_D16_UNORM,              ///< 16-bit float depth
        };
        /// @brief Verify if a depth/stencil format contains stencil component
        constexpr inline bool _hasStencilComponent(DepthStencilFormat format) noexcept { 
          return (format == DepthStencilFormat::d32_f_s8_ui || format == DepthStencilFormat::d24_unorm_s8_ui); 
        }
        
        
        // -- color/alpha blending --
        
        /// @brief Color/alpha blend factor
        /// @remarks When using separate color/alpha blending, the alpha factor must be zero/one or
        ///          a value with a name containing "Alpha" (ex: sourceAlpha, destInvAlpha...).
        enum class BlendFactor : int/*D3D11_BLEND*/ {
          zero            = D3D11_BLEND_ZERO,          ///< All zero        (0,0,0,0)
          one             = D3D11_BLEND_ONE,           ///< All one         (1,1,1,1)
          sourceColor     = D3D11_BLEND_SRC_COLOR,     ///< Source color    (sR,sG,sB,sA)
          sourceInvColor  = D3D11_BLEND_INV_SRC_COLOR, ///< Source opposite (1-sR,1-sG,1-sB,1-sA)
          sourceAlpha     = D3D11_BLEND_SRC_ALPHA,     ///< Source alpha    (sA,sA,sA,sA)
          sourceInvAlpha  = D3D11_BLEND_INV_SRC_ALPHA, ///< Src. alpha opposite (1-sA,1-sA,1-sA,1-sA)
          destColor       = D3D11_BLEND_DEST_COLOR,    ///< Dest. color     (dR,dG,dB,dA)
          destInvColor    = D3D11_BLEND_INV_DEST_COLOR,///< Dest. opposite  (1-dR,1-dG,1-dB,1-dA)
          destAlpha       = D3D11_BLEND_DEST_ALPHA,    ///< Dest. alpha     (dA,dA,dA,dA)
          destInvAlpha    = D3D11_BLEND_INV_DEST_ALPHA,///< Dest. alpha opposite (1-dA,1-dA,1-dA,1-dA)
          sourceAlphaSat  = D3D11_BLEND_SRC_ALPHA_SAT, ///< Alpha saturation clamp (f,f,f,1) with f = min(sA,1-dA)
          dualSrcColor    = D3D11_BLEND_SRC1_COLOR,    ///< Dual-source color
          dualSrcInvColor = D3D11_BLEND_INV_SRC1_COLOR,///< Dual-source opposite
          dualSrcAlpha    = D3D11_BLEND_SRC1_ALPHA,    ///< Dual-source alpha
          dualSrcInvAlpha = D3D11_BLEND_INV_SRC1_ALPHA,///< Dual-source alpha opposite
          constantColor   = D3D11_BLEND_BLEND_FACTOR,    ///< Constant (cR,cG,cB,cA) - constant color provided at binding
          constantInvColor= D3D11_BLEND_INV_BLEND_FACTOR,///< Constant (1-cR,1-cG,1-cB,1-cA) - opposite of constant color provided at binding
          constantAlpha   = D3D11_BLEND_BLEND_FACTOR,    ///< Constant (-,-,-,cA) - constant provided at binding - only for alpha (separate blending)
          constantInvAlpha= D3D11_BLEND_INV_BLEND_FACTOR ///< Constant (-,-,-,1-cA) - opposite of constant provided at binding - only for alpha (separate blending)
        };
        /// @brief Color/alpha blend operator
        enum class BlendOp : int/*D3D11_BLEND_OP*/ {
          none        = (D3D11_BLEND_OP)-1,          ///< Disable all blending for render target
          add         = D3D11_BLEND_OP_ADD,          ///< Source RGBA + dest. RGBA
          subtract    = D3D11_BLEND_OP_SUBTRACT,     ///< Source RGBA - dest. RGBA
          revSubtract = D3D11_BLEND_OP_REV_SUBTRACT, ///< Dest. RGBA - source RGBA
          minimum     = D3D11_BLEND_OP_MIN,          ///< min(source RGBA, dest RGBA)
          maximum     = D3D11_BLEND_OP_MAX           ///< max(source RGBA, dest RGBA)
        };
        /// @brief Blend operation state (color/alpha blend params for a render-target)
        struct BlendOpState final {
          BlendFactor srcColorFactor  = BlendFactor::one;  ///< Operation to perform on pixel shader output RGB value
          BlendFactor destColorFactor = BlendFactor::zero; ///< Operation to perform on existing render-target RGB value
          BlendOp colorBlendOp        = BlendOp::add;      ///< Blend operation between srcColorFactor and destColorFactor
          BlendFactor srcAlphaFactor  = BlendFactor::one;  ///< Operation to perform on pixel shader output alpha value
          BlendFactor destAlphaFactor = BlendFactor::zero; ///< Operation to perform on existing render-target alpha value
          BlendOp alphaBlendOp        = BlendOp::add;      ///< Blend operation between srcAlphaFactor and destAlphaFactor
          ColorComponentFlag targetWriteMask = ColorComponentFlag::all; ///< Bit-mask specifying which RGBA components are enabled for writing
        };
        
        
        // -- depth/stencil testing --
        
        /// @brief Depth/stencil comparison type for depth/stencil testing and shadow samplers
        enum class StencilCompare : int/*D3D11_COMPARISON_FUNC*/ {
          never        = D3D11_COMPARISON_NEVER,        ///< always fail
          less         = D3D11_COMPARISON_LESS,         ///< success: source < existing-ref
          lessEqual    = D3D11_COMPARISON_LESS_EQUAL,   ///< success: source <= existing-ref
          equal        = D3D11_COMPARISON_EQUAL,        ///< success: source == existing-ref
          notEqual     = D3D11_COMPARISON_NOT_EQUAL,    ///< success: source != existing-ref
          greaterEqual = D3D11_COMPARISON_GREATER_EQUAL,///< success: source >= existing-ref
          greater      = D3D11_COMPARISON_GREATER,      ///< success: source > existing-ref
          always       = D3D11_COMPARISON_ALWAYS        ///< always succeed (default value)
        };
        /// @brief Depth/stencil operation to perform
        enum class StencilOp : int/*D3D11_STENCIL_OP*/ {
          keep           = D3D11_STENCIL_OP_KEEP,     ///< Keep existing stencil value
          zero           = D3D11_STENCIL_OP_ZERO,     ///< Set stencil value to 0
          replace        = D3D11_STENCIL_OP_REPLACE,  ///< Replace stencil value with reference value
          invert         = D3D11_STENCIL_OP_INVERT,   ///< Invert stencil value
          incrementClamp = D3D11_STENCIL_OP_INCR_SAT, ///< Increment stencil value (clamp result)
          decrementClamp = D3D11_STENCIL_OP_DECR_SAT, ///< Decrement stencil value (clamp result)
          incrementWrap  = D3D11_STENCIL_OP_INCR,     ///< Increment stencil value (wrap result)
          decrementWrap  = D3D11_STENCIL_OP_DECR      ///< Decrement stencil value (wrap result)
        };
        /// @brief Depth/stencil operation state (depth/stencil test comparison + operations performed based on result)
        struct StencilOpState {
          StencilOp failed;      ///< Operation on stencil pixel when stencil test fails
          StencilOp depthFailed; ///< Operation on stencil pixel when depth test fails (stencil test passes)
          StencilOp passed;      ///< Operation on stencil pixel when depth/stencil test passes
          StencilCompare comp; ///< Stencil test comparison
        };
      }
    }
  }
  _P_FLAGS_OPERATORS(pandora::video::d3d11::ColorComponentFlag, int);
# undef NODRAWTEXT
# undef NOGDI
# undef NOBITMAP
# undef NOMCX
# undef NOSERVICE
#endif
