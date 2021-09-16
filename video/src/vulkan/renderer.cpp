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
--------------------------------------------------------------------------------
Includes hpp implementations at the end of the file 
(grouped object improves compiler optimizations + greatly reduces executable size)
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

# include "video/window_resource.h"
# include "video/vulkan/api/vulkan_loader.h"
# include "video/vulkan/renderer.h"
# include "video/vulkan/_private/_vulkan_resource.h"
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

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
    if (featureLevel != VK_API_VERSION_1_0) // V1.1+: remove extensions not needed anymore
      baseExtCount -= 1;
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


// -- feature / extension support -- -------------------------------------------

  VkPhysicalDeviceFeatures Renderer::defaultFeatures() noexcept {
    VkPhysicalDeviceFeatures defaultFeatures;
    memset(&defaultFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    defaultFeatures.alphaToOne = defaultFeatures.depthBiasClamp = defaultFeatures.depthBounds = defaultFeatures.depthClamp
      = defaultFeatures.dualSrcBlend = defaultFeatures.fillModeNonSolid = defaultFeatures.geometryShader = defaultFeatures.imageCubeArray
      = defaultFeatures.independentBlend = defaultFeatures.multiDrawIndirect = defaultFeatures.multiViewport
      = defaultFeatures.samplerAnisotropy = defaultFeatures.shaderClipDistance = defaultFeatures.shaderCullDistance
      = defaultFeatures.shaderResourceMinLod = defaultFeatures.shaderStorageImageExtendedFormats = defaultFeatures.tessellationShader = VK_TRUE;
    return defaultFeatures;
  }

  // Extract features ('inOutAvailable') that are both requested by user ('requested') and available in device ('inOutAvailable')
  static void __getSupportedFeatures(const VkPhysicalDeviceFeatures& requested, VkPhysicalDeviceFeatures& inOutAvailable) noexcept {
    char* outEnd = ((char*)&inOutAvailable) + sizeof(VkPhysicalDeviceFeatures); // raw iteration -> no missing feature if vulkan is updated
    for (uint64_t* out = (uint64_t*)&inOutAvailable, *req = (uint64_t*)&requested; (char*)out < outEnd; ++out, ++req)
      *out = (*out & *req);

    __if_constexpr ((sizeof(VkPhysicalDeviceFeatures) & (sizeof(uint64_t) - 1)) != 0) { // not a multiple of 8 bytes -> copy last block
      char* out = outEnd - sizeof(uint64_t);
      char* req = ((char*)&requested) + sizeof(VkPhysicalDeviceFeatures) - sizeof(uint64_t);
      *(uint64_t*)out = (*(uint64_t*)out & *(uint64_t*)req);
    }
  }

  // ---

  template <size_t _ArraySize>
  static inline std::unique_ptr<const char*[]> __getSupportedExtensions(const char* extensions[_ArraySize], uint32_t& outExtCount) {
    bool results[_ArraySize];
    outExtCount = (uint32_t)VulkanLoader::instance().findExtensions(extensions, _ArraySize, results);
    if (outExtCount == 0)
      return nullptr;
    
    std::unique_ptr<const char*[]> supportedExt(new const char*[outExtCount]);
    if (outExtCount == _ArraySize) {
      memcpy(supportedExt.get(), extensions, _ArraySize*sizeof(*extensions));
    }
    else {
      bool* curResult = results;
      const char** endExt = extensions + _ArraySize;
      for (const char** curExt = extensions, **out = supportedExt.get(); curExt < endExt; ++curExt, ++curResult) {
        if (*curResult) {
          *out = *curExt;
          ++out;
        }
      }
    }
#   if !defined(_CPP_REVISION) || _CPP_REVISION != 14
      return supportedExt;
#   else
      return std::move(supportedExt);
#   endif
  }

  // List of default/standard device extensions (used if no custom list provided)
  static std::unique_ptr<const char*[]> __defaultDeviceExtensions(uint32_t& outExtCount, uint32_t featureLevel) {
    if (__P_VK_API_VERSION_NOVARIANT(featureLevel) >= __P_VK_API_VERSION_NOVARIANT(VK_API_VERSION_1_2)) {
      const char* extensions[] {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
      };
      return __getSupportedExtensions<sizeof(extensions)/sizeof(*extensions)>(extensions, outExtCount);
    }
    else if (featureLevel == VK_API_VERSION_1_1) {
      const char* extensions[] { // extensions promoted to core 1.2 + same as above
        "VK_KHR_8bit_storage",
        "VK_KHR_draw_indirect_count",
        "VK_KHR_shader_float16_int8",
        "VK_KHR_shader_float_controls",
        "VK_KHR_spirv_1_4", // not available in 1.0
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
      };
      return __getSupportedExtensions<sizeof(extensions)/sizeof(*extensions)>(extensions, outExtCount);
    }
    else { // 1.0
      const char* extensions[] { // extensions promoted to core 1.1 + same as above (when available)
        "VK_KHR_storage_buffer_storage_class",
        "VK_KHR_16bit_storage",
        "VK_KHR_bind_memory2",
        "VK_KHR_multiview",
        "VK_KHR_shader_draw_parameters",
        "VK_KHR_8bit_storage",
        "VK_KHR_draw_indirect_count",
        "VK_KHR_shader_float16_int8",
        "VK_KHR_shader_float_controls",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
      };
      return __getSupportedExtensions<sizeof(extensions)/sizeof(*extensions)>(extensions, outExtCount);
    }
  }


// -- display adapter detection -- ---------------------------------------------

# ifdef _WINDOWS
    // Convert system string (monitor ID, monitor description, adapter name...) to UTF-8
    static inline pandora::memory::LightString __convertSystemString(const pandora::memory::LightWString& source) {
      return WindowResource::systemStringToUtf8(source.c_str(), source.size());
    }
# endif

    // Rate quality of a physical device, based on available features
    static inline uint32_t __rateHardwareAdapter(VkPhysicalDeviceProperties deviceProperties,
                                                 VkPhysicalDeviceFeatures deviceFeatures) noexcept {
      uint32_t deviceScore = 0;
      switch (deviceProperties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   deviceScore += 2048; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: deviceScore += 1024; break;
        default: break;
      }
      if (deviceProperties.limits.maxImageDimension2D != 0xFFFFFFFFu)
        deviceScore += deviceProperties.limits.maxImageDimension2D;
      if (deviceFeatures.geometryShader)
        deviceScore += 16384;
      if (deviceFeatures.tessellationShader)
        deviceScore += 2048;
      if (deviceFeatures.multiViewport)
        deviceScore += 512;
      return deviceScore;
    }

    // Detect display monitor association with an adapter
    // warning: only supported if VK_KHR_display extension is present
    static inline bool __isDeviceConnectedToKhrDisplay(VkPhysicalDevice& device, const pandora::memory::LightString& monitorName,
                                                       const pandora::memory::LightString& monitorDesc) {
      uint32_t displayCount = 0;
      if (vkGetPhysicalDeviceDisplayPropertiesKHR(device, &displayCount, nullptr) == VK_SUCCESS && displayCount) {
        auto displays = std::unique_ptr<VkDisplayPropertiesKHR[]>(new VkDisplayPropertiesKHR[displayCount]);
        VkResult result = vkGetPhysicalDeviceDisplayPropertiesKHR(device, &displayCount, displays.get());

        if (result == VK_SUCCESS || result == VK_INCOMPLETE) {
          for (const VkDisplayPropertiesKHR* it = displays.get(); displayCount; --displayCount, ++it) {
            if (it->displayName != nullptr && (monitorName == it->displayName || monitorDesc == it->displayName))
              return true;
          }
        }
      }
      return false;
    }

    // Find graphics command queue family for an adapter
    static inline bool __findGraphicsCommandQueueFamily(VkPhysicalDevice device, bool useSparseBinding,
                                                        uint32_t& outFamilyIndex) noexcept {
      // query queue families
      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
      if (queueFamilyCount == 0)
        return false;
      auto queueFamilies = std::unique_ptr<VkQueueFamilyProperties[]>(new VkQueueFamilyProperties[queueFamilyCount]);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.get());

      // find best family (if available)
      uint32_t bestFamily = (uint32_t)-1;
      uint32_t bestFamilyQueueCount = 0;
      uint32_t bestFamilyScore = 0;

      for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        VkQueueFamilyProperties& family = queueFamilies[i];
        if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (!useSparseBinding || (family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT))) {
          uint32_t familyScore = 0;
          if (family.queueFlags & VK_QUEUE_COMPUTE_BIT)
            familyScore += 2048;
          if (family.queueFlags & VK_QUEUE_TRANSFER_BIT)
            familyScore += 1024;
          if (family.queueFlags & VK_QUEUE_PROTECTED_BIT)
            familyScore += 256;
          if (family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
            familyScore += 256;

          if (familyScore > bestFamilyScore || (familyScore == bestFamilyScore && family.queueCount > bestFamilyQueueCount)) {
            bestFamily = i;
            bestFamilyQueueCount = family.queueCount;
            bestFamilyScore = familyScore;
          }
        }
      }
      if (bestFamily != (uint32_t)-1) {
        outFamilyIndex = bestFamily;
        return true;
      }
      return false;
    }

  // ---

  // Find primary hardware adapter (physical device)
  static VkPhysicalDevice __getHardwareAdapter(VkInstance instance, const VkPhysicalDeviceFeatures& requestedFeatures,
                                               const pandora::hardware::DisplayMonitor& monitor, uint32_t featureLevel,
                                               uint32_t& outQueueFamilyIndex) {
    // query physical devices
    uint32_t physicalDeviceCount = 0;
    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS || physicalDeviceCount == 0)
      throw std::runtime_error("Vulkan: failed to find compatible GPUs");

    auto physicalDevices = std::unique_ptr<VkPhysicalDevice[]>(new VkPhysicalDevice[physicalDeviceCount]);
    memset(physicalDevices.get(), 0, physicalDeviceCount*sizeof(VkPhysicalDevice));
    VkResult queryResult = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.get());
    if (queryResult != VK_SUCCESS && queryResult != VK_INCOMPLETE)
      throw std::runtime_error("Vulkan: failed to query physical devices");

#   ifdef _WINDOWS
      pandora::memory::LightString adapterName = __convertSystemString(monitor.adapterName());
      pandora::memory::LightString monitorName = __convertSystemString(monitor.attributes().id);
      pandora::memory::LightString monitorDesc = __convertSystemString(monitor.attributes().description);
#   else
      const pandora::memory::LightString& adapterName = monitor.adapterName();
      const pandora::memory::LightString& monitorName = monitor.attributes().id;
      const pandora::memory::LightString& monitorDesc = monitor.attributes().description;
#   endif

    // find best physical device for monitor
    VulkanLoader& loader = VulkanLoader::instance();
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    uint32_t bestDeviceScore = 0;
    featureLevel = __P_VK_API_VERSION_NOVARIANT(featureLevel);

    for (VkPhysicalDevice* it = physicalDevices.get(); physicalDeviceCount; --physicalDeviceCount, ++it) {
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(*it, &deviceProperties);

      if (__P_VK_API_VERSION_NOVARIANT(deviceProperties.apiVersion) >= featureLevel) {
        VkPhysicalDeviceFeatures deviceFeatures{ VK_FALSE };
        vkGetPhysicalDeviceFeatures(*it, &deviceFeatures);
        uint32_t deviceScore = __rateHardwareAdapter(deviceProperties, deviceFeatures); // quality score

        // identify monitor (use target monitor as a priority):
        // - if monitor.adapterName is supported by window manager, use it (if not, use VK_KHR_display if available)
        // - deviceProperties.deviceName may be NULL or generic on some OS or drivers -> use VK_KHR_display as fallback
        if ((!adapterName.empty() && adapterName == deviceProperties.deviceName)
        ||  (loader.vk.isKhrDisplaySupported && __isDeviceConnectedToKhrDisplay(*it, monitorName, monitorDesc)) ) {
          deviceScore |= 0x40000000u;
        }

        // store current device if: highest score + has valid command queue family (+ store queue family index if best device)
        if (deviceScore > bestDeviceScore
        &&  __findGraphicsCommandQueueFamily(*it, (deviceFeatures.sparseBinding && requestedFeatures.sparseBinding), outQueueFamilyIndex)) {
          bestDevice = *it;
          bestDeviceScore = deviceScore;
        }
      }
    }
    return bestDevice;
  }


// -- device/context creation -- -----------------------------------------------

  // Create Vulkan instance and rendering device
  Renderer::Renderer(const pandora::hardware::DisplayMonitor& monitor, std::shared_ptr<VulkanInstance> instance,
                     const VkPhysicalDeviceFeatures& requestedFeatures, bool areFeaturesRequired,
                     const char** deviceExtensions, size_t extensionCount)
    : _instance((instance != nullptr) ? std::move(instance) : VulkanInstance::create()), // throws
      _physicalDevice(VK_NULL_HANDLE),
      _deviceContext(VK_NULL_HANDLE),
      _graphicsCommandQueue(VK_NULL_HANDLE) {
    // find hardware adapter for monitor
    this->_physicalDevice = __getHardwareAdapter(this->_instance->vkInstance(), requestedFeatures, monitor,
                                                 this->_instance->featureLevel(), this->_commandQueueFamilyIndex); // throws
    if (this->_physicalDevice == VK_NULL_HANDLE)
      throw std::runtime_error("Vulkan: failed to find compatible GPU");

    // feature support detection
    this->_features = std::make_unique<VkPhysicalDeviceFeatures>();
    if (areFeaturesRequired)
      memcpy(this->_features.get(), &requestedFeatures, sizeof(VkPhysicalDeviceFeatures));
    else {
      memset(this->_features.get(), 0, sizeof(VkPhysicalDeviceFeatures));
      vkGetPhysicalDeviceFeatures(this->_physicalDevice, this->_features.get());
      __getSupportedFeatures(requestedFeatures, *(this->_features));
    }

    this->_physicalDeviceInfo = std::make_unique<VkPhysicalDeviceProperties>();
    memset(this->_physicalDeviceInfo.get(), 0, sizeof(VkPhysicalDeviceProperties));
    vkGetPhysicalDeviceProperties(this->_physicalDevice, this->_physicalDeviceInfo.get());

    // create rendering device
    VkDeviceQueueCreateInfo cmdQueueInfo;
    memset(&cmdQueueInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    cmdQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    cmdQueueInfo.queueFamilyIndex = this->_commandQueueFamilyIndex;
    cmdQueueInfo.queueCount = 1;
    float queuePriority = 1.0f;
    cmdQueueInfo.pQueuePriorities = &queuePriority;
    
    VkDeviceCreateInfo deviceInfo;
    memset(&deviceInfo, 0, sizeof(VkDeviceCreateInfo));
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pQueueCreateInfos = &cmdQueueInfo;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pEnabledFeatures = this->_features.get();
#   if defined(_DEBUG) || !defined(NDEBUG)
      try {
        const char* debugLayer = __sdkDebugLayerName();
        if (VulkanLoader::instance().findLayer(debugLayer)) {
          deviceInfo.ppEnabledLayerNames = &debugLayer;
          deviceInfo.enabledLayerCount = 1;
        }
      }
      catch (...) {}
#   endif

    std::unique_ptr<const char* []> defaultExt = nullptr;
    if (deviceExtensions != nullptr && extensionCount != 0) {
      deviceInfo.ppEnabledExtensionNames = deviceExtensions;
      deviceInfo.enabledExtensionCount = (uint32_t)extensionCount;
    }
    else {
      defaultExt = __defaultDeviceExtensions(deviceInfo.enabledExtensionCount, this->_instance->featureLevel());
      deviceInfo.ppEnabledExtensionNames = defaultExt.get();
    }

    VkResult result = vkCreateDevice(this->_physicalDevice, &deviceInfo, nullptr, &(this->_deviceContext));
    if (result != VK_SUCCESS)
      throwError(result, "Vulkan: failed to create logical device");
    vkGetDeviceQueue(this->_deviceContext, this->_commandQueueFamilyIndex, 0, &(this->_graphicsCommandQueue));
  }


// -- device/context destruction/move -- ---------------------------------------

  // Destroy device and context resources
  void Renderer::_destroy() noexcept {
    if (this->_deviceContext != VK_NULL_HANDLE) {
      vkDestroyDevice(this->_deviceContext, nullptr);
      this->_features = nullptr;
      this->_physicalDeviceInfo = nullptr;
      this->_graphicsCommandQueue = VK_NULL_HANDLE;
      this->_deviceContext = VK_NULL_HANDLE;
      this->_physicalDevice = VK_NULL_HANDLE;
    }
    this->_instance.reset();
  }

  Renderer::Renderer(Renderer&& rhs) noexcept
    : _instance(std::move(rhs._instance)),
      _features(std::move(rhs._features)),
      _physicalDeviceInfo(std::move(rhs._physicalDeviceInfo)),
      _physicalDevice(rhs._physicalDevice),
      _deviceContext(rhs._deviceContext),
      _graphicsCommandQueue(rhs._graphicsCommandQueue),
      _commandQueueFamilyIndex(rhs._commandQueueFamilyIndex) {
    rhs._instance = nullptr;
    rhs._features = nullptr;
    rhs._physicalDeviceInfo = nullptr;
    rhs._physicalDevice = VK_NULL_HANDLE;
    rhs._deviceContext = VK_NULL_HANDLE;
    rhs._graphicsCommandQueue = VK_NULL_HANDLE;
  }
  Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    _destroy();
    this->_instance = std::move(rhs._instance);
    this->_features = std::move(rhs._features);
    this->_physicalDeviceInfo = std::move(rhs._physicalDeviceInfo);
    this->_physicalDevice = rhs._physicalDevice;
    this->_deviceContext = rhs._deviceContext;
    this->_graphicsCommandQueue = rhs._graphicsCommandQueue;
    this->_commandQueueFamilyIndex = rhs._commandQueueFamilyIndex;

    rhs._instance = nullptr;
    rhs._features = nullptr;
    rhs._physicalDeviceInfo = nullptr;
    rhs._physicalDevice = VK_NULL_HANDLE;
    rhs._deviceContext = VK_NULL_HANDLE;
    rhs._graphicsCommandQueue = VK_NULL_HANDLE;
    return *this;
  }


// -- accessors -- -------------------------------------------------------------
  
  // Read device adapter VRAM size
  bool Renderer::getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    memoryProperties.memoryHeapCount = 0;
    vkGetPhysicalDeviceMemoryProperties(this->_physicalDevice, &memoryProperties);
    
    VkDeviceSize localByteSize = 0, sharedByteSize = 0;
    uint32_t remainingHeaps = memoryProperties.memoryHeapCount;
    for (const VkMemoryHeap* it = memoryProperties.memoryHeaps; remainingHeaps; --remainingHeaps, ++it) {
      if (it->flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        localByteSize += it->size;
      else
        sharedByteSize += it->size;
    }

    if (this->_physicalDeviceInfo->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      outDedicatedRam = (size_t)localByteSize;
      outSharedRam = (size_t)sharedByteSize;
    }
    else {
      outDedicatedRam = 0;
      outSharedRam = (size_t)localByteSize + (size_t)sharedByteSize;
    }
    return (memoryProperties.memoryHeapCount != 0);
  }
  
  // Convert standard sRGB(A) color to device RGB(A)
  void Renderer::toGammaCorrectColor(const float colorRgba[4], ColorChannel outRgba[4]) noexcept {
    //...
  }

  // Flush command buffers
  void Renderer::flush() noexcept {
    //...
  }


// -- feature support -- -------------------------------------------------------

  // Verify if HDR functionalities are supported on current system
  bool Renderer::isHdrAvailable() const noexcept {
    return (this->_instance->featureLevel() != VK_API_VERSION_1_0 && this->_features->shaderStorageImageExtendedFormats);
  }

  // Verify if a display monitor can display HDR colors
  bool Renderer::isMonitorHdrCapable(const pandora::hardware::DisplayMonitor& target, pandora::video::WindowHandle window) const noexcept {
    if (window == (pandora::video::WindowHandle)0)
      return false;

    VkSurfaceKHR screenSurface = VK_NULL_HANDLE;
    try {
      if (VulkanLoader::instance().createWindowSurface(this->_instance->vkInstance(), window, nullptr, screenSurface) != VK_SUCCESS)
        return false;
    }
    catch (...) { return false; }

    bool isHdrAvailable = false;
    try {
      uint32_t formatCount = 0;
      if (vkGetPhysicalDeviceSurfaceFormatsKHR(this->_physicalDevice, screenSurface, &formatCount, nullptr) == VK_SUCCESS && formatCount) {
        auto formats = std::unique_ptr<VkSurfaceFormatKHR[]>(new VkSurfaceFormatKHR[formatCount]);
        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(this->_physicalDevice, screenSurface, &formatCount, formats.get());
        if (result == VK_SUCCESS || result == VK_INCOMPLETE) {
          for (const VkSurfaceFormatKHR* it = formats.get(); !isHdrAvailable && formatCount; --formatCount, ++it) {
            switch (it->colorSpace) {
              case VK_COLOR_SPACE_HDR10_HLG_EXT:
              case VK_COLOR_SPACE_HDR10_ST2084_EXT:
              case VK_COLOR_SPACE_BT709_LINEAR_EXT:
              case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
              case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
              case VK_COLOR_SPACE_DOLBYVISION_EXT: isHdrAvailable = true; break;
              default: break;
            }
          }
        }
      }
    }
    catch (...) {}
    vkDestroySurfaceKHR(this->_instance->vkInstance(), screenSurface, nullptr);
    return isHdrAvailable;
  }
 

// -- render target operations -------------------------------------------------

  // Replace rasterizer viewport(s) (3D -> 2D projection rectangle(s)) -- multi-viewport support
  void Renderer::setViewports(const Viewport* viewports, size_t numberViewports) noexcept {
    if (numberViewports) {
      if (numberViewports > maxViewports())
        numberViewports = maxViewports();
    
      if (numberViewports <= 16) { // avoid huge cost of dynamic alloc if possible
        VkViewport values[16]{};
        VkViewport* out = &values[numberViewports - 1u];
        for (const Viewport* it = &viewports[numberViewports - 1u]; it >= viewports; --it, --out)
          memcpy((void*)out, (void*)it->descriptor(), sizeof(VkViewport));
        //vkCmdSetViewport(<CMDQUEUE...>, 0, numberViewports, values);
      }
      else {
        auto values = std::unique_ptr<VkViewport[]>(new VkViewport[numberViewports]);
        VkViewport* out = &values[numberViewports - 1u];
        for (const Viewport* it = &viewports[numberViewports - 1u]; it >= viewports; --it, --out)
          memcpy((void*)out, (void*)it->descriptor(), sizeof(VkViewport));
        //vkCmdSetViewport(<CMDQUEUE...>, 0, numberViewports, values.get());
      }
    }
  }
  // Replace rasterizer viewport (3D -> 2D projection rectangle)
  void Renderer::setViewport(const Viewport& viewports) noexcept {
    //vkCmdSetViewport(<CMDQUEUE...>, 0, 1, viewport.descriptor());
  }

  // Set rasterizer scissor-test rectangle(s)
  void Renderer::setScissorRectangles(const ScissorRectangle* rectangles, size_t numberRectangles) noexcept {
    if (numberRectangles) {
      if (numberRectangles > maxViewports())
        numberRectangles = maxViewports();

      if (numberRectangles <= 16) { // avoid huge cost of dynamic alloc if possible
        VkRect2D values[16]{};
        VkRect2D* out = &values[numberRectangles - 1u];
        for (const ScissorRectangle* it = &rectangles[numberRectangles - 1u]; it >= rectangles; --it, --out)
          memcpy((void*)out, (void*)it->descriptor(), sizeof(VkRect2D));
        //vkCmdSetScissor(<CMDQUEUE...>, 0, numberViewports, values);
      }
      else {
        auto values = std::unique_ptr<VkRect2D[]>(new VkRect2D[numberRectangles]);
        VkRect2D* out = &values[numberRectangles - 1u];
        for (const ScissorRectangle* it = &rectangles[numberRectangles - 1u]; it >= rectangles; --it, --out)
          memcpy((void*)out, (void*)it->descriptor(), sizeof(VkRect2D));
        //vkCmdSetScissor(<CMDQUEUE...>, 0, numberViewports, values.get());
      }
    }
  }
  // Set rasterizer scissor-test rectangle
  void setScissorRectangle(const ScissorRectangle& rectangle) noexcept {
    //vkCmdSetScissor(<CMDQUEUE...>, 0, 1, rectangle.descriptor());
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
      case VK_ERROR_FRAGMENTED_POOL: return "FRAGMENTED_POOL";
      case VK_ERROR_OUT_OF_POOL_MEMORY: return "OUT_OF_POOL_MEMORY";
      case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "INVALID_EXTERNAL_HANDLE";
      case VK_ERROR_FRAGMENTATION: return "FRAGMENTATION";
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
