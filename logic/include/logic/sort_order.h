/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
------------------------------------------------------------------------
Description : sort & search order types
*******************************************************************************/
#pragma once

#include <cstdint>

namespace pandora {
  namespace logic {
    
    /// @brief Order in which elements are sorted in a collection
    enum class SortOrder : uint32_t {
      asc = 0u, ///< ascending: from smallest to greatest
      desc = 1u ///< descending: from greatest to smallest
    };

  }
}
