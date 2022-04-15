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


// -- depth/stencil params -- --------------------------------------------------

  DepthStencilBufferParams::DepthStencilBufferParams() noexcept {
    _params.imageType = VK_IMAGE_TYPE_2D;
    _params.extent.depth = 1u;
    _params.mipLevels = 1u;
    _params.arrayLayers = 1u;
    _params.format = VK_FORMAT_D32_SFLOAT;
    _params.tiling = VK_IMAGE_TILING_OPTIMAL;
    _params.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _params.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    _params.samples = VK_SAMPLE_COUNT_1_BIT;
    _params.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }
  
  DepthStencilBufferParams::DepthStencilBufferParams(DepthStencilFormat format, uint32_t width,
                                                     uint32_t height, uint32_t sampleCount) noexcept {
    _params.imageType = VK_IMAGE_TYPE_2D;
    _params.extent.width = width;
    _params.extent.height = height;
    _params.extent.depth = 1u;
    _params.mipLevels = 1u;
    _params.arrayLayers = 1u;
    _params.format = (VkFormat)format;
    _params.tiling = VK_IMAGE_TILING_OPTIMAL;
    _params.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    _params.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    _params.samples = (VkSampleCountFlagBits)sampleCount;
    _params.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  } 
  
  DepthStencilBufferParams& DepthStencilBufferParams::sharingMode(uint32_t* concurrentQueueFamilies,
                                                                  uint32_t queueCount) noexcept {
    if (concurrentQueueFamilies == nullptr || queueCount == 0) {
      _params.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      _params.pQueueFamilyIndices = nullptr;
      _params.queueFamilyIndexCount = 0;
    }
    else {
      _params.sharingMode = VK_SHARING_MODE_CONCURRENT;
      _params.pQueueFamilyIndices = concurrentQueueFamilies;
      _params.queueFamilyIndexCount = queueCount;
    }
    return *this;
  }


// -- depth/stencil buffer builder/container -- --------------------------------

  static constexpr inline const char* __depthBufferCreationError() noexcept { return "Buffer: creation error"; }

  // Initialize depth/stencil buffer builder
  DepthStencilBuffer::Builder::Builder(DeviceResourceManager device, DepthStencilBufferParams& params)
    : _device(device), _params(params.descriptorPtr()) {
    if (params.descriptor().extent.width == 0 || params.descriptor().extent.height == 0)
      throw std::invalid_argument("DepthStencil: width/height is 0");
    
    // create buffer descriptor
    VkResult result = vkCreateImage(device->context(), _params, nullptr, &_bufferHandle);
    if (result != VK_SUCCESS || _bufferHandle == VK_NULL_HANDLE)
      throwError(result, __depthBufferCreationError());

    // read memory requirements
    const VkImageMemoryRequirementsInfo2 requirementsInfo {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
      nullptr,
      _bufferHandle
    };
    vkGetImageMemoryRequirements2(device->context(), &requirementsInfo, &_memoryReq);
    
    _memoryTypeIndex = device->memoryProps().findMemoryTypeIndex(_memoryReq.memoryRequirements.memoryTypeBits,
                                                                 (VkMemoryPropertyFlags)0,
                                                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (_memoryTypeIndex == MemoryProps::indexNotFound())
      __throwMemoryTypeNotFound();
  }
  
  // ---
  
  // Destroy depth/stencil buffer handle
  void DepthStencilBuffer::Builder::_destroyBuffer() noexcept {
    if (_bufferHandle != VK_NULL_HANDLE)
      vkDestroyImage(_device->context(), _bufferHandle, nullptr);
  }
  // Destroy depth/stencil buffer object
  void DepthStencilBuffer::release() noexcept {
    if (_bufferHandle) {
      if (_bufferView) {
        vkDestroyImageView(_context, _bufferView, nullptr);
        _bufferView = VK_NULL_HANDLE;
      }
      vkDestroyImage(_context, _bufferHandle, nullptr);
      _bufferHandle = VK_NULL_HANDLE;
      
      if (_allocation) {
        vkFreeMemory(_context, _allocation, nullptr);
        _allocation = VK_NULL_HANDLE;
      }
    }
  }
  
  // ---
  
  // Create DepthStencilBuffer object
  DepthStencilBuffer DepthStencilBuffer::Builder::build(FeatureMode dedicatedAllocMode) {
    if (_bufferHandle == VK_NULL_HANDLE) { // already built -> create new handle
      VkResult result = vkCreateImage(_device->context(), _params, nullptr, &_bufferHandle);
      if (result != VK_SUCCESS || _bufferHandle == VK_NULL_HANDLE)
        throwError(result, __depthBufferCreationError());
    }

    // memory allocation
    DeviceContext context = _device->context();
    const bool isDedicatedAlloc = (dedicatedAllocMode == FeatureMode::force
                               || (dedicatedAllocMode == FeatureMode::autodetect
                                  && (_allocReq.requiresDedicatedAllocation
                                   || _allocReq.prefersDedicatedAllocation)) );
    auto memory = __allocImage(context, _bufferHandle, _memoryReq.memoryRequirements.size,
                               _memoryTypeIndex, isDedicatedAlloc); // throws
    
    // depth/stencil image view
    auto bufferView = __createImageView(context, _bufferHandle, _params->format,
                                        VK_IMAGE_ASPECT_DEPTH_BIT, 1u, 1u, 0);
    if (bufferView == VK_NULL_HANDLE) {
      vkFreeMemory(context, memory, nullptr);
      throw std::runtime_error("DepthStencil: image view creation error");
    }

    auto buffer = _bufferHandle;
    _bufferHandle = VK_NULL_HANDLE; // reset _bufferHandle in case 'build' is called again
    return DepthStencilBuffer(context, buffer, bufferView, (DepthStencilFormat)_params->format,
                              _params->extent.width, _params->extent.height, memory);
  }
  
  // ---
  
  DepthStencilBuffer::DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept 
    : _bufferView(rhs._bufferView),
      _bufferHandle(rhs._bufferHandle),
      _allocation(rhs._allocation),
      _context(std::move(rhs._context)),
      _pixelSize(rhs._pixelSize),
      _format(rhs._format) {
    rhs._bufferView = VK_NULL_HANDLE;
    rhs._bufferHandle = VK_NULL_HANDLE;
    rhs._allocation = VK_NULL_HANDLE;
  }
  DepthStencilBuffer& DepthStencilBuffer::operator=(DepthStencilBuffer&& rhs) noexcept {
    release();
    _bufferView = rhs._bufferView;
    _bufferHandle = rhs._bufferHandle;
    _allocation = rhs._allocation;
    _context = std::move(rhs._context);
    _pixelSize = rhs._pixelSize;
    _format = rhs._format;
    rhs._bufferView = VK_NULL_HANDLE;
    rhs._bufferHandle = VK_NULL_HANDLE;
    rhs._allocation = VK_NULL_HANDLE;
    return *this;
  }

#endif
