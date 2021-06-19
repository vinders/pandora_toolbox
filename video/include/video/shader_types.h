/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>

namespace pandora {
  namespace video {
    /// @brief Rendering shader category
    enum class ShaderType : uint32_t {
      vertex        = 0u, ///< Vertex shader: process input vertex data -> vertex output
      tesselControl = 1u, ///< Tessellation control/hull shader: tessellate primitive -> geometry patch
      tesselEval    = 2u, ///< Tessellation evaluation/domain shader: calculate new vertex positions
      geometry      = 3u, ///< Geometry shader: modify/duplicate primitive
      fragment      = 4u, ///< Fragment/pixel shader: process rasterized fragment -> pixel/depth output
      compute       = 5u  ///< Compute shader: GPU calculations
    };
    
    // -- buffer types --
    
    /// @brief Rendering data buffer type
    enum class DataBufferType : uint32_t {
      constant    = 0u, ///< Constant buffer data for shader stage(s): 
                        ///  * can be bound with any shader stage(s): Renderer.bind<...>ConstantBuffers.
                        ///  * should contain data useful as a whole for shaders:
                        ///    -> buffer entirely copied in GPU cache during Draw calls, to be available for each vertex/pixel.
                        ///    -> recommended for view/projection/world matrices, transformations, lights and options.
                        ///    -> not appropriate for big data blocks of which only a few bytes are read by each vertex/pixel.
      vertexArray = 1u, ///< Vertex data input for vertex shader
                        ///  * can be bound with vertex shader stage: Renderer.bindVertexArrayBuffer(s).
                        ///  * contains vertices to process in renderer.
                        ///  * can be used with an optional index buffer, to improve speed and bandwidth.
                        ///  * can also be used as instance buffers, to duplicate meshes many times (example: trees, leaves...).
                        ///  * Common practice: - geometry centered around (0;0;0) -> vertex buffers;
                        ///                     - world matrix to offset the entire model in the environment -> constant buffer;
                        ///                     - combine world matrix with view/projection matrix to reposition vertices in vertex shader.
      vertexIndex = 2u  ///< Indexes of vertex array buffer(s)
                        ///  * can be bound with vertex shader stage: Renderer.bindVertexArrayBuffer(s).
                        ///  * optionally used with vertex array buffer(s), to improve speed and bandwidth.
                        ///  * contains indexes to allow removal of redundant/common vertices from associated vertex array buffer(s).
    };
    /// @brief Basic topology - vertex buffer data interpretation mode 
    enum class VertexTopology : uint32_t {
      points        = 0u, ///< List of points
      lines         = 1u, ///< List of separate line segments
      lineStrips    = 2u, ///< Poly-line - line between each vertex and the previous one
      triangles     = 3u, ///< List of separate triangles
      triangleStrip = 4u  ///< Triangle-strip - triangle for each vertex and the previous two
    };
  }
}
