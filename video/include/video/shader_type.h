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
  }
}
