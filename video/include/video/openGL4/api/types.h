/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
OpenGL4 - bindings with native types (same labels/values as other renderers: only namespace differs)
*******************************************************************************/
#pragma once

#if defined(_VIDEO_OPENGL4_SUPPORT)
# include <cstdint>
# include "./opengl.h"
# include <system/_private/_enum_flags.h>

  namespace pandora {
    namespace video {
      namespace openGL4 {
        using DeviceHandle = void*;       ///< Physical device access/resources (HGLRC / NSOpenGLContext* / EGLContext / GLXContext / OSMesaContext)
        using DeviceContext = GLuint;     ///< Device rendering context
        using SwapChainHandle = GLuint;   ///< Swap-chain (framebuffer container/swapper)
        
        using TextureHandle1D = GLuint;   ///< 1D texture resource container
        using TextureHandle2D = GLuint;   ///< 2D texture resource container
        using TextureHandle3D = GLuint;   ///< 3D texture resource container
        using BufferHandle = GLuint;      ///< Vertex/index/constant/resource buffer
        using RenderTargetView = GLuint;  ///< Bindable render-target view for renderer (shader output buffer)
        using DepthStencilView = GLuint;  ///< Bindable depth/stencil view for renderer
        using TextureView = GLuint;       ///< Bindable texture view for shaders
        
        using InputLayoutHandle = GLuint; ///< Input layout representation, for shader input stage
        using ColorChannel = float;       ///< R/G/B/A color value
        
        
        // -- rasterizer settings --
    
        /// @brief Rasterizer filling mode
        enum class FillMode : GLenum {
          fill    = GL_FILL,       ///< Filled/solid polygons
          lines   = GL_LINE,       ///< Lines/wireframe
          linesAA = GL_LINE_SMOOTH ///< Anti-aliased lines/wireframe
        };
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
        
        /// @brief RGBA color component - bit-mask flags
        enum class ColorComponentFlag : int {
          none  = 0,
          red   = 0x1,
          green = 0x2,
          blue  = 0x4,
          alpha = 0x8,
          all   = 0xF
        };
        
#       define __P_OPENGL4_FORMAT(componentFormat, baseOrder, type)  ((uint64_t)componentFormat | (((uint64_t)baseOrder) << 16) | (((uint64_t)type) << 32))
        
        /// @brief Color/normal/depth/stencil data formats (vertex array buffers, depth/stencil buffers, textures...)
        /// @remarks - HDR rendering / color space: RG(BA) components between 10 and 32 bits ('rgba16_f_scRGB' or 'rgb10a2_unorm_hdr10' recommended).
        ///          - SDR rendering / color space: other component types ('rgba8_sRGB' or 'rgba8_unorm' recommended).
        ///          - HDR rendering on SDR devices: shaders will need to convert color values (or they'll be clipped when displayed).
        ///          - Additional data formats can be created using: createDataFormat(formatWithSizeAndType,baseOrderFormat,valueGlType).
        enum class DataFormat : uint64_t/*format + order + type*/ {
          unknown = (uint64_t)GL_NONE, ///< Unknown/empty format
          
          // HDR / 32 bit
          rgba32_f = __P_OPENGL4_FORMAT(GL_RGBA32F, GL_RGBA, GL_FLOAT), ///< R32G32B32A32 float
          rgb32_f = __P_OPENGL4_FORMAT(GL_RGB32F, GL_RGB, GL_FLOAT),    ///< R32G32B32 float
          rg32_f = __P_OPENGL4_FORMAT(GL_RG32F, GL_RG, GL_FLOAT),       ///< R32G32 float
          r32_f = __P_OPENGL4_FORMAT(GL_R32F, GL_RED, GL_FLOAT),        ///< R32 float
          
          rgba32_ui = __P_OPENGL4_FORMAT(GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT), ///< R32G32B32A32 unsigned int
          rgb32_ui = __P_OPENGL4_FORMAT(GL_RGB32UI, GL_RGB, GL_UNSIGNED_INT),    ///< R32G32B32 unsigned int
          rg32_ui = __P_OPENGL4_FORMAT(GL_RG32UI, GL_RG, GL_UNSIGNED_INT),       ///< R32G32 unsigned int
          r32_ui = __P_OPENGL4_FORMAT(GL_R32UI, GL_RED, GL_UNSIGNED_INT),        ///< R32 unsigned int

          rgba32_i = __P_OPENGL4_FORMAT(GL_RGBA32I, GL_RGBA, GL_INT), ///< R32G32B32A32 int
          rgb32_i = __P_OPENGL4_FORMAT(GL_RGB32I, GL_RGB, GL_INT),    ///< R32G32B32 int
          rg32_i = __P_OPENGL4_FORMAT(GL_RG32I, GL_RG, GL_INT),       ///< R32G32 int
          r32_i = __P_OPENGL4_FORMAT(GL_R32I, GL_RED, GL_INT),        ///< R32 int
          
          // HDR / 16 bit
          rgba16_f_scRGB = __P_OPENGL4_FORMAT(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT), ///< HDR-FP16/scRGB: R16G16B16A16 float
          rg16_f = __P_OPENGL4_FORMAT(GL_RG16F, GL_RG, GL_HALF_FLOAT),             ///< R16G16 float
          r16_f = __P_OPENGL4_FORMAT(GL_R16F, GL_RED, GL_HALF_FLOAT),              ///< R16 float
          
          rgba16_ui = __P_OPENGL4_FORMAT(GL_RGBA16UI, GL_RGBA, GL_UNSIGNED_SHORT), ///< R16G16B16A16 unsigned int
          rg16_ui = __P_OPENGL4_FORMAT(GL_RG16UI, GL_RG, GL_UNSIGNED_SHORT),       ///< R16G16 unsigned int
          r16_ui = __P_OPENGL4_FORMAT(GL_R16UI, GL_RED, GL_UNSIGNED_SHORT),        ///< R16 unsigned int
          
          rgba16_i = __P_OPENGL4_FORMAT(GL_RGBA16I, GL_RGBA, GL_SHORT), ///< R16G16B16A16 int
          rg16_i = __P_OPENGL4_FORMAT(GL_RG16I, GL_RG, GL_SHORT),       ///< R16G16 int
          r16_i = __P_OPENGL4_FORMAT(GL_R16I, GL_RED, GL_SHORT),        ///< R16 int
          
          rgba16_unorm = __P_OPENGL4_FORMAT(GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT), ///< R16G16B16A16 normalized unsigned int
          rg16_unorm = __P_OPENGL4_FORMAT(GL_RG16, GL_RG, GL_UNSIGNED_SHORT),       ///< R16G16 normalized unsigned int
          r16_unorm = __P_OPENGL4_FORMAT(GL_R16, GL_RED, GL_UNSIGNED_SHORT),        ///< R16 normalized unsigned int
          
          rgba16_snorm = __P_OPENGL4_FORMAT(GL_RGBA16_SNORM, GL_RGBA, GL_SHORT), ///< R16G16B16A16 normalized int
          rg16_snorm = __P_OPENGL4_FORMAT(GL_RG16_SNORM, GL_RG, GL_SHORT),       ///< R16G16 normalized int
          r16_snorm = __P_OPENGL4_FORMAT(GL_R16_SNORM, GL_RED, GL_SHORT),        ///< R16 normalized int
          
          // HDR / 10-11 bit
          rgb10a2_unorm_hdr10 = __P_OPENGL4_FORMAT(GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV), ///< HDR-10/BT.2100: R10G10B10A2 normalized unsigned int
          rgb10a2_ui = __P_OPENGL4_FORMAT(GL_RGB10_A2UI, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV),        ///< R10G10B10A2 unsigned int
          rg11b10_f = __P_OPENGL4_FORMAT(GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV),     ///< R11G11B10 float
          
          // 9 bit
          rgb9e5_uf = __P_OPENGL4_FORMAT(GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV), ///< R9G9B9E5 float (shared exponent)
          
          // 8 bit
          rgba8_ui = __P_OPENGL4_FORMAT(GL_RGBA8UI, GL_RGBA, GL_UNSIGNED_BYTE), ///< R8G8B8A8 unsigned int
          rg8_ui = __P_OPENGL4_FORMAT(GL_RG8UI, GL_RG, GL_UNSIGNED_BYTE),       ///< R8G8 unsigned int
          r8_ui = __P_OPENGL4_FORMAT(GL_R8UI, GL_RED, GL_UNSIGNED_BYTE),        ///< R8 unsigned int
          
          rgba8_i = __P_OPENGL4_FORMAT(GL_RGBA8I, GL_RGBA, GL_BYTE), ///< R8G8B8A8 int
          rg8_i = __P_OPENGL4_FORMAT(GL_RG8I, GL_RG, GL_BYTE),       ///< R8G8 int
          r8_i = __P_OPENGL4_FORMAT(GL_R8I, GL_RED, GL_BYTE),        ///< R8 int
          
          rgba8_sRGB = __P_OPENGL4_FORMAT(GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE),    ///< R8G8B8A8 normalized unsigned int (SRGB)
          bgra8_sRGB = __P_OPENGL4_FORMAT(GL_SRGB8_ALPHA8, GL_BGRA, GL_UNSIGNED_BYTE),    ///< B8G8R8A8 normalized unsigned int (SRGB)
          rgba8_unorm = __P_OPENGL4_FORMAT(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE), ///< R8G8B8A8 normalized unsigned int
          bgra8_unorm = __P_OPENGL4_FORMAT(GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE), ///< B8G8R8A8 normalized unsigned int
          rg8_unorm = __P_OPENGL4_FORMAT(GL_RG8, GL_RG, GL_UNSIGNED_BYTE),       ///< R8G8 normalized unsigned int
          r8_unorm = __P_OPENGL4_FORMAT(GL_R8, GL_RED, GL_UNSIGNED_BYTE),        ///< R8 normalized unsigned int
          a8_unorm = __P_OPENGL4_FORMAT(GL_R8, GL_RED, GL_UNSIGNED_BYTE),           ///< A8 normalized unsigned int
          
          rgba8_snorm = __P_OPENGL4_FORMAT(GL_RGBA8_SNORM, GL_RGBA, GL_BYTE), ///< R8G8B8A8 normalized int
          rg8_snorm = __P_OPENGL4_FORMAT(GL_RG8_SNORM, GL_RG, GL_BYTE),     ///< R8G8 normalized int
          r8_snorm = __P_OPENGL4_FORMAT(GL_R8_SNORM, GL_RED, GL_BYTE),       ///< R8 normalized int
          
          // 4-6 bit
          rgb5a1_unorm = __P_OPENGL4_FORMAT(GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1), ///< B5G5R5A1 normalized unsigned int
          r5g6b5_unorm = __P_OPENGL4_FORMAT(GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5),  ///< B5G6R5 normalized unsigned int
          rgba4_unorm = __P_OPENGL4_FORMAT(GL_RGBA4, GL_RGBA,  GL_UNSIGNED_SHORT_4_4_4_4),    ///< B4G4R4A4 normalized unsigned int
          
          // depth/stencil
          d32_f = __P_OPENGL4_FORMAT(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT),                             ///< D32 float
          d16_unorm = __P_OPENGL4_FORMAT(GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT),                 ///< D16 normalized unsigned int
          d32_f_s8_ui = __P_OPENGL4_FORMAT(GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV), ///< D32 float / S8 unsigned int
          d24_unorm_s8_ui = __P_OPENGL4_FORMAT(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8),           ///< D24 normalized unsigned int / S8 unsigned int
        };
        
        /// @brief Create additional DataFormat from native GLenum (example: createDataFormat(GL_SRGB8_ALPHA8,GL_RGBA,GL_UNSIGNED_BYTE))
        constexpr inline DataFormat createDataFormat(GLenum componentFormat, GLenum baseOrder, GLenum type) noexcept { 
          return (DataFormat)__P_OPENGL4_FORMAT(componentFormat, baseOrder, type); 
        }
        constexpr inline GLenum _getDataFormatComponents(DataFormat format) noexcept { return static_cast<GLenum>((unsigned int)format & 0x0000FFFFu); }
        constexpr inline GLenum _getDataFormatBaseOrder(DataFormat format) noexcept { return static_cast<GLenum>(((uint64_t)format >> 16) & 0x0000FFFFuLL); }
        constexpr inline GLenum _getDataFormatType(DataFormat format) noexcept { return static_cast<GLenum>(((uint64_t)format >> 32) & 0x00FFFFFFuLL); }
#       undef __P_OPENGL4_FORMAT
        
        /// @brief Primitive topology - vertex interpretation mode (tessellation patches excluded)
        /// @remarks <...>Adj = topology with adjacency: generation of additional adjacent vertices (based on buffer vertices):
        ///                     - those vertices are only visible in the vertex shader and geometry shader;
        ///                     - if some culling is configured before geometry shader stage, adjacent vertices may disappear;
        ///                     - can't be used with tessellation shaders.
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
        constexpr inline bool hasStencilComponent(DepthStencilFormat format) noexcept { 
          return (format == DepthStencilFormat::d32_f_s8_ui || format == DepthStencilFormat::d24_unorm_s8_ui); 
        }
        
        /// @brief Basic StaticBuffer / DynamicBuffer content type
        enum class BaseBufferType : unsigned int {
          uniform     = 0u, ///< Constant/uniform data buffer for shader stage(s): 
                            ///  * can be bound with any shader stage(s): Renderer.bind<...>ConstantBuffers.
                            ///  * should contain data useful as a whole for shaders:
                            ///    -> buffer entirely copied in GPU cache during Draw calls, to be available for each vertex/pixel.
                            ///    -> recommended for view/projection/world matrices, transformations, lights and options.
                            ///    -> not appropriate for big data blocks of which only a few bytes are read by each vertex/pixel.
          vertex      = 1u, ///< Vertex data buffer for vertex shader
                            ///  * can be bound with vertex shader stage: Renderer.bindVertexArrayBuffer(s).
                            ///  * contains vertices to process in renderer.
                            ///  * can be used with an optional index buffer, to improve speed and bandwidth.
                            ///  * can also be used as instance buffers, to duplicate meshes many times (example: trees, leaves...).
                            ///  * Common practice: - geometry centered around (0;0;0) -> vertex buffers;
                            ///                     - world matrix to offset the entire model in the environment -> combined with camera view into constant buffer;
                            ///                     - vertices repositioned in vertex shader by world/view matrix and projection matrix.
          vertexIndex = 2u  ///< Indices of vertex buffer(s)
                            ///  * can be bound with vertex shader stage: Renderer.bindVertexArrayBuffer(s).
                            ///  * optionally used with vertex array buffer(s), to improve speed and bandwidth.
                            ///  * contains indices to allow removal of redundant/common vertices from associated vertex array buffer(s).
        };
        /// @brief Resource / texture memory usage - ResourceBuffer / Texture<...>
        /// @remarks Different enums used by glBufferData / glBufferStorage (4.4+), depending on API version.
        enum class ResourceUsage : unsigned int {
          immutable = 0,  ///< Immutable: GPU memory initialized at creation, not visible from CPU:
                          ///             - fastest GPU access: ideal for static resources that don't change (textures, geometry...);
          staticGpu = 1,  ///< Static: GPU memory, not directly visible from CPU:
                          ///          - fast GPU access: ideal for static resources rarely updated or for small resources;
                          ///          - indirect CPU write access: should be used when NOT updated frequently (or if data is small).
          dynamicCpu = 2, ///< Dynamic: CPU mappable memory, visible from GPU (through PCIe):
                          ///           - slow GPU read access: should be used when data isn't kept for multiple frames;
                          ///           - very fast CPU write access: ideal for constants/uniforms and vertices/indices re-written by CPU every frame.
          staging   = 3   ///< Staging: CPU-only mappable memory:
                          ///           - no GPU access: should not be used for display or render-targets;
                          ///           - very fast CPU read/write access: usable to manipulate resource data, or to read buffer content from CPU;
                          ///           - staged content is usually used to populate/read/update static resources (textures, geometry...)
                          ///             -> copied with data transfers from/to staticGpu resources (which can be accessed by the GPU).
        };
        
        
        // -- color/alpha blending --

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
          add         = GL_FUNC_ADD,              ///< Source RGBA + dest. RGBA
          subtract    = GL_FUNC_SUBTRACT,         ///< Source RGBA - dest. RGBA
          revSubtract = GL_FUNC_REVERSE_SUBTRACT, ///< Dest. RGBA - source RGBA
          minimum     = GL_MIN,                   ///< min(source RGBA, dest RGBA)
          maximum     = GL_MAX                    ///< max(source RGBA, dest RGBA)
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
        
        
        // -- shaders --

        /// @brief Shader pipeline stage type
        enum class ShaderType : uint32_t {
          vertex   = GL_VERTEX_SHADER, ///< Vertex shader: process input vertex data -> outputs vertex projection.
          tessCtrl = GL_TESS_CONTROL_SHADER, ///< Tessellation control/hull shader: tessellate primitive -> outputs geometry patch.
          tessEval = GL_TESS_EVALUATION_SHADER, ///< Tessellation evaluation/domain shader: calculate new vertex positions.
          geometry = GL_GEOMETRY_SHADER, ///< Geometry shader: modify/duplicate primitive.
          fragment = GL_FRAGMENT_SHADER, ///< Fragment/pixel shader: process rasterized fragment -> outputs pixel/depth data.
          compute  = GL_COMPUTE_SHADER  ///< Compute shader: GPU calculations (not supported below OpenGL 4.3).
        };
      }
    }
  }
  _P_FLAGS_OPERATORS(pandora::video::openGL4::ColorComponentFlag, int);
#endif
