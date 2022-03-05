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
# ifdef _WINDOWS
#   ifndef __MINGW32__
#     pragma warning(push)
#     pragma warning(disable: 4100)  // disable warnings about unused params
#     pragma warning(disable: 6387)  // disable warnings about legacy file readers
#     pragma warning(disable: 26812) // disable warnings about vulkan unscoped enums
#     pragma warning(disable: 4701)  // disable init warnings about glm memory
#     pragma warning(disable: 6001)  // disable init warnings about glm memory
#     pragma warning(disable: 26451) // disable init warnings about vulkan and glm types
#     pragma warning(disable: 26495) // disable init warnings about vulkan and glm types
#   endif
#   ifndef NOMINMAX
#     define NOMINMAX            // no min/max macros
#     define WIN32_LEAN_AND_MEAN // exclude rare MFC libraries
#   endif
# endif
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <cerrno>
# include <cmath>
# include <stdexcept>
# include <memory/light_string.h>
# define GLM_FORCE_DEPTH_ZERO_TO_ONE // force Vulkan depth range (0.0 to 1.0) instead of OpenGL
# include <glm/glm.hpp>
# include <glm/gtc/color_space.hpp>

# include "video/window_resource.h"
# include "video/vulkan/api/vulkan_loader.h"
# include "video/vulkan/api/_private/_dynamic_array.h"
# include "video/vulkan/api/_private/_glslang_utils.h"
# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(default: 4701)  // restore init warnings after including glm
#   pragma warning(default: 6001)  // restore init warnings after including glm
#   pragma warning(default: 26451) // restore init warnings after including vulkan and glm
#   pragma warning(default: 26495) // restore init warnings after including vulkan and glm
# endif

# include "video/vulkan/_private/_error.h"
# include "video/vulkan/_private/_shared_resource.h"
# include "video/vulkan/renderer.h"
# include "video/vulkan/swap_chain.h"
# include "video/vulkan/shader.h"
# include "video/vulkan/graphics_pipeline.h"
# include "video/vulkan/depth_stencil_buffer.h"

# include "video/vulkan/_private/_resource_io.h"
# include "video/vulkan/buffer.h"
// # include "video/vulkan/texture.h"
// # include "video/vulkan/texture_reader.h"
// # include "video/vulkan/texture_writer.h"
// # include "video/vulkan/camera_utils.h"
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
      VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
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
    VkApplicationInfo appInfo{};
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

    auto enabledExtensions = DynamicArray<const char*>(enabledExtCount);
    memcpy(enabledExtensions.value, instanceBaseExt, baseExtCount*sizeof(*instanceBaseExt));
    if (additionalExtCount != 0)
      memcpy(&enabledExtensions.value[baseExtCount], instanceAdditionalExts, additionalExtCount*sizeof(*instanceAdditionalExts));
    if (loader.vk.isKhrDisplaySupported)
      enabledExtensions.value[enabledExtCount - 1] = "VK_KHR_display";

    // create vulkan instance
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.ppEnabledExtensionNames = enabledExtensions.value;
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
    VkPhysicalDeviceFeatures defaultFeatures{};
    defaultFeatures.alphaToOne = defaultFeatures.depthBiasClamp = defaultFeatures.depthBounds = defaultFeatures.depthClamp
      = defaultFeatures.dualSrcBlend = defaultFeatures.fillModeNonSolid = defaultFeatures.imageCubeArray
      = defaultFeatures.independentBlend = defaultFeatures.multiDrawIndirect = defaultFeatures.multiViewport
      = defaultFeatures.samplerAnisotropy = defaultFeatures.shaderClipDistance = defaultFeatures.shaderCullDistance
      = defaultFeatures.shaderResourceMinLod = defaultFeatures.shaderStorageImageExtendedFormats
#     ifndef __P_DISABLE_GEOMETRY_STAGE
        = defaultFeatures.geometryShader
#     endif
#     ifndef __P_DISABLE_TESSELLATION_STAGE
        = defaultFeatures.tessellationShader
#     endif
      = VK_TRUE;
    return defaultFeatures;
  }

  // Extract features ('inOutAvailable') that are both requested by user ('requested') and available in device ('inOutAvailable')
  static void __getSupportedFeatures(const VkPhysicalDeviceFeatures& requested, VkPhysicalDeviceFeatures* inOutAvailable) noexcept {
    uint64_t* outEnd = ((uint64_t*)inOutAvailable) + sizeof(VkPhysicalDeviceFeatures)/sizeof(uint64_t); // raw iteration -> no missing feature if vulkan is updated
    for (uint64_t* out = (uint64_t*)inOutAvailable, *req = (uint64_t*)&requested; out < outEnd; ++out, ++req)
      *out = (*out & *req);

    __if_constexpr ((sizeof(VkPhysicalDeviceFeatures) & (sizeof(uint64_t) - 1)) != 0) { // not a multiple of 8 bytes -> copy last block
      char* out = ((char*)inOutAvailable) + sizeof(VkPhysicalDeviceFeatures) - sizeof(uint64_t);
      char* req = ((char*)&requested) + sizeof(VkPhysicalDeviceFeatures) - sizeof(uint64_t);
      *(uint64_t*)out = (*(uint64_t*)out & *(uint64_t*)req);
    }
  }

  // ---

  template <size_t _ArraySize>
  static inline DynamicArray<const char*> __getSupportedExtensions(const char* extensions[_ArraySize], uint32_t& outExtCount) {
    bool results[_ArraySize];
    outExtCount = (uint32_t)VulkanLoader::instance().findExtensions(extensions, _ArraySize, results);
    if (outExtCount == 0)
      return DynamicArray<const char*>{};
    
    DynamicArray<const char*> supportedExt(outExtCount);
    if (outExtCount == _ArraySize) {
      memcpy(supportedExt.value, extensions, _ArraySize*sizeof(*extensions));
    }
    else {
      bool* curResult = results;
      const char** endExt = extensions + _ArraySize;
      for (const char** curExt = extensions, **out = supportedExt.value; curExt < endExt; ++curExt, ++curResult) {
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
  static DynamicArray<const char*> __defaultDeviceExtensions(uint32_t& outExtCount, uint32_t featureLevel) {
    if (__P_VK_API_VERSION_NOVARIANT(featureLevel) >= __P_VK_API_VERSION_NOVARIANT(VK_API_VERSION_1_2)) {
      const char* extensions[] {
        "VK_EXT_extended_dynamic_state",
        "VK_EXT_depth_clip_enable",
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
        "VK_EXT_extended_dynamic_state",
        "VK_EXT_depth_clip_enable",
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
        "VK_EXT_extended_dynamic_state",
        "VK_EXT_depth_clip_enable",
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
        deviceScore += (deviceProperties.limits.maxImageDimension2D >= 8192)
                       ? 7168u + (deviceProperties.limits.maxImageDimension2D >> 3)
                       : deviceProperties.limits.maxImageDimension2D;
      if (deviceFeatures.geometryShader)
        deviceScore += 16384;
      if (deviceFeatures.tessellationShader)
        deviceScore += 2048;
      if (deviceFeatures.multiViewport)
        deviceScore += 512;
      
      uint32_t highestSampleCount = 64u;
      uint32_t sampleCountMask = ((uint32_t)deviceProperties.limits.framebufferColorSampleCounts
                                & (uint32_t)deviceProperties.limits.framebufferDepthSampleCounts);
      while ((sampleCountMask & highestSampleCount) == 0 && highestSampleCount > 1u)
        highestSampleCount >>= 1;
      deviceScore += (highestSampleCount << 4); // 16 (x1) to 1024 (x64)
      
      return deviceScore;
    }

    // Detect display monitor association with an adapter
    // warning: only supported if VK_KHR_display extension is present
    static inline bool __isDeviceConnectedToKhrDisplay(VkPhysicalDevice& device, const pandora::memory::LightString& monitorName,
                                                       const pandora::memory::LightString& monitorDesc) {
      uint32_t displayCount = 0;
      if (vkGetPhysicalDeviceDisplayPropertiesKHR(device, &displayCount, nullptr) == VK_SUCCESS && displayCount) {
        auto displays = DynamicArray<VkDisplayPropertiesKHR>(displayCount);
        VkResult result = vkGetPhysicalDeviceDisplayPropertiesKHR(device, &displayCount, displays.value);

        if (result == VK_SUCCESS || result == VK_INCOMPLETE) {
          for (const VkDisplayPropertiesKHR* it = displays.value; displayCount; --displayCount, ++it) {
            if (it->displayName != nullptr && (monitorName == it->displayName || monitorDesc == it->displayName))
              return true;
          }
        }
      }
      return false;
    }

    // ---

    struct _VkLinkedFamily final {
      uint32_t index = 0;
      uint32_t queueCount = 0;
      uint32_t score = 0;
      _VkLinkedFamily* next = nullptr;
    };

    // Find graphics command queue family for an adapter
    static inline bool __findGraphicsCommandQueues(VkPhysicalDevice device, bool useSparseBinding, size_t minQueueCount,
                                                   DynamicArray<uint32_t>& outQueueFamilyIndices) noexcept {
      // query queue families
      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
      if (queueFamilyCount == 0)
        return false;
      auto queueFamilies = DynamicArray<VkQueueFamilyProperties>(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.value);

      // find compatible families (if available) - ordered from best to worst
      DynamicArray<_VkLinkedFamily> buffers(queueFamilyCount);
      _VkLinkedFamily* orderedResults = nullptr;
      uint32_t orderedResultsLength = 0;

      for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        VkQueueFamilyProperties& family = queueFamilies.value[i];
        if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT) && family.queueCount >= (uint32_t)minQueueCount
        && (!useSparseBinding || (family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT))) {
          _VkLinkedFamily* entry = &(buffers.value[i]);
          entry->index = i;
          entry->queueCount = family.queueCount;
          entry->score = 0;

          if (family.queueFlags & VK_QUEUE_COMPUTE_BIT)
            entry->score += 2048;
          if (family.queueFlags & VK_QUEUE_TRANSFER_BIT)
            entry->score += 1024;
          if (family.queueFlags & VK_QUEUE_PROTECTED_BIT)
            entry->score += 256;
          if (family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
            entry->score += 256;

          _VkLinkedFamily** res = &orderedResults;
          while (*res != nullptr && (entry->score < (*res)->score || (entry->score == (*res)->score && family.queueCount <= (*res)->queueCount)) )
            res = &((*res)->next);
          entry->next = (*res != nullptr) ? (*res)->next : nullptr;
          *res = entry;
          ++orderedResultsLength;
        }
      }

      if (orderedResults != nullptr) {
        outQueueFamilyIndices = DynamicArray<uint32_t>(orderedResultsLength);
        uint32_t* out = &(outQueueFamilyIndices.value[0]);
        for (_VkLinkedFamily* it = orderedResults; it != nullptr; ++out, it = it->next)
          *out = it->index;
        return true;
      }
      return false;
    }

  // ---

  // Find primary hardware adapter (physical device)
  static VkPhysicalDevice __getHardwareAdapter(VkInstance instance, const VkPhysicalDeviceFeatures& requestedFeatures,
                                               const pandora::hardware::DisplayMonitor& monitor, uint32_t featureLevel,
                                               size_t minQueueCount, DynamicArray<uint32_t>& outQueueFamilyIndices) {
    VulkanLoader& loader = VulkanLoader::instance();

    // query physical devices
    uint32_t physicalDeviceCount = 0;
    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS || physicalDeviceCount == 0)
      throw std::runtime_error("Vulkan: failed to find compatible GPUs");

    auto physicalDevices = DynamicArray<VkPhysicalDevice>(physicalDeviceCount);
    memset(physicalDevices.value, 0, physicalDeviceCount*sizeof(VkPhysicalDevice));
    VkResult queryResult = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.value);
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
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    uint32_t bestDeviceScore = 0;
    featureLevel = __P_VK_API_VERSION_NOVARIANT(featureLevel);

    for (VkPhysicalDevice* it = physicalDevices.value; physicalDeviceCount; --physicalDeviceCount, ++it) {
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
        &&  __findGraphicsCommandQueues(*it, (deviceFeatures.sparseBinding && requestedFeatures.sparseBinding),
                                        minQueueCount, outQueueFamilyIndices)) {
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
                     const DeviceExtensions& extensions, size_t commandQueueCount)
    : _instance((instance != nullptr) ? std::move(instance) : VulkanInstance::create()) { // throws
    // find hardware adapter for monitor
    if (commandQueueCount < 1u)
      commandQueueCount = 1u;
    DynamicArray<uint32_t> cmdQueueFamilyIndices;
    auto physicalDevice = __getHardwareAdapter(this->_instance->vkInstance(), requestedFeatures, monitor,
                                               this->_instance->featureLevel(), commandQueueCount, cmdQueueFamilyIndices); // throws
    if (physicalDevice == VK_NULL_HANDLE)
      throw std::runtime_error("Vulkan: failed to find compatible GPU");

    // feature support detection
    this->_features = std::make_unique<VkPhysicalDeviceFeatures>();
    if (areFeaturesRequired)
      memcpy(this->_features.get(), &requestedFeatures, sizeof(VkPhysicalDeviceFeatures));
    else {
      memset(this->_features.get(), 0, sizeof(VkPhysicalDeviceFeatures));
      vkGetPhysicalDeviceFeatures(physicalDevice, this->_features.get());
      __getSupportedFeatures(requestedFeatures, this->_features.get());
    }

    this->_physicalDeviceInfo = std::make_unique<VkPhysicalDeviceProperties>();
    memset(this->_physicalDeviceInfo.get(), 0, sizeof(VkPhysicalDeviceProperties));
    vkGetPhysicalDeviceProperties(physicalDevice, this->_physicalDeviceInfo.get());

    // command queue params
    DynamicArray<VkDeviceQueueCreateInfo> cmdQueueInfos(cmdQueueFamilyIndices.length());
    memset(cmdQueueInfos.value, 0, cmdQueueFamilyIndices.length()*sizeof(VkDeviceQueueCreateInfo));
    DynamicArray<float> queuePriorities(commandQueueCount);
    for (size_t i = 0; i < commandQueueCount; ++i)
      queuePriorities.value[i] = 1.0f;

    for (size_t i = 0; i < cmdQueueFamilyIndices.length(); ++i) {
      VkDeviceQueueCreateInfo& cmdQueueInfo = cmdQueueInfos.value[i];
      cmdQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      cmdQueueInfo.queueFamilyIndex = cmdQueueFamilyIndices.value[i];
      cmdQueueInfo.queueCount = (uint32_t)commandQueueCount;
      cmdQueueInfo.pQueuePriorities = queuePriorities.value;
    }

    // create rendering device - command queues
    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pQueueCreateInfos = cmdQueueInfos.value;
    deviceInfo.queueCreateInfoCount = (uint32_t)cmdQueueInfos.length();
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

    // create rendering device - extensions
    DynamicArray<const char*> defaultExt;
    if (extensions.deviceExtensions != nullptr && extensions.extensionCount != 0) {
      deviceInfo.ppEnabledExtensionNames = extensions.deviceExtensions;
      deviceInfo.enabledExtensionCount = (uint32_t)extensions.extensionCount;
    }
    else {
      defaultExt = __defaultDeviceExtensions(deviceInfo.enabledExtensionCount, this->_instance->featureLevel());
      deviceInfo.ppEnabledExtensionNames = defaultExt.value;
    }
    for (uint32_t i = 0; i < deviceInfo.enabledExtensionCount; ++i)
      this->_deviceExtensions.emplace(deviceInfo.ppEnabledExtensionNames[i]);
    
    _isDynamicRenderingSupported = false;
#   if defined(VK_HEADER_VERSION) && VK_HEADER_VERSION >= 197
      VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingInfo{};
      if (extensions.allowDynamicRendering) {
        if (__P_VK_API_VERSION_NOVARIANT(this->_instance->featureLevel()) > __P_VK_API_VERSION_NOVARIANT(VK_API_VERSION_1_2)
        || this->_deviceExtensions.find("VK_KHR_dynamic_rendering") != this->_deviceExtensions.end()) {
          dynamicRenderingInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
          dynamicRenderingInfo.dynamicRendering = VK_TRUE;
          deviceInfo.pNext = &dynamicRenderingInfo;
          _isDynamicRenderingSupported = true;
        }
      }
#   endif
    _isExtendedDynamicStateSupported = (__P_VK_API_VERSION_NOVARIANT(this->_instance->featureLevel()) > __P_VK_API_VERSION_NOVARIANT(VK_API_VERSION_1_2)
                                     || this->_deviceExtensions.find("VK_EXT_extended_dynamic_state") != this->_deviceExtensions.end());

    // create logical rendering device (context)
    VkDevice deviceContextHandle = VK_NULL_HANDLE;
    VkResult result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &deviceContextHandle);
    if (result != VK_SUCCESS || deviceContextHandle == VK_NULL_HANDLE)
      throwError(result, "Vulkan: failed to create logical device");
    this->_deviceContext = ScopedDeviceContext(deviceContextHandle, physicalDevice);

    // get command queue handles
    DynamicArray<CommandQueues> graphicsQueuesPerFamily(cmdQueueFamilyIndices.length());
    for (size_t i = 0; i < cmdQueueFamilyIndices.length(); ++i) {
      CommandQueues& family = graphicsQueuesPerFamily.value[i];
      family.familyIndex = cmdQueueFamilyIndices.value[i];
      family.commandQueues = DynamicArray<VkQueue>(commandQueueCount);
      for (size_t queue = 0; queue < commandQueueCount; ++queue) {
        VkQueue* target = &(family.commandQueues.value[queue]);
        vkGetDeviceQueue(deviceContextHandle, family.familyIndex, (uint32_t)queue, target);
        if (*target == VK_NULL_HANDLE)
          throw std::runtime_error("Vulkan: failed to obtain command queue access");
      }
    }
    this->_deviceContext._setGraphicsQueues(std::move(graphicsQueuesPerFamily));
    
    // create command pool for short-lived operations (copies...)
    VkCommandPool transientCommandPool = VK_NULL_HANDLE;
    uint32_t transientCommandQueueFamily = cmdQueueFamilyIndices.value[0];//TODO: choose queue with transfer capability (if possible, also graphics caps)
    VkCommandPoolCreateInfo transientPoolInfo{};
    transientPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transientPoolInfo.queueFamilyIndex = transientCommandQueueFamily;
    transientPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    result = vkCreateCommandPool(deviceContextHandle, &transientPoolInfo, nullptr, &transientCommandPool);
    if (result != VK_SUCCESS || transientCommandPool == VK_NULL_HANDLE)
      throwError(result, "Vulkan: transient command pool not created");
    this->_deviceContext._setTransientCommandPool(transientCommandPool, transientCommandQueueFamily);
  }


// -- device/context destruction/move -- ---------------------------------------

  // Destroy rendering context handle
  void ScopedDeviceContext::release() noexcept {
    if (this->_deviceContext != VK_NULL_HANDLE) {
      if (this->_transientCommandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(this->_deviceContext, this->_transientCommandPool, nullptr);
      this->_graphicsQueuesPerFamily.clear();
      
      vkDeviceWaitIdle(this->_deviceContext);
      try { vkDestroyDevice(this->_deviceContext, nullptr); } catch (...) {}
      this->_deviceContext = VK_NULL_HANDLE;
      this->_physicalDevice = VK_NULL_HANDLE;
    }
  }
  // Destroy device and context resources
  void Renderer::release() noexcept {
    if (this->_deviceContext.context() != VK_NULL_HANDLE) {
      this->_attachedPipeline = nullptr; // unbind
      flush();

      this->_deviceContext.release();
      this->_features = nullptr;
      this->_physicalDeviceInfo = nullptr;
    }
    this->_instance.reset();
  }

  Renderer::Renderer(Renderer&& rhs) noexcept
    : _deviceContext(std::move(rhs._deviceContext)),
      _instance(std::move(rhs._instance)),
      _deviceExtensions(std::move(rhs._deviceExtensions)),
      _features(std::move(rhs._features)),
      _physicalDeviceInfo(std::move(rhs._physicalDeviceInfo)),
      _isDynamicRenderingSupported(rhs._isDynamicRenderingSupported),
      _isExtendedDynamicStateSupported(rhs._isExtendedDynamicStateSupported),
      _attachedPipeline(rhs._attachedPipeline) {
    rhs._instance = nullptr;
    rhs._features = nullptr;
    rhs._physicalDeviceInfo = nullptr;
  }
  Renderer& Renderer::operator=(Renderer&& rhs) noexcept {
    release();
    this->_deviceContext = std::move(rhs._deviceContext);
    this->_instance = std::move(rhs._instance);
    this->_deviceExtensions = std::move(rhs._deviceExtensions);
    this->_features = std::move(rhs._features);
    this->_physicalDeviceInfo = std::move(rhs._physicalDeviceInfo);
    this->_isDynamicRenderingSupported = rhs._isDynamicRenderingSupported;
    this->_isExtendedDynamicStateSupported = rhs._isExtendedDynamicStateSupported;
    this->_attachedPipeline = rhs._attachedPipeline;
    rhs._instance = nullptr;
    rhs._features = nullptr;
    rhs._physicalDeviceInfo = nullptr;
    return *this;
  }


// -- accessors -- -------------------------------------------------------------
  
  // Read device adapter VRAM size
  bool Renderer::getAdapterVramSize(size_t& outDedicatedRam, size_t& outSharedRam) const noexcept {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    memoryProperties.memoryHeapCount = 0;
    vkGetPhysicalDeviceMemoryProperties((VkPhysicalDevice)_deviceContext.device(), &memoryProperties);
    
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

  // ---
  
  // Find first color format supported (from a list of candidates, ordered from best to worst)
  DataFormat Renderer::findSupportedDataFormat(const DataFormat* candidates, size_t count,
                                               FormatAttachment attachmentType, VkImageTiling tiling) const noexcept {
    for (const DataFormat* endIt = candidates + (intptr_t)count; candidates < endIt; ++candidates) {
      VkFormatProperties formatProps{};
      vkGetPhysicalDeviceFormatProperties((VkPhysicalDevice)_deviceContext.device(),
                                          _getDataFormatComponents(*candidates), &formatProps);
      
      if (tiling == VK_IMAGE_TILING_OPTIMAL) {
        if (formatProps.optimalTilingFeatures & (VkFormatFeatureFlags)attachmentType)
          return *candidates;
      }
      else if (formatProps.linearTilingFeatures & (VkFormatFeatureFlags)attachmentType)
        return *candidates;
    }
    return DataFormat::unknown;
  }
  // Find first depth/stencil format supported (from a list of candidates, ordered from best to worst)
  DepthStencilFormat Renderer::findSupportedDepthStencilFormat(const DepthStencilFormat* candidates,
                                                               size_t count, VkImageTiling tiling) const noexcept {
    for (const DepthStencilFormat* endIt = candidates + (intptr_t)count; candidates < endIt; ++candidates) {
      VkFormatProperties formatProps{};
      vkGetPhysicalDeviceFormatProperties((VkPhysicalDevice)_deviceContext.device(), (VkFormat)*candidates, &formatProps);
      
      if (tiling == VK_IMAGE_TILING_OPTIMAL) {
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
          return *candidates;
      }
      else if (formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        return *candidates;
    }
    return (DepthStencilFormat)0;
  }
  
  // Convert standard sRGB(A) color to gamma-correct linear RGB(A)
  void Renderer::sRgbToGammaCorrectColor(const float colorRgba[4], ColorChannel outRgba[4]) noexcept {
    glm::vec3 gammaCorrect = glm::convertSRGBToLinear(glm::vec3(colorRgba[0], colorRgba[1], colorRgba[2]));
    outRgba[0] = gammaCorrect.r;
    outRgba[1] = gammaCorrect.g;
    outRgba[2] = gammaCorrect.b;
    outRgba[3] = colorRgba[3];
  }

  // ---

  // Flush command buffers
  void Renderer::flush() noexcept {
    /*if (commandBuffer == VK_NULL_HANDLE || commandBufferMode != recording)
      return;
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      vkResetCommandBuffer(commandBuffer, 0);
      return;
    }

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;

    VkFence fence;
    if (vkCreateFence(this->_logicalDeviceContextCopy, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
      return;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    if (vkQueueSubmit(this->_graphicsCommandQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
      vkDestroyFence(this->_logicalDeviceContextCopy, fence, nullptr);
      return;
    }

    vkWaitForFences(this->_logicalDeviceContextCopy, 1, &fence, true, 10000000000);
    vkResetFences(this->_logicalDeviceContextCopy, 1, &fence);
    vkDestroyFence(this->_logicalDeviceContextCopy, fence, nullptr);*/
  }


// -- render target operations -------------------------------------------------

  // Replace rasterizer viewport(s) (3D -> 2D projection rectangle(s)) -- multi-viewport support
  void Renderer::setViewports(const Viewport* viewports, size_t numberViewports) noexcept {
    if (numberViewports > maxViewports())
      numberViewports = maxViewports();
    
    __if_constexpr (sizeof(Viewport) == sizeof(VkViewport)) {
      //vkCmdSetViewport(<CMDQUEUE...>, 0, numberViewports, (const VkViewport*)viewports);
    }
    else { // realign data (only for weird compilers where alignment differs)
      auto data = DynamicArray<VkViewport>(numberViewports);
      if (numberViewports) {
        VkViewport* out = &(data.value[numberViewports - 1u]);
        for (const Viewport* it = &viewports[numberViewports - 1u]; it >= viewports; --it, --out)
          memcpy((void*)out, (void*)it, sizeof(VkViewport));
      }
      //vkCmdSetViewport(<CMDQUEUE...>, 0, numberViewports, data.get());
    }
  }
  // Replace rasterizer viewport (3D -> 2D projection rectangle)
  void Renderer::setViewport(const Viewport& viewports) noexcept {
    //vkCmdSetViewport(<CMDQUEUE...>, 0, 1, (const VkViewport*)&viewport);
  }

  // Set rasterizer scissor-test rectangle(s)
  void Renderer::setScissorRectangles(const ScissorRectangle* rectangles, size_t numberRectangles) noexcept {
    if (numberRectangles > maxViewports())
      numberRectangles = maxViewports();

    __if_constexpr (sizeof(ScissorRectangle) == sizeof(VkRect2D)) {
      //vkCmdSetScissor(<CMDQUEUE...>, 0, numberRectangles, (const VkRect2D*)rectangles);
    }
    else { // realign data (only for weird compilers where alignment differs)
      auto data = DynamicArray<VkRect2D>(numberRectangles);
      if (numberRectangles) {
        VkRect2D* out = &(data.value[numberRectangles - 1u]);
        for (const ScissorRectangle* it = &rectangles[numberRectangles - 1u]; it >= rectangles; --it, --out)
          memcpy((void*)out, (void*)it, sizeof(VkRect2D));
      }
      //vkCmdSetScissor(<CMDQUEUE...>, 0, numberRectangles, data.get());
    }
  }
  // Set rasterizer scissor-test rectangle
  void Renderer::setScissorRectangle(const ScissorRectangle& rectangle) noexcept {
    //vkCmdSetScissor(<CMDQUEUE...>, 0, 1, (const VkRect2D*)&rectangle);
  }


// -----------------------------------------------------------------------------
// Include hpp implementations
// -----------------------------------------------------------------------------
# include "./swap_chain.hpp"
# include "./_error.hpp"
# include "./graphics_pipeline.hpp"
# include "./depth_stencil_buffer.hpp"
# include "./buffer.hpp"
// # include "./texture.hpp"
# include "./shader.hpp"
// # include "./camera_utils.hpp"
# undef __if_constexpr

# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
