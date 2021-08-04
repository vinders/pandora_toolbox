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
