/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pandora { 
  namespace hardware {
    /// @brief Display position/area ("virtual desktop" coordinates)
    struct DisplayArea final {
      int32_t x;       ///< Horizontal position of a screen/context. Can be negative if it's not the primary display.
      int32_t y;       ///< Vertical position of a screen/context. Can be negative if it's not the primary display.
      uint32_t width;  ///< Horizontal size of a screen/context (pixels on desktops, points on iOS/Android).
      uint32_t height; ///< Vertical size of a screen/context (pixels on desktops, points on iOS/Android).
    };
    /// @brief Display mode settings for a monitor
    struct DisplayMode final {
      uint32_t width;       ///< Horizontal size (pixels).
      uint32_t height;      ///< Vertical size (pixels).
      uint32_t bitDepth;    ///< Color depth (bits per pixel). Usually 32.
      uint32_t refreshRate; ///< Display rate (milli-hertz)
    };
    constexpr inline uint32_t undefinedRefreshRate() noexcept { return 0; } ///< Unknown refresh rate

    // ---

    /// @class DisplayMonitor
    /// @brief Display monitor description (handle and attributes) + discovery utilities.
    /// @warning On Android, <system/android_app.h> must have been initialized with the app state object.
    class DisplayMonitor final {
    public:
#     if defined(_WINDOWS)
        using Handle = void*;
        using WindowHandle = void*;
        using DeviceId = std::wstring;
        using String = std::wstring;
#     else
#       if defined(__APPLE__) || defined(_P_ENABLE_LINUX_WAYLAND)
          using Handle = void*;
          using WindowHandle = void*;
#       else
          using Handle = uint64_t;
          using WindowHandle = int32_t;
#       endif
        using DeviceId = std::string;
        using String = std::string;
#     endif

      /// @brief Attributes of a display monitor on a system
      /// @warning Measures in 'screenArea' and 'workArea' only use physical pixels if the process is DPI aware.
      struct Attributes final {
        DeviceId id;            ///< Unique identifier on the system.
        String description;     ///< Description string of the monitor (usually a type of monitor or a brand).
        DisplayArea screenArea; ///< Display position/area of the entire screen in "virtual desktop". Can contain negative values if not a primary monitor.
        DisplayArea workArea;   ///< Max work area in "virtual desktop": screen area excluding taskbar/toolbars. Can contain negative values if not a primary monitor.
        bool isPrimary;         ///< Primary or secondary display monitor.
      };

      // -- monitor description --

      /// @brief Get primary/default monitor description
      /// @warning Throws on system init failure
      DisplayMonitor();
      /// @brief Get monitor description from handle
      /// @remarks - handles should not be stored in files or options -> they will change everytime the process is restarted
      ///          - usePrimaryAsDefault: if the monitor can't be found, the primary/default monitor will be used instead
      ///          - handles can be invalidated during runtime (new screen plugged, automatic GPU switching)
      /// @warning Throws invalid_argument if handle is invalid and !usePrimaryAsDefault, or if system init failure
      DisplayMonitor(Handle monitorHandle, bool usePrimaryAsDefault);
      /// @brief Get monitor description by identifier (or primary if not found and usePrimaryAsDefault==true)
      /// @remarks - usePrimaryAsDefault: if the monitor can't be found, the primary/default monitor will be used instead
      ///          - ID value can safely be stored (as long as the screen isn't unplugged or plugged somewhere else, the ID remains the same)
      /// @warning Throws invalid_argument if ID not found and !usePrimaryAsDefault, or if system init failure
      DisplayMonitor(const DisplayMonitor::DeviceId& id, bool usePrimaryAsDefault);
      /// @brief Get monitor description by index (or primary if not found and usePrimaryAsDefault==true)
      /// @remarks - usePrimaryAsDefault: if the monitor can't be found, the primary/default monitor will be used instead
      ///          - index can change during runtime (new screen plugged, automatic GPU switching)
      /// @warning Throws invalid_argument if index not found and !usePrimaryAsDefault, or if system init failure
      explicit DisplayMonitor(bool usePrimaryAsDefault, uint32_t index);

      DisplayMonitor(const DisplayMonitor&) = delete;
      DisplayMonitor(DisplayMonitor&&) = default;
      DisplayMonitor& operator=(const DisplayMonitor&) = delete;
      DisplayMonitor& operator=(DisplayMonitor&&) = default;
      ~DisplayMonitor() = default;

      /// @brief Get list of monitors available for display
      static std::vector<DisplayMonitor> listAvailableMonitors();

      // -- accessors --

      /// @brief Get native monitor handle
      inline Handle handle() noexcept { return this->_handle; }
      /// @brief Get display attributes of monitor
      inline const Attributes& attributes() const noexcept { return this->_attributes; }
      /// @brief Read associated adapter name/brand
      /// @warning - May be empty if virtual monitor (or running in a VM with no GPU support)
      ///          - Not supported on Apple systems (Mac, iOS) and Wayland (display server on some Linux environments)
      String adapterName() const;

      // -- display mode --

      /// @brief Read current display mode of a monitor
      DisplayMode getDisplayMode() const noexcept;
      
      /// @brief Change display mode of a monitor
      /// @warning - Not thread safe: do not call simultaneously in multiple threads
      ///          - Not supported by Wayland (display server on some Linux environments) -> must be changed via fullscreen window
      bool setDisplayMode(const DisplayMode& mode);
      /// @brief Reset monitor to its default display mode
      /// @warning - Not thread safe: do not call simultaneously in multiple threads
      ///          - Not supported by Wayland (display server on some Linux environments) -> must be changed via fullscreen window
      bool setDefaultDisplayMode();

      /// @brief Read available display modes for a monitor
      std::vector<DisplayMode> listAvailableDisplayModes() const;

      // -- DPI awareness --

      /// @brief Set DPI awareness for current process (if supported on current system)
      /// @warning - on Windows, it is recommended to use a manifest instead of this call.
      ///          - this function should be called once at the beginning of the process to avoid weird behaviors.
      /// @returns success
      static bool setDpiAwareness(bool isEnabled) noexcept;

      /// @brief Read per-window/per-monitor DPI if supported, or system DPI value
      /// @warning The process must be DPI aware (on Windows: requires manifest or calling setDpiAwareness(true))
      /// @remarks The 'windowHandle' argument is only useful on Windows, to read DPI per monitor (instead of system DPI)
      void getMonitorDpi(uint32_t& outDpiX, uint32_t& outDpiY, WindowHandle windowHandle = (WindowHandle)0) const noexcept;
      /// @brief Read window/monitor scaling factor if supported, or system scaling factor
      /// @warning The process must be DPI aware (on Windows: requires manifest or calling setDpiAwareness(true))
      /// @remarks The 'windowHandle' argument is only useful on Windows, to read monitor scaling (instead of system scaling)
      void getMonitorScaling(float& outScaleX, float& outScaleY, DisplayMonitor::WindowHandle windowHandle = (WindowHandle)0) const noexcept;

      // -- metrics --

      /// @brief Measure, depending on monitor config and DPI, expected window size, based on client area.
      /// @remarks Native style flags are optional. They'll only be used if the style can't be read from the window handle.
      DisplayArea convertClientAreaToWindowArea(const DisplayArea& clientArea, WindowHandle windowHandle, bool hasMenu = false, 
                                                uint32_t nativeStyleFlags = 0, uint32_t nativeSecondaryFlags = 0) const noexcept;

    private:
#     if defined(_WINDOWS)
        Attributes _attributes{ L"", L"", { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, false };
#     else
        Attributes _attributes{ "", "", { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, false };
#     endif

#     if defined(__APPLE__)
        mutable Handle _handle = (Handle)0;
#     else
        Handle _handle = (Handle)0;
#     endif
      
#     if defined(__APPLE__)
        DisplayMonitor(uint32_t displayId);
        uint32_t _unitNumber = 0;
#     elif defined(_P_ENABLE_LINUX_WAYLAND)
        uint32_t _id = 0;
#     elif !defined(__ANDROID__) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
        Handle _controller = (Handle)0;
#     endif
    };
  }
}
