/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
# include <cstring>
# include <stdexcept>
# include "video/d3d11/static_buffer.h"
# include "video/d3d11/dynamic_buffer.h"
# include "video/d3d11/depth_stencil_buffer.h"

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


// -- helpers -- ---------------------------------------------------------------

static D3D11_BIND_FLAG __toBindFlag(pandora::video::DataBufferType type) {
  switch (type) {
    case pandora::video::DataBufferType::constant: return D3D11_BIND_CONSTANT_BUFFER;
    case pandora::video::DataBufferType::vertexArray: return D3D11_BIND_VERTEX_BUFFER;
    case pandora::video::DataBufferType::vertexIndex: return D3D11_BIND_INDEX_BUFFER;
    default: return D3D11_BIND_SHADER_RESOURCE;
  }
}


// -- static buffer -- ---------------------------------------------------------

  // Create data buffer (to store data for shader stages)
  StaticBuffer::StaticBuffer(Renderer& renderer, pandora::video::DataBufferType type, size_t bufferByteSize) 
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("StaticBuffer: buffer size can't be 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    
    auto result = ((ID3D11Device*)renderer.device())->CreateBuffer(&bufferDescriptor, nullptr, (ID3D11Buffer**)&(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "StaticBuffer: could not create static buffer");
  }

  // Create data buffer (to store data for shader stages) with initial value
  StaticBuffer::StaticBuffer(Renderer& renderer, pandora::video::DataBufferType type, 
                             size_t bufferByteSize, const void* initData, bool isImmutable)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("StaticBuffer: buffer size can't be 0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    
    if (isImmutable) {
      if (initData == nullptr)
        throw std::invalid_argument("StaticBuffer: initData can't be NULL with immutable buffers");
      bufferDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
    }
    else
      bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
    
    D3D11_SUBRESOURCE_DATA subResData;
    ZeroMemory(&subResData, sizeof(subResData));
    subResData.pSysMem = initData;
    auto result = ((ID3D11Device*)renderer.device())->CreateBuffer(&bufferDescriptor, initData ? &subResData : nullptr, 
                                                                   (ID3D11Buffer**)&(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "StaticBuffer: could not create static buffer");
  }

  // ---

  // Destroy/release static buffer instance
  void StaticBuffer::release() noexcept {
    if (this->_buffer) {
      try {
        ((ID3D11Buffer*)this->_buffer)->Release();
        this->_buffer = nullptr;
      }
      catch (...) {}
    }
  }

  StaticBuffer::StaticBuffer(StaticBuffer&& rhs) noexcept
    : _buffer(rhs._buffer),
      _bufferSize(rhs._bufferSize),
      _type(rhs._type) {
    rhs._buffer = nullptr;
  }
  StaticBuffer& StaticBuffer::operator=(StaticBuffer&& rhs) noexcept {
    release();
    this->_buffer = rhs._buffer;
    this->_bufferSize = rhs._bufferSize;
    this->_type = rhs._type;
    rhs._buffer = nullptr;
    return *this;
  }

  // ---

  // Write buffer data (has no effect if buffer is immutable)
  void StaticBuffer::write(Renderer& renderer, const void* sourceData) {
    ((ID3D11DeviceContext*)renderer.context())->UpdateSubresource((ID3D11Buffer*)this->_buffer, 0, nullptr, sourceData, 0, 0);
  }


// -- dynamic buffer -- --------------------------------------------------------

  // Create data buffer (to store data for shader stages)
  DynamicBuffer::DynamicBuffer(Renderer& renderer, pandora::video::DataBufferType type, size_t bufferByteSize)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("DynamicBuffer: buffer size can't be 0");
    if (type == pandora::video::DataBufferType::constant && renderer.featureLevel() == Renderer::DeviceLevel::direct3D_11_0)
      throw std::invalid_argument("DynamicBuffer: dynamic constant buffers not supported with Direct3D 11.0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
    bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    auto result = ((ID3D11Device*)renderer.device())->CreateBuffer(&bufferDescriptor, nullptr, (ID3D11Buffer**)&(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "DynamicBuffer: could not create dynamic buffer");
  }

  // Create data buffer (to store data for shader stages) with initial value
  DynamicBuffer::DynamicBuffer(Renderer& renderer, pandora::video::DataBufferType type, 
                               size_t bufferByteSize, const void* initData)
    : _bufferSize(bufferByteSize), _type(type) {
    if (bufferByteSize == 0)
      throw std::invalid_argument("DynamicBuffer: buffer size can't be 0");
    if (type == pandora::video::DataBufferType::constant && renderer.featureLevel() == Renderer::DeviceLevel::direct3D_11_0)
      throw std::invalid_argument("DynamicBuffer: dynamic constant buffers not supported with Direct3D 11.0");
    
    D3D11_BUFFER_DESC bufferDescriptor = {};
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferByteSize;
    bufferDescriptor.BindFlags = __toBindFlag(type);
    bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
    bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    D3D11_SUBRESOURCE_DATA subResData;
    ZeroMemory(&subResData, sizeof(subResData));
    subResData.pSysMem = initData;
    auto result = ((ID3D11Device*)renderer.device())->CreateBuffer(&bufferDescriptor, initData ? &subResData : nullptr, 
                                                                   (ID3D11Buffer**)&(this->_buffer));
    if (FAILED(result) || this->_buffer == nullptr)
      throwError(result, "DynamicBuffer: could not create dynamic buffer");
  }
  
  // ---

  // Destroy/release static buffer instance
  void DynamicBuffer::release() noexcept {
    if (this->_buffer) {
      try {
        ((ID3D11Buffer*)this->_buffer)->Release();
        this->_buffer = nullptr;
      }
      catch (...) {}
    }
  }

  DynamicBuffer::DynamicBuffer(DynamicBuffer&& rhs) noexcept
    : _buffer(rhs._buffer),
      _bufferSize(rhs._bufferSize),
      _type(rhs._type) {
    rhs._buffer = nullptr;
  }
  DynamicBuffer& DynamicBuffer::operator=(DynamicBuffer&& rhs) noexcept {
    release();
    this->_buffer = rhs._buffer;
    this->_bufferSize = rhs._bufferSize;
    this->_type = rhs._type;
    rhs._buffer = nullptr;
    return *this;
  }

  // ---

  // Write buffer data and discard previous data - recommended for first write of the buffer for a frame
  bool DynamicBuffer::writeDiscard(Renderer& renderer, const void* sourceData) {
    // lock GPU access
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    auto lockResult = ((ID3D11DeviceContext*)renderer.context())->Map((ID3D11Buffer*)this->_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(lockResult) || mappedResource.pData == nullptr)
      return false;

    memcpy(mappedResource.pData, sourceData, this->_bufferSize);
    ((ID3D11DeviceContext*)renderer.context())->Unmap((ID3D11Buffer*)this->_buffer, 0);
    return true;
  }
  // Vertex/index buffers: write buffer data with no overwrite - recommended for subsequent writes of the buffer within same frame.
  // Constant buffers: same as 'writeDiscard'.
  bool DynamicBuffer::write(Renderer& renderer, const void* sourceData) {
    // lock GPU access
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    auto writeMode = (this->_type != pandora::video::DataBufferType::constant) ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD;
    auto lockResult = ((ID3D11DeviceContext*)renderer.context())->Map((ID3D11Buffer*)this->_buffer, 0, writeMode, 0, &mappedResource);
    if (FAILED(lockResult) || mappedResource.pData == nullptr)
      return false;

    memcpy(mappedResource.pData, sourceData, this->_bufferSize);
    ((ID3D11DeviceContext*)renderer.context())->Unmap((ID3D11Buffer*)this->_buffer, 0);
    return true;
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
