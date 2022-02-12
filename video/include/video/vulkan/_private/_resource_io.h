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
# include "video/vulkan/api/types.h" // includes vulkan

  namespace pandora {
    namespace video {
      namespace vulkan {
        // Create buffer view (for render target buffer, depth buffer, texture buffer...)
        // -> throws runtime_error on failure
        VkImageView __createBufferView(DeviceContext context, VkImage bufferImage, VkFormat bufferFormat,
                                       VkImageAspectFlags type, uint32_t layerCount = 1u,
                                       uint32_t mipLevels = 1u, uint32_t mostDetailedMip = 0);
        
        // ---
      
        // Find device memory index for requested type bits
        // -> throws out_of_range if not found
        uint32_t __findMemoryTypeIndex(VkPhysicalDevice device, uint32_t memoryTypeBits,
                                       VkMemoryPropertyFlags resourceUsage);
        // Allocate device memory for buffer image (depth/texture)
        // -> throws runtime_error on failure
        VkDeviceMemory __allocBufferImage(DeviceContext context, DeviceHandle physDevice,
                                          VkImage bufferImage, VkMemoryPropertyFlags resourceUsage);
      }
    }
  }
#endif
