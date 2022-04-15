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


// -- device memory pool -- ----------------------------------------------------

  // Create memory pool allocation targetting a specific memory type (for suballocations)
  VkDeviceMemory DeviceMemoryPool::_allocMemoryPool(DeviceContext context, size_t poolSize,
                                                    uint32_t memoryTypeIndex) { // throws
    assert(poolSize != 0 && (poolSize & 0xFFu) == 0); // invalid pool size
    assert(memoryTypeIndex != 0xFFFFFFFFu); // index not found or not searched
    
    // memory allocation
    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = poolSize;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    VkDeviceMemory allocation = VK_NULL_HANDLE;
    VkResult result = vkAllocateMemory(context, &allocInfo, nullptr, &allocation);
    if (result != VK_SUCCESS || allocation == VK_NULL_HANDLE)
      throwError(result, "DeviceMemoryPool: allocation error");
    return allocation;
  }

  void DeviceMemoryPool::release() noexcept {
    if (_allocation != VK_NULL_HANDLE) {
      vkFreeMemory(_context, _allocation, nullptr);
      _allocation = VK_NULL_HANDLE;
    }
  }
  
  // ---
  
  DeviceMemoryPool::DeviceMemoryPool(DeviceMemoryPool&& rhs) noexcept
    : _allocation(rhs._allocation), _context(rhs._context),
      _poolSize(rhs._poolSize), _memoryTypeIndex(rhs._memoryTypeIndex) {
    rhs._allocation = VK_NULL_HANDLE;
  }
  DeviceMemoryPool& DeviceMemoryPool::operator=(DeviceMemoryPool&& rhs) noexcept {
    _allocation=rhs._allocation; _context=rhs._context;
    _poolSize=rhs._poolSize; _memoryTypeIndex=rhs._memoryTypeIndex;
    rhs._allocation = VK_NULL_HANDLE;
    return *this;
  }

#endif
