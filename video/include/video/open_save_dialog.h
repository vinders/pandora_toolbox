/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <string>
#include "video/window_handle.h"

namespace pandora {
  namespace video {
    /// @class OpenSaveDialog
    /// @brief File open / save dialog box
    class OpenSaveDialog final {
    public:
      //OpenSaveDialog();
      
      //avoir objet -> initialisé avec params + params optionnels sous forme d'accesseur
      // show pour afficher -> return Result + stocke path(s) dans var membre
      
      //classe différente pour open et save ???

      /// @brief User action chosen to close dialog (or failure if an error occurred)
      enum class Result : uint32_t {
        failure   = 0u, ///< error during dialog box creation
        chosen    = 1u, ///< user has chosen a file path
        cancelled = 2u  ///< user has cancelled file selection
      };
      
      // ---
      
      //TODO

      /// @brief Get last error message (in case of Result::failure)
      /// @returns Last error (if available) or empty string
      //static std::string getLastError();
    };
  }
}
