/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if !defined(_WINDOWS)
# include <cstdint>
# include <memory/light_string.h>
# include "video/message_box.h"

  namespace pandora {
    namespace video {
      namespace __MessageBox {
        // -- error messages --

        // set last error message (in case of Result::failure)
        void setLastError(const char* error) noexcept;
        // get copy of last error message (in case of Result::failure)
        pandora::memory::LightString getLastError() noexcept;
        
        
        // -- action button labels --
        
        // convert portable set of actions to list of button labels
        // params: outLabels: must be a pre-allocated 'const char*[3]'
        // returns: number of actions
        // note: on macOS/iOS: will reverse order of buttons (usual order on Apple systems)
        uint32_t toActionLabels(pandora::video::MessageBox::ActionType actions, const char** outLabels) noexcept;
        
        // convert custom actions to list of button labels
        // params: outLabels: must be a pre-allocated 'const char*[3]'
        // returns: number of actions
        // note: on macOS/iOS: will reverse order of buttons (usual order on Apple systems)
        uint32_t toActionLabels(const char* action1, const char* action2, const char* action3, const char** outLabels) noexcept;
        
        // ---
        
        // convert action index to portable dialog result
        // params: - index: user action index (1 - 3, or 0 for errors)
        //         - maxActions: number of possible actions (1 - 3)
        // note: restore order on macOS/iOS (usual order reversed on Apple systems)
        pandora::video::MessageBox::Result toDialogResult(uint32_t index, uint32_t maxActions) noexcept;
      }
    }
  }
#endif
