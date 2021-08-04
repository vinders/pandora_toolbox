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

#if !defined(_WINDOWS) && defined(__ANDROID__)
# include <cstddef>
# include <stdexcept>
# include <android_native_app_glue.h>

  namespace pandora {
    namespace system {
      /// @class AndroidApp
      /// @brief Singleton to access Android application state anywhere
      /// @warning Must be explicitly initialized before using any display/monitor/window class.
      class AndroidApp final {
      public:
        AndroidApp() = default;
        
        /// @brief Initialize application state
        /// @warning - Should be called (with a non-null value) in the application entry point
        ///          - Any call to 'state()' without a previous initialization will result in an exception
        ///          - The required value is available in "void android_main(struct android_app* state) {...}"
        inline void init(struct android_app* state) noexcept { this->_state = state; }
        
        /// @brief Verify if state has been initialized
        inline bool isInitialized() const noexcept { return (this->_state != nullptr); }
        
        /// @brief Get application state (must have been initialized !)
        /// @throws logic_error if not initialized (init(...) must have been called before)
        struct android_app& state() {
          if (this->_state == nullptr)
            throw std::logic_error("AndroidApp.state(): android_app state not initialized");
          return *(this->_state);
        }
        
        /// @brief Get unique instance of Android application state
        static AndroidApp& instance() noexcept {
          static AndroidApp _instance;
          return _instance;
        }
        
      private:
        struct android_app* _state = nullptr;
      };
    }
  }
#endif
