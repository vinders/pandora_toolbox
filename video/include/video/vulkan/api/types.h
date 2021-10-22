/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------------
Vulkan - bindings with native types (same labels/values as other renderers: only namespace differs)
*******************************************************************************/
#pragma once

#if defined(_VIDEO_VULKAN_SUPPORT)
# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(push)
#   pragma warning(disable: 26812) // disable warnings about vulkan enums
# endif
# include <cstdint>
# include "./vulkan.h"
# include <system/_private/_enum_flags.h>
# include "./_private/_dynamic_array.h"
# ifndef VK_API_VERSION_MAJOR
#   define VK_API_VERSION_MAJOR(version) VK_VERSION_MAJOR(version)
#   define VK_API_VERSION_MINOR(version) VK_VERSION_MINOR(version)
# endif
# define __P_VK_API_VERSION_NOVARIANT(version) (version & 0x1FFFFFFFu)

  namespace pandora {
    namespace video {
      namespace vulkan {
        using DeviceHandle = VkPhysicalDevice;  ///< Physical device access/resources
        using DeviceContext = VkDevice;         ///< Device rendering context
        using DeviceResourceManager = VkDevice; ///< Device resource manager
        using SwapChainHandle = VkSwapchainKHR; ///< Swap-chain (framebuffer container/swapper)
        
        using TextureHandle = VkImage;          ///< Generic texture resource handle
        using TextureHandle1D = VkImage;        ///< 1D texture resource container
        using TextureHandle2D = VkImage;        ///< 2D texture resource container
        using TextureHandle3D = VkImage;        ///< 3D texture resource container
        using BufferHandle = VkBuffer;          ///< Vertex/index/constant/resource buffer
        using RenderTargetView = VkImageView; ///< Bindable render-target view for renderer (shader output buffer)
        using DepthStencilView = VkImageView;   ///< Bindable depth/stencil view for renderer
        using TextureView = VkImageView;        ///< Bindable texture view for shaders
        using ColorChannel = float;             ///< R/G/B/A color value
        
        struct InputLayoutDescription final {
          DynamicArray<VkVertexInputBindingDescription> bindings;
          DynamicArray<VkVertexInputAttributeDescription> attributes;
        };
        using InputLayoutHandle = const InputLayoutDescription*; ///< Input layout representation, for shader input stage
        
        
        // -- rasterizer settings --
        
        /// @brief Rasterizer filling mode
        enum class FillMode : int/*VkPolygonMode*/ {
          fill  = VK_POLYGON_MODE_FILL, ///< Filled/solid polygons
          lines = VK_POLYGON_MODE_LINE, ///< Lines/wireframe
          linesAA = (VK_POLYGON_MODE_LINE | 0x8000000) ///< Anti-aliased lines/wireframe
        };
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
        
        
        // -- component data formats --

        /// @brief SDR/HDR RGB color space
        enum class ColorSpace : int/*VkColorSpaceKHR*/ {
          unknown      = -1,
          sRgb         = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, ///< SDR sRGB non-linear color space (rgba8_sRGB / rgba8_unorm)
          scRgb        = VK_COLOR_SPACE_BT709_LINEAR_EXT,   ///< HDR scRGB linear color space (rgba16_f_scRGB)
          hdr10_bt2084 = VK_COLOR_SPACE_HDR10_ST2084_EXT    ///< HDR-10 / BT.2084 non-linear color space (rgb10a2_unorm_hdr10)
        };
        
        /// @brief RGBA color component - bit-mask flags
        enum class ColorComponentFlag : int/*VkColorComponentFlagBits*/ {
          none  = 0,
          red   = VK_COLOR_COMPONENT_R_BIT,
          green = VK_COLOR_COMPONENT_G_BIT,
          blue  = VK_COLOR_COMPONENT_B_BIT,
          alpha = VK_COLOR_COMPONENT_A_BIT,
          all   = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
        };
        
#       define __P_VULKAN_FORMAT(vulkanFormat, bytesPerPixel)  ((uint64_t)vulkanFormat | (((uint64_t)bytesPerPixel) << 32))
        
        /// @brief Color/normal/depth/stencil data formats (vertex array buffers, depth/stencil buffers, textures...)
        /// @remarks - HDR rendering / color space: RG(BA) components between 10 and 32 bits ('rgba16_f_scRGB' or 'rgb10a2_unorm_hdr10' recommended).
        ///          - SDR rendering / color space: other component types ('rgba8_sRGB' or 'rgba8_unorm' recommended).
        ///          - HDR rendering on SDR devices: shaders will need to convert color values (or they'll be clipped when displayed).
        ///          - Additional data formats can be created using: createDataFormat(VkFormat,bytesPerPixel).
        enum class DataFormat : uint64_t/*VkFormat + size*/ {
          unknown = VK_FORMAT_UNDEFINED, ///< Unknown/empty format
          
          // HDR / 32 bit
          rgba32_f = __P_VULKAN_FORMAT(VK_FORMAT_R32G32B32A32_SFLOAT,16),///< R32G32B32A32 float
          rgb32_f = __P_VULKAN_FORMAT(VK_FORMAT_R32G32B32_SFLOAT,12),    ///< R32G32B32 float
          rg32_f = __P_VULKAN_FORMAT(VK_FORMAT_R32G32_SFLOAT, 8),        ///< R32G32 float
          r32_f = __P_VULKAN_FORMAT(VK_FORMAT_R32_SFLOAT, 4),            ///< R32 float
          
          rgba32_ui = __P_VULKAN_FORMAT(VK_FORMAT_R32G32B32A32_UINT,16),///< R32G32B32A32 unsigned int
          rgb32_ui = __P_VULKAN_FORMAT(VK_FORMAT_R32G32B32_UINT,12),    ///< R32G32B32 unsigned int
          rg32_ui = __P_VULKAN_FORMAT(VK_FORMAT_R32G32_UINT, 8),        ///< R32G32 unsigned int
          r32_ui = __P_VULKAN_FORMAT(VK_FORMAT_R32_UINT, 4),            ///< R32 unsigned int

          rgba32_i = __P_VULKAN_FORMAT(VK_FORMAT_R32G32B32A32_SINT,16),///< R32G32B32A32 int
          rgb32_i = __P_VULKAN_FORMAT(VK_FORMAT_R32G32B32_SINT,12),    ///< R32G32B32 int
          rg32_i = __P_VULKAN_FORMAT(VK_FORMAT_R32G32_SINT, 8),        ///< R32G32 int
          r32_i = __P_VULKAN_FORMAT(VK_FORMAT_R32_SINT, 4),            ///< R32 int
          
          // HDR / 16 bit
          rgba16_f_scRGB = __P_VULKAN_FORMAT(VK_FORMAT_R16G16B16A16_SFLOAT, 8), ///< R16G16B16A16 float
          rg16_f = __P_VULKAN_FORMAT(VK_FORMAT_R16G16_SFLOAT, 4),               ///< R16G16 float
          r16_f = __P_VULKAN_FORMAT(VK_FORMAT_R16_SFLOAT, 2),                   ///< R16 float
          
          rgba16_ui = __P_VULKAN_FORMAT(VK_FORMAT_R16G16B16A16_UINT, 8),///< R16G16B16A16 unsigned int
          rg16_ui = __P_VULKAN_FORMAT(VK_FORMAT_R16G16_UINT, 4),        ///< R16G16 unsigned int
          r16_ui = __P_VULKAN_FORMAT(VK_FORMAT_R16_UINT, 2),            ///< R16 unsigned int
          
          rgba16_i = __P_VULKAN_FORMAT(VK_FORMAT_R16G16B16A16_SINT, 8),///< R16G16B16A16 int
          rg16_i = __P_VULKAN_FORMAT(VK_FORMAT_R16G16_SINT, 4),        ///< R16G16 int
          r16_i = __P_VULKAN_FORMAT(VK_FORMAT_R16_SINT, 2),            ///< R16 int
          
          rgba16_unorm = __P_VULKAN_FORMAT(VK_FORMAT_R16G16B16A16_UNORM, 8),///< R16G16B16A16 normalized unsigned int
          rg16_unorm = __P_VULKAN_FORMAT(VK_FORMAT_R16G16_UNORM, 4),        ///< R16G16 normalized unsigned int
          r16_unorm = __P_VULKAN_FORMAT(VK_FORMAT_R16_UNORM, 2),            ///< R16 normalized unsigned int
          
          rgba16_snorm = __P_VULKAN_FORMAT(VK_FORMAT_R16G16B16A16_SNORM, 8),///< R16G16B16A16 normalized int
          rg16_snorm = __P_VULKAN_FORMAT(VK_FORMAT_R16G16_SNORM, 4),        ///< R16G16 normalized int
          r16_snorm = __P_VULKAN_FORMAT(VK_FORMAT_R16_SNORM, 2),            ///< R16 normalized int
          
          // HDR / 10-11 bit
          rgb10a2_unorm_hdr10 = __P_VULKAN_FORMAT(VK_FORMAT_A2R10G10B10_UNORM_PACK32, 4), ///< HDR-10/BT.2100: R10G10B10A2 normalized unsigned int
          rgb10a2_ui = __P_VULKAN_FORMAT(VK_FORMAT_A2R10G10B10_UINT_PACK32, 4), ///< R10G10B10A2 unsigned int
          rg11b10_f = __P_VULKAN_FORMAT(VK_FORMAT_B10G11R11_UFLOAT_PACK32, 4),   ///< R11G11B10 float
          
          // 9 bit
          rgb9e5_uf = __P_VULKAN_FORMAT(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, 4), ///< R9G9B9E5 float (shared exponent)
          
          // 8 bit
          rgba8_ui = __P_VULKAN_FORMAT(VK_FORMAT_R8G8B8A8_UINT, 4),///< R8G8B8A8 unsigned int
          rg8_ui = __P_VULKAN_FORMAT(VK_FORMAT_R8G8_UINT, 2),      ///< R8G8 unsigned int
          r8_ui = __P_VULKAN_FORMAT(VK_FORMAT_R8_UINT, 1),         ///< R8 unsigned int
          
          rgba8_i = __P_VULKAN_FORMAT(VK_FORMAT_R8G8B8A8_SINT, 4),///< R8G8B8A8 int
          rg8_i = __P_VULKAN_FORMAT(VK_FORMAT_R8G8_SINT, 2),      ///< R8G8 int
          r8_i = __P_VULKAN_FORMAT(VK_FORMAT_R8_SINT, 1),         ///< R8 int
          
          rgba8_sRGB = __P_VULKAN_FORMAT(VK_FORMAT_R8G8B8A8_SRGB, 4), ///< R8G8B8A8 normalized unsigned int (sRGB)
          bgra8_sRGB = __P_VULKAN_FORMAT(VK_FORMAT_B8G8R8A8_SRGB, 4), ///< B8G8R8A8 normalized unsigned int (sRGB)
          rgba8_unorm = __P_VULKAN_FORMAT(VK_FORMAT_R8G8B8A8_UNORM, 4),///< R8G8B8A8 normalized unsigned int
          bgra8_unorm = __P_VULKAN_FORMAT(VK_FORMAT_B8G8R8A8_UNORM, 4),///< B8G8R8A8 normalized unsigned int
          rg8_unorm = __P_VULKAN_FORMAT(VK_FORMAT_R8G8_UNORM, 2),      ///< R8G8 normalized unsigned int
          r8_unorm = __P_VULKAN_FORMAT(VK_FORMAT_R8_UNORM, 1),         ///< R8 normalized unsigned int
          a8_unorm = __P_VULKAN_FORMAT(VK_FORMAT_R8_SNORM, 1),         ///< A8 normalized unsigned int
          
          rgba8_snorm = __P_VULKAN_FORMAT(VK_FORMAT_R8G8B8A8_SNORM, 4),///< R8G8B8A8 normalized int
          rg8_snorm = __P_VULKAN_FORMAT(VK_FORMAT_R8G8_SNORM, 2),      ///< R8G8 normalized int
          r8_snorm = __P_VULKAN_FORMAT(VK_FORMAT_R8_SNORM, 1),         ///< R8 normalized int
          
          // 4-6 bit
          rgb5a1_unorm = __P_VULKAN_FORMAT(VK_FORMAT_B5G5R5A1_UNORM_PACK16, 2), ///< B5G5R5A1 normalized unsigned int
          r5g6b5_unorm = __P_VULKAN_FORMAT(VK_FORMAT_B5G6R5_UNORM_PACK16, 2),   ///< B5G6R5 normalized unsigned int
          rgba4_unorm = __P_VULKAN_FORMAT(VK_FORMAT_B4G4R4A4_UNORM_PACK16, 2),  ///< B4G4R4A4 normalized unsigned int
          
          // depth/stencil
          d32_f = __P_VULKAN_FORMAT(VK_FORMAT_D32_SFLOAT, 4),                 ///< D32 float
          d16_unorm = __P_VULKAN_FORMAT(VK_FORMAT_D16_UNORM, 2),              ///< D16 normalized unsigned int
          d32_f_s8_ui = __P_VULKAN_FORMAT(VK_FORMAT_D32_SFLOAT_S8_UINT, 8),   ///< D32 float / S8 unsigned int
          d24_unorm_s8_ui = __P_VULKAN_FORMAT(VK_FORMAT_D24_UNORM_S8_UINT, 4),///< D24 normalized unsigned int / S8 unsigned int
        };
        
        /// @brief Create additional DataFormat from native VkFormat (example: createDataFormat(VK_FORMAT_BC6H_SFLOAT_BLOCK,6))
        constexpr inline DataFormat createDataFormat(VkFormat format, uint32_t bytesPerPixel) noexcept { 
          return (DataFormat)__P_VULKAN_FORMAT(format, bytesPerPixel); 
        }
        constexpr inline VkFormat _getDataFormatComponents(DataFormat format) noexcept { return static_cast<VkFormat>((uint64_t)format & 0xFFFFFFFFuLL); }
#       undef __P_VULKAN_FORMAT
        
        /// @brief Primitive topology - vertex interpretation mode (tessellation patches excluded)
        /// @remarks <...>Adj = topology with adjacency: generation of additional adjacent vertices (based on buffer vertices):
        ///                     - those vertices are only visible in the vertex shader and geometry shader;
        ///                     - if some culling is configured before geometry shader stage, adjacent vertices may disappear;
        ///                     - can't be used with tessellation shaders.
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
        /// @brief Verify if a depth/stencil format contains stencil component
        constexpr inline bool hasStencilComponent(DepthStencilFormat format) noexcept { 
          return (format == DepthStencilFormat::d32_f_s8_ui || format == DepthStencilFormat::d24_unorm_s8_ui); 
        }
        
        /// @brief Basic StaticBuffer / DynamicBuffer content type
        enum class BaseBufferType : int/*VkBufferUsageFlagBits*/ {
          uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,  ///< Constant/uniform data buffer for shader stage(s): 
                                                         ///  * can be bound with any shader stage(s): Renderer.bind<...>ConstantBuffers.
                                                         ///  * should contain data useful as a whole for shaders:
                                                         ///    -> buffer entirely copied in GPU cache during Draw calls, to be available for each vertex/pixel.
                                                         ///    -> recommended for view/projection/world matrices, transformations, lights and options.
                                                         ///    -> not appropriate for big data blocks of which only a few bytes are read by each vertex/pixel.
          vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,    ///< Vertex data buffer for vertex shader
                                                         ///  * can be bound with vertex shader stage: Renderer.bindVertexArrayBuffer(s).
                                                         ///  * contains vertices to process in renderer.
                                                         ///  * can be used with an optional index buffer, to improve speed and bandwidth.
                                                         ///  * can also be used as instance buffers, to duplicate meshes many times (example: trees, leaves...).
                                                         ///  * Common practice: - geometry centered around (0;0;0) -> vertex buffers;
                                                         ///                     - world matrix to offset the entire model in the environment -> combined with camera view into constant buffer;
                                                         ///                     - vertices repositioned in vertex shader by world/view matrix and projection matrix.
          vertexIndex = VK_BUFFER_USAGE_INDEX_BUFFER_BIT ///< Indices of vertex buffer(s)
                                                         ///  * can be bound with vertex shader stage: Renderer.bindVertexArrayBuffer(s).
                                                         ///  * optionally used with vertex array buffer(s), to improve speed and bandwidth.
                                                         ///  * contains indices to allow removal of redundant/common vertices from associated vertex array buffer(s).
        };
        /// @brief Resource / texture memory usage - ResourceBuffer / Texture<...>
        enum class ResourceUsage : int/*VkMemoryPropertyFlagBits*/ {
          immutable = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ///< Immutable: GPU memory initialized at creation, not visible from CPU:
                                                           ///          - fastest GPU access: ideal for static resources that don't change (textures, geometry...);
          staticGpu = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT|VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,///< Static: GPU memory, not directly visible from CPU:
                                                           ///          - fast GPU access: ideal for static resources rarely updated or for small resources;
                                                           ///          - indirect CPU write access: should be used when NOT updated frequently (or if data is small);
                                                           ///          - mapped if supported by GPU / defaults to 'VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT' if not supported (indirect CPU access).
          dynamicCpu = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,///< Dynamic: CPU mappable memory, visible from GPU (through PCIe):
                                                           ///           - slow GPU read access: should be used when data isn't kept for multiple frames;
                                                           ///           - very fast CPU write access: ideal for constants/uniforms and vertices/indices re-written by CPU every frame.
          staging   = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  ///< Staging: CPU-only mappable memory:
                                                           ///           - no GPU access: should not be used for display or render-targets;
                                                           ///           - very fast CPU read/write access: usable to manipulate resource data, or to read buffer content from CPU;
                                                           ///           - staged content is usually used to populate/read/update static resources (textures, geometry...)
                                                           ///             -> copied with data transfers from/to staticGpu resources (which can be accessed by the GPU).
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
        
        
        // -- shaders --
        
        /// @brief Shader pipeline stage type
        enum class ShaderType : int/*VkShaderStageFlagBits*/ {
          vertex   = VK_SHADER_STAGE_VERTEX_BIT,   ///< Vertex shader: process input vertex data -> outputs vertex projection.
          tessCtrl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,    ///< Tessellation control/hull shader: tessellate primitive -> outputs geometry patch.
          tessEval = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, ///< Tessellation evaluation/domain shader: calculate new vertex positions.
          geometry = VK_SHADER_STAGE_GEOMETRY_BIT, ///< Geometry shader: modify/duplicate primitive.
          fragment = VK_SHADER_STAGE_FRAGMENT_BIT, ///< Fragment/pixel shader: process rasterized fragment -> outputs pixel/depth data.
          compute  = VK_SHADER_STAGE_COMPUTE_BIT   ///< Compute shader: GPU calculations (not supported below OpenGL 4.3).
        };
      }
    }
  }
  _P_FLAGS_OPERATORS(pandora::video::vulkan::ColorComponentFlag, int);
# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
