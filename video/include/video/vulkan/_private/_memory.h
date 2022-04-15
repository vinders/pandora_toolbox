/*******************************************************************************
MIT License
Copyright (c) 2022 Romain Vinders

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
# include "video/vulkan/api/types.h" // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        /// @brief Hardware memory heap properties
        class MemoryProps final {
        public:
          static constexpr inline uint32_t indexNotFound() noexcept { return (uint32_t)-1; }
        
          /// @brief Initialize device memory properties
          MemoryProps(VkPhysicalDevice device) noexcept;
          
          MemoryProps() = default; ///< Empty container
          MemoryProps(const MemoryProps&) = default;
          MemoryProps(MemoryProps&&) noexcept = default;
          MemoryProps& operator=(const MemoryProps&) = default;
          MemoryProps& operator=(MemoryProps&&) noexcept = default;
          ~MemoryProps() noexcept = default;
          
          // -- accessors --
          
          /// @brief Get memory heap properties (raw)
          inline const VkPhysicalDeviceMemoryProperties& props() const noexcept {
            return this->_memoryProps;
          }
          
          /// @brief Find suitable memory type for a specific resource usage
          /// @remarks Returns index of memory supporting:
          ///          * 'preferred|required' flags if available;
          ///          * otherwise only 'required' flags.
          /// @returns Memory type index (success) or MemoryProps::indexNotFound()
          uint32_t findMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags required,
                                       VkMemoryPropertyFlags preferred) const noexcept;
        
          /// @brief Get available memory property flags of a memory type
          inline VkMemoryPropertyFlags getPropertyFlags(uint32_t memoryTypeIndex) const noexcept {
            return _memoryProps.memoryTypes[memoryTypeIndex].propertyFlags;
          }
          
        private:
          VkPhysicalDeviceMemoryProperties _memoryProps{};
        };
        
        
        // -- memory allocation -- ---------------------------------------------
        
        // Verify if an individual buffer allocation should be dedicated (true) or use default driver mode
        static constexpr inline bool __isDedicatedBuffer(FeatureMode allocMode, VkBool32 isRequired,
                                                         VkBool32 isPreferred, VkDeviceSize bufferSize) noexcept {
          return (allocMode == FeatureMode::force
              || (allocMode == FeatureMode::autodetect
                 && (isRequired || (isPreferred && bufferSize >= (VkDeviceSize)32768)) ) );
        }
        void __throwMemoryTypeNotFound(); // Throw out_of_range exception if no memory type index could be found
        
        // ---
        
        // Individual memory allocation and binding for buffer
        // -> throws runtime_error on failure
        VkDeviceMemory __allocBuffer(DeviceContext context, VkBuffer buffer, VkDeviceSize requiredSize,
                                     uint32_t memoryTypeIndex, bool isDedicatedAlloc);
        // Memory suballocation and binding for buffer
        // -> throws runtime_error on failure
        void __bindBuffer(DeviceContext context, VkBuffer buffer, VkDeviceSize requiredSize,
                          VkDeviceMemory memoryPool, size_t byteOffset);
        
        // ---
                          
        // Individual memory allocation and binding for buffer
        // -> throws runtime_error on failure
        VkDeviceMemory __allocImage(DeviceContext context, VkImage image, VkDeviceSize requiredSize,
                                    uint32_t memoryTypeIndex, bool isDedicatedAlloc);
        // Create image view (for render-target, depth buffer, texture buffer...)
        // -> returns VK_NULL_HANDLE on failure
        VkImageView __createImageView(DeviceContext context, VkImage image, VkFormat format,
                                      VkImageAspectFlags typeFlags, uint32_t layerCount = 1u,
                                      uint32_t mipLevels = 1u, uint32_t mostDetailedMip = 0) noexcept;
      }
    }
  }
  
#endif
