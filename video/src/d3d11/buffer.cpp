/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
# include <cstring>
# include <stdexcept>
# include "video/d3d11/constant_buffer.h"
# include "video/d3d11/depth_stencil_buffer.h"
# include "video/d3d11/vertex_buffer.h"

# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
# include <system/api/windows_api.h>
# include "video/d3d11/api/d3d_11.h"
# include "video/d3d11/_private/_d3d_resource.h"

  using namespace pandora::video::d3d11;
  using namespace pandora::video;


// -- constant buffer creation -- ----------------------------------------------

  // Create constant buffer
  ConstantBuffer::ConstantBuffer(Renderer& renderer, size_t bufferSize) { // throws
    if (bufferSize == 0)
      throw std::invalid_argument("ConstantBuffer: buffer size can't be 0");
  
    D3D11_BUFFER_DESC constDescriptor = {};
    ZeroMemory(&constDescriptor, sizeof(constDescriptor));
    constDescriptor.ByteWidth = (UINT)bufferSize;
    constDescriptor.Usage = D3D11_USAGE_DEFAULT;
    constDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    
    auto result = ((ID3D11Device*)renderer.device())->CreateBuffer(&constDescriptor, nullptr, (ID3D11Buffer**)&(this->_constantBuffer));
    if (FAILED(result) || this->_constantBuffer == nullptr)
      throwError(result, "ConstantBuffer: could not create constant buffer");
  }
  
  // Create constant buffer with initial data
  ConstantBuffer::ConstantBuffer(Renderer& renderer, size_t bufferSize, const void* initData, bool isImmutable) { // throws
    if (bufferSize == 0)
      throw std::invalid_argument("ConstantBuffer: buffer size can't be 0");

    D3D11_BUFFER_DESC constDescriptor = {};
    ZeroMemory(&constDescriptor, sizeof(constDescriptor));
    constDescriptor.ByteWidth = (UINT)bufferSize;
    constDescriptor.Usage = D3D11_USAGE_DEFAULT;
    constDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (isImmutable) {
      if (initData == nullptr)
        throw std::invalid_argument("ConstantBuffer: initData can't be NULL with immutable buffers");
      constDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
    }
    else
      constDescriptor.Usage = D3D11_USAGE_DEFAULT;
    
    D3D11_SUBRESOURCE_DATA initialData;
    ZeroMemory(&initialData, sizeof(initialData));
    initialData.pSysMem = initData;
    
    auto result = ((ID3D11Device*)renderer.device())->CreateBuffer(&constDescriptor, initData ? &initialData : nullptr, (ID3D11Buffer**)&(this->_constantBuffer));
    if (FAILED(result) || this->_constantBuffer == nullptr)
      throwError(result, "ConstantBuffer: could not create constant buffer");
  }
  
  // ---

  // Destroy constant buffer
  void ConstantBuffer::release() noexcept {
    if (this->_constantBuffer) {
      try {
        ((ID3D11Buffer*)this->_constantBuffer)->Release();
        this->_constantBuffer = nullptr;
      }
      catch (...) {}
    }
  }
  
  ConstantBuffer::ConstantBuffer(ConstantBuffer&& rhs) noexcept 
    : _constantBuffer(rhs._constantBuffer) {
    rhs._constantBuffer = nullptr;
  }
  ConstantBuffer& ConstantBuffer::operator=(ConstantBuffer&& rhs) noexcept {
    release();
    this->_constantBuffer = rhs._constantBuffer;
    rhs._constantBuffer = nullptr;
    return *this;
  }
  
  // ---

  // Write buffer data
  void ConstantBuffer::write(Renderer& renderer, const void* sourceData) {
    ((ID3D11DeviceContext*)renderer.context())->UpdateSubresource((ID3D11Buffer*)this->_constantBuffer, 0, nullptr, sourceData, 0, 0);
  }


// -- depth/stencil buffer creation -- -----------------------------------------

  // Create depth/stencil buffer for existing renderer/render-target
  DepthStencilBuffer::DepthStencilBuffer(Renderer& renderer, pandora::video::ComponentFormat format, 
                                         uint32_t width, uint32_t height) { // throws
    if (width == 0 || height == 0)
      throw std::invalid_argument("DepthStencilBuffer: invalid width/height: values must not be 0");
    
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
    
    auto result = ((ID3D11Device*)renderer.device())->CreateTexture2D(&depthDescriptor, nullptr, (ID3D11Texture2D**)&(this->_depthStencilBuffer));
    if (FAILED(result) || this->_depthStencilBuffer == nullptr) {
      throwError(result, "DepthStencilBuffer: could not create depth/stencil buffer"); return;
    }
    
    // create depth/stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDescriptor;
    ZeroMemory(&depthViewDescriptor, sizeof(depthViewDescriptor));
    depthViewDescriptor.Format = depthDescriptor.Format;
    depthViewDescriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthViewDescriptor.Texture2D.MipSlice = 0;
    
    result = ((ID3D11Device*)renderer.device())->CreateDepthStencilView((ID3D11Texture2D*)this->_depthStencilBuffer, &depthViewDescriptor, 
                                                                    (ID3D11DepthStencilView**)&(this->_depthStencilView));
    if (FAILED(result) || this->_depthStencilView == nullptr)
      throwError(result, "DepthStencilBuffer: could not create depth/stencil view");
    
    this->_settings.width = width;
    this->_settings.height = height;
    this->_settings.format = format;
  }

  // Destroy depth/stencil buffer
  void DepthStencilBuffer::release() noexcept {
    if (this->_depthStencilBuffer) {
      try {
        if (this->_depthStencilView) {
          ((ID3D11DepthStencilView*)this->_depthStencilView)->Release();
          this->_depthStencilView = nullptr;
        }
        ((ID3D11Texture2D*)this->_depthStencilBuffer)->Release();
        this->_depthStencilBuffer = nullptr;
      }
      catch (...) {}
    }
  }
  
  DepthStencilBuffer::DepthStencilBuffer(DepthStencilBuffer&& rhs) noexcept 
    : _depthStencilView(rhs._depthStencilView),
      _depthStencilBuffer(rhs._depthStencilBuffer) {
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_DepthStencilBufferConfig));
    rhs._depthStencilBuffer = rhs._depthStencilView = nullptr;
  }
  DepthStencilBuffer& DepthStencilBuffer::operator=(DepthStencilBuffer&& rhs) noexcept {
    release();
    memcpy((void*)&_settings, (void*)&rhs._settings, sizeof(_DepthStencilBufferConfig));
    this->_depthStencilBuffer = rhs._depthStencilBuffer;
    this->_depthStencilView = rhs._depthStencilView;
    rhs._depthStencilBuffer = rhs._depthStencilView = nullptr;
    return *this;
  }

#endif
