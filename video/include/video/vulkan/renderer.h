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
# include <memory>
# include <hardware/display_monitor.h>
# include "./api/types.h"      // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        class SwapChain;

        /// @class VulkanInstance
        /// @brief Vulkan driver client instance, used to initialize Renderer objects
        class VulkanInstance final {
        public:
          VulkanInstance(const VulkanInstance&) = delete;
          VulkanInstance& operator=(const VulkanInstance&) = delete;
          ~VulkanInstance() noexcept; ///< Destroy Vulkan instance

          /// @brief Create Vulkan instance for Renderer object(s)
          /// @param appName                Application title (instance identification) or NULL.
          /// @param appVersion             Application version, built with VK_MAKE_VERSION(major, minor, patch).
          /// @param featureLevel           Feature level supported by the application (ex: VK_API_VERSION_1_2).
          /// @param instanceAdditionalExts Array of additional instance extension to enable (or NULL).
          ///                               Note that 'surface', 'platform-specific surface', 'get physical device properties2', 'display'
          ///                               and 'driver properties' extensions are already enabled if supported (you should NOT specify them in your array).
          ///                               Before using specific extensions, make sure they're supported (VulkanLoader::findExtensions).
          /// @param additionalExtCount     Array size for 'instanceAdditionalExts'.
          /// @throws - runtime_error: creation failure (vulkan not supported, missing extensions, feature level too high...).
          ///         - bad_alloc: allocation failure.
          static std::shared_ptr<VulkanInstance> create(const char* appName = nullptr,
                                                        uint32_t appVersion = VK_MAKE_VERSION(1,0,0),
                                                        uint32_t featureLevel = VK_API_VERSION_1_2,
                                                        const char** instanceAdditionalExts = nullptr,
                                                        size_t additionalExtCount = 0);

          inline VkInstance vkInstance() const noexcept { return this->_instance; }     ///< Get vulkan instance
          inline uint32_t featureLevel() const noexcept { return this->_featureLevel; } ///< Get instance API level

        private:
          VulkanInstance(const VkInstanceCreateInfo& instanceInfo, uint32_t featureLevel); // private: call 'VulkanInstance::create' instead

          VkInstance _instance = VK_NULL_HANDLE;
          uint32_t _featureLevel = VK_API_VERSION_1_2;
#         if defined(_DEBUG) || !defined(NDEBUG)
            VkDebugUtilsMessengerEXT _messenger = VK_NULL_HANDLE;
#         endif
        };

      }
    }
  }
#endif
