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


// -- depth/stencil buffer -- --------------------------------------------------

  // Create depth/stencil buffer for existing renderer/render-target
  DepthStencilBuffer::DepthStencilBuffer(Renderer& renderer, DepthStencilFormat format, uint32_t width,
                                         uint32_t height, uint32_t sampleCount, VkImageTiling tiling)
    : _context(renderer.resourceManager()), _pixelSize(_toPixelSize(width, height)), _format(format) { // throws
    if (width == 0 || height == 0)
      throw std::invalid_argument("DepthStencil: width/height is 0");
    
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1u;
    imageInfo.mipLevels = 1u;
    imageInfo.arrayLayers = 1u;
    imageInfo.format = (VkFormat)format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = (VkSampleCountFlagBits)sampleCount;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(renderer.context(), &imageInfo, nullptr, &(this->_depthStencilBuffer));
    if (result != VK_SUCCESS || this->_depthStencilBuffer == VK_NULL_HANDLE)
      throwError(result, "DepthStencil: buffer creation error");

    this->_depthStencilMemory = __allocBufferImage(this->_context->context(), this->_context->device(),
                                                   this->_depthStencilBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    this->_depthStencilView = __createBufferView(this->_context->context(), this->_depthStencilBuffer, (VkFormat)format,
                                                 VK_IMAGE_ASPECT_DEPTH_BIT, 1u, 1u, 0);
  }

  // Destroy depth/stencil buffer
  void DepthStencilBuffer::release() noexcept {
    if (this->_depthStencilBuffer) {
      if (this->_depthStencilView) {
        vkDestroyImageView(this->_context->context(), this->_depthStencilView, nullptr);
        this->_depthStencilView = VK_NULL_HANDLE;
      }
      vkDestroyImage(this->_context->context(), this->_depthStencilBuffer, nullptr);
      this->_depthStencilBuffer = VK_NULL_HANDLE;
      
      if (this->_depthStencilMemory) {
        vkFreeMemory(this->_context->context(), this->_depthStencilMemory, nullptr);
        this->_depthStencilMemory = VK_NULL_HANDLE;
      }
    }
  }
  
  // ---
  
  DepthStencilBuffer::DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept 
    : _depthStencilView(rhs._depthStencilView),
      _depthStencilBuffer(rhs._depthStencilBuffer),
      _depthStencilMemory(rhs._depthStencilMemory),
      _context(std::move(rhs._context)),
      _pixelSize(rhs._pixelSize),
      _format(rhs._format) {
    rhs._depthStencilView = VK_NULL_HANDLE;
    rhs._depthStencilBuffer = VK_NULL_HANDLE;
    rhs._depthStencilMemory = VK_NULL_HANDLE;
    rhs._context = nullptr;
  }
  DepthStencilBuffer& DepthStencilBuffer::operator=(DepthStencilBuffer&& rhs) noexcept {
    release();
    this->_depthStencilBuffer = rhs._depthStencilBuffer;
    this->_depthStencilView = rhs._depthStencilView;
    this->_depthStencilMemory = rhs._depthStencilMemory;
    this->_context = std::move(rhs._context);
    this->_pixelSize = rhs._pixelSize;
    this->_format = rhs._format;
    rhs._depthStencilView = VK_NULL_HANDLE;
    rhs._depthStencilBuffer = VK_NULL_HANDLE;
    rhs._depthStencilMemory = VK_NULL_HANDLE;
    rhs._context = nullptr;
    return *this;
  }
  
  
// -- buffer memory allocation -- ----------------------------------------------

  BufferMemory::BufferMemory(VkDeviceMemory deviceMemory, const DeviceResourceManager& context) noexcept
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
    if (memoryTypeBits == 0 || bufferCount == 0)
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
