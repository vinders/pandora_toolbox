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


// -- depth/stencil buffer -- --------------------------------------------------

  // Create depth/stencil buffer for existing renderer/render-target
  DepthStencilBuffer::DepthStencilBuffer(DeviceResourceManager device, DepthStencilFormat format, 
                                         uint32_t width, uint32_t height, uint32_t sampleCount)
    : _pixelSize(_toPixelSize(width, height)), _format(format) { // throws
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
    if (sampleCount > 1u && !Renderer::_isSampleCountSupported(device, (DXGI_FORMAT)format, sampleCount,
                                                               depthDescriptor.SampleDesc.Quality)) {
      depthDescriptor.SampleDesc.Count = 1u;
      depthDescriptor.SampleDesc.Quality = 0;
    }
    
    auto result = device->CreateTexture2D(&depthDescriptor, nullptr, &(this->_depthStencilBuffer));
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
    
    result = device->CreateDepthStencilView(this->_depthStencilBuffer, &depthViewDescriptor,
                                            &(this->_depthStencilView));
    if (FAILED(result) || this->_depthStencilView == nullptr)
      throwError(result, "DepthStencil: view not created");
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
