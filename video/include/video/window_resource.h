/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory>
#ifdef _WINDOWS
# include <system/api/windows_app.h>
#endif
#include "video/window_handle.h"

namespace pandora {
  namespace video {
    /// @brief Standard system icons
    enum class SystemIcon : uint32_t {
      app        = 0u, ///< Default application icon
      info       = 1u, ///< Information icon
      question   = 2u, ///< Question mark icon
      warning    = 3u, ///< Warning icon
      error      = 4u, ///< Error icon
      security   = 5u, ///< Security icon
      system     = 6u  ///< System logo
    };

    /// @brief Standard system mouse pointers
    enum class SystemCursor : uint32_t {
      pointer     = 0u, ///< Standard pointer
      hand        = 1u, ///< Hand pointer (for link/button/action)
      wait        = 2u, ///< Wait cursor (hourglass / clock)
      waitPointer = 3u, ///< Pointer with wait cursor (hourglass / clock)
      textIBeam   = 4u, ///< I-beam pointer (text selection "I")
      help        = 5u, ///< Help pointer (question mark / book)
      forbidden   = 6u, ///< Invalid/forbidden (slashed circle / cross)
      crosshair   = 7u, ///< Crosshair
      arrowV            = 8u, ///< Vertical arrow
      doubleArrowV      = 9u, ///< Double-pointed arrow (vertical)
      doubleArrowH      = 10u,///< Double-pointed arrow (horizontal)
      doubleArrow_NE_SW = 11u,///< Double-pointed arrow (oblique North-East -> South-West)
      doubleArrow_NW_SE = 12u,///< Double-pointed arrow (oblique North-West -> South-East)
      fourPointArrow    = 13u ///< Four-pointed arrow
    };
    
    // ---
    
    /// @class WindowResource
    /// @brief RAII window resource container (icon, cursor, color brush)
    class WindowResource final {
    public:
      /// @brief Resource type
      enum class Category : uint32_t {
        icon       = 0u, ///< Image displayed in caption / taskbar
        cursor     = 1u, ///< Custom mouse pointer
        colorBrush = 2u  ///< Painting color brush (background / text)
      };
      
      /// @brief Initialize window resource container
      WindowResource(WindowResourceHandle handle, Category type) : _handle(handle), _type(type) {}
      /// @brief Destroy window resource
      ~WindowResource() noexcept;
      
      WindowResource(const WindowResource&) = delete;
      WindowResource(WindowResource&& rhs) noexcept : _handle(rhs._handle), _type(rhs._type) { rhs._handle = (WindowResourceHandle)0; }
      WindowResource& operator=(const WindowResource&) = delete;
      WindowResource& operator=(WindowResource&& rhs) noexcept { 
        _handle = rhs._handle; _type = rhs._type; rhs._handle = (WindowResourceHandle)0; 
        return *this; 
      }
      
      
      // -- accessors --
      
      inline operator bool() const noexcept { return (this->_handle != (WindowResourceHandle)0); } ///< Verify if valid container (or moved)
      inline bool isValid() const noexcept { return (this->_handle != (WindowResourceHandle)0); }  ///< Verify if valid container (or moved)
      
      inline WindowResourceHandle handle() const noexcept { return this->_handle; } ///< Get native resource handle
      inline Category type() const noexcept { return this->_type; } ///< Get resource type


      // -- icon builder --
      
      /// @brief Create standard system icon
      /// @returns On success: valid icon resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildIcon(SystemIcon id) noexcept;
      
      /// @brief Create custom icon from external file
      /// @remarks To use default system icon size, set width or height to 0.
      /// @returns On success: valid icon resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildIconFromFile(const char* uri, uint32_t width = 0, uint32_t height = 0) noexcept;
#     ifdef _WINDOWS
        static std::shared_ptr<WindowResource> buildIconFromFile(const wchar_t* uri, uint32_t width = 0, uint32_t height = 0) noexcept;
#     endif

      /// @brief Create custom icon from app package
      /// @remarks: - Windows: embedded in program and referenced in .rc file -> use resource ID
      ///           - MacOS/iOS: located in application package -> use relative path/file-name
      ///           - Linux/unix: located in application directory -> use relative path/file-name
      /// @warning On Windows: - <system/windows_app.h> must have been initialized with the app state object
      ///                      - or the 'instance' param must not be NULL (if NULL, uses windows_app.h).
      /// @throws logic_error when app state not initialized
      /// @returns On success: valid icon resource. On failure: nullptr.
#     ifdef _WINDOWS
        static std::shared_ptr<WindowResource> buildIconFromPackage(const char* id, pandora::system::AppInstanceHandle instance = nullptr);
        static std::shared_ptr<WindowResource> buildIconFromPackage(const wchar_t* id, pandora::system::AppInstanceHandle instance = nullptr);
#     else
        static std::shared_ptr<WindowResource> buildIconFromPackage(const char* id);
#     endif


      // -- cursor builder --
      
      /// @brief Create standard system mouse pointer
      /// @returns On success: valid cursor resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildCursor(SystemCursor id) noexcept;
      
      /// @brief Create custom mouse pointer from external file
      /// @returns On success: valid cursor resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildCursorFromFile(const char* uri) noexcept;
#     ifdef _WINDOWS
        static std::shared_ptr<WindowResource> buildCursorFromFile(const wchar_t* uri) noexcept;
#     endif

      /// @brief Create custom mouse pointer from app package
      /// @remarks: - Windows: embedded in program and referenced in .rc file -> use resource ID
      ///           - MacOS/iOS: located in application package -> use relative path/file-name
      ///           - Linux/unix: located in application directory -> use relative path/file-name
      /// @warning On Windows: - <system/windows_app.h> must have been initialized with the app state object
      ///                      - or the 'instance' param must not be NULL (if NULL, uses windows_app.h).
      /// @throws logic_error when app state not initialized
      /// @returns On success: valid cursor resource. On failure: nullptr.
#     ifdef _WINDOWS
        static std::shared_ptr<WindowResource> buildCursorFromPackage(const char* id, pandora::system::AppInstanceHandle instance = nullptr);
        static std::shared_ptr<WindowResource> buildCursorFromPackage(const wchar_t* id, pandora::system::AppInstanceHandle instance = nullptr);
#     else
        static std::shared_ptr<WindowResource> buildCursorFromPackage(const char* id);
#     endif


      // -- color brush builder --
      
      using Color = uint32_t; ///< RGB color representation
      
      /// @brief Get RGB color reference
      static constexpr inline Color rgbColor(uint8_t red, uint8_t green, uint8_t blue) noexcept {
        return static_cast<Color>((uint32_t)red | (((uint32_t)green) << 8) | (((uint32_t)blue) << 16));
      }
      /// @brief Get default system window background reference
      static constexpr inline Color systemWindowColor() noexcept { return static_cast<Color>(-1L); }
      /// @brief Get default system text color reference
      static constexpr inline Color systemTextColor() noexcept { return static_cast<Color>(-2L); }
      /// @brief Get transparent color reference
      static constexpr inline Color transparentColor() noexcept { return static_cast<Color>(-3L); }
      
      /// @brief Create color style resource
      /// @returns On success: valid color resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildColorBrush(Color color) noexcept;
      
      
    private:
      WindowResourceHandle _handle { (WindowResourceHandle)0 };
      Category _type { Category::icon };
    };
  }
}
