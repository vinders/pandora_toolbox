/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#if defined(_WINDOWS)
# include <cstddef>
# include <stdexcept>

  namespace pandora {
    namespace system {
      using AppInstanceHandle = void*; ///< HINSTANCE handle from WinMain
      
      /// @class WindowsApp
      /// @brief Singleton to access Windows window-mode application handle anywhere
      /// @warning Must be explicitly initialized before using any window/window-resource class.
      class WindowsApp final {
      public:
        WindowsApp() = default;
        
        /// @brief Initialize window-mode application handle
        /// @warning - Should be called (with a non-null value) in the application entry point
        ///          - Any call to 'handle()' without a previous initialization will result in an exception
        ///          - DLL: the required value is available in "BOOL DllMain(HINSTANCE dllInstanceHandle, DWORD, LPVOID) { ... }"
        ///          - exe: * the required value is available in "int WinMain(HINSTANCE appInstanceHandle, HINSTANCE, LPSTR, int) { ... }"
        ///                 * the required value is also available with "GetModuleHandle(NULL)"
        inline void init(AppInstanceHandle handle) noexcept { this->_handle = handle; }
        
        /// @brief Verify if handle has been initialized
        inline bool isInitialized() const noexcept { return (this->_handle != nullptr); }
        
        /// @brief Get window-mode application handle (must have been initialized !)
        /// @throws logic_error if not initialized (init(...) must have been called before)
        AppInstanceHandle handle() {
          if (this->_handle == nullptr)
            throw std::logic_error("WindowsApp.handle(): HINSTANCE handle not initialized");
          return this->_handle;
        }
        
        /// @brief Get unique instance of Windows window-mode application handle
        static WindowsApp& instance() noexcept {
          static WindowsApp _instance;
          return _instance;
        }
        
      private:
        AppInstanceHandle _handle = nullptr;
      };
    }
  }
#endif
