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
# if defined(_WINDOWS)
#   ifndef __MINGW32__
#     pragma warning(push)
#     pragma warning(disable: 26812) // disable warnings about vulkan enums
#     pragma warning(disable: 4100)  // disable warnings about unused params
#   endif
# endif
# include <cstddef>
# include <cstdlib>
# include <cstring>
# include <cmath>
# include <stdexcept>
# include <memory/light_string.h>

# include "video/vulkan/api/vulkan_loader.h"
# include "video/vulkan/renderer.h"
# include "video/vulkan/_private/_vulkan_resource.h"

  using namespace pandora::video::vulkan;
  using namespace pandora::video;


// -----------------------------------------------------------------------------
// renderer.h
// -----------------------------------------------------------------------------

// -- debug validation layer -- ------------------------------------------------

# if defined(_DEBUG) || !defined(NDEBUG)
    typedef VkResult (APIENTRY* __vk_CreateDebugUtilsMessengerEXT)(VkInstance,const VkDebugUtilsMessengerCreateInfoEXT*,
                                                                   const VkAllocationCallbacks*,VkDebugUtilsMessengerEXT*);
    typedef VkResult (APIENTRY* __vk_DestroyDebugUtilsMessengerEXT)(VkInstance,VkDebugUtilsMessengerEXT,const VkAllocationCallbacks*);
    static const char* __sdkDebugLayerName() noexcept { return "VK_LAYER_KHRONOS_validation"; }

    // ---

    static constexpr inline const char* __toSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT severity) noexcept {
      return (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
           ? "ERROR"
           : ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARNING" : "INFO");
    }
    static constexpr inline const char* __toMessageTypeString(VkDebugUtilsMessageTypeFlagsEXT type) noexcept {
      return (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
           ? "PERFORMANCE"
           : ((type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) ? "VALIDATION" : "GENERAL");
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL __debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                        void*) {
      if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        fprintf(stderr, "[VULKAN] %s %s: %s\n", __toMessageTypeString(type), __toSeverityString(severity), callbackData->pMessage);
      return VK_FALSE;
    }

    // ---

    static void __createDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT& outMessenger) noexcept {
      VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
      memset(&messengerInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
      messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      messengerInfo.pfnUserCallback = __debugCallback;

      VulkanLoader& loader = VulkanLoader::instance();
      auto creator = (__vk_CreateDebugUtilsMessengerEXT)loader.getVulkanInstanceFunction(instance, "vkCreateDebugUtilsMessengerEXT");
      if (creator == nullptr || creator(instance, &messengerInfo, nullptr, &outMessenger) != VK_SUCCESS)
        outMessenger = VK_NULL_HANDLE;
    }

    static void __destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT& messenger) noexcept {
      if (messenger != VK_NULL_HANDLE) {
        VulkanLoader& loader = VulkanLoader::instance();
        auto destr = (__vk_DestroyDebugUtilsMessengerEXT)loader.getVulkanInstanceFunction(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (destr != nullptr) {
          destr(instance, messenger, nullptr);
          messenger = VK_NULL_HANDLE;
        }
      }
    }
# endif


// -- vulkan instance creation -- ----------------------------------------------

  // Create Vulkan instance for Renderer object(s)
  std::shared_ptr<VulkanInstance> VulkanInstance::create(const char* appName, uint32_t appVersion,
                                                         uint32_t featureLevel, const char** instanceAdditionalExts,
                                                         size_t additionalExtCount) { // throws
    if (!VulkanLoader::isSupported())
      throw std::runtime_error("Vulkan not supported on current system");
    VulkanLoader& loader = VulkanLoader::instance();

    // application identification
    VkApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName;
    appInfo.applicationVersion = appVersion;
    appInfo.apiVersion = featureLevel;
    appInfo.pEngineName = "Pandora Toolbox";
    appInfo.engineVersion = VK_MAKE_VERSION(__P_PANDORA_VIDEO_LIB_VERSION_MAJOR, __P_PANDORA_VIDEO_LIB_VERSION_MINOR,
                                            __P_PANDORA_VIDEO_LIB_VERSION_PATCH);

    // set list of extensions to enable:
    // -> surface + platform-specific surface + supported display/device info + additional user extensions
    const char* instanceBaseExt[] = {
      "VK_KHR_surface",
      loader.getPlatformSurfaceExtensionId(),
      // below: no longer needed in V1.1+
      "VK_KHR_get_physical_device_properties2" // part of vulkan core 1.1+
    };
    size_t baseExtCount = sizeof(instanceBaseExt)/sizeof(*instanceBaseExt);
    if (featureLevel >= VK_API_VERSION_1_1) { // V1.1+: remove extensions not needed anymore
      baseExtCount -= 1;
    }
    if (instanceAdditionalExts == nullptr)
      additionalExtCount = 0;

    size_t enabledExtCount = baseExtCount + additionalExtCount;
    if (loader.vk.isKhrDisplaySupported)
      ++enabledExtCount;

    auto enabledExtensions = std::unique_ptr<const char*[]>(new const char*[enabledExtCount]);
    memcpy(enabledExtensions.get(), instanceBaseExt, baseExtCount*sizeof(*instanceBaseExt));
    if (additionalExtCount != 0)
      memcpy(&enabledExtensions[baseExtCount], instanceAdditionalExts, additionalExtCount*sizeof(*instanceAdditionalExts));
    if (loader.vk.isKhrDisplaySupported)
      enabledExtensions[enabledExtCount - 1] = "VK_KHR_display";

    // create vulkan instance
    VkInstanceCreateInfo instanceInfo;
    memset(&instanceInfo, 0, sizeof(instanceInfo));
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.ppEnabledExtensionNames = enabledExtensions.get();
    instanceInfo.enabledExtensionCount = (uint32_t)enabledExtCount;

#   if defined(_DEBUG) || !defined(NDEBUG)
      try {
        const char* debugLayer = __sdkDebugLayerName();
        if (loader.findLayer(debugLayer)) {
          instanceInfo.ppEnabledLayerNames = &debugLayer;
          instanceInfo.enabledLayerCount = 1;
        }
      }
      catch (...) {}
#   endif
    return std::shared_ptr<VulkanInstance>(new VulkanInstance(instanceInfo, featureLevel)); // throws -- no make_shared due to private constructor
  }

  // ---

  // Create Vulkan instance container
  VulkanInstance::VulkanInstance(const VkInstanceCreateInfo& instanceInfo, uint32_t featureLevel) // throws
    : _featureLevel(featureLevel) {
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &(this->_instance));
    if (result != VK_SUCCESS)
      throwError(result, "Vulkan instance creation failed");

#   if defined(_DEBUG) || !defined(NDEBUG)
      __createDebugMessenger(this->_instance, this->_messenger);
#   endif
  }
  // Destroy Vulkan instance container
  VulkanInstance::~VulkanInstance() noexcept {
    if (this->_instance != VK_NULL_HANDLE) {
#     if defined(_DEBUG) || !defined(NDEBUG)
        __destroyDebugMessenger(this->_instance, this->_messenger);
#     endif
      vkDestroyInstance(this->_instance, nullptr);
    }
  }


// -----------------------------------------------------------------------------
// _vulkan_resource.h -- error messages
// -----------------------------------------------------------------------------

  // Exception class with LightString
  class RuntimeException final : public std::runtime_error {
  public:
    RuntimeException(std::shared_ptr<pandora::memory::LightString>&& msg) noexcept : std::runtime_error(msg->c_str()), _message(std::move(msg)) {}
    RuntimeException(const RuntimeException& rhs) noexcept : std::runtime_error(rhs._message->c_str()), _message(rhs._message) {}
    RuntimeException(RuntimeException&& rhs) noexcept : std::runtime_error(rhs._message->c_str()), _message(rhs._message) {}
    RuntimeException& operator=(const RuntimeException& rhs) noexcept { std::runtime_error::operator=(rhs); _message = rhs._message; return *this; }
    virtual ~RuntimeException() noexcept { _message.reset(); }
  private:
    std::shared_ptr<pandora::memory::LightString> _message;
  };
  
  // ---
  
  static inline const char* __getVulkanError(VkResult result) {
    switch (result) {
      case VK_NOT_READY: return "NOT_READY";
      case VK_TIMEOUT: return "TIMEOUT";
      case VK_EVENT_SET: return "EVENT_SET";
      case VK_EVENT_RESET: return "EVENT_RESET";
      case VK_INCOMPLETE: return "INCOMPLETE";
      case VK_ERROR_OUT_OF_HOST_MEMORY: return "OUT_OF_HOST_MEMORY";
      case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "OUT_OF_DEVICE_MEMORY";
      case VK_ERROR_INITIALIZATION_FAILED: return "INITIALIZATION_FAILED";
      case VK_ERROR_DEVICE_LOST: return "DEVICE_LOST";
      case VK_ERROR_MEMORY_MAP_FAILED: return "MEMORY_MAP_FAILED";
      case VK_ERROR_LAYER_NOT_PRESENT: return "LAYER_NOT_PRESENT";
      case VK_ERROR_EXTENSION_NOT_PRESENT: return "EXTENSION_NOT_PRESENT";
      case VK_ERROR_FEATURE_NOT_PRESENT: return "FEATURE_NOT_PRESENT";
      case VK_ERROR_INCOMPATIBLE_DRIVER: return "INCOMPATIBLE_DRIVER";
      case VK_ERROR_TOO_MANY_OBJECTS: return "TOO_MANY_OBJECTS";
      case VK_ERROR_FORMAT_NOT_SUPPORTED: return "FORMAT_NOT_SUPPORTED";
      case VK_ERROR_SURFACE_LOST_KHR: return "SURFACE_LOST_KHR";
      case VK_ERROR_OUT_OF_DATE_KHR: return "OUT_OF_DATE_KHR";
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "INCOMPATIBLE_DISPLAY_KHR";
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "NATIVE_WINDOW_IN_USE_KHR"; // window already used by a surface or another API
      case VK_ERROR_VALIDATION_FAILED_EXT: return "VALIDATION_FAILED_EXT";
      case VK_SUBOPTIMAL_KHR: return "SUBOPTIMAL_KHR"; // swapchain doesn't match surface properties exactly, but can still be used
      default: return "INTERNAL_ERROR";
    }
  }

  void pandora::video::vulkan::throwError(VkResult result, const char* messageContent) {
    const char* vkError = __getVulkanError(result);

    // pre-compute total size to avoid having multiple dynamic allocs
    size_t prefixSize = strlen(messageContent);
    size_t errorSize = strlen(vkError);
    auto message = std::make_shared<pandora::memory::LightString>(prefixSize + 2u + errorSize);
    
    // copy message in preallocated string
    if (!message->empty()) { // if no alloc failure
      memcpy((void*)message->data(),                 messageContent, prefixSize*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize]),    ": ",         size_t{2u}*sizeof(char));
      memcpy((void*)&(message->data()[prefixSize+2u]), vkError,      errorSize *sizeof(char));
    }
    throw RuntimeException(std::move(message));
  }

# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
