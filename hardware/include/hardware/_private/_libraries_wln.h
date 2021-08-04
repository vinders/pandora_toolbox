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

#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cstddef>
# include <cstdint>
# include <mutex>
# include <unistd.h>
# include <wayland-client.h>
# include "wayland-xdg-shell-client-protocol.h"
# include "wayland-xdg-decoration-client-protocol.h"
# include "wayland-viewporter-client-protocol.h"
# include "wayland-relative-pointer-unstable-v1-client-protocol.h"
# include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
# include "wayland-idle-inhibit-unstable-v1-client-protocol.h"
// # include <xkbcommon/xkbcommon.h>
// # ifdef _P_ENABLE_LINUX_XKBCOMMON_COMPOSE
// #   include <xkbcommon/xkbcommon-compose.h>
// # endif

# define __P_HARDWARE_WAYLAND_BASE_DPI 96.0f

  namespace pandora {
    namespace hardware {
      using LibHandle = void*;
      
      // display mode description
      struct WaylandDisplayMode {
        uint32_t width;  // px
        uint32_t height; // px
        uint32_t refreshRate; // mHz
      };
      
      // output description (linked list item)
      struct WaylandOutput {
        uint32_t id;
        wl_output* output;
        char* description;
        
        int32_t x;
        int32_t y;
        int32_t physWidth;
        int32_t physHeight;
        uint32_t scaleFactor;
        
        WaylandDisplayMode* modes;
        size_t modesLength;
        uint32_t currentMode;
        int32_t preferredMode;
        
        WaylandOutput* next;
      };
      
      // library loader - Wayland
      struct LibrariesWayland final {
        LibrariesWayland() = default;
        ~LibrariesWayland() noexcept { shutdown(); }
        
        struct {
          std::recursive_mutex outputsLock;
          WaylandOutput*    outputs       = nullptr;
          
          wl_display*       display       = nullptr;
          wl_registry*      registry      = nullptr;
          wl_compositor*    compositor    = nullptr;
          wl_subcompositor* subCompositor = nullptr;
          wl_shm*           sharedMemory  = nullptr;
          uint32_t          compositorVersion = 0;
        } wl;
        struct {
          xdg_wm_base*                windowManagerBase = nullptr;
          zxdg_decoration_manager_v1* decorationManager = nullptr;
        } xdg;
        struct {
          wp_viewporter*                   viewporter    = nullptr;
          zwp_relative_pointer_manager_v1* relativePointerManager = nullptr;
          zwp_pointer_constraints_v1*      pointerConstraints = nullptr;
          zwp_idle_inhibit_manager_v1*     idleInhibitManager = nullptr;
        } wp;
        
        // utilities
        static void getScreenPixelSize(WaylandOutput& data, uint32_t& outWidth, uint32_t& outHeight) noexcept;
        static uint32_t getMonitorDpi(WaylandOutput& data) noexcept;
        
        // initialize available libraries
        void init();
        // close libraries
        void shutdown() noexcept;
        
        // get global instance
        static inline LibrariesWayland& instance() {
          if (!_libs._isInit)
            _libs.init();
          return _libs;
        }
      
        // -- reserved for internal usage --
        
        bool _isOutputAvailable(WaylandOutput* data) noexcept;
        void _insertOutput(uint32_t id, struct wl_registry* registry) noexcept;
        void _eraseOutput(uint32_t id) noexcept;
        
        static void _windowManagerPingHandler(void* data, struct xdg_wm_base* windowManagerBase, uint32_t serial);
        static void _globalRegistryHandler(void* data, struct wl_registry* registry, uint32_t id, 
                                           const char* interface, uint32_t version);
        static void _globalRegistryRemoveHandler(void* data, struct wl_registry* registry, uint32_t id);
        
        static void _outputGeometryHandler(void* data, struct wl_output* output, int32_t x, int32_t y, 
                                           int32_t physWidth, int32_t physHeight, int32_t subPixel, 
                                           const char* brand, const char* model, int32_t transform);
        static void _outputModeHandler(void* data, struct wl_output* output, uint32_t flags,
                                       int32_t width, int32_t height, int32_t refreshRate);
        static void _outputScaleHandler(void* data, struct wl_output* output, int32_t factor);
        static void _outputDoneHandler(void* data, struct wl_output* output);
      
      private:
        bool _isInit = false;
        static LibrariesWayland _libs;
      };
    }
  }
#endif
