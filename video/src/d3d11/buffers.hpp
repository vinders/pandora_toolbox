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
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp


// -----------------------------------------------------------------------------
// static_buffer.h
// -----------------------------------------------------------------------------

  // Create data buffer (to store data for shader stages)
  StaticBuffer::StaticBuffer(Renderer& renderer, BaseBufferType type, size_t bufferByteSize) 
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = (D3D11_BIND_FLAG)type;
    bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation error");
  }

  // Create data buffer (to store data for shader stages) with initial value
  StaticBuffer::StaticBuffer(Renderer& renderer, BaseBufferType type, size_t bufferByteSize,
                             const void* initData, bool isImmutable)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = (D3D11_BIND_FLAG)type;
    
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
  DynamicBuffer::DynamicBuffer(Renderer& renderer, BaseBufferType type, size_t bufferByteSize)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = (D3D11_BIND_FLAG)type;
    bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
    bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    auto result = renderer.device()->CreateBuffer(&bufferDescriptor, nullptr, &(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "Buffer: creation error");
  }

  // Create data buffer (to store data for shader stages) with initial value
  DynamicBuffer::DynamicBuffer(Renderer& renderer, BaseBufferType type, size_t bufferByteSize, const void* initData)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("Buffer: size is 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = (D3D11_BIND_FLAG)type;
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
  // Constant/uniform buffers: same as 'writeDiscard'.
  bool DynamicBuffer::write(Renderer& renderer, const void* sourceData) {
    // lock GPU access
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    auto writeMode = (this->_type != BaseBufferType::uniform) ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD;
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
  DepthStencilBuffer::DepthStencilBuffer(Renderer& renderer, DepthStencilFormat format, 
                                         uint32_t width, uint32_t height, uint32_t sampleCount) { // throws
    if (width == 0 || height == 0)
      throw std::invalid_argument("DepthStencil: width/height is 0");
    
    // create compatible depth/stencil buffer
    D3D11_TEXTURE2D_DESC depthDescriptor;
    ZeroMemory(&depthDescriptor, sizeof(depthDescriptor));
    depthDescriptor.Width = (UINT)width;
    depthDescriptor.Height = (UINT)height;
    depthDescriptor.MipLevels = 1;
    depthDescriptor.ArraySize = 1;
    depthDescriptor.Format = (DXGI_FORMAT)format;
    depthDescriptor.SampleDesc.Count = (UINT)sampleCount;
    depthDescriptor.SampleDesc.Quality = 0;
    depthDescriptor.Usage = D3D11_USAGE_DEFAULT;
    depthDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (sampleCount > 1u && !renderer._isSampleCountSupported((DXGI_FORMAT)format, sampleCount, depthDescriptor.SampleDesc.Quality)) {
      depthDescriptor.SampleDesc.Count = 1u;
      depthDescriptor.SampleDesc.Quality = 0;
    }
    
    auto result = renderer.device()->CreateTexture2D(&depthDescriptor, nullptr, &(this->_depthStencilBuffer));
    if (FAILED(result) || this->_depthStencilBuffer == nullptr) {
      throwError(result, "DepthStencil: creation error"); return;
    }
    
    // create depth/stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDescriptor;
    ZeroMemory(&depthViewDescriptor, sizeof(depthViewDescriptor));
    depthViewDescriptor.Format = (DXGI_FORMAT)format;
    if (depthDescriptor.SampleDesc.Count > 1u) {
      depthViewDescriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    }
    else {
      depthViewDescriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
      depthViewDescriptor.Texture2D.MipSlice = 0;
    }
    
    
    result = renderer.device()->CreateDepthStencilView(this->_depthStencilBuffer, &depthViewDescriptor, &(this->_depthStencilView));
    if (FAILED(result) || this->_depthStencilView == nullptr)
      throwError(result, "DepthStencil: view not created");
    
    this->_pixelSize = _toPixelSize(width, height);
    this->_format = format;
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
      _depthStencilBuffer(rhs._depthStencilBuffer),
      _pixelSize(rhs._pixelSize),
      _format(rhs._format) {
    rhs._depthStencilBuffer = nullptr;
    rhs._depthStencilView = nullptr;
  }
  DepthStencilBuffer& DepthStencilBuffer::operator=(DepthStencilBuffer&& rhs) noexcept {
    release();
    this->_depthStencilBuffer = rhs._depthStencilBuffer;
    this->_depthStencilView = rhs._depthStencilView;
    this->_pixelSize = rhs._pixelSize;
    this->_format = rhs._format;
    rhs._depthStencilBuffer = nullptr;
    rhs._depthStencilView = nullptr;
    return *this;
  }

#endif
