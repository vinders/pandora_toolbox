/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
OpenGL4 - bindings with native types (same labels/values as other renderers: only namespace differs)
*******************************************************************************/
#pragma once

#if defined(_VIDEO_OPENGL4_SUPPORT)
# include "./opengl.h"
# include <system/_private/_enum_flags.h>

  namespace pandora {
    namespace video {
      namespace openGL4 {
        // -- rasterizer settings --
    
        /// @brief Rasterizer culling mode
        enum class CullMode : GLenum {
          none      = GL_NONE, ///< No culling / all polygons
          cullBack  = GL_BACK, ///< Back-face culling (hide back-facing polygons)
          cullFront = GL_FRONT ///< Front-face culling (hide front-facing polygons)
        };

        /// @brief Texture addressing mode (out-of-bounds coord management)
        enum class TextureWrap : GLenum {
          clampToBorder = GL_CLAMP_TO_BORDER,     ///< Coordinates outside of the textures are set to the configured border color (set in sampler descriptor or shader).
          clampToEdge   = GL_CLAMP_TO_EDGE,       ///< Coordinates outside of the textures are set to the edge color.
          repeat        = GL_REPEAT,              ///< Texture is repeated at every junction.
          repeatMirror  = GL_MIRRORED_REPEAT,     ///< Texture is repeated and flipped at every junction.
          mirrorOnce    = GL_MIRROR_CLAMP_TO_EDGE ///< Texture is mirrored once horizontally/vertically below '0' coords, then clamped to each edge colors.
        };
        /// @brief Texture minify/magnify/mip-map filter type
        /// @remarks Values such as GL_LINEAR_MIPMAP_LINEAR are obtained by combining 
        ///          separate minify/magnify/mip-map filters in RendererStateFactory.
        enum class TextureFilter : GLenum {
          nearest = GL_NEAREST, ///< Use nearest point
          linear = GL_LINEAR    ///< Linear interpolation
        };
        
        
        // -- component data formats --
        
        /// @brief Color/normal/depth/stencil data formats (vertex array buffers, depth/stencil buffers, textures...)
        /// @remarks - HDR rendering / color space: RG(BA) components between 10 and 32 bits ('rgba16_f_scRGB' or 'rgb10a2_unorm_hdr10' recommended).
        ///          - SDR rendering / color space: other component types ('rgba8_sRGB' or 'rgba8_unorm' recommended).
        ///          - HDR rendering on SDR devices: shaders will need to convert color values (or they'll be clipped when displayed).
        /// @warning Block-compression types (bc*) are not available with OpenGL.
        enum class DataFormat : GLenum {
          unknown = GL_NONE, ///< Unknown/empty format
          
          // HDR / 32 bit
          rgba32_f = GL_RGBA32F, ///< R32G32B32A32 float
          rgb32_f = GL_RGB32F,   ///< R32G32B32 float
          rg32_f = GL_RG32F,     ///< R32G32 float
          r32_f = GL_R32F,       ///< R32 float
          
          rgba32_ui = GL_RGBA32UI, ///< R32G32B32A32 unsigned int
          rgb32_ui = GL_RGB32UI,   ///< R32G32B32 unsigned int
          rg32_ui = GL_RG32UI,     ///< R32G32 unsigned int
          r32_ui = GL_R32UI,       ///< R32 unsigned int

          rgba32_i = GL_RGBA32I, ///< R32G32B32A32 int
          rgb32_i = GL_RGB32I,   ///< R32G32B32 int
          rg32_i = GL_RG32I,     ///< R32G32 int
          r32_i = GL_R32I,       ///< R32 int
          
          // HDR / 16 bit
          rgba16_f_scRGB = GL_RGBA16F, ///< HDR-FP16/scRGB: R16G16B16A16 float
          rg16_f = GL_RG16F,           ///< R16G16 float
          r16_f = GL_R16F,             ///< R16 float
          
          rgba16_ui = GL_RGBA16UI, ///< R16G16B16A16 unsigned int
          rg16_ui = GL_RG16UI,     ///< R16G16 unsigned int
          r16_ui = GL_R16UI,       ///< R16 unsigned int
          
          rgba16_i = GL_RGBA16I, ///< R16G16B16A16 int
          rg16_i = GL_RG16I,     ///< R16G16 int
          r16_i = GL_R16I,       ///< R16 int
          
          rgba16_unorm = GL_RGBA16, ///< R16G16B16A16 normalized unsigned int
          rg16_unorm = GL_RG16,     ///< R16G16 normalized unsigned int
          r16_unorm = GL_R16,       ///< R16 normalized unsigned int
          
          rgba16_snorm = GL_RGBA16_SNORM, ///< R16G16B16A16 normalized int
          rg16_snorm = GL_RG16_SNORM,     ///< R16G16 normalized int
          r16_snorm = GL_R16_SNORM,       ///< R16 normalized int
          
          // HDR / 10-11 bit
          rgb10a2_unorm_hdr10 = GL_RGB10_A2, ///< HDR-10/BT.2100: R10G10B10A2 normalized unsigned int
          rgb10a2_ui = GL_RGB10_A2UI,        ///< R10G10B10A2 unsigned int
          rg11b10_f = GL_R11F_G11F_B10F,     ///< R11G11B10 float
          
          // 9 bit
          rgb9e5_uf = GL_RGB9_E5, ///< R9G9B9E5 float (shared exponent)
          
          // 8 bit
          rgba8_ui = GL_RGBA8UI, ///< R8G8B8A8 unsigned int
          rg8_ui = GL_RG8UI,     ///< R8G8 unsigned int
          r8_ui = GL_R8UI,       ///< R8 unsigned int
          
          rgba8_i = GL_RGBA8I, ///< R8G8B8A8 int
          rg8_i = GL_RG8I,     ///< R8G8 int
          r8_i = GL_R8I,       ///< R8 int
          
          rgba8_sRGB = GL_SRGB8_ALPHA8, ///< R8G8B8A8 normalized unsigned int (SRGB)
          bgra8_sRGB = GL_SRGB8_ALPHA8, ///< B8G8R8A8 normalized unsigned int (SRGB)
          rgba8_unorm = GL_RGBA8, ///< R8G8B8A8 normalized unsigned int
          bgra8_unorm = GL_RGBA8, ///< B8G8R8A8 normalized unsigned int
          rg8_unorm = GL_RG8,     ///< R8G8 normalized unsigned int
          r8_unorm = GL_R8,       ///< R8 normalized unsigned int
          a8_unorm = GL_R8_SNORM, ///< A8 normalized unsigned int
          
          rgba8_snorm = GL_RGBA8_SNORM, ///< R8G8B8A8 normalized int
          rg8_snorm = GL_RG8_SNORM,     ///< R8G8 normalized int
          r8_snorm = GL_R8_SNORM,       ///< R8 normalized int
          
          // 4-6 bit
          rgb5a1_unorm = GL_RGB5_A1, ///< B5G5R5A1 normalized unsigned int
          r5g6b5_unorm = GL_RGB565,  ///< B5G6R5 normalized unsigned int
          rgba4_unorm = GL_RGBA4,    ///< B4G4R4A4 normalized unsigned int
          
          // depth/stencil
          d32_f = GL_DEPTH_COMPONENT32F,         ///< D32 float
          d16_unorm = GL_DEPTH_COMPONENT16,      ///< D16 normalized unsigned int
          d32_f_s8_ui = GL_DEPTH32F_STENCIL8,    ///< D32 float / S8 unsigned int
          d24_unorm_s8_ui = GL_DEPTH24_STENCIL8, ///< D24 normalized unsigned int / S8 unsigned int

          // block-compression (not supported in OpenGL)
          bc6h_uf = GL_NONE, ///< not supported
          bc6h_f = GL_NONE,  ///< not supported
          
          bc7_sRGB  = GL_NONE, ///< not supported
          bc7_unorm = GL_NONE  ///< not supported
        };
        
        /// @brief Primitive topology - vertex interpretation mode (tessellation patches excluded)
        /// @remarks <...>Adj = topology with adjacency: generation of additional adjacent vertices (based on buffer vertices):
        ///                     - those vertices are only visible in the vertex shader and geometry shader;
        ///                     - if some culling is configured before geometry shader stage, adjacent vertices may disappear;
        ///                     - can't be used with tessellation shaders;
        ///                     - has no effect with 'points' topology.
        enum class VertexTopology : GLenum {
          points           = GL_POINTS,                  ///< List of points
          lines            = GL_LINES,                   ///< List of separate line segments
          linesAdj         = GL_LINES_ADJACENCY,         ///< List of separate line segments - with adjacency
          lineStrips       = GL_LINE_STRIP,              ///< Poly-line - line between each vertex and the previous one
          lineStripsAdj    = GL_LINE_STRIP_ADJACENCY,    ///< Poly-line - line between each vertex and the previous one - with adjacency
          triangles        = GL_TRIANGLES,               ///< List of separate triangles
          trianglesAdj     = GL_TRIANGLES_ADJACENCY,     ///< List of separate triangles - with adjacency
          triangleStrip    = GL_TRIANGLE_STRIP,          ///< Triangle-strip - triangle for each vertex and the previous two
          triangleStripAdj = GL_TRIANGLE_STRIP_ADJACENCY ///< Triangle-strip - triangle for each vertex and the previous two - with adjacency
        };
        
        /// @brief Index data formats (vertex index buffers)
        enum class VertexIndexFormat : GLenum {
          r32_ui = GL_R32UI, ///< 32-bit unsigned integers
          r16_ui = GL_R16UI  ///< 16-bit unsigned integers
        };
        /// @brief Depth/stencil data formats (depth/stencil buffers)
        enum class DepthStencilFormat : GLenum {
          d32_f = GL_DEPTH_COMPONENT32F,         ///< 32-bit float depth (recommended)
          d32_f_s8_ui = GL_DEPTH32F_STENCIL8,    ///< 32-bit float depth / 8-bit stencil
          d24_unorm_s8_ui = GL_DEPTH24_STENCIL8, ///< 24-bit float depth / 8-bit stencil (recommended)
          d16_unorm = GL_DEPTH_COMPONENT16,      ///< 16-bit float depth
        };
        /// @brief Verify if a depth/stencil format contains stencil component
        constexpr inline bool _hasStencilComponent(DepthStencilFormat format) noexcept { 
          return (format == DepthStencilFormat::d32_f_s8_ui || format == DepthStencilFormat::d24_unorm_s8_ui); 
        }
        
        
        // -- color/alpha blending --
        
        /// @brief RGBA color component - bit-mask flags
        enum class ColorComponentFlag : int {
          none  = 0,
          red   = 0x1,
          green = 0x2,
          blue  = 0x4,
          alpha = 0x8,
          all   = 0xF
        };
        
        /// @brief Color/alpha blend factor
        /// @remarks When using separate color/alpha blending, the alpha factor must be zero/one or
        ///          a value with a name containing "Alpha" (ex: sourceAlpha, destInvAlpha...).
        enum class BlendFactor : GLenum {
          zero            = GL_ZERO,                 ///< All zero        (0,0,0,0)
          one             = GL_ONE,                  ///< All one         (1,1,1,1)
          sourceColor     = GL_SRC_COLOR,            ///< Source color    (sR,sG,sB,sA)
          sourceInvColor  = GL_ONE_MINUS_SRC_COLOR,  ///< Source opposite (1-sR,1-sG,1-sB,1-sA)
          sourceAlpha     = GL_SRC_ALPHA,            ///< Source alpha    (sA,sA,sA,sA)
          sourceInvAlpha  = GL_ONE_MINUS_SRC_ALPHA,  ///< Src. alpha opposite (1-sA,1-sA,1-sA,1-sA)
          destColor       = GL_DST_COLOR,            ///< Dest. color     (dR,dG,dB,dA)
          destInvColor    = GL_ONE_MINUS_DST_COLOR,  ///< Dest. opposite  (1-dR,1-dG,1-dB,1-dA)
          destAlpha       = GL_DST_ALPHA,            ///< Dest. alpha     (dA,dA,dA,dA)
          destInvAlpha    = GL_ONE_MINUS_DST_ALPHA,  ///< Dest. alpha opposite (1-dA,1-dA,1-dA,1-dA)
          sourceAlphaSat  = GL_SRC_ALPHA_SATURATE,   ///< Alpha saturation clamp (f,f,f,1) with f = min(sA,1-dA)
          dualSrcColor    = GL_SRC1_COLOR,           ///< Dual-source color
          dualSrcInvColor = GL_ONE_MINUS_SRC1_COLOR, ///< Dual-source opposite
          dualSrcAlpha    = GL_SRC1_ALPHA,           ///< Dual-source alpha
          dualSrcInvAlpha = GL_ONE_MINUS_SRC1_ALPHA, ///< Dual-source alpha opposite
          constantColor   = GL_CONSTANT_COLOR,       ///< Constant (cR,cG,cB,cA) - constant color provided at binding (requires 'GL_ARB_imaging' extension)
          constantInvColor= GL_ONE_MINUS_CONSTANT_COLOR,///< Constant (1-cR,1-cG,1-cB,1-cA) - opposite of constant color provided at binding
                                                        ///  (requires 'GL_ARB_imaging' extension)
          constantAlpha   = GL_CONSTANT_ALPHA,          ///< Constant (-,-,-,cA) - constant provided at binding - only for alpha (separate blending)
          constantInvAlpha= GL_ONE_MINUS_CONSTANT_ALPHA ///< Constant (-,-,-,1-cA) - opposite of constant provided at binding - only for alpha (separate blending)
        };
        /// @brief Color/alpha blend operator
        enum class BlendOp : GLenum {
          none        = GL_NONE,                  ///< Disable all blending for render target
          add         = GL_FUNC_ADD,              ///< Source RGBA + dest. RGBA
          subtract    = GL_FUNC_SUBTRACT,         ///< Source RGBA - dest. RGBA
          revSubtract = GL_FUNC_REVERSE_SUBTRACT, ///< Dest. RGBA - source RGBA
          minimum     = GL_MIN,                   ///< min(source RGBA, dest RGBA)
          maximum     = GL_MAX                    ///< max(source RGBA, dest RGBA)
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
        
        /// @brief Depth/stencil comparison type for depth/stencil tests and shadow samplers
        enum class StencilCompare : uint32_t {
          never        = 0u, ///< always fail
          less         = 1u, ///< success: source < existing-ref
          lessEqual    = 2u, ///< success: source <= existing-ref
          equal        = 3u, ///< success: source == existing-ref
          notEqual     = 4u, ///< success: source != existing-ref
          greaterEqual = 5u, ///< success: source >= existing-ref
          greater      = 6u, ///< success: source > existing-ref
          always       = 7u  ///< always succeed (default value)
        };
        /// @brief Depth/stencil operation to perform
        enum class StencilOp : uint32_t {
          keep           = 0u, ///< Keep existing stencil value
          zero           = 1u, ///< Set stencil value to 0
          replace        = 2u, ///< Replace stencil value with reference value
          invert         = 3u, ///< Invert stencil value
          incrementClamp = 4u, ///< Increment stencil value (clamp result)
          decrementClamp = 5u, ///< Decrement stencil value (clamp result)
          incrementWrap  = 6u, ///< Increment stencil value (wrap result)
          decrementWrap  = 7u  ///< Decrement stencil value (wrap result)
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
  _P_FLAGS_OPERATORS(pandora::video::openGL4::ColorComponentFlag, int);
#endif
