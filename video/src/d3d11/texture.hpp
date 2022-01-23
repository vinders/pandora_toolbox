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
# ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable : 6387)
# endif
// includes + namespaces: in renderer.cpp

static constexpr inline const char* __error_resCreationFailed() noexcept { return "Texture: resource creation failure"; }
static constexpr inline const char* __error_viewCreationFailed() noexcept { return "Texture: view creation failure"; }


// -- sampler builder -- -------------------------------------------------------

  SamplerParams::SamplerParams() noexcept {
    ZeroMemory(&_params, sizeof(D3D11_SAMPLER_DESC));
    _params.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    _params.AddressU = _params.AddressV = _params.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    _params.MaxAnisotropy = 1;
    _params.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    _params.MinLOD = 1.f;
  }
  
  void SamplerParams::_init(const TextureWrap textureWrapUVW[3], float lodMin, float lodMax) noexcept {
    ZeroMemory(&_params, sizeof(D3D11_SAMPLER_DESC));
    textureWrap(textureWrapUVW);
    _params.MinLOD = lodMin;
    _params.MaxLOD = lodMax;
  }
  
  SamplerParams& SamplerParams::borderColor(const ColorChannel rgba[4]) noexcept {
    if (rgba)
      memcpy(&_params.BorderColor[0], &rgba[0], 4u*sizeof(ColorChannel));
    else
      memset(&_params.BorderColor[0], 0, 4u*sizeof(ColorChannel));
    return *this;
  }

  // ---

  // Create sampler filter state - can be used to change sampler filter state when needed (setSamplerState)
  SamplerState SamplerBuilder::create(const SamplerParams& params) {
    ID3D11SamplerState* stateData = nullptr;
    auto result = this->_device->CreateSamplerState(&(params.descriptor()), &stateData);
    if (FAILED(result) || stateData == nullptr)
      throwError(result, "Factory: sampler error");
    return SamplerState(stateData);
  }


// -- texture params -- --------------------------------------------------------

uint32_t Texture1DParams::maxMipLevels(uint32_t width) noexcept {
  return 1u + (uint32_t)log2f((float)width + 0.01f);
}
uint32_t Texture2DParams::maxMipLevels(uint32_t width, uint32_t height) noexcept {
  uint32_t maxSize = (width >= height) ? width : height;
  return 1u + (uint32_t)log2f((float)maxSize + 0.01f);
}
uint32_t Texture3DParams::maxMipLevels(uint32_t width, uint32_t height, uint32_t depth) noexcept {
  uint32_t maxSize = (width >= height) ? width : height;
  if (depth > maxSize)
    maxSize = depth;
  return 1u + (uint32_t)log2f((float)maxSize + 0.01f);
}


// -- texture containers -- ----------------------------------------------------

  // Create 1D texture resource and view from params
  Texture1D::Texture1D(DeviceHandle device, const D3D11_TEXTURE1D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor,
                       uint32_t texelBytes, const uint8_t** initData)
    : _writeMode((descriptor.Usage != D3D11_USAGE_STAGING) 
                 ? ((descriptor.Usage != D3D11_USAGE_DYNAMIC) ? (D3D11_MAP)0 : D3D11_MAP_WRITE_DISCARD)
                 : D3D11_MAP_WRITE),
      _rowBytes((uint32_t)descriptor.Width * texelBytes),
      _mipLevels((descriptor.MipLevels != 0)
                 ? (uint8_t)descriptor.MipLevels
                 : (uint8_t)Texture1DParams::maxMipLevels(descriptor.Width)) { // throws
    HRESULT result;
    if (initData != nullptr) {
      uint32_t dataCount = descriptor.ArraySize ? this->_mipLevels * descriptor.ArraySize : this->_mipLevels;
      D3D11_SUBRESOURCE_DATA* subResData = (D3D11_SUBRESOURCE_DATA*)calloc(dataCount, sizeof(D3D11_SUBRESOURCE_DATA));
      if (subResData == nullptr)
        throw std::bad_alloc{};

      initData = &initData[dataCount - 1u];
      for (D3D11_SUBRESOURCE_DATA* it = &subResData[dataCount - 1u]; it >= subResData; --it, --initData) {
        it->pSysMem = *initData;
      }
      result = device->CreateTexture1D(&descriptor, subResData, &(this->_texture));
      free(subResData);
    }
    else
      result = device->CreateTexture1D(&descriptor, nullptr, &(this->_texture));
    
    if (FAILED(result) || this->_texture == nullptr)
      throwError(result, __error_resCreationFailed());
    if (descriptor.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
      result = device->CreateShaderResourceView(this->_texture, &viewDescriptor, &(this->_resourceView));
      if (FAILED(result) || this->_resourceView == nullptr)
        throwError(result, __error_viewCreationFailed());
    }
  }
  // Destroy 1D texture resource
  void Texture1D::release() noexcept {
    if (this->_resourceView) {
      try { this->_resourceView->Release(); } catch(...) {}
      this->_resourceView = nullptr;
    }
    if (this->_texture) {
      try { this->_texture->Release(); } catch(...) {}
      this->_texture = nullptr;
    }
  }
  
  // Create 2D texture resource and view from params
  Texture2D::Texture2D(DeviceHandle device, const D3D11_TEXTURE2D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor,
                       uint32_t texelBytes, const uint8_t** initData)
    : _writeMode((descriptor.Usage != D3D11_USAGE_STAGING) 
                 ? ((descriptor.Usage != D3D11_USAGE_DYNAMIC) ? (D3D11_MAP)0 : D3D11_MAP_WRITE_DISCARD)
                 : D3D11_MAP_WRITE),
      _rowBytes((uint32_t)descriptor.Width * texelBytes),
      _height((uint32_t)descriptor.Height),
      _mipLevels((descriptor.MipLevels != 0)
                 ? (uint8_t)descriptor.MipLevels
                 : (uint8_t)Texture2DParams::maxMipLevels(descriptor.Width, descriptor.Height)) { // throws
    HRESULT result;
    if (initData != nullptr) {
      uint32_t dataCount = descriptor.ArraySize ? this->_mipLevels * descriptor.ArraySize : this->_mipLevels;
      D3D11_SUBRESOURCE_DATA* subResData = (D3D11_SUBRESOURCE_DATA*)calloc(dataCount, sizeof(D3D11_SUBRESOURCE_DATA));
      if (subResData == nullptr)
        throw std::bad_alloc{};
      
      D3D11_SUBRESOURCE_DATA* it = subResData;
      for (uint32_t i = 0, level = 0; i < dataCount; ++i, ++level, ++it, ++initData) {
        if (level == this->_mipLevels)
          level = 0;
        it->pSysMem = *initData;
        it->SysMemPitch = (UINT)this->_rowBytes >> level;
      }
      result = device->CreateTexture2D(&descriptor, subResData, &(this->_texture));
      free(subResData);
    }
    else
      result = device->CreateTexture2D(&descriptor, nullptr, &(this->_texture));
    
    if (FAILED(result) || this->_texture == nullptr)
      throwError(result, __error_resCreationFailed());
    if (descriptor.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
      result = device->CreateShaderResourceView(this->_texture, &viewDescriptor, &(this->_resourceView));
      if (FAILED(result) || this->_resourceView == nullptr)
        throwError(result, __error_viewCreationFailed());
    }
  }
  // Destroy 2D texture resource
  void Texture2D::release() noexcept {
    if (this->_resourceView) {
      try { this->_resourceView->Release(); } catch(...) {}
      this->_resourceView = nullptr;
    }
    if (this->_texture) {
      try { this->_texture->Release(); } catch(...) {}
      this->_texture = nullptr;
    }
  }
  
  // Create 3D texture resource and view from params
  Texture3D::Texture3D(DeviceHandle device, const D3D11_TEXTURE3D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC& viewDescriptor,
                       uint32_t texelBytes, const uint8_t** initData)
    : _writeMode((descriptor.Usage != D3D11_USAGE_STAGING) 
                 ? ((descriptor.Usage != D3D11_USAGE_DYNAMIC) ? (D3D11_MAP)0 : D3D11_MAP_WRITE_DISCARD)
                 : D3D11_MAP_WRITE),
      _rowBytes((uint32_t)descriptor.Width * texelBytes),
      _height((uint32_t)descriptor.Height),
      _depth((uint32_t)descriptor.Depth),
      _mipLevels((descriptor.MipLevels != 0)
                 ? (uint8_t)descriptor.MipLevels 
                 : (uint8_t)Texture3DParams::maxMipLevels(descriptor.Width, descriptor.Height, descriptor.Depth)) { // throws
    HRESULT result;
    if (initData != nullptr) {
      D3D11_SUBRESOURCE_DATA* subResData = (D3D11_SUBRESOURCE_DATA*)calloc(this->_mipLevels, sizeof(D3D11_SUBRESOURCE_DATA));
      if (subResData == nullptr)
        throw std::bad_alloc{};

      UINT depthPitch = static_cast<UINT>(this->_rowBytes * this->_height);
      D3D11_SUBRESOURCE_DATA* it = subResData;
      for (uint32_t i = 0; i < this->_mipLevels; ++i, ++it, ++initData) {
        it->pSysMem = *initData;
        it->SysMemPitch = (UINT)this->_rowBytes >> i;
        it->SysMemSlicePitch = depthPitch >> (i+i);
      }
      result = device->CreateTexture3D(&descriptor, subResData, &(this->_texture));
      free(subResData);
    }
    else
      result = device->CreateTexture3D(&descriptor, nullptr, &(this->_texture));
    
    if (FAILED(result) || this->_texture == nullptr)
      throwError(result, __error_resCreationFailed());
    if (descriptor.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
      result = device->CreateShaderResourceView(this->_texture, &viewDescriptor, &(this->_resourceView));
      if (FAILED(result) || this->_resourceView == nullptr)
        throwError(result, __error_viewCreationFailed());
    }
  }
  // Destroy 3D texture resource
  void Texture3D::release() noexcept {
    if (this->_resourceView) {
      try { this->_resourceView->Release(); } catch(...) {}
      this->_resourceView = nullptr;
    }
    if (this->_texture) {
      try { this->_texture->Release(); } catch(...) {}
      this->_texture = nullptr;
    }
  }
  
  // ---
  
  // Create 2D render-target texture resource and view from params
  TextureTarget2D::TextureTarget2D(DeviceHandle device, D3D11_TEXTURE2D_DESC& descriptor, const D3D11_SHADER_RESOURCE_VIEW_DESC* viewDescriptor,
                                   uint32_t texelBytes, const uint8_t** initData)
    : _writeMode((descriptor.Usage != D3D11_USAGE_DYNAMIC) ? (D3D11_MAP)0 : D3D11_MAP_WRITE_DISCARD),
      _rowBytes((uint32_t)descriptor.Width * texelBytes),
      _width((uint32_t)descriptor.Width),
      _height((uint32_t)descriptor.Height),
      _mipLevels((descriptor.MipLevels != 0)
                 ? (uint8_t)descriptor.MipLevels
                 : (uint8_t)Texture2DParams::maxMipLevels(descriptor.Width, descriptor.Height)) { // throws

    D3D11_SUBRESOURCE_DATA* subResData = nullptr;
    if (initData != nullptr) {
      uint32_t dataCount = descriptor.ArraySize ? descriptor.ArraySize : 1;
      subResData = (D3D11_SUBRESOURCE_DATA*)calloc(dataCount, sizeof(D3D11_SUBRESOURCE_DATA));
      if (subResData == nullptr)
        throw std::bad_alloc{};
      
      D3D11_SUBRESOURCE_DATA* it = subResData;
      for (uint32_t i = 0, level = 0; i < dataCount; ++i, ++level, ++it, ++initData) {
        if (level == this->_mipLevels)
          level = 0;
        it->pSysMem = *initData;
        it->SysMemPitch = (UINT)this->_rowBytes >> level;
      }
    }
    
    if (viewDescriptor != nullptr) { // shader resource -> view created
      auto originalBindFlags = descriptor.BindFlags;
      auto originalMiscFlags = descriptor.MiscFlags;
      descriptor.BindFlags = (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
      if (this->_mipLevels != 1u)
        descriptor.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
      
      HRESULT result = device->CreateTexture2D(&descriptor, subResData, &(this->_texture));
      descriptor.BindFlags = originalBindFlags;
      descriptor.MiscFlags = originalMiscFlags;
      if (subResData)
        free(subResData);
      if (FAILED(result) || this->_texture == nullptr)
        throwError(result, __error_resCreationFailed());

      result = device->CreateShaderResourceView(this->_texture, viewDescriptor, &(this->_resourceView));
      if (FAILED(result) || this->_resourceView == nullptr)
        throwError(result, __error_viewCreationFailed());
    }
    else {
      auto originalBindFlags = descriptor.BindFlags;
      descriptor.BindFlags = D3D11_BIND_RENDER_TARGET;
      
      HRESULT result = device->CreateTexture2D(&descriptor, subResData, &(this->_texture));
      descriptor.BindFlags = originalBindFlags;
      if (subResData)
        free(subResData);
      if (FAILED(result) || this->_texture == nullptr)
        throwError(result, __error_resCreationFailed());
    }
    
    D3D11_RTV_DIMENSION targetDim = (descriptor.ArraySize == 1) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    CD3D11_RENDER_TARGET_VIEW_DESC targetDescriptor(targetDim, descriptor.Format);
    HRESULT result = device->CreateRenderTargetView(this->_texture, &targetDescriptor, &(this->_renderTargetView));
    if (FAILED(result) || this->_renderTargetView == nullptr)
      throwError(result, "TextureTarget: render-target not created");
  }
  // Destroy 2D render-target texture resource
  void TextureTarget2D::release() noexcept {
    if (this->_renderTargetView) {
      try { this->_renderTargetView->Release(); } catch(...) {}
      this->_renderTargetView = nullptr;
    }
    if (this->_resourceView) {
      try { this->_resourceView->Release(); } catch(...) {}
      this->_resourceView = nullptr;
    }
    if (this->_texture) {
      try { this->_texture->Release(); } catch(...) {}
      this->_texture = nullptr;
    }
  }

# ifdef _MSC_VER
#   pragma warning(pop)
# endif
#endif
