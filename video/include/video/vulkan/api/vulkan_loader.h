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

#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cstdint>
# include "video/window_handle.h"
# include "video/vulkan/api/vulkan.h"

# ifndef APIENTRY
#   ifdef _WINDOWS
#     define APIENTRY __stdcall
#   else
#     define APIENTRY
#   endif
# endif

  namespace pandora {
    namespace video {
      namespace vulkan {
        using LibHandle = void*;
        using FunctionPtr = void*;
        
        /// @brief Platform-specific instance surface extensions
        enum class PlatformExtension : int {
          unknown                = 0,
          KHR_win32_surface   = 1, // windows
          EXT_metal_surface   = 2, // macOS / iOS
          MVK_macos_surface   = 3, // macOS
          MVK_ios_surface     = 4, // iOS
          KHR_xlib_surface    = 5, // linux / unix / BSD
          KHR_xcb_surface     = 6, // linux / unix / BSD
          KHR_wayland_surface = 7, // linux-wayland
          KHR_android_surface = 8  // android
        };
        
        // vulkan lib functions
        typedef void (APIENTRY* __vk_voidFunction)(void);
        typedef __vk_voidFunction (APIENTRY* __vk_GetInstanceProcAddr)(VkInstance,const char*);
        typedef VkResult (APIENTRY* __vk_EnumerateInstanceExtensionProperties)(const char*,uint32_t*,VkExtensionProperties*);
        
        
        /// @brief library loader - Vulkan
        /// @remarks Vulkan libs are already properly initialized by the Renderer class.
        ///          Using this outside of the Renderer class is useful:
        ///          - to verify the API support;
        ///          - to check supported extensions;
        ///          - to get vulkan function bindings for custom usage.
        struct VulkanLoader final {
          VulkanLoader() = default;
          ~VulkanLoader() noexcept { shutdown(); }
          
          struct {
            LibHandle instance = nullptr;
            PlatformExtension platformExtension = PlatformExtension::unknown;

            __vk_GetInstanceProcAddr                  GetInstanceProcAddr_ = nullptr;
            __vk_EnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties_ = nullptr;
          } vk;

          /// @brief Initialize libraries (called automatically by 'instance()')
          void init();
          /// @brief Close libraries (called automatically at application shutdown)
          void shutdown() noexcept;
          
          
          // -- access / bindings --
        
          /// @brief Get global instance (+ load library during first call)
          /// @throws runtime_error/bad_alloc on init failure / if not supported.
          static inline VulkanLoader& instance() {
            if (!_libs._isInit)
              _libs.init();
            return _libs;
          }
          
          /// @brief Get extension ID used for current platform surface (ex: "VK_KHR_win32_surface")
          const char* getPlatformSurfaceExtensionId() const noexcept;
          /// @brief Get Vulkan function binding to dynamic library
          /// @returns Function pointer (or NULL if not supported)
          /// @remarks If the function is used more than once, this binding should be stored to be reusable efficiently.
          FunctionPtr getVulkanInstanceFunction(VkInstance instance, const char* functionName) noexcept;
          
          /// @brief Verify if vulkan is supported on current system
          /// @remarks Vulkan will be initialized by the call. If you don't intend to use it in your program,
          ///          you might want to call 'VulkanLoader::instance().shutdown()' after this.
          static inline bool isSupported() noexcept {
            try {
              _libs.init();
              return true;
            }
            catch (...) { return false; }
          }
          
          
          // -- device / window management --
          
          /// @brief Determine whether a queue family of a physical device supports presentation on system display
          bool getDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device, uint32_t queueFamily) noexcept;
          /// @brief Create surface associated with a window for display
          VkResult createWindowSurface(VkInstance instance, pandora::video::WindowHandle window,
                                       const VkAllocationCallbacks* allocator,
                                       VkSurfaceKHR& outSurface) noexcept;
        
        private:
          bool _isInit = false;
          static VulkanLoader _libs;
        };
        
      }
    }
  }
#endif
