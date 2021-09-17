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
#if defined(_VIDEO_VULKAN_SUPPORT)
# include <cstdlib>
# include <cstring>
# include <memory>
# include <stdexcept>
# include "video/vulkan/api/vulkan_loader.h"

# if defined(_WINDOWS)
#   ifndef __MINGW32__
#     pragma warning(push)
#     pragma warning(disable: 26812) // disable warnings about vulkan enums
#   endif
#   define NODRAWTEXT
#   define NOGDI
#   define NOBITMAP
#   define NOMCX
#   define NOGDICAPMASKS
#   define NOVIRTUALKEYCODES
#   define NOWINSTYLES
#   define NOSYSMETRICS
#   define NOMENUS
#   define NOICONS
#   define NOKEYSTATES
#   define NOSHOWWINDOW
#   define NOSCROLL
#   define NOSOUND
#   define NOKANJI
#   define NOHELP
#   define NOPROFILER
#   define NODEFERWINDOWPOS
#   define NOTAPE
#   include <system/api/windows_api.h>
#   include <system/api/windows_app.h>
    using pandora::system::WindowsApp;
# else
#   include <dlfcn.h>
#   ifdef __APPLE__
#     include <TargetConditionals.h>
#     include "video/vulkan/_private/_vulkan_libs_apple.h"
#   elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
#     ifdef _P_ENABLE_LINUX_WAYLAND
#       include <hardware/_private/_libraries_wln.h>
        using pandora::hardware::LibrariesWayland;
#     else
#       include <hardware/_private/_libraries_x11.h>
        using pandora::hardware::LibrariesX11;
#     endif
#   endif
# endif
  using namespace pandora::video::vulkan;

  VulkanLoader VulkanLoader::_libs{};


  // -- library binding utilities -- -------------------------------------------
  
# if defined(_WINDOWS)
#   define __P_LIBVULKAN_FILE1      "vulkan-1.dll"
#   define __P_LIBVULKAN_FILE2      "vulkan-1-999-0-0-0.dll"
#   define __P_VULKAN_PLATFORM_EXT  PlatformExtension::KHR_win32_surface
#   define __P_VULKAN_PLATFORM_EXT_NAME "VK_KHR_win32_surface"
# elif defined(__ANDROID__)
#   define __P_LIBVULKAN_FILE1      "libvulkan.so"
#   define __P_LIBVULKAN_FILE2      "libvulkan.so.1"
#   define __P_LIBVULKAN_FILE3      "libvulkan-1.so"
#   define __P_VULKAN_PLATFORM_EXT  PlatformExtension::KHR_android_surface
#   define __P_VULKAN_PLATFORM_EXT_NAME "VK_KHR_android_surface"

# elif defined(__APPLE__)
#   define __P_LIBVULKAN_FILE1                "libvulkan.1.dylib"
#   define __P_LIBVULKAN_FILE2                "libvulkan.dylib"
#   define __P_LIBVULKAN_FILE3                "libMoltenVK.dylib"
#   define __P_VULKAN_PLATFORM_EXT            PlatformExtension::EXT_metal_surface // preferred extension first
#   define __P_VULKAN_PLATFORM_EXT_NAME       "VK_EXT_metal_surface"
#   if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#     define __P_VULKAN_PLATFORM_EXT_ALT      PlatformExtension::MVK_ios_surface   // other extension (if preferred not available)
#     define __P_VULKAN_PLATFORM_EXT_ALT_NAME "VK_MVK_ios_surface"
#   else
#     define __P_VULKAN_PLATFORM_EXT_ALT      PlatformExtension::MVK_macos_surface // other extension (if preferred not available)
#     define __P_VULKAN_PLATFORM_EXT_ALT_NAME "VK_MVK_macos_surface"
#   endif

# elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
#   define __P_LIBVULKAN_FILE1            "libvulkan.so.1"
#   define __P_LIBVULKAN_FILE2            "libvulkan.so"
#   define __P_LIBVULKAN_FILE3            "libvulkan-1.so"
#   if defined(_P_ENABLE_LINUX_WAYLAND)
#     define __P_VULKAN_PLATFORM_EXT      PlatformExtension::KHR_wayland_surface
#     define __P_VULKAN_PLATFORM_EXT_NAME "VK_KHR_wayland_surface"
#   else
      // note: some xlib extensions expose features with no implementation -> prefer xcb if available
#     define __P_VULKAN_PLATFORM_EXT          PlatformExtension::KHR_xcb_surface  // preferred extension first
#     define __P_VULKAN_PLATFORM_EXT_NAME     "VK_KHR_xcb_surface"
#     define __P_VULKAN_PLATFORM_EXT_ALT      PlatformExtension::KHR_xlib_surface // other extension (if preferred not available)
#     define __P_VULKAN_PLATFORM_EXT_ALT_NAME "VK_KHR_xlib_surface"
#     define __P_IF_VULKAN_PREFERRED_PLATFORM_AVAILABLE \
             bool isLibOk; \
             try{ isLibOk=LibrariesX11::instance().xcb.isAvailable; }catch(...){ isLibOk=false; } \
             if (isLibOk)
#   endif
# endif
  
# if defined(_WINDOWS)
    static inline LibHandle _loadLibrary(const char fileName[]) noexcept {
      return LoadLibraryA(fileName);
    }
    static inline void _freeLibrary(LibHandle lib) noexcept {
      FreeLibrary((HMODULE)lib);
    }
    template <typename _Signature>
    static inline _Signature _getSymbolAddress(LibHandle lib, const char name[]) noexcept {
      return (_Signature)GetProcAddress((HMODULE)lib, name);
    }
# else
    static inline LibHandle _loadLibrary(const char fileName[]) noexcept {
#     if defined(__APPLE__)
        LibHandle inst = dlopen(fileName, RTLD_NOW | RTLD_LOCAL);
        if (inst == nullptr)
          inst = _getLocalVulkanLoader_apple(fileName);
        return inst;
#     else
        return dlopen(fileName, RTLD_NOW | RTLD_LOCAL);
#     endif
    }
    static inline void _freeLibrary(LibHandle lib) noexcept {
      dlclose(lib);
    }
    template <typename _Signature>
    static inline _Signature _getSymbolAddress(LibHandle lib, const char name[]) noexcept {
      return (_Signature)dlsym(lib, name);
    }
# endif

  // ---
  
  // Try to find preferred available platform surface extension
  // -> throws on failure
  static PlatformExtension _findPlatformSurfaceExtension(__vk_EnumerateInstanceExtensionProperties enumerator,
                                                         bool& outHasKhrDisplay) {
    bool hasBaseKhr = false;
    outHasKhrDisplay = false;
    PlatformExtension platformExt = PlatformExtension::unknown;

    // query extensions
    uint32_t extCount = 0;
    if (enumerator(nullptr, &extCount, nullptr) != VK_SUCCESS || extCount == 0)
      throw std::runtime_error("Vulkan: failed to count extensions");

    auto allExt = std::unique_ptr<VkExtensionProperties[]>(new VkExtensionProperties[extCount]);
    if (enumerator(nullptr, &extCount, allExt.get()))
      throw std::runtime_error("Vulkan: failed to query extensions");
    
    // search for platform extension
    VkExtensionProperties* endIt = allExt.get() + (intptr_t)extCount;
    for (VkExtensionProperties* it = allExt.get(); it < endIt; ++it) {
      if (!hasBaseKhr && strcmp(it->extensionName, "VK_KHR_surface") == 0) {
        hasBaseKhr = true;
      }
      else if (!outHasKhrDisplay && strcmp(it->extensionName, "VK_KHR_display") == 0) {
        outHasKhrDisplay = true;
      }
      else if (strcmp(it->extensionName, __P_VULKAN_PLATFORM_EXT_NAME) == 0) { // preferred platform extension
#       ifdef __P_IF_VULKAN_PREFERRED_PLATFORM_AVAILABLE
          __P_IF_VULKAN_PREFERRED_PLATFORM_AVAILABLE {
            platformExt = __P_VULKAN_PLATFORM_EXT;
          }
#       else
          platformExt = __P_VULKAN_PLATFORM_EXT;
          if (hasBaseKhr)
            break;
#       endif
      }
#     ifdef __P_VULKAN_PLATFORM_EXT_ALT // other platform extension (if preferred not available)
        else if (platformExt == PlatformExtension::unknown) {
          if (strcmp(it->extensionName, __P_VULKAN_PLATFORM_EXT_ALT_NAME) == 0)
            platformExt = __P_VULKAN_PLATFORM_EXT_ALT;
        }
#     endif
    }
    
    if (!hasBaseKhr || platformExt == PlatformExtension::unknown)
      throw std::runtime_error("Vulkan: missing platform surface extensions");
    return platformExt;
  }
  

  // -- init / access / bindings -- --------------------------------------------
  
  void VulkanLoader::init() {
    if (this->_isInit)
      return;
    
    // load vulkan library
    this->vk.instance = _loadLibrary(__P_LIBVULKAN_FILE1);
    if (this->vk.instance == nullptr) {
      this->vk.instance = _loadLibrary(__P_LIBVULKAN_FILE2);
      if (this->vk.instance == nullptr) {
#       ifdef __P_LIBVULKAN_FILE3
          this->vk.instance = _loadLibrary(__P_LIBVULKAN_FILE3);
          if (this->vk.instance == nullptr)
#       endif
        throw std::runtime_error("Vulkan: loader not found (" __P_LIBVULKAN_FILE1 ")");
      }
    }
    
    // bind functions
    this->vk.GetInstanceProcAddr_ = _getSymbolAddress<__vk_GetInstanceProcAddr>(this->vk.instance, "vkGetInstanceProcAddr");
    if (this->vk.GetInstanceProcAddr_ == nullptr) {
      shutdown();
      throw std::runtime_error("Vulkan: loader has no vkGetInstanceProcAddr");
    }
    
    this->vk.EnumerateInstanceExtensionProperties_ = (__vk_EnumerateInstanceExtensionProperties)
                                                     this->vk.GetInstanceProcAddr_(nullptr, "vkEnumerateInstanceExtensionProperties");
    if (this->vk.EnumerateInstanceExtensionProperties_ == nullptr) {
      shutdown();
      throw std::runtime_error("Vulkan: vkEnumerateInstanceExtensionProperties not found");
    }

    this->vk.EnumerateInstanceLayerProperties_ = (__vk_EnumerateInstanceLayerProperties)
                                                 this->vk.GetInstanceProcAddr_(nullptr, "vkEnumerateInstanceLayerProperties");
    if (this->vk.EnumerateInstanceLayerProperties_ == nullptr) {
      shutdown();
      throw std::runtime_error("Vulkan: vkEnumerateInstanceLayerProperties not found");
    }
    
    // find preferred platform surface extension
    try {
      this->vk.platformExtension = _findPlatformSurfaceExtension(this->vk.EnumerateInstanceExtensionProperties_, this->vk.isKhrDisplaySupported);
    }
    catch (...) { shutdown(); throw; }

    this->_isInit = true; // init success
  }
  
  // ---

  void VulkanLoader::shutdown() noexcept {
    if (this->vk.instance != nullptr) {
      _freeLibrary(this->vk.instance);
      this->vk.instance = nullptr;
    }
    this->_isInit = false;
  }
  

  // -- extension / layer support -- -------------------------------------------
  
  const char* VulkanLoader::getPlatformSurfaceExtensionId() const noexcept {
    if (this->vk.platformExtension == __P_VULKAN_PLATFORM_EXT)
      return __P_VULKAN_PLATFORM_EXT_NAME;
#   ifdef __P_VULKAN_PLATFORM_EXT_ALT
    else if (this->vk.platformExtension == __P_VULKAN_PLATFORM_EXT_ALT)
      return __P_VULKAN_PLATFORM_EXT_ALT_NAME;
#   endif
    return ""; // should not happen (verified during init)
  }
  
  size_t VulkanLoader::findExtensions(const char** extensions, size_t length, bool* outResults) const {
    uint32_t availableExtCount = 0;
    if (this->vk.EnumerateInstanceExtensionProperties_(nullptr, &availableExtCount, nullptr) != VK_SUCCESS || availableExtCount == 0)
      throw std::runtime_error("Vulkan: failed to count extensions");

    auto availableExt = std::unique_ptr<VkExtensionProperties[]>(new VkExtensionProperties[availableExtCount]);
    VkResult queryResult = this->vk.EnumerateInstanceExtensionProperties_(nullptr, &availableExtCount, availableExt.get());
    if (queryResult != VK_SUCCESS && queryResult != VK_INCOMPLETE)
      throw std::runtime_error("Vulkan: failed to query extensions");

    size_t numberFound = 0;
    memset(outResults, 0, length*sizeof(*outResults)); // set all results to false
    
    VkExtensionProperties* endIt = availableExt.get() + (intptr_t)availableExtCount;
    for (const char** currentExt = extensions; length; --length, ++currentExt, ++outResults) {
      for (VkExtensionProperties* it = availableExt.get(); it < endIt; ++it) {
        if (strcmp(*currentExt, it->extensionName) == 0) {
          it->extensionName[0] = '\0'; // remove from list -> faster comparison
          
          *outResults = true;
          ++numberFound;
          break;
        }
      }
    }
    return numberFound;
  }

  bool VulkanLoader::findLayer(const char* layerName) const {
    uint32_t availableLayerCount = 0;
    if (this->vk.EnumerateInstanceLayerProperties_(&availableLayerCount, nullptr) != VK_SUCCESS || availableLayerCount == 0)
      throw std::runtime_error("Vulkan: failed to count layers");

    auto availableLayers = std::unique_ptr<VkLayerProperties[]>(new VkLayerProperties[availableLayerCount]);
    VkResult queryResult = this->vk.EnumerateInstanceLayerProperties_(&availableLayerCount, availableLayers.get());
    if (queryResult != VK_SUCCESS && queryResult != VK_INCOMPLETE)
      throw std::runtime_error("Vulkan: failed to query layers");

    VkLayerProperties* endIt = availableLayers.get() + (intptr_t)availableLayerCount;
    for (VkLayerProperties* it = availableLayers.get(); it < endIt; ++it) {
      if (strcmp(layerName, it->layerName) == 0)
        return true;
    }
    return false;
  }


  // -- device / window management -- ------------------------------------------
  
# if defined(_WINDOWS)
    typedef VkBool32 (APIENTRY* __vk_GetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice,uint32_t);
    typedef VkResult (APIENTRY* __vk_CreateWin32SurfaceKHR)(VkInstance,const VkWin32SurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
# elif defined(__ANDROID__)
    typedef VkResult (APIENTRY* __vk_CreateAndroidSurfaceKHR)(VkInstance,const VkAndroidSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
# elif defined(_P_ENABLE_LINUX_WAYLAND)
    typedef VkBool32 (APIENTRY* __vk_GetPhysicalDeviceWaylandPresentationSupportKHR)(VkPhysicalDevice,uint32_t,struct wl_display*);
    typedef VkResult (APIENTRY* __vk_CreateWaylandSurfaceKHR)(VkInstance,const VkWaylandSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
# elif !defined(__APPLE__)
    typedef VkBool32 (APIENTRY* __vk_GetPhysicalDeviceXlibPresentationSupportKHR)(VkPhysicalDevice,uint32_t,Display*,VisualID);
    typedef VkBool32 (APIENTRY* __vk_GetPhysicalDeviceXcbPresentationSupportKHR)(VkPhysicalDevice,uint32_t,xcb_connection_t*,xcb_visualid_t);
    typedef VkResult (APIENTRY* __vk_CreateXlibSurfaceKHR)(VkInstance,const VkXlibSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
    typedef VkResult (APIENTRY* __vk_CreateXcbSurfaceKHR)(VkInstance,const VkXcbSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
# endif

  // ---

  // Determine whether a queue family of a physical device supports presentation on system display
  bool VulkanLoader::getDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device, uint32_t queueFamily) noexcept {
#   if defined(_WINDOWS)
      auto deviceSupportGetter = (__vk_GetPhysicalDeviceWin32PresentationSupportKHR)
                                 getVulkanInstanceFunction(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
      return (deviceSupportGetter != nullptr && deviceSupportGetter(device, queueFamily));

#   elif defined(__APPLE__) || defined(__ANDROID__)
      return true;
      
#   elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
      try {
#       if defined(_P_ENABLE_LINUX_WAYLAND)
          auto deviceSupportGetter = (__vk_GetPhysicalDeviceWaylandPresentationSupportKHR)
                                     getVulkanInstanceFunction(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
          return (deviceSupportGetter != nullptr && deviceSupportGetter(device, queueFamily, LibrariesWayland::instance().wl.display));
#       else
          LibrariesX11& libs = LibrariesX11::instance();
          VisualID visualId = libs.xlib.VisualIDFromVisual_(DefaultVisual(libs.displayServer, libs.screenIndex));
          
          if (this->vk.platformExtension == PlatformExtension::VK_KHR_xcb_surface) {
            auto deviceSupportGetter = (__vk_GetPhysicalDeviceXcbPresentationSupportKHR)
                                       getVulkanInstanceFunction(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
            xcb_connection_t* connection = libs.xcb.GetXCBConnection_(libs.displayServer);
            return (deviceSupportGetter != nullptr && connection != nullptr
                 && deviceSupportGetter(device, queueFamily, connection, visualId));
          }
          else {
            auto deviceSupportGetter = (__vk_GetPhysicalDeviceXlibPresentationSupportKHR)
                                       getVulkanInstanceFunction(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
            return (deviceSupportGetter != nullptr && deviceSupportGetter(device, queueFamily, libs.displayServer, visualId));
          }
#       endif
      }
      catch (...) { return false; }
#   endif
  }
 
  // ---

# if defined(__APPLE__)
#   if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#     define __P_VK_PLATFORM_CREATE_SURFACE_MVK "vkCreateIOSSurfaceMVK"
#   else
#     define __P_VK_PLATFORM_CREATE_SURFACE_MVK "vkCreateMacOSSurfaceMVK"
#   endif
# endif
 
  // Create surface associated with a window for display
  VkResult VulkanLoader::createWindowSurface(VkInstance instance, pandora::video::WindowHandle window,
                                             const VkAllocationCallbacks* allocator, VkSurfaceKHR& outSurface) noexcept {
#   if defined(_WINDOWS) // Windows
      auto surfaceCreator = (__vk_CreateWin32SurfaceKHR)
                            getVulkanInstanceFunction(instance, "vkCreateWin32SurfaceKHR");
      if (surfaceCreator != nullptr) {
        VkWin32SurfaceCreateInfoKHR params;
        memset(&params, 0, sizeof(params));
        params.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        params.hinstance = (HINSTANCE)GetWindowLongPtrW((HWND)window, GWLP_HINSTANCE);
        if (!params.hinstance) {
          params.hinstance = WindowsApp::instance().isInitialized()
                           ? (HINSTANCE)WindowsApp::instance().handle()
                           : (HINSTANCE)GetModuleHandle(NULL);
        }
        params.hwnd = (HWND)window;
        return surfaceCreator(instance, &params, allocator, &outSurface);
      }
 
#   elif defined(__APPLE__) // MacOS / iOS
      if (this->vk.platformExtension == PlatformExtension::VK_EXT_metal_surface) {
        auto surfaceCreator = getVulkanInstanceFunction(instance, "vkCreateMetalSurfaceEXT");
        if (surfaceCreator != nullptr)
          return _createWindowSurface_apple(surfaceCreator, true, instance, window, allocator, outSurface);
      }
      else {
        auto surfaceCreator = getVulkanInstanceFunction(instance, __P_VK_PLATFORM_CREATE_SURFACE_MVK);
        if (surfaceCreator != nullptr)
            return _createWindowSurface_apple(surfaceCreator, false, instance, window, allocator, outSurface);
      }
      
#   elif defined(__ANDROID__) // Android
      auto surfaceCreator = (__vk_CreateAndroidSurfaceKHR)
                            getVulkanInstanceFunction(instance, "vkCreateAndroidSurfaceKHR");
      if (surfaceCreator != nullptr) {
        //TODO
      }
      
#   elif defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)
      try {
#       if defined(_P_ENABLE_LINUX_WAYLAND) // Linux - Wayland
          auto surfaceCreator = (__vk_CreateWaylandSurfaceKHR)
                                getVulkanInstanceFunction(instance, "vkCreateWaylandSurfaceKHR");
          if (surfaceCreator != nullptr) {
            VkWaylandSurfaceCreateInfoKHR params;
            memset(&params, 0, sizeof(params));
            params.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            params.display = LibrariesWayland::instance().wl.display;
            params.surface = (wl_surface*)window;
            return surfaceCreator(instance, &params, allocator, &outSurface);
          }
#       else // Linux/Unix - X11
          if (this->vk.platformExtension == PlatformExtension::VK_KHR_xcb_surface) {
            xcb_connection_t* connection = libs.xcb.GetXCBConnection_(LibrariesX11::instance().displayServer);
            auto surfaceCreator = (__vk_CreateXcbSurfaceKHR)
                                  getVulkanInstanceFunction(instance, "vkCreateXcbSurfaceKHR");
            if (connection != nullptr && surfaceCreator != nullptr) {
              VkXcbSurfaceCreateInfoKHR params;
              memset(&params, 0, sizeof(params));
              params.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
              params.connection = connection;
              params.window = (xcb_window_t)window;
              return surfaceCreator(instance, &params, allocator, &outSurface);
            }
          }
          else {
            auto surfaceCreator = (__vk_CreateXlibSurfaceKHR)
                                  getVulkanInstanceFunction(instance, "vkCreateXlibSurfaceKHR");
            if (surfaceCreator != nullptr) {
              VkXlibSurfaceCreateInfoKHR params;
              memset(&params, 0, sizeof(params));
              params.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
              params.dpy = LibrariesX11::instance().displayServer;
              params.window = (Window)window;
              return surfaceCreator(instance, &params, allocator, &outSurface);
            }
          }
#       endif
      }
      catch (...) {}
#   endif
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }

# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
