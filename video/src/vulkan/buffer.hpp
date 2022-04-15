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


// -- buffer params -- ---------------------------------------------------------

  void pandora::video::vulkan::__setBufferSharingMode(VkBufferCreateInfo& outBufferInfo,
                                                      uint32_t* concurrentQueueFamilies,
                                                      uint32_t queueCount) noexcept {
    if (concurrentQueueFamilies == nullptr || queueCount == 0) {
      outBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      outBufferInfo.pQueueFamilyIndices = nullptr;
      outBufferInfo.queueFamilyIndexCount = 0;
    }
    else {
      outBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
      outBufferInfo.pQueueFamilyIndices = concurrentQueueFamilies;
      outBufferInfo.queueFamilyIndexCount = queueCount;
    }
  }


// -- buffer builder/container -- ----------------------------------------------

  static constexpr inline const char* __bufferCreationError() noexcept { return "Buffer: creation error"; }
  
  // Create buffer instance (vertex/index/uniform/...) + read memory requirements
  VkBuffer pandora::video::vulkan::__createBuffer(DeviceContext context, const VkBufferCreateInfo& params,
                                                  VkMemoryRequirements2& outRequirements) {
    if (params.size == 0)
      throw std::invalid_argument("Buffer: size 0");
    
    // create buffer descriptor
    VkBuffer bufferHandle = VK_NULL_HANDLE;
    VkResult result = vkCreateBuffer(context, &params, nullptr, &bufferHandle);
    if (result != VK_SUCCESS || bufferHandle == VK_NULL_HANDLE)
      throwError(result, __bufferCreationError());

    // read memory requirements
    const VkBufferMemoryRequirementsInfo2 requirementsInfo {
      VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
      nullptr,
      bufferHandle
    };
    vkGetBufferMemoryRequirements2(context, &requirementsInfo, &outRequirements);
    if (outRequirements.memoryRequirements.size < params.size)
      outRequirements.memoryRequirements.size = params.size;
    if (outRequirements.memoryRequirements.alignment == 0)
      outRequirements.memoryRequirements.alignment = (params.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) ? 256 : 16;
    
    return bufferHandle;
  }
  
  // Create buffer instance (vertex/index/uniform/...)
  VkBuffer pandora::video::vulkan::__createBuffer(DeviceContext context, const VkBufferCreateInfo& params) {
    VkBuffer bufferHandle = VK_NULL_HANDLE;
    VkResult result = vkCreateBuffer(context, &params, nullptr, &bufferHandle);
    if (result != VK_SUCCESS || bufferHandle == VK_NULL_HANDLE)
      throwError(result, __bufferCreationError());
    return bufferHandle;
  }
  
  // ---
  
  // Destroy/release buffer instance (only alloc verified)
  void pandora::video::vulkan::__destroyBuffer(DeviceContext context, BufferHandle handle,
                                               VkDeviceMemory allocation) noexcept {
    vkDestroyBuffer(context, handle, nullptr);
    if (allocation != VK_NULL_HANDLE)
      vkFreeMemory(context, allocation, nullptr);
  }


// -- buffer read/write -- -----------------------------------------------------
  
  void pandora::video::vulkan::__throwWriteError() { throw std::runtime_error("Buffer: write error"); }
  
  // Write dynamic/staging buffer via memory mapping
  bool pandora::video::vulkan::__writeMappedBuffer(DeviceContext context, VkMemoryPropertyFlags memoryUsage,
                                                   VkDeviceMemory allocation, size_t allocOffset,
                                                   size_t bufferSize, const void* sourceData) noexcept {
    assert(allocation != VK_NULL_HANDLE && sourceData != nullptr);
    void* mapped = nullptr;
    VkResult result = vkMapMemory(context, allocation, allocOffset, (VkDeviceSize)bufferSize, 0, &mapped);
    if (result != VK_SUCCESS || mapped == nullptr)
      return false;
    
    memcpy(mapped, sourceData, bufferSize); // copy
    
    // if buffer is not host-coherent, flush it before unmapping
    if ((memoryUsage & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
      VkMappedMemoryRange range{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
      range.memory = allocation;
      range.offset = allocOffset;
      range.size = bufferSize;
      vkFlushMappedMemoryRanges(context, 1u, &range);
    }
    vkUnmapMemory(context, allocation);
    return true;
  }

  // Write local/static buffer by filling and copying a staging buffer
  bool pandora::video::vulkan::__writeLocalBuffer(DeviceContext context, const MemoryProps& memoryProps,
                                                  VkCommandPool commandPool, VkQueue commandQueue,
                                                  BufferHandle localBuffer, size_t bufferSize,
                                                  const void* sourceData) noexcept {
    assert(localBuffer != VK_NULL_HANDLE && sourceData != nullptr);
    bool isSuccess = false;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    try {
      // create temporary staging buffer
      VkMemoryDedicatedRequirementsKHR allocReq { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR };
      VkMemoryRequirements2 memoryReq {
          VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
          &allocReq
      };
      
      VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
      bufferInfo.size = (VkDeviceSize)bufferSize;
      bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      stagingBuffer = __createBuffer(context, bufferInfo, memoryReq); // throws
      
      constexpr const auto stagingUsageFlags = _toRequiredMemoryUsageFlags(ResourceUsage::staging);
      uint32_t stagingMemIndex = memoryProps.findMemoryTypeIndex(memoryReq.memoryRequirements.memoryTypeBits,
                                                                 stagingUsageFlags, stagingUsageFlags);
      if (stagingMemIndex != MemoryProps::indexNotFound()) {
        auto stagingMem = __allocBuffer(context, stagingBuffer, memoryReq.memoryRequirements.size, // throws
                                        stagingMemIndex, allocReq.requiresDedicatedAllocation);
        
        // write data into staging buffer + copy staging to local buffer
        const VkBufferCopy copyRegion{ 0, 0, bufferSize };
        isSuccess = (__writeMappedBuffer(context, stagingUsageFlags, stagingMem, 0, bufferSize, sourceData)
                  && __copyLocalBuffer(context, commandPool, commandQueue,
                                       stagingBuffer, localBuffer, &copyRegion, 1) );
        
        vkDestroyBuffer(context, stagingBuffer, nullptr);
        vkFreeMemory(context, stagingMem, nullptr);
      }
      else
        vkDestroyBuffer(context, stagingBuffer, nullptr);
    }
    catch (...) {
      if (stagingBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(context, stagingBuffer, nullptr);
    }
    return isSuccess;
  }
  
  // ---
  
  // Copy mappable buffer content to other mappable buffer
  bool pandora::video::vulkan::__copyMappedBuffer(DeviceContext context, VkDeviceMemory source,
                                                  size_t sourceOffset, VkDeviceMemory dest, size_t destOffset,
                                                  size_t bufferSize, bool isHostCoherent) noexcept {
    assert(source != VK_NULL_HANDLE && dest != VK_NULL_HANDLE);
    void* sourceData = nullptr;
    void* destData = nullptr;
    
    bool isSuccess = false;
    VkResult result = vkMapMemory(context, source, sourceOffset, (VkDeviceSize)bufferSize, 0, &sourceData);
    if (result == VK_SUCCESS && sourceData != nullptr) {
      result = vkMapMemory(context, dest, destOffset, (VkDeviceSize)bufferSize, 0, &destData);
      if (result == VK_SUCCESS && destData != nullptr) {
        if (isHostCoherent) {
          memcpy(destData, sourceData, bufferSize); // copy
        }
        // if source/dest buffer is not host-coherent, refresh before reading and flush before unmapping
        else {
          VkMappedMemoryRange rangeSrc{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
          rangeSrc.memory = source;
          rangeSrc.offset = sourceOffset;
          rangeSrc.size = bufferSize;
          vkInvalidateMappedMemoryRanges(context, 1u, &rangeSrc);

          memcpy(destData, sourceData, bufferSize); // copy

          VkMappedMemoryRange rangeDest{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
          rangeDest.memory = dest;
          rangeDest.offset = destOffset;
          rangeDest.size = bufferSize;
          vkFlushMappedMemoryRanges(context, 1u, &rangeDest);
        }
        vkUnmapMemory(context, dest);
        isSuccess = true;
      }
      vkUnmapMemory(context, source);
    }
    return isSuccess;
  }
  
  // Copy buffer content to other buffer (command list)
  bool pandora::video::vulkan::__copyLocalBuffer(DeviceContext context, VkCommandPool commandPool,
                                                 VkQueue copyCommandQueue, BufferHandle source, BufferHandle dest,
                                                 const VkBufferCopy* regions, uint32_t regionCount) noexcept {
    assert(source != VK_NULL_HANDLE && dest != VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    bool isSuccess = false;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkResult result = vkAllocateCommandBuffers(context, &allocInfo, &commandBuffer);
    if (result == VK_SUCCESS && commandBuffer != VK_NULL_HANDLE) {
      
      VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      if (vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS) { // begin recording
        vkCmdCopyBuffer(commandBuffer, source, dest, regionCount, regions);

        if (vkEndCommandBuffer(commandBuffer) == VK_SUCCESS) { // end recording
          VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
          submitInfo.commandBufferCount = 1;
          submitInfo.pCommandBuffers = &commandBuffer;
          isSuccess = (vkQueueSubmit(copyCommandQueue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
          vkQueueWaitIdle(copyCommandQueue);
        }
      }
      vkFreeCommandBuffers(context, commandPool, 1, &commandBuffer);
    }
    return isSuccess;
  }
  
  // ---
  
  // Map staging buffer memory
  bool MappedBufferIO::_open(size_t offset, size_t size, IOMode mode, bool isHostCoherent) noexcept {
    assert(_mapped == nullptr);
    void* mapped = nullptr;
    VkResult result = vkMapMemory(_context, _memory, offset, (VkDeviceSize)size, 0, &mapped);
    if (result == VK_SUCCESS) {
      _mapped = mapped;
      
      if (!isHostCoherent) {
        // if source buffer is not host-coherent, refresh it before reading
        if ((mode & IOMode::read) == IOMode::read) {
          VkMappedMemoryRange range{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
          range.memory = _memory;
          range.offset = offset;
          range.size = size;
          vkInvalidateMappedMemoryRanges(_context, 1u, &range);
        }
        if ((mode & IOMode::write) == IOMode::write) {
          _flushedOffset = offset; // will be flushed before unmapping
          _flushedSize = size;
        }
      }
    }
    return (_mapped != nullptr);
  }
  
  // Map dynamic buffer memory
  bool MappedBufferIO::_openWriteMode(size_t offset, size_t size, bool isHostCoherent) noexcept {
    assert(_mapped == nullptr);
    void* mapped = nullptr;
    VkResult result = vkMapMemory(_context, _memory, offset, (VkDeviceSize)size, 0, &mapped);
    if (result == VK_SUCCESS) {
      _mapped = mapped;
      
      if (!isHostCoherent) {
        _flushedOffset = offset; // will be flushed before unmapping
        _flushedSize = size;
      }
    }
    return (_mapped != nullptr);
  }
  
  // Unmap data buffer (close access)
  void MappedBufferIO::close() noexcept {
    if (_mapped != nullptr) {
      assert(_memory != VK_NULL_HANDLE);
      
      // if buffer is not host-coherent, flush it before unmapping
      if (_flushedSize) {
        VkMappedMemoryRange range{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
        range.memory = _memory;
        range.offset = _flushedOffset;
        range.size = _flushedSize;
        vkFlushMappedMemoryRanges(_context, 1u, &range);
        
        _flushedSize = 0;
      }
      vkUnmapMemory(_context, _memory);
      _mapped = nullptr;
    }
  }
  
#endif
