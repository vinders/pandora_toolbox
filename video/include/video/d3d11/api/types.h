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
Direct3D11 - bindings with native types (same labels/values as other renderers: only namespace differs)
*******************************************************************************/
#pragma once

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# ifndef _P_NO_WIN32_FILTER
#   define NOMINMAX
#   define NODRAWTEXT
#   define NOGDI
#   define NOBITMAP
#   define NOMCX
#   define NOSERVICE
# endif
# include <cstdint>
# include "./d3d_11.h"
# include <system/_private/_enum_flags.h>

  namespace pandora {
    namespace video {
      namespace d3d11 {
        using DeviceHandle = ID3D11Device*;               ///< Physical device access/resources
        using DeviceContext = ID3D11DeviceContext*;       ///< Device rendering context
        using DeviceResourceManager = ID3D11Device*;      ///< Device resource manager
        using SwapChainHandle = void*;                    ///< Swap-chain (framebuffer container/swapper) - IDXGISwapChain*/IDXGISwapChain1*
        
        using TextureHandle = ID3D11Resource*;            ///< Generic texture resource handle
        using TextureHandle1D = ID3D11Texture1D*;         ///< 1D texture resource container
        using TextureHandle2D = ID3D11Texture2D*;         ///< 2D texture resource container
        using TextureHandle3D = ID3D11Texture3D*;         ///< 3D texture resource container
        using BufferHandle = ID3D11Buffer*;               ///< Vertex/index/constant/resource buffer
        using RenderTargetView = ID3D11RenderTargetView*; ///< Bindable render-target view for renderer (shader output buffer)
        using DepthStencilView = ID3D11DepthStencilView*; ///< Bindable depth/stencil view for renderer
        using TextureView = ID3D11ShaderResourceView*;    ///< Bindable texture view for shaders
        
        using InputLayoutHandle = ID3D11InputLayout*;     ///< Input layout representation, for shader input stage
        using ColorChannel = FLOAT;                       ///< R/G/B/A color value
        
        
        // -- rasterizer settings --
        
        /// @brief Rasterizer filling mode
        enum class FillMode : int/*D3D11_FILL_MODE*/ {
          fill    = D3D11_FILL_SOLID,     ///< Filled/solid polygons
          lines   = D3D11_FILL_WIREFRAME, ///< Lines/wireframe
          linesAA = (D3D11_FILL_WIREFRAME | 0x8000000) ///< Anti-aliased lines/wireframe
        };
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

        /// @brief SDR/HDR RGB color space
        enum class ColorSpace : int/*DXGI_COLOR_SPACE_TYPE*/ {
          unknown      = -1,
          sRgb         = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709,    ///< SDR sRGB non-linear color space (rgba8_sRGB / rgba8_unorm)
          scRgb        = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709,    ///< HDR scRGB linear color space (rgba16_f_scRGB)
          hdr10_bt2084 = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020  ///< HDR-10 / BT.2084 non-linear color space (rgb10a2_unorm_hdr10)
        };
        
        /// @brief RGBA color component - bit-mask flags
        enum class ColorComponentFlag : int/*D3D11_COLOR_WRITE_ENABLE*/ {
          none  = 0,
          red   = D3D11_COLOR_WRITE_ENABLE_RED,
          green = D3D11_COLOR_WRITE_ENABLE_GREEN,
          blue  = D3D11_COLOR_WRITE_ENABLE_BLUE,
          alpha = D3D11_COLOR_WRITE_ENABLE_ALPHA,
          all   = D3D11_COLOR_WRITE_ENABLE_ALL
        };
        
#       define __P_D3D11_BIND_NONE  0x00000000u // usually not supported
#       define __P_D3D11_BIND_COLOR 0x40000000u // valid for color attachment
#       define __P_D3D11_BIND_CLRBL 0xC0000000u // valid for color attachment with blending
#       define __P_D3D11_BIND_DEPTH 0x80000000u // valid for depth attachment

        /// @brief Data format attachment types
        enum class FormatAttachment : uint32_t {
          unknown = __P_D3D11_BIND_NONE,       ///< Unknown format support
          color = __P_D3D11_BIND_COLOR,        ///< Format supported as color attachment
          colorBlend = __P_D3D11_BIND_CLRBL,   ///< Format supported as color attachment with blending allowed
          depthStencil = __P_D3D11_BIND_DEPTH, ///< Format supported as depth/stencil attachment
        };
        
#       define __P_D3D11_FORMAT(dxgiFormat, bytesPerPixel, attachment)  ((uint32_t)dxgiFormat | (((uint32_t)bytesPerPixel) << 24) | (uint32_t)attachment)
        
        /// @brief Color/normal/depth/stencil data formats (vertex array buffers, depth/stencil buffers, textures...)
        /// @remarks - HDR rendering / color space: RG(BA) components between 10 and 32 bits ('rgba16_f_scRGB' or 'rgb10a2_unorm_hdr10' recommended).
        ///          - SDR rendering / color space: other component types ('rgba8_sRGB' or 'rgba8_unorm' recommended).
        ///          - HDR rendering on SDR devices: shaders will need to convert color values (or they'll be clipped when displayed).
        ///          - Additional data formats can be created using: createDataFormat(DXGI_FORMAT,bytesPerPixel,FormatAttachment).
        enum class DataFormat : uint32_t/*DXGI_FORMAT + size*/ {
          unknown = DXGI_FORMAT_UNKNOWN, ///< Unknown/empty format
          
          // HDR / 32 bit
          rgba32_f = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32B32A32_FLOAT, 16, __P_D3D11_BIND_CLRBL),///< R32G32B32A32 float
          rgb32_f = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32B32_FLOAT, 12, __P_D3D11_BIND_NONE),     ///< R32G32B32 float
          rg32_f = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32_FLOAT, 8, __P_D3D11_BIND_CLRBL),         ///< R32G32 float
          r32_f = __P_D3D11_FORMAT(DXGI_FORMAT_R32_FLOAT, 4, __P_D3D11_BIND_CLRBL),             ///< R32 float
          
          rgba32_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32B32A32_UINT, 16, __P_D3D11_BIND_COLOR),///< R32G32B32A32 unsigned int
          rgb32_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32B32_UINT, 12, __P_D3D11_BIND_NONE),     ///< R32G32B32 unsigned int
          rg32_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32_UINT, 8, __P_D3D11_BIND_COLOR),         ///< R32G32 unsigned int
          r32_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R32_UINT, 4, __P_D3D11_BIND_COLOR),             ///< R32 unsigned int

          rgba32_i = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32B32A32_SINT, 16, __P_D3D11_BIND_COLOR),///< R32G32B32A32 int
          rgb32_i = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32B32_SINT, 12, __P_D3D11_BIND_NONE),     ///< R32G32B32 int
          rg32_i = __P_D3D11_FORMAT(DXGI_FORMAT_R32G32_SINT, 8, __P_D3D11_BIND_COLOR),         ///< R32G32 int
          r32_i = __P_D3D11_FORMAT(DXGI_FORMAT_R32_SINT, 4, __P_D3D11_BIND_COLOR),             ///< R32 int
          
          // HDR / 16 bit
          rgba16_f_scRGB = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16B16A16_FLOAT, 8, __P_D3D11_BIND_CLRBL), ///< R16G16B16A16 float
          rg16_f = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16_FLOAT, 4, __P_D3D11_BIND_CLRBL),        ///< R16G16 float
          r16_f = __P_D3D11_FORMAT(DXGI_FORMAT_R16_FLOAT, 2, __P_D3D11_BIND_CLRBL),            ///< R16 float
          
          rgba16_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16B16A16_UINT, 8, __P_D3D11_BIND_COLOR),///< R16G16B16A16 unsigned int
          rg16_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16_UINT, 4, __P_D3D11_BIND_COLOR),        ///< R16G16 unsigned int
          r16_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R16_UINT, 2, __P_D3D11_BIND_COLOR),            ///< R16 unsigned int
          
          rgba16_i = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16B16A16_SINT, 8, __P_D3D11_BIND_COLOR), ///< R16G16B16A16 int
          rg16_i = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16_SINT, 4, __P_D3D11_BIND_COLOR),         ///< R16G16 int
          r16_i = __P_D3D11_FORMAT(DXGI_FORMAT_R16_SINT, 2, __P_D3D11_BIND_COLOR),             ///< R16 int
          
          rgba16_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16B16A16_UNORM, 8, __P_D3D11_BIND_CLRBL),///< R16G16B16A16 normalized unsigned int
          rg16_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16_UNORM, 4, __P_D3D11_BIND_CLRBL),        ///< R16G16 normalized unsigned int
          r16_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_R16_UNORM, 2, __P_D3D11_BIND_CLRBL),            ///< R16 normalized unsigned int
          
          rgba16_snorm = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16B16A16_SNORM, 8, __P_D3D11_BIND_CLRBL),///< R16G16B16A16 normalized int
          rg16_snorm = __P_D3D11_FORMAT(DXGI_FORMAT_R16G16_SNORM, 4, __P_D3D11_BIND_CLRBL),        ///< R16G16 normalized int
          r16_snorm = __P_D3D11_FORMAT(DXGI_FORMAT_R16_SNORM, 2, __P_D3D11_BIND_CLRBL),            ///< R16 normalized int
          
          // HDR / 10-11 bit
          rgb10a2_unorm_hdr10 = __P_D3D11_FORMAT(DXGI_FORMAT_R10G10B10A2_UNORM, 4, __P_D3D11_BIND_CLRBL), ///< HDR-10/BT.2100: R10G10B10A2 normalized unsigned int
          rgb10a2_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R10G10B10A2_UINT, 4, __P_D3D11_BIND_NONE), ///< R10G10B10A2 unsigned int
          rg11b10_f = __P_D3D11_FORMAT(DXGI_FORMAT_R11G11B10_FLOAT, 4, __P_D3D11_BIND_CLRBL),  ///< R11G11B10 float
          
          // 9 bit
          rgb9e5_uf = __P_D3D11_FORMAT(DXGI_FORMAT_R9G9B9E5_SHAREDEXP, 4, __P_D3D11_BIND_NONE),///< R9G9B9E5 float (shared exponent)
          
          // 8 bit
          rgba8_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8B8A8_UINT, 4, __P_D3D11_BIND_COLOR),///< R8G8B8A8 unsigned int
          rg8_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8_UINT, 2, __P_D3D11_BIND_COLOR),      ///< R8G8 unsigned int
          r8_ui = __P_D3D11_FORMAT(DXGI_FORMAT_R8_UINT, 1, __P_D3D11_BIND_COLOR),         ///< R8 unsigned int
          
          rgba8_i = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8B8A8_SINT, 4, __P_D3D11_BIND_COLOR), ///< R8G8B8A8 int
          rg8_i = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8_SINT, 2, __P_D3D11_BIND_COLOR),       ///< R8G8 int
          r8_i = __P_D3D11_FORMAT(DXGI_FORMAT_R8_SINT, 1, __P_D3D11_BIND_COLOR),          ///< R8 int
          
          rgba8_sRGB = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 4, __P_D3D11_BIND_CLRBL), ///< R8G8B8A8 normalized unsigned int (sRGB)
          bgra8_sRGB = __P_D3D11_FORMAT(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, 4, __P_D3D11_BIND_CLRBL), ///< B8G8R8A8 normalized unsigned int (sRGB)
          rgba8_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8B8A8_UNORM, 4, __P_D3D11_BIND_CLRBL),///< R8G8B8A8 normalized unsigned int
          bgra8_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_B8G8R8A8_UNORM, 4, __P_D3D11_BIND_CLRBL),///< B8G8R8A8 normalized unsigned int
          rg8_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8_UNORM, 2, __P_D3D11_BIND_CLRBL),      ///< R8G8 normalized unsigned int
          r8_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_R8_UNORM, 1, __P_D3D11_BIND_CLRBL),         ///< R8 normalized unsigned int
          a8_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_A8_UNORM, 1, __P_D3D11_BIND_NONE),          ///< A8 normalized unsigned int
          
          rgba8_snorm = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8B8A8_SNORM, 4, __P_D3D11_BIND_CLRBL),///< R8G8B8A8 normalized int
          rg8_snorm = __P_D3D11_FORMAT(DXGI_FORMAT_R8G8_SNORM, 2, __P_D3D11_BIND_CLRBL),      ///< R8G8 normalized int
          r8_snorm = __P_D3D11_FORMAT(DXGI_FORMAT_R8_SNORM, 1, __P_D3D11_BIND_CLRBL),         ///< R8 normalized int
          
          // 4-6 bit
          rgb5a1_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_B5G5R5A1_UNORM, 2, __P_D3D11_BIND_NONE),///< B5G5R5A1 normalized unsigned int
          r5g6b5_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_B5G6R5_UNORM, 2, __P_D3D11_BIND_NONE),  ///< B5G6R5 normalized unsigned int
          rgba4_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_B4G4R4A4_UNORM, 2, __P_D3D11_BIND_NONE), ///< B4G4R4A4 normalized unsigned int -- requires Win8.1+
          
          // depth/stencil
          d32_f = __P_D3D11_FORMAT(DXGI_FORMAT_D32_FLOAT, 4, __P_D3D11_BIND_DEPTH),                  ///< D32 float
          d16_unorm = __P_D3D11_FORMAT(DXGI_FORMAT_D16_UNORM, 2, __P_D3D11_BIND_DEPTH),              ///< D16 normalized unsigned int
          d32_f_s8_ui = __P_D3D11_FORMAT(DXGI_FORMAT_D32_FLOAT_S8X24_UINT, 8, __P_D3D11_BIND_DEPTH), ///< D32 float / S8 unsigned int
          d24_unorm_s8_ui = __P_D3D11_FORMAT(DXGI_FORMAT_D24_UNORM_S8_UINT, 4, __P_D3D11_BIND_DEPTH),///< D24 normalized unsigned int / S8 unsigned int
        };

        /// @brief Create additional DataFormat from native DXGI_FORMAT (example: createDataFormat(DXGI_FORMAT_BC6H_SF16,6,FormatAttachment::color))
        constexpr inline DataFormat createDataFormat(DXGI_FORMAT format, uint32_t bytesPerPixel,
                                                     FormatAttachment attachment = FormatAttachment::color) noexcept {
          return (DataFormat)__P_D3D11_FORMAT(format, bytesPerPixel, attachment); 
        }
        constexpr inline DXGI_FORMAT _getDataFormatComponents(DataFormat format) noexcept { return static_cast<DXGI_FORMAT>((uint32_t)format & 0x00FFFFu); }
        constexpr inline FormatAttachment _getDataFormatBindFlag(DataFormat format) noexcept { return static_cast<FormatAttachment>((uint32_t)format & 0xC0000000u); }
        constexpr inline uint32_t _getDataFormatBytesPerPixel(DataFormat format) noexcept { return (((uint32_t)format >> 24) & 0x3Fu); }
        
        template <typename _TextureDesc>
        inline uint32_t _setTextureFormat(DataFormat format, _TextureDesc& outDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& outView) {
          outView.Format = outDesc.Format = _getDataFormatComponents(format);
          return _getDataFormatBytesPerPixel(format);
        }
#       undef __P_D3D11_FORMAT
      
        /// @brief Primitive topology - vertex interpretation mode (tessellation patches excluded)
        /// @remarks <...>Adj = topology with adjacency: generation of additional adjacent vertices (based on buffer vertices):
        ///                     - those vertices are only visible in the vertex shader and geometry shader;
        ///                     - if some culling is configured before geometry shader stage, adjacent vertices may disappear;
        ///                     - can't be used with tessellation shaders.
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
        constexpr inline bool hasStencilComponent(DepthStencilFormat format) noexcept { 
          return (format == DepthStencilFormat::d32_f_s8_ui || format == DepthStencilFormat::d24_unorm_s8_ui); 
        }
        
        /// @brief Data Buffer content type
        enum class BufferType : int/*D3D11_BIND_FLAG*/ {
          uniform = D3D11_BIND_CONSTANT_BUFFER,  ///< Constant/uniform data buffer for shader stage(s): 
                                                 ///  * can be bound with any shader stage(s): Renderer.bind<...>ConstantBuffers.
                                                 ///  * should contain data useful as a whole for shaders:
                                                 ///    -> buffer entirely copied in GPU cache during Draw calls, to be available for each vertex/pixel.
                                                 ///    -> recommended for view/projection/world matrices, transformations, lights and options.
                                                 ///    -> not appropriate for big data blocks of which only a few bytes are read by each vertex/pixel.
          vertex = D3D11_BIND_VERTEX_BUFFER,     ///< Vertex data buffer for vertex shader
                                                 ///  * can be bound with vertex shader stage: Renderer.bindVertexArrayBuffer(s).
                                                 ///  * contains vertices to process in renderer.
                                                 ///  * can be used with an optional index buffer, to improve speed and bandwidth.
                                                 ///  * can also be used as instance buffers, to duplicate meshes many times (example: trees, leaves...).
                                                 ///  * Common practice: - geometry centered around (0;0;0) -> vertex buffers;
                                                 ///                     - world matrix to offset the entire model in the environment -> combined with camera view into constant buffer;
                                                 ///                     - vertices repositioned in vertex shader by world/view matrix and projection matrix.
          vertexIndex = D3D11_BIND_INDEX_BUFFER  ///< Indices of vertex buffer(s)
                                                 ///  * can be bound with vertex shader stage: Renderer.bindVertexArrayBuffer(s).
                                                 ///  * optionally used with vertex array buffer(s), to improve speed and bandwidth.
                                                 ///  * contains indices to allow removal of redundant/common vertices from associated vertex array buffer(s).
        };
        
#       define __P_D3D11_RES_USAGE(usage, cpuAccess)  ((uint32_t)usage | (((uint32_t)cpuAccess) << 8))

        /// @brief Resource / texture memory usage - ResourceBuffer / Texture<...>
        enum class ResourceUsage : uint32_t/*D3D11_USAGE + D3D11_CPU_ACCESS_FLAG*/ {
          immutable  = __P_D3D11_RES_USAGE(D3D11_USAGE_IMMUTABLE, 0), ///< Immutable: GPU memory initialized at creation, not visible from CPU:
                                                                      ///             - fastest GPU access: ideal for static resources that don't change (textures, geometry...);
          staticGpu  = __P_D3D11_RES_USAGE(D3D11_USAGE_DEFAULT, 0),   ///< Static: GPU memory, not directly visible from CPU:
                                                                      ///          - fast GPU access: ideal for static resources rarely updated or for small resources;
                                                                      ///          - indirect CPU write access: should be used when NOT updated frequently (or if data is small).
          dynamicCpu = __P_D3D11_RES_USAGE(D3D11_USAGE_DYNAMIC,
                                           D3D11_CPU_ACCESS_WRITE),   ///< Dynamic: CPU mappable memory, visible from GPU (through PCIe):
                                                                      ///           - slow GPU read access: should be used when data isn't kept for multiple frames;
                                                                      ///           - very fast CPU write access: ideal for constants/uniforms and vertices/indices re-written by CPU every frame.
          staging    = __P_D3D11_RES_USAGE(D3D11_USAGE_STAGING,
                       (D3D11_CPU_ACCESS_WRITE|D3D11_CPU_ACCESS_READ))///< Staging: CPU-only mappable memory:
                                                                      ///           - no GPU access: should not be used for display or render-targets;
                                                                      ///           - very fast CPU read/write access: usable to manipulate resource data, or to read buffer content from CPU;
                                                                      ///           - staged content is usually used to populate/read/update static resources (textures, geometry...)
                                                                      ///             -> copied with data transfers from/to staticGpu resources (which can be accessed by the GPU).
        };

        constexpr inline D3D11_USAGE _getResourceUsageType(ResourceUsage usage) noexcept { return static_cast<D3D11_USAGE>((uint32_t)usage & 0x00FFu); }
        constexpr inline UINT _getResourceUsageCpuAccess(ResourceUsage usage) noexcept { return static_cast<UINT>((uint32_t)usage >> 8); }
        
        template <typename _TextureDesc>
        inline void _setTextureUsage(ResourceUsage type, _TextureDesc& outDesc) {
          outDesc.Usage = _getResourceUsageType(type);
          outDesc.CPUAccessFlags = _getResourceUsageCpuAccess(type);
          outDesc.BindFlags = (type != ResourceUsage::staging) ? D3D11_BIND_SHADER_RESOURCE : 0;
        }
#       undef __P_D3D11_RES_USAGE

        /// @brief Dynamic resource / texture memory mapping
        enum class DynamicMapping : int/*D3D11_MAP*/ {
          discard = D3D11_MAP_WRITE_DISCARD, ///< Discard previous data (as soon as GPU stops using it), then write dynamic buffer data.
                                             ///  Recommended for each first write of the buffer for a frame.
          subsequent = D3D11_MAP_WRITE_NO_OVERWRITE ///< Write dynamic buffer data, and keep previous data in zones not re-written.
                                                    ///  Recommended for subsequent writes of a buffer within the same frame.
                                                    ///  No overwrite of previous data currently used by GPU should occur!
        };
        /// @brief Staging resource / texture memory mapping
        enum class StagedMapping : int/*D3D11_MAP*/ {
          read = D3D11_MAP_READ,            ///< Read-only access
          write = D3D11_MAP_WRITE,          ///< Write-only access
          readWrite = D3D11_MAP_READ_WRITE  ///< Read-write access
        };
        
        
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
          add         = D3D11_BLEND_OP_ADD,          ///< Source RGBA + dest. RGBA
          subtract    = D3D11_BLEND_OP_SUBTRACT,     ///< Source RGBA - dest. RGBA
          revSubtract = D3D11_BLEND_OP_REV_SUBTRACT, ///< Dest. RGBA - source RGBA
          minimum     = D3D11_BLEND_OP_MIN,          ///< min(source RGBA, dest RGBA)
          maximum     = D3D11_BLEND_OP_MAX           ///< max(source RGBA, dest RGBA)
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
        
        
        // -- shaders --
        
        /// @brief Shader pipeline stage type
        enum class ShaderType : unsigned int {
          vertex   = 0, ///< Vertex shader: process input vertex data -> outputs vertex projection.
          fragment = 1, ///< Fragment/pixel shader: process rasterized fragment -> outputs pixel/depth data.
#         ifndef __P_DISABLE_GEOMETRY_STAGE
            geometry = 2, ///< Geometry shader: modify/duplicate primitive.
#           ifndef __P_DISABLE_TESSELLATION_STAGE
              tessCtrl = 3, ///< Tessellation control/hull shader: tessellate primitive -> outputs geometry patch.
              tessEval = 4, ///< Tessellation evaluation/domain shader: calculate new vertex positions.
#           endif
#         else
#           ifndef __P_DISABLE_TESSELLATION_STAGE
              tessCtrl = 2,
              tessEval = 3,
#           endif
#         endif
          compute  = 0xFF  ///< Compute shader: GPU calculations (not supported below OpenGL 4.3).
        };

#       if !defined(__P_DISABLE_TESSELLATION_STAGE)
#         define __P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX ((unsigned int)ShaderType::tessEval)
#       elif !defined(__P_DISABLE_GEOMETRY_STAGE)
#         define __P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX ((unsigned int)ShaderType::geometry)
#       else
#         define __P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX ((unsigned int)ShaderType::fragment)
#       endif
      }
    }
  }
  _P_FLAGS_OPERATORS(pandora::video::d3d11::ColorComponentFlag, int);
  _P_FLAGS_OPERATORS(pandora::video::d3d11::BufferType, int);

# ifndef _P_NO_WIN32_FILTER
#   undef NODRAWTEXT
#   undef NOGDI
#   undef NOBITMAP
#   undef NOMCX
#   undef NOSERVICE
# endif
#endif
