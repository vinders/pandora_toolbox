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


// -- data buffer (constant/uniform, vertex, index) -- -------------------------

  // Create buffer container (vertex/index/uniform)
  VkBuffer pandora::video::vulkan::__createBufferContainer(DeviceContext context, size_t byteSize, VkBufferUsageFlags type,
                                                           uint32_t* concurrentQueueFamilies, uint32_t queueCount) { // throws
    if (byteSize == 0)
      throw std::invalid_argument("Buffer: size 0");
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = (VkDeviceSize)byteSize;
    bufferInfo.usage = type;
    
    if (concurrentQueueFamilies == nullptr || queueCount == 0) {
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else {
      bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
      bufferInfo.pQueueFamilyIndices = concurrentQueueFamilies;
      bufferInfo.queueFamilyIndexCount = queueCount;
    }
    
    VkBuffer buffer;
    VkResult result = vkCreateBuffer(context, &bufferInfo, nullptr, &buffer);
    if (result != VK_SUCCESS || buffer == VK_NULL_HANDLE)
      throwError(result, "Buffer: creation failure");
    return buffer;
  }
  // Destroy/release static buffer instance
  void pandora::video::vulkan::__destroyBufferContainer(DeviceContext context, BufferHandle handle, VkDeviceMemory allocation) noexcept {
    vkDestroyBuffer(context, handle, nullptr);
    if (allocation != VK_NULL_HANDLE)
      vkFreeMemory(context, allocation, nullptr);
  }
  
  // Create buffer view (for render target buffer, depth buffer, texture buffer...)
  VkImageView pandora::video::vulkan::__createBufferView(DeviceContext context, VkImage bufferImage, VkFormat bufferFormat,
                                                        VkImageAspectFlags type, uint32_t layerCount,
                                                        uint32_t mipLevels, uint32_t mostDetailedMip) { // throws
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = bufferImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = bufferFormat;
    viewInfo.subresourceRange.aspectMask = type;
    viewInfo.subresourceRange.baseMipLevel = mostDetailedMip;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layerCount;
    
    VkImageView imageView = VK_NULL_HANDLE;
    VkResult result = vkCreateImageView(context, &viewInfo, nullptr, &imageView);
    if (result != VK_SUCCESS || imageView == VK_NULL_HANDLE)
      throwError(result, "Resource: view not created");
    return imageView;
  }
  
  // ---
  
  // Find memory type for a requested resource memory usage
  uint32_t pandora::video::vulkan::__findMemoryTypeIndex(VkPhysicalDevice device, uint32_t memoryTypeBits,
                                                         VkMemoryPropertyFlags resourceUsage) {
    VkPhysicalDeviceMemoryProperties memoryProps;
    vkGetPhysicalDeviceMemoryProperties(device, &memoryProps);
    
    for (uint32_t i = 0; i < memoryProps.memoryTypeCount; ++i) {
      if ((memoryTypeBits & (1u << i))
      && (memoryProps.memoryTypes[i].propertyFlags & resourceUsage) == resourceUsage) {
        return i;
      }
    }
    throw std::out_of_range("Buffer: no suitable memory type");
  }
  
  // Allocate device memory for buffer image
  VkDeviceMemory pandora::video::vulkan::__allocBufferImage(DeviceContext context, DeviceHandle physDevice,
                                                            VkImage bufferImage, VkMemoryPropertyFlags resourceUsage) { // throws
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(context, bufferImage, &requirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = __findMemoryTypeIndex(physDevice, requirements.memoryTypeBits, resourceUsage);

    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkResult result = vkAllocateMemory(context, &allocInfo, nullptr, &imageMemory);
    if (result != VK_SUCCESS || imageMemory == VK_NULL_HANDLE)
      throwError(result, "BufferImage: memory allocation failure");

    result = vkBindImageMemory(context, bufferImage, imageMemory, 0);
    if (result != VK_SUCCESS) {
      vkFreeMemory(context, imageMemory, nullptr);
      throwError(result, "BufferImage: memory binding failure");
    }
    return imageMemory;
  }
  // Allocate device memory for buffer container
  VkDeviceMemory pandora::video::vulkan::__allocBufferContainer(DeviceContext context, DeviceHandle physDevice,
                                                                VkBuffer buffer, VkMemoryPropertyFlags resourceUsage) { // throws
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context, buffer, &requirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = __findMemoryTypeIndex(physDevice, requirements.memoryTypeBits, resourceUsage);

    VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
    VkResult result = vkAllocateMemory(context, &allocInfo, nullptr, &bufferMemory);
    if (result != VK_SUCCESS || bufferMemory == VK_NULL_HANDLE)
      throwError(result, "Buffer: memory allocation failure");

    result = vkBindBufferMemory(context, buffer, bufferMemory, 0);
    if (result != VK_SUCCESS) {
      vkFreeMemory(context, bufferMemory, nullptr);
      throwError(result, "Buffer: memory binding failure");
    }
    return bufferMemory;
  }
  
  // ---
  
  // Write dynamic/staging buffer via memory mapping
  bool pandora::video::vulkan::__writeMappedDataBuffer(DeviceContext context, size_t bufferSize, VkDeviceMemory allocation,
                                                       size_t allocOffset, const void* sourceData) noexcept {
    assert(allocation != VK_NULL_HANDLE && sourceData != nullptr);
    
    void* mapped = nullptr;
    VkResult result = vkMapMemory(context, allocation, allocOffset, (VkDeviceSize)bufferSize, 0, &mapped);
    if (result != VK_SUCCESS || mapped == nullptr)
      return false;
    
    memcpy(mapped, sourceData, bufferSize);
    vkUnmapMemory(context, allocation);
    return true;
  }
  
  // Write static/immutable buffer by filling and copying a staging buffer
  bool pandora::video::vulkan::__writeWithStagingBuffer(DeviceContext context, DeviceHandle physDevice, VkCommandPool commandPool,
                                                        VkQueue copyCommandQueue, BufferHandle buffer, size_t bufferSize,
                                                        const void* sourceData) noexcept {
    assert(buffer != VK_NULL_HANDLE && sourceData != nullptr);
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    bool reply = false;
    try {
      stagingBuffer = __createBufferContainer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr, 0);
      auto stagingMemory = __allocBufferContainer(context, physDevice, stagingBuffer,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      
      reply = (__writeMappedDataBuffer(context, bufferSize, stagingMemory, 0, sourceData)
            && __copyDataBuffer(context, commandPool, copyCommandQueue, stagingBuffer, buffer, bufferSize) );
      vkDestroyBuffer(context, stagingBuffer, nullptr);
      vkFreeMemory(context, stagingMemory, nullptr);
    }
    catch (...) {
      if (stagingBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(context, stagingBuffer, nullptr);
    }
    return reply;
  }
  
  // ---
  
  // Copy mappable buffer into other mappable buffer
  bool pandora::video::vulkan::__copyMappedDataBuffer(DeviceContext context, size_t bufferSize, VkDeviceMemory source,
                                                      size_t sourceOffset, VkDeviceMemory dest, size_t destOffset) noexcept {
    assert(source != VK_NULL_HANDLE && dest != VK_NULL_HANDLE);
    void* sourceData = nullptr;
    void* mapped = nullptr;
    
    bool reply = false;
    VkResult result = vkMapMemory(context, source, sourceOffset, (VkDeviceSize)bufferSize, 0, &sourceData);
    if (result == VK_SUCCESS && sourceData != nullptr) {
      result = vkMapMemory(context, dest, destOffset, (VkDeviceSize)bufferSize, 0, &mapped);
      if (result == VK_SUCCESS && mapped != nullptr) {
        memcpy(mapped, sourceData, bufferSize);
        vkUnmapMemory(context, dest);
        reply = true;
      }
      vkUnmapMemory(context, source);
    }
    return reply;
  }
  
  // Copy buffer into other buffer (command list)
  bool pandora::video::vulkan::__copyDataBuffer(DeviceContext context, VkCommandPool commandPool, VkQueue copyCommandQueue,
                                                BufferHandle source, BufferHandle dest, size_t bufferSize) noexcept {
    assert(source != VK_NULL_HANDLE && dest != VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    bool reply = false;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkResult result = vkAllocateCommandBuffers(context, &allocInfo, &commandBuffer);
    if (result == VK_SUCCESS && commandBuffer != VK_NULL_HANDLE) {
      
      VkCommandBufferBeginInfo beginInfo{}; // begin recording
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      if (vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS) {
        VkBufferCopy copyRegion{};
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, source, dest, 1, &copyRegion);

        if (vkEndCommandBuffer(commandBuffer) == VK_SUCCESS) { // end recording
          VkSubmitInfo submitInfo{};
          submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
          submitInfo.commandBufferCount = 1;
          submitInfo.pCommandBuffers = &commandBuffer;
          reply = (vkQueueSubmit(copyCommandQueue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
          vkQueueWaitIdle(copyCommandQueue);
        }
      }
      vkFreeCommandBuffers(context, commandPool, 1, &commandBuffer);
    }
    return reply;
  }
  
  // ---
  
  // Map dynamic/staging data buffer for read/write operations
  void* pandora::video::vulkan::__mapDataBuffer(DeviceContext context, size_t bufferSize,
                                                VkDeviceMemory allocation, size_t allocOffset) noexcept {
    assert(allocation != VK_NULL_HANDLE);
    
    void* mapped = nullptr;
    VkResult result = vkMapMemory(context, allocation, allocOffset, (VkDeviceSize)bufferSize, 0, &mapped);
    return (result == VK_SUCCESS) ? mapped : nullptr;
  }
  // Unmap dynamic/staging data buffer
  void pandora::video::vulkan::__unmapDataBuffer(DeviceContext context, VkDeviceMemory allocation) noexcept {
    assert(allocation != VK_NULL_HANDLE);
    vkUnmapMemory(context, allocation);
  }


// -- buffer memory allocation -- ----------------------------------------------

  BufferMemory::BufferMemory(VkDeviceMemory deviceMemory, DeviceResourceManager context) noexcept
    : _handle(deviceMemory, context, vkFreeMemory) {}

  // Create memory allocation (for multiple buffer suballocations)
  BufferMemory BufferMemory::create(Renderer& renderer, ResourceUsage mode, size_t byteSize,
                                    const BufferHandle* eachBufferType, size_t bufferCount) {
    uint32_t memoryTypeBits = 0xFFFFFFFFu;
    VkMemoryRequirements requirements;
    for (const BufferHandle* it = eachBufferType; bufferCount; --bufferCount, ++it) {
      vkGetBufferMemoryRequirements(renderer.context(), *it, &requirements);
      memoryTypeBits &= requirements.memoryTypeBits;
    }
    if (memoryTypeBits == 0)
      throw std::out_of_range("BufferMemory: no common memory type");
    
    VkMemoryPropertyFlags memoryMode;
    switch (mode) {
      case ResourceUsage::immutable: memoryMode = Buffer<ResourceUsage::immutable>::_getMemoryUsageFlags(); break;
      case ResourceUsage::dynamicCpu: memoryMode = Buffer<ResourceUsage::dynamicCpu>::_getMemoryUsageFlags(); break;
      case ResourceUsage::staging: memoryMode = Buffer<ResourceUsage::staging>::_getMemoryUsageFlags(); break;
      case ResourceUsage::staticGpu:
      default: memoryMode = Buffer<ResourceUsage::staticGpu>::_getMemoryUsageFlags(); break;
    }
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = byteSize;
    allocInfo.memoryTypeIndex = __findMemoryTypeIndex(renderer.device(), memoryTypeBits, memoryMode);
    
    VkDeviceMemory allocation = VK_NULL_HANDLE;
    VkResult result = vkAllocateMemory(renderer.context(), &allocInfo, nullptr, &allocation);
    if (result != VK_SUCCESS || allocation == VK_NULL_HANDLE)
      throwError(result, "BufferMemory: allocation failure");
    return BufferMemory(allocation, renderer.resourceManager());
  }
  
  // Bind buffer to suballocation in existing memory allocation
  void BufferMemory::_bind(BufferHandle buffer, size_t byteOffset, VkDeviceMemory& outAllocation) {
    if (outAllocation != VK_NULL_HANDLE)
      throw std::logic_error("BufferMemory: buffer already bound");
    
    VkResult result = vkBindBufferMemory(_handle.context(), (VkBuffer)buffer, _handle.value(), byteOffset);
    if (result != VK_SUCCESS)
      throwError(result, "BufferMemory: binding failure");
    outAllocation = _handle.value();
  }

  // ---
  
  MappedBufferIO::MappedBufferIO(Buffer<ResourceUsage::dynamicCpu>& buffer)
    : _context(buffer.context()), _buffer(buffer.allocation()) { // throws
    this->_mapped = __mapDataBuffer(buffer.context(), buffer.size(),
                                    buffer.allocation(), buffer.allocationOffset());
    if (this->_mapped == nullptr)
      throw std::runtime_error("MappedBufferIO: open error");
  }
  MappedBufferIO::MappedBufferIO(Buffer<ResourceUsage::staging>& buffer, StagedMapping)
    : _context(buffer.context()), _buffer(buffer.allocation()) { // throws
    this->_mapped = __mapDataBuffer(buffer.context(), buffer.size(),
                                    buffer.allocation(), buffer.allocationOffset());
    if (this->_mapped == nullptr)
      throw std::runtime_error("MappedBufferIO: open error");
  }

#endif
