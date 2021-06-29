/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Vulkan - bindings with native types (same labels/values as other renderers: only namespace differs)
*******************************************************************************/
#pragma once

#if defined(_VIDEO_VULKAN_SUPPORT)
# include "./vulkan.h"

  namespace pandora {
    namespace video {
      namespace vulkan {
        // -- rasterizer settings --
    
        /// @brief Rasterizer culling mode
        enum class CullMode : int/*VkCullModeFlagBits*/ {
          none      = VK_CULL_MODE_NONE,     ///< No culling / all polygons
          cullBack  = VK_CULL_MODE_BACK_BIT, ///< Back-face culling (hide back-facing polygons)
          cullFront = VK_CULL_MODE_FRONT_BIT ///< Front-face culling (hide front-facing polygons)
        };

        /// @brief Texture addressing mode (out-of-bounds coord management)
        enum class TextureWrap : int/*VkSamplerAddressMode*/ {
          clampToBorder = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,     ///< Coordinates outside of the textures are set to the configured border color (set in sampler descriptor or shader).
          clampToEdge   = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,       ///< Coordinates outside of the textures are set to the edge color.
          repeat        = VK_SAMPLER_ADDRESS_MODE_REPEAT,              ///< Texture is repeated at every junction.
          repeatMirror  = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,     ///< Texture is repeated and flipped at every junction.
          mirrorOnce    = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE ///< Texture is mirrored once horizontally/vertically below '0' coords, then clamped to each edge colors.
        };
        /// @brief Texture minify/magnify/mip-map filter type
        enum class TextureFilter : int/*VkFilter*/ {
          nearest = VK_FILTER_NEAREST, ///< Use nearest point
          linear = VK_FILTER_LINEAR    ///< Linear interpolation
        };
        
        
        // -- color/alpha blending --
        
        /// @brief Color/alpha blend factor
        /// @remarks When using separate color/alpha blending, the alpha factor must be zero/one or
        ///          a value with a name containing "Alpha" (ex: sourceAlpha, destInvAlpha...).
        enum class BlendFactor : int/*VkBlendFactor*/ {
          zero            = VK_BLEND_FACTOR_ZERO,                 ///< All zero        (0,0,0,0)
          one             = VK_BLEND_FACTOR_ONE,                  ///< All one         (1,1,1,1)
          sourceColor     = VK_BLEND_FACTOR_SRC_COLOR,            ///< Source color    (sR,sG,sB,sA)
          sourceInvColor  = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,  ///< Source opposite (1-sR,1-sG,1-sB,1-sA)
          sourceAlpha     = VK_BLEND_FACTOR_SRC_ALPHA,            ///< Source alpha    (sA,sA,sA,sA)
          sourceInvAlpha  = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,  ///< Src. alpha opposite (1-sA,1-sA,1-sA,1-sA)
          destColor       = VK_BLEND_FACTOR_DST_COLOR,            ///< Dest. color     (dR,dG,dB,dA)
          destInvColor    = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,  ///< Dest. opposite  (1-dR,1-dG,1-dB,1-dA)
          destAlpha       = VK_BLEND_FACTOR_DST_ALPHA,            ///< Dest. alpha     (dA,dA,dA,dA)
          destInvAlpha    = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,  ///< Dest. alpha opposite (1-dA,1-dA,1-dA,1-dA)
          sourceAlphaSat  = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,   ///< Alpha saturation clamp (f,f,f,1) with f = min(sA,1-dA)
          dualSrcColor    = VK_BLEND_FACTOR_SRC1_COLOR,           ///< Dual-source color
          dualSrcInvColor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR, ///< Dual-source opposite
          dualSrcAlpha    = VK_BLEND_FACTOR_SRC1_ALPHA,           ///< Dual-source alpha
          dualSrcInvAlpha = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA, ///< Dual-source alpha opposite
          constantColor   = VK_BLEND_FACTOR_CONSTANT_COLOR,       ///< Constant (cR,cG,cB,cA) - constant color provided at binding
          constantInvColor= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,///< Constant (1-cR,1-cG,1-cB,1-cA) - opposite of constant color provided at binding
          constantAlpha   = VK_BLEND_FACTOR_CONSTANT_ALPHA,       ///< Constant (-,-,-,cA) - constant provided at binding - only for alpha (separate blending)
          constantInvAlpha= VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA ///< Constant (-,-,-,1-cA) - opposite of constant provided at binding - only for alpha (separate blending)
        };
        /// @brief Color/alpha blend operator
        enum class BlendOp : int/*VkBlendOp*/ {
          none        = (VkBlendOp)-1,                ///< Disable all blending for render target
          add         = VK_BLEND_OP_ADD,              ///< Source RGBA + dest. RGBA
          subtract    = VK_BLEND_OP_SUBTRACT,         ///< Source RGBA - dest. RGBA
          revSubtract = VK_BLEND_OP_REVERSE_SUBTRACT, ///< Dest. RGBA - source RGBA
          minimum     = VK_BLEND_OP_MIN,              ///< min(source RGBA, dest RGBA)
          maximum     = VK_BLEND_OP_MAX               ///< max(source RGBA, dest RGBA)
        };
        
        
        // -- depth/stencil testing --
        
        /// @brief Depth/stencil comparison type for depth/stencil tests and shadow samplers
        enum class StencilCompare : int/*VkCompareOp*/ {
          never        = VK_COMPARE_OP_NEVER,            ///< always fail
          less         = VK_COMPARE_OP_LESS,             ///< success: source < existing-ref
          lessEqual    = VK_COMPARE_OP_LESS_OR_EQUAL,    ///< success: source <= existing-ref
          equal        = VK_COMPARE_OP_EQUAL,            ///< success: source == existing-ref
          notEqual     = VK_COMPARE_OP_NOT_EQUAL,        ///< success: source != existing-ref
          greaterEqual = VK_COMPARE_OP_GREATER_OR_EQUAL, ///< success: source >= existing-ref
          greater      = VK_COMPARE_OP_GREATER,          ///< success: source > existing-ref
          always       = VK_COMPARE_OP_ALWAYS            ///< always succeed (default value)
        };
        /// @brief Depth/stencil operation to perform
        enum class StencilOp : int/*VkStencilOp*/ {
          keep           = VK_STENCIL_OP_KEEP,               ///< Keep existing stencil value
          zero           = VK_STENCIL_OP_ZERO,               ///< Set stencil value to 0
          replace        = VK_STENCIL_OP_REPLACE,            ///< Replace stencil value with reference value
          invert         = VK_STENCIL_OP_INVERT,             ///< Invert stencil value
          incrementClamp = VK_STENCIL_OP_INCREMENT_AND_CLAMP,///< Increment stencil value (clamp result)
          decrementClamp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,///< Decrement stencil value (clamp result)
          incrementWrap  = VK_STENCIL_OP_INCREMENT_AND_WRAP, ///< Increment stencil value (wrap result)
          decrementWrap  = VK_STENCIL_OP_DECREMENT_AND_WRAP  ///< Decrement stencil value (wrap result)
        };
        
        
        // -- Component data formats --
        
        /// @brief Color/normal/depth/stencil data formats (vertex array buffers, depth/stencil buffers, textures...)
        /// @remarks - HDR rendering / color space: RG(BA) components between 10 and 32 bits ('rgba16_f_scRGB' or 'rgb10a2_unorm_hdr10' recommended).
        ///          - SDR rendering / color space: other component types ('rgba8_sRGB' or 'rgba8_unorm' recommended).
        ///          - HDR rendering on SDR devices: shaders will need to convert color values (or they'll be clipped when displayed).
        enum class DataFormat : int/*VkFormat*/ {
          unknown = VK_FORMAT_UNDEFINED, ///< Unknown/empty format
          
          // HDR / 32 bit
          rgba32_f = VK_FORMAT_R32G32B32A32_SFLOAT,///< R32G32B32A32 float
          rgb32_f = VK_FORMAT_R32G32B32_SFLOAT,    ///< R32G32B32 float
          rg32_f = VK_FORMAT_R32G32_SFLOAT,        ///< R32G32 float
          r32_f = VK_FORMAT_R32_SFLOAT,            ///< R32 float
          
          rgba32_ui = VK_FORMAT_R32G32B32A32_UINT,///< R32G32B32A32 unsigned int
          rgb32_ui = VK_FORMAT_R32G32B32_UINT,    ///< R32G32B32 unsigned int
          rg32_ui = VK_FORMAT_R32G32_UINT,        ///< R32G32 unsigned int
          r32_ui = VK_FORMAT_R32_UINT,            ///< R32 unsigned int

          rgba32_i = VK_FORMAT_R32G32B32A32_SINT,///< R32G32B32A32 int
          rgb32_i = VK_FORMAT_R32G32B32_SINT,    ///< R32G32B32 int
          rg32_i = VK_FORMAT_R32G32_SINT,        ///< R32G32 int
          r32_i = VK_FORMAT_R32_SINT,            ///< R32 int
          
          // HDR / 16 bit
          rgba16_f_scRGB = VK_FORMAT_R16G16B16A16_SFLOAT, ///< R16G16B16A16 float
          rg16_f = VK_FORMAT_R16G16_SFLOAT,               ///< R16G16 float
          r16_f = VK_FORMAT_R16_SFLOAT,                   ///< R16 float
          
          rgba16_ui = VK_FORMAT_R16G16B16A16_UINT,///< R16G16B16A16 unsigned int
          rg16_ui = VK_FORMAT_R16G16_UINT,        ///< R16G16 unsigned int
          r16_ui = VK_FORMAT_R16_UINT,            ///< R16 unsigned int
          
          rgba16_i = VK_FORMAT_R16G16B16A16_SINT,///< R16G16B16A16 int
          rg16_i = VK_FORMAT_R16G16_SINT,        ///< R16G16 int
          r16_i = VK_FORMAT_R16_SINT,            ///< R16 int
          
          rgba16_unorm = VK_FORMAT_R16G16B16A16_UNORM,///< R16G16B16A16 normalized unsigned int
          rg16_unorm = VK_FORMAT_R16G16_UNORM,        ///< R16G16 normalized unsigned int
          r16_unorm = VK_FORMAT_R16_UNORM,            ///< R16 normalized unsigned int
          
          rgba16_snorm = VK_FORMAT_R16G16B16A16_SNORM,///< R16G16B16A16 normalized int
          rg16_snorm = VK_FORMAT_R16G16_SNORM,        ///< R16G16 normalized int
          r16_snorm = VK_FORMAT_R16_SNORM,            ///< R16 normalized int
          
          // HDR / 10-11 bit
          rgb10a2_unorm_hdr10 = VK_FORMAT_A2R10G10B10_UNORM_PACK32, ///< HDR-10/BT.2100: R10G10B10A2 normalized unsigned int
          rgb10a2_ui = VK_FORMAT_A2R10G10B10_UINT_PACK32, ///< R10G10B10A2 unsigned int
          rg11b10_f = VK_FORMAT_B10G11R11_UFLOAT_PACK32,   ///< R11G11B10 float
          
          // 9 bit
          rgb9e5_uf = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, ///< R9G9B9E5 float (shared exponent)
          
          // 8 bit
          rgba8_ui = VK_FORMAT_R8G8B8A8_UINT,///< R8G8B8A8 unsigned int
          rg8_ui = VK_FORMAT_R8G8_UINT,      ///< R8G8 unsigned int
          r8_ui = VK_FORMAT_R8_UINT,         ///< R8 unsigned int
          
          rgba8_i = VK_FORMAT_R8G8B8A8_SINT,///< R8G8B8A8 int
          rg8_i = VK_FORMAT_R8G8_SINT,      ///< R8G8 int
          r8_i = VK_FORMAT_R8_SINT,         ///< R8 int
          
          rgba8_sRGB = VK_FORMAT_R8G8B8A8_SRGB, ///< R8G8B8A8 normalized unsigned int (sRGB)
          bgra8_sRGB = VK_FORMAT_B8G8R8A8_SRGB, ///< B8G8R8A8 normalized unsigned int (sRGB)
          rgba8_unorm = VK_FORMAT_R8G8B8A8_UNORM,///< R8G8B8A8 normalized unsigned int
          bgra8_unorm = VK_FORMAT_B8G8R8A8_UNORM,///< B8G8R8A8 normalized unsigned int
          rg8_unorm = VK_FORMAT_R8G8_UNORM,      ///< R8G8 normalized unsigned int
          r8_unorm = VK_FORMAT_R8_UNORM,         ///< R8 normalized unsigned int
          a8_unorm = VK_FORMAT_R8_SNORM,         ///< A8 normalized unsigned int
          
          rgba8_snorm = VK_FORMAT_R8G8B8A8_SNORM,///< R8G8B8A8 normalized int
          rg8_snorm = VK_FORMAT_R8G8_SNORM,      ///< R8G8 normalized int
          r8_snorm = VK_FORMAT_R8_SNORM,         ///< R8 normalized int
          
          // 4-6 bit
          rgb5a1_unorm = VK_FORMAT_B5G5R5A1_UNORM_PACK16, ///< B5G5R5A1 normalized unsigned int
          r5g6b5_unorm = VK_FORMAT_B5G6R5_UNORM_PACK16,   ///< B5G6R5 normalized unsigned int
          rgba4_unorm = VK_FORMAT_B4G4R4A4_UNORM_PACK16,  ///< B4G4R4A4 normalized unsigned int
          
          // depth/stencil
          d32_f = VK_FORMAT_D32_SFLOAT,                 ///< D32 float
          d16_unorm = VK_FORMAT_D16_UNORM,              ///< D16 normalized unsigned int
          d32_f_s8_ui = VK_FORMAT_D32_SFLOAT_S8_UINT,   ///< D32 float / S8 unsigned int
          d24_unorm_s8_ui = VK_FORMAT_D24_UNORM_S8_UINT,///< D24 normalized unsigned int / S8 unsigned int

          // block-compression (not supported in OpenGL)
          bc6h_uf = VK_FORMAT_BC6H_UFLOAT_BLOCK, ///< BC6H unsigned float
          bc6h_f = VK_FORMAT_BC6H_SFLOAT_BLOCK,  ///< BC6H float
          
          bc7_sRGB  = VK_FORMAT_BC7_SRGB_BLOCK, ///< BC7 normalized unsigned int (sRGB)
          bc7_unorm = VK_FORMAT_BC7_UNORM_BLOCK ///< BC7 normalized unsigned int
        };
        
        /// @brief Primitive topology - vertex interpretation mode (tessellation patches excluded)
        /// @remarks <...>Adj = topology with adjacency: generation of additional adjacent vertices (based on buffer vertices):
        ///                     - those vertices are only visible in the vertex shader and geometry shader;
        ///                     - if some culling is configured before geometry shader stage, adjacent vertices may disappear;
        ///                     - can't be used with tessellation shaders;
        ///                     - has no effect with 'points' topology.
        enum class VertexTopology : int/*VkPrimitiveTopology*/ {
          points           = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,                   ///< List of points
          lines            = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,                    ///< List of separate line segments
          linesAdj         = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,     ///< List of separate line segments - with adjacency
          lineStrips       = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,                   ///< Poly-line - line between each vertex and the previous one
          lineStripsAdj    = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,    ///< Poly-line - line between each vertex and the previous one - with adjacency
          triangles        = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                ///< List of separate triangles
          trianglesAdj     = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY, ///< List of separate triangles - with adjacency
          triangleStrip    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,               ///< Triangle-strip - triangle for each vertex and the previous two
          triangleStripAdj = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY ///< Triangle-strip - triangle for each vertex and the previous two - with adjacency
        };
        /// @brief Index data formats (vertex index buffers)
        enum class VertexIndexFormat : int/*VkFormat*/ {
          r32_ui = VK_FORMAT_R32_UINT, ///< 32-bit unsigned integers
          r16_ui = VK_FORMAT_R16_UINT  ///< 16-bit unsigned integers
        };
        /// @brief Depth/stencil data formats (depth/stencil buffers)
        enum class DepthStencilFormat : int/*VkFormat*/ {
          d32_f = VK_FORMAT_D32_SFLOAT,                  ///< 32-bit float depth (recommended)
          d32_f_s8_ui = VK_FORMAT_D32_SFLOAT_S8_UINT,    ///< 32-bit float depth / 8-bit stencil
          d24_unorm_s8_ui = VK_FORMAT_D24_UNORM_S8_UINT, ///< 24-bit float depth / 8-bit stencil (recommended)
          d16_unorm = VK_FORMAT_D16_UNORM,               ///< 16-bit float depth
        };
      }
    }
  }
#endif
