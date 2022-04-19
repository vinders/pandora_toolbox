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
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_VIDEO_VULKAN_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- memory properties -- -----------------------------------------------------

# define __P_VULKAN_MEMORY_TYPE_BITS  (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT  \
                                     | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  \
                                     | VK_MEMORY_PROPERTY_HOST_CACHED_BIT   \
                                     | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
                                     // not VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
                                     // -> additional coherence is always good to have,
                                     //    even if not part of the requested bits

  // Initialize device memory properties
  MemoryProps::MemoryProps(VkPhysicalDevice device) noexcept {
    vkGetPhysicalDeviceMemoryProperties(device, &_memoryProps);
  }
  
  // ---
  
  // Find suitable memory type for a specific resource usage
  uint32_t MemoryProps::findMemoryTypeIndex(uint32_t memoryTypeBits, VkMemoryPropertyFlags required,
                                            VkMemoryPropertyFlags preferred) const noexcept {
    uint32_t index = MemoryProps::indexNotFound();
    
    // find type with preferred flags if available
    for (uint32_t i = 0; i < this->_memoryProps.memoryTypeCount; ++i) {
      if (memoryTypeBits & (1u << i)) {
        const VkMemoryPropertyFlags flags = this->_memoryProps.memoryTypes[i].propertyFlags;
        
        // if possible, the type should only have the requested flags
        // e.g.: if 'DEVICE_LOCAL_BIT' is requested,
        //       but 'DEVICE_LOCAL_BIT|HOST_VISIBLE_BIT' is found before it, keep searching
        if ((flags & (preferred | __P_VULKAN_MEMORY_TYPE_BITS)) == preferred)
          return i;
        // only choose type with extra flags if no type exists without them
        if ((flags & preferred) == preferred && index == MemoryProps::indexNotFound())
          index = i;
      }
    }
    
    // if 'preferred' not found, search for type with required flags
    if (index == MemoryProps::indexNotFound()) {
      for (uint32_t i = 0; i < this->_memoryProps.memoryTypeCount; ++i) {
        if (memoryTypeBits & (1u << i)) {
          const VkMemoryPropertyFlags flags = this->_memoryProps.memoryTypes[i].propertyFlags;
          
          // if possible, the type should only have the requested flags
          if ((flags & (required | __P_VULKAN_MEMORY_TYPE_BITS)) == required)
            return i;
          if ((flags & required) == required && index == MemoryProps::indexNotFound())
            index = i;
        }
      }
    }
    return index;
  }
  
  void pandora::video::vulkan::__throwMemoryTypeNotFound() {
    throw std::out_of_range("Vulkan: no suitable memory type found");
  }
  void pandora::video::vulkan::__throwWriteError() { throw std::runtime_error("Vulkan: resource write error"); }


// -- memory allocation -- -----------------------------------------------------

  static constexpr inline const char* __bufferBindingError() noexcept { return "Buffer: memory binding error"; }

  // Individual memory allocation and binding for buffer
  VkDeviceMemory pandora::video::vulkan::__allocBuffer(DeviceContext context, VkBuffer buffer,
                                                       VkDeviceSize requiredSize, uint32_t memoryTypeIndex,
                                                       bool isDedicatedAlloc) {
    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = requiredSize;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    // create memory allocation
    VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
    VkResult result;
    if (isDedicatedAlloc) { // dedicated allocation
      VkMemoryDedicatedAllocateInfo dedicatedAllocInfo{ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR };
      dedicatedAllocInfo.buffer = buffer;
      allocInfo.pNext = &dedicatedAllocInfo;
      result = vkAllocateMemory(context, &allocInfo, nullptr, &bufferMemory);
    }
    else { // default driver allocation
      result = vkAllocateMemory(context, &allocInfo, nullptr, &bufferMemory);
    }
    if (result != VK_SUCCESS || bufferMemory == VK_NULL_HANDLE)
      throwError(result, "Buffer: allocation error");
    
    // bind buffer to allocation
    result = vkBindBufferMemory(context, buffer, bufferMemory, 0);
    if (result != VK_SUCCESS) {
      vkFreeMemory(context, bufferMemory, nullptr);
      throwError(result, __bufferBindingError());
    }
    return bufferMemory;
  }
  
  // Memory suballocation and binding for buffer
  void pandora::video::vulkan::__bindBuffer(DeviceContext context, VkBuffer buffer, VkDeviceSize requiredSize,
                                            VkDeviceMemory memoryPool, size_t byteOffset) {
    VkResult result = vkBindBufferMemory(context, (VkBuffer)buffer, memoryPool, byteOffset);
    if (result != VK_SUCCESS) {
      vkDestroyBuffer(context, buffer, nullptr);
      throwError(result, __bufferBindingError());
    }
  }
  
  // ---

  // Individual memory allocation and binding for depth/stencil image or texture
  VkDeviceMemory pandora::video::vulkan::__allocImage(DeviceContext context, VkImage image,
                                                      VkDeviceSize requiredSize, uint32_t memoryTypeIndex,
                                                      bool isDedicatedAlloc) {
    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = requiredSize;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    // create memory allocation
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkResult result;
    if (isDedicatedAlloc) { // dedicated allocation
      VkMemoryDedicatedAllocateInfo dedicatedAllocInfo{ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR };
      dedicatedAllocInfo.image = image;
      allocInfo.pNext = &dedicatedAllocInfo;
      result = vkAllocateMemory(context, &allocInfo, nullptr, &imageMemory);
    }
    else { // default driver allocation
      result = vkAllocateMemory(context, &allocInfo, nullptr, &imageMemory);
    }
    if (result != VK_SUCCESS || imageMemory == VK_NULL_HANDLE)
      throwError(result, "Image: allocation error");
    
    // bind buffer to allocation
    result = vkBindImageMemory(context, image, imageMemory, 0);
    if (result != VK_SUCCESS) {
      vkFreeMemory(context, imageMemory, nullptr);
      throwError(result, "Image: memory binding error");
    }
    return imageMemory;
  }
  
  // Create image view (for render-target, depth buffer, texture buffer...)
  VkImageView pandora::video::vulkan::__createImageView(DeviceContext context, VkImage image, VkFormat format,
                                                        VkImageAspectFlags typeFlags, uint32_t layerCount,
                                                        uint32_t mipLevels, uint32_t mostDetailedMip) noexcept {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = typeFlags;
    viewInfo.subresourceRange.baseMipLevel = mostDetailedMip;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layerCount;
    
    VkImageView imageView = VK_NULL_HANDLE;
    VkResult result = vkCreateImageView(context, &viewInfo, nullptr, &imageView);
    return (result == VK_SUCCESS) ? imageView : VK_NULL_HANDLE;
  }

#endif
