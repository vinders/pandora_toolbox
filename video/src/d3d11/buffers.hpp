/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp


// -----------------------------------------------------------------------------
// static_buffer.h
// -----------------------------------------------------------------------------

  static D3D11_BIND_FLAG __toBindFlag(pandora::video::DataBufferType type) {
    switch (type) {
      case pandora::video::DataBufferType::constant:    return D3D11_BIND_CONSTANT_BUFFER;
      case pandora::video::DataBufferType::vertexArray: return D3D11_BIND_VERTEX_BUFFER;
      case pandora::video::DataBufferType::vertexIndex: return D3D11_BIND_INDEX_BUFFER;
      default: return D3D11_BIND_SHADER_RESOURCE;
    }
  }

  // Create data buffer (to store data for shader stages)
  StaticBuffer::StaticBuffer(Renderer& renderer, pandora::video::DataBufferType type, size_t bufferByteSize) 
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation error");
  }

  // Create data buffer (to store data for shader stages) with initial value
  StaticBuffer::StaticBuffer(Renderer& renderer, pandora::video::DataBufferType type, 
                             size_t bufferByteSize, const void* initData, bool isImmutable)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    
    if (isImmutable) {
      if (initData == nullptr)
        throw std::invalid_argument("Buffer: immutable with NULL initData");
      bufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
    }
    else
      bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    
    D3D11_SUBRESOURCE_DATA subResData;
    ZeroMemory(&subResData, sizeof(subResData));
    subResData.pSysMem = initData;
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, initData ? &subResData : nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation error");
  }


// -----------------------------------------------------------------------------
// dynamic_buffer.h
// -----------------------------------------------------------------------------

  // Create data buffer (to store data for shader stages)
  DynamicBuffer::DynamicBuffer(Renderer& renderer, pandora::video::DataBufferType type, size_t bufferByteSize)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
    bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation error");
  }

  // Create data buffer (to store data for shader stages) with initial value
  DynamicBuffer::DynamicBuffer(Renderer& renderer, pandora::video::DataBufferType type, 
                               size_t bufferByteSize, const void* initData)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
    bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    D3D11_SUBRESOURCE_DATA subResData;
    ZeroMemory(&subResData, sizeof(subResData));
    subResData.pSysMem = initData;
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, initData ? &subResData : nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation error");
  }

  // ---

  // Write buffer data and discard previous data - recommended for first write of the buffer for a frame
  bool DynamicBuffer::writeDiscard(Renderer& renderer, const void* sourceData) {
    // lock GPU access
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    auto lockResult = renderer.context()->Map(this->_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(lockResult) || mappedResource.pData == nullptr)
      return false;

    memcpy(mappedResource.pData, sourceData, this->_bufferSize);
    renderer.context()->Unmap(this->_buffer, 0);
    return true;
  }
  // Vertex/index buffers: write buffer data with no overwrite - recommended for subsequent writes of the buffer within same frame.
  // Constant buffers: same as 'writeDiscard'.
  bool DynamicBuffer::write(Renderer& renderer, const void* sourceData) {
    // lock GPU access
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    auto writeMode = (this->_type != pandora::video::DataBufferType::constant) ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD;
    auto lockResult = renderer.context()->Map(this->_buffer, 0, writeMode, 0, &mappedResource);
    if (FAILED(lockResult) || mappedResource.pData == nullptr)
      return false;

    memcpy(mappedResource.pData, sourceData, this->_bufferSize);
    renderer.context()->Unmap(this->_buffer, 0);
    return true;
  }


// -----------------------------------------------------------------------------
// depth_stencil_buffer.h
// -----------------------------------------------------------------------------

  // Create depth/stencil buffer for existing renderer/render-target
  DepthStencilBuffer::DepthStencilBuffer(Renderer& renderer, pandora::video::ComponentFormat format, 
                                         uint32_t width, uint32_t height) { // throws
    if (width == 0 || height == 0)
      throw std::invalid_argument("DepthStencil: width/height is 0");
    
    // create compatible depth/stencil buffer
    D3D11_TEXTURE2D_DESC depthDescriptor;
    ZeroMemory(&depthDescriptor, sizeof(depthDescriptor));
    depthDescriptor.Width = (UINT)width;
    depthDescriptor.Height = (UINT)height;
    depthDescriptor.MipLevels = 1;
    depthDescriptor.ArraySize = 1;
    depthDescriptor.Format = (DXGI_FORMAT)Renderer::toDxgiFormat(format);
    depthDescriptor.SampleDesc.Count = 1;
    depthDescriptor.SampleDesc.Quality = 0;
    depthDescriptor.Usage = D3D11_USAGE_DEFAULT;
    depthDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    
    auto result = renderer.device()->CreateTexture2D(&depthDescriptor, nullptr, &(this->_depthStencilBuffer));
    if (FAILED(result) || this->_depthStencilBuffer == nullptr) {
      throwError(result, "DepthStencil: creation error"); return;
    }
    
    // create depth/stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDescriptor;
    ZeroMemory(&depthViewDescriptor, sizeof(depthViewDescriptor));
    depthViewDescriptor.Format = depthDescriptor.Format;
    depthViewDescriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthViewDescriptor.Texture2D.MipSlice = 0;
    
    result = renderer.device()->CreateDepthStencilView(this->_depthStencilBuffer, &depthViewDescriptor, &(this->_depthStencilView));
    if (FAILED(result) || this->_depthStencilView == nullptr)
      throwError(result, "DepthStencil: view not created");
    
    this->_settings.width = width;
    this->_settings.height = height;
    this->_settings.format = format;
  }

  // Destroy depth/stencil buffer
  void DepthStencilBuffer::release() noexcept {
    if (this->_depthStencilBuffer) {
      if (this->_depthStencilView) {
        try { this->_depthStencilView->Release(); } catch (...) {}
        this->_depthStencilView = nullptr;
      }
      try { this->_depthStencilBuffer->Release(); } catch (...) {}
      this->_depthStencilBuffer = nullptr;
    }
  }
  
  // ---
  
  DepthStencilBuffer::DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept 
    : _depthStencilView(rhs._depthStencilView),
      _depthStencilBuffer(rhs._depthStencilBuffer) {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_DepthStencilBufferConfig));
    rhs._depthStencilBuffer = nullptr;
    rhs._depthStencilView = nullptr;
  }
  DepthStencilBuffer& DepthStencilBuffer::operator=(DepthStencilBuffer&& rhs) noexcept {
    release();
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_DepthStencilBufferConfig));
    this->_depthStencilBuffer = rhs._depthStencilBuffer;
    this->_depthStencilView = rhs._depthStencilView;
    rhs._depthStencilBuffer = nullptr;
    rhs._depthStencilView = nullptr;
    return *this;
  }

#endif
