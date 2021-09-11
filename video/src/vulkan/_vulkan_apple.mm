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
#if defined(__APPLE__) && defined(_VIDEO_VULKAN_SUPPORT)
# include <cstdint>
# include <dlfcn.h>
# include "video/vulkan/api/vulkan.h"

# include <TargetConditionals.h>
# if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#   include <UIKit/UIKit.h>
# else
#   import <Carbon/Carbon.h>
#   import <Cocoa/Cocoa.h>
# endif
# include "video/vulkan/api/_private/_vulkan_apple.h"


  // -- vulkan loader -- -------------------------------------------------------

  static inline void* _loadLibrary(const char* fileName) noexcept {
    return dlopen(fileName, RTLD_LAZY | RTLD_LOCAL);
  }
  
  // ---

  void* pandora::video::vulkan::_getLocalVulkanLoader_apple(const char* fileName) noexcept {
    void* instance = NULL;
    @try {
      CFBundleRef mainBundle = CFBundleGetMainBundle();
      if (!mainBundle)
        return NULL;
      CFURLRef vulkanUrl = CFBundleCopyAuxiliaryExecutableURL(mainBundle, CFSTR(fileName));
      if (!vulkanUrl)
        return NULL;

      char path[PATH_MAX];
      if (CFURLGetFileSystemRepresentation(vulkanUrl, true, (UInt8*) path, sizeof(path) - 1))
        instance = _loadLibrary(path);
      
      CFRelease(vulkanUrl);
    }
    @catch (NSException*) {}
    return instance;
  }
  
  
  // -- vulkan surface creation -- ---------------------------------------------
  
  typedef VkResult (APIENTRY* __vk_CreateMetalSurfaceEXT)(VkInstance,const VkMetalSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
# if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    typedef VkResult (APIENTRY* __vk_CreateIOSSurfaceMVK)(VkInstance,const VkIOSSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
# else
    typedef VkResult (APIENTRY* __vk_CreateMacOSSurfaceMVK)(VkInstance,const VkMacOSSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
# endif

  VkResult pandora::video::vulkan::_createWindowSurface_apple(void* creator, bool isMetalExt, VkInstance instance,
                                                              void* window_, const VkAllocationCallbacks* allocator,
                                                              VkSurfaceKHR& outSurface) noexcept {
    @autoreleasepool {
#     if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
        UIWindow* window = (UIWindow*)window_;
        return VK_INCOMPLETE;// TODO
        
#     elif MAC_OS_X_VERSION_MAX_ALLOWED >= 101100
        NSWindow* window = (NSWindow*)window_;
        NSView* view = [window contentView];
        if (!view)
          return VK_ERROR_LAYER_NOT_PRESENT;
        
        // dynamically load to avoid extra dependency
        NSBundle* coreBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
        if (!coreBundle)
          return VK_ERROR_INITIALIZATION_FAILED;
      
        // create metal layer
        CAMetalLayer* metalLayer = [[coreBundle classNamed:@"CAMetalLayer"] layer]];
        if (!metalLayer)
          return VK_ERROR_INITIALIZATION_FAILED;
        [metalLayer setContentsScale:[window backingScaleFactor]];
        [view setLayer:window->ns.layer];
        [view setWantsLayer:YES];
        
        // metal extension
        if (isMetalExt) {
          VkMetalSurfaceCreateInfoEXT params;
          memset(&params, 0, sizeof(params));
          params.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
          params.pLayer = window->ns.layer;
          return ((__vk_CreateMetalSurfaceEXT)creator)(instance, &params, allocator, &outSurface);
        }
        // system extension
        else {
          VkMacOSSurfaceCreateInfoMVK params;
          memset(&params, 0, sizeof(params));
          params.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
          params.pView = window->ns.view;
          return ((__vk_CreateMacOSSurfaceMVK)creator)(instance, &params, allocator, &outSurface);
        }
#     else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
#     endif
    } // autoreleasepool
  }

#endif
