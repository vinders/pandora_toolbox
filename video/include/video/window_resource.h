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
    class WindowMenu;

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
        colorBrush = 2u, ///< Painting color brush (background / text)
        menu       = 3u  ///< Window menu handle
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
      static std::shared_ptr<WindowResource> buildIcon(SystemIcon id);
      
      /// @brief Create custom icon from external file
      /// @remarks To use default system icon size, set width or height to 0.
      /// @returns On success: valid icon resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildIconFromFile(const char* uri, uint32_t width = 0, uint32_t height = 0);
#     ifdef _WINDOWS
        static std::shared_ptr<WindowResource> buildIconFromFile(const wchar_t* uri, uint32_t width = 0, uint32_t height = 0);
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
      static std::shared_ptr<WindowResource> buildCursor(SystemCursor id);
      
      /// @brief Create custom mouse pointer from external file
      /// @returns On success: valid cursor resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildCursorFromFile(const char* uri);
#     ifdef _WINDOWS
        static std::shared_ptr<WindowResource> buildCursorFromFile(const wchar_t* uri);
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
      
      /// @brief Get RGB color reference (red/green/blue: 0-255)
      static constexpr inline Color rgbColor(uint32_t red, uint32_t green, uint32_t blue) noexcept {
        return static_cast<Color>(red | (green << 8) | (blue << 16));
      }
      /// @brief Get default system window background reference
      static constexpr inline Color systemWindowColor() noexcept { return static_cast<Color>(-1L); }
      /// @brief Get default system text color reference
      static constexpr inline Color systemTextColor() noexcept { return static_cast<Color>(-2L); }
      /// @brief Get transparent color reference
      static constexpr inline Color transparentColor() noexcept { return static_cast<Color>(-3L); }
      
      /// @brief Create color style resource
      /// @returns On success: valid color resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildColorBrush(Color color);
      
      
      // -- menu container builder --

      /// @brief Create window menu resource
      /// @warning 'mainMenu' must be a menubar (WindowMenu.isSubMenu==false), not a sub-menu.
      /// @returns On success: valid menu resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildMenu(WindowMenu&& mainMenu);
      /// @brief Create native menu container (to guarantee destruction) -- native menu
      /// @returns On success: valid menu resource. On failure: nullptr.
      static std::shared_ptr<WindowResource> buildMenu(MenuHandle handle);
      
      
    private:
      WindowResourceHandle _handle { (WindowResourceHandle)0 };
      Category _type { Category::icon };
    };

    // ---

    /// @class WindowMenu
    /// @brief Window menubar/sub-menu builder
    class WindowMenu final {
    public:
      /// @brief Menu item category
      enum class ItemType : uint32_t {
        text       = 0u,
        checkboxOn = 1u,
        checkboxOff= 2u,
        radioOn    = 3u,
        radioOff   = 4u
      };

      /// @brief Create main menubar (isSubMenu==false) or popup sub-menu (isSubMenu==true)
      WindowMenu(bool isSubMenu);
      WindowMenu(const WindowMenu&) = delete;
      WindowMenu(WindowMenu&& rhs) noexcept : _handle(rhs._handle), _isSubMenu(rhs._isSubMenu) { rhs._handle = (MenuHandle)0; }
      WindowMenu& operator=(const WindowMenu&) = delete;
      WindowMenu& operator=(WindowMenu&& rhs) noexcept { 
        _handle = rhs._handle; _isSubMenu = rhs._isSubMenu; rhs._handle = (MenuHandle)0; 
        return *this; 
      }
      ~WindowMenu() noexcept;

      inline bool isSubMenu() const noexcept { return this->_isSubMenu; } ///< Verify if instance is sub-menu (true) or main menu

      /// @brief Add menu command item
      /// @param id         Unique identifier of menu command (for menu event handler)
      /// @param label      Text to display for menu item
      /// @param isEnabled  Item can be selected (true) or is grayed (false)
      /// @warning 'item' must be a sub-menu (isSubMenu==true).
      /// @returns Success
#     ifdef _WINDOWS
        bool insertItem(uint32_t id, const wchar_t* label, ItemType type = ItemType::text, bool isEnabled = true) noexcept;
#     else
        bool insertItem(uint32_t id, const char* label, ItemType type = ItemType::text, bool isEnabled = true) noexcept;
#     endif
      /// @brief Add sub-menu
      /// @warning 'item' must be a sub-menu (isSubMenu==true).
      /// @returns Success
#     ifdef _WINDOWS
        bool insertSubMenu(WindowMenu&& item, const wchar_t* label) noexcept;
#     else
        bool insertSubMenu(WindowMenu&& item, const char* label) noexcept;
#     endif
      /// @brief Insert item separator (vertical separator in sub-menus, horizontal separator in main menu)
      void insertSeparator() noexcept;

      // ---

      /// @brief Change state of a checkbox/radio-button
      static void changeCheckItemState(MenuHandle menu, uint32_t id, bool isChecked, bool isEnabled = true) noexcept;
      /// @brief Change checkboxes/radio-buttons selection
      /// @param isUncheckedEnabled  Unchecked item is still checkable (true) or is grayed (false)
      static void changeCheckItemState(MenuHandle menu, uint32_t checkId, uint32_t uncheckId, bool isUncheckedEnabled = true) noexcept;
      /// @brief Change state of any other item type
      static void changeItemState(MenuHandle menu, uint32_t id, bool isEnabled) noexcept;

    private:
      friend class pandora::video::WindowResource;
      MenuHandle _handle = (MenuHandle)0;
      bool _isSubMenu = false;
    };
  }
}
