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


// -- data buffer (constant/uniform, vertex, index) -- -------------------------

  BufferHandle pandora::video::d3d11::__createDataBuffer(DeviceHandle device, UINT type, size_t bufferSize,
                                                         D3D11_USAGE resourceUsage, UINT cpuAccess,
                                                         D3D11_SUBRESOURCE_DATA* resourceData) { // throws
    if (bufferSize == 0)
      throw std::invalid_argument("Buffer: size 0");
    assert(resourceUsage != D3D11_USAGE_IMMUTABLE || resourceData != nullptr);
    
    D3D11_BUFFER_DESC bufferDescriptor;
    ZeroMemory(&bufferDescriptor, sizeof(bufferDescriptor));
    bufferDescriptor.ByteWidth = (UINT)bufferSize;
    bufferDescriptor.BindFlags = type;
    bufferDescriptor.Usage = resourceUsage;
    bufferDescriptor.CPUAccessFlags = cpuAccess;
    
    BufferHandle handle = nullptr;
    auto result = device->CreateBuffer(&bufferDescriptor, resourceData, &handle);
    if (FAILED(result) || handle == nullptr)
      throwError(result, "Buffer: creation error");
    return handle;
  }
  
  bool pandora::video::d3d11::__writeMappedDataBuffer(DeviceContext context, BufferHandle buffer, size_t bufferSize,
                                                      D3D11_MAP mode, const void* sourceData) noexcept {
    assert(buffer != nullptr && sourceData != nullptr);
    
    D3D11_MAPPED_SUBRESOURCE mapped;
    ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
    if (FAILED(context->Map((ID3D11Resource*)buffer, 0, mode, 0, &mapped)) || mapped.pData == nullptr)
      return false;

    memcpy(mapped.pData, sourceData, bufferSize);
    context->Unmap((ID3D11Resource*)buffer, 0);
    return true;
  }
  
  // ---
  
  void MappedBufferIO::_open_construct(DeviceContext context, BufferHandle buffer, D3D11_MAP mode) { // throws
    auto result = context->Map((ID3D11Resource*)buffer, 0, mode, 0, &_mapped);
    if (FAILED(result) || _mapped.pData == nullptr) {
      _mapped.pData = nullptr;
      throwError(result, "MappedBufferIO: open error");
    }
  }
  bool MappedBufferIO::_reopen(DeviceContext context, BufferHandle buffer, D3D11_MAP mode) noexcept {
    close();
    this->_context = context;
    this->_buffer = buffer;
    
    ZeroMemory(&_mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
    if (FAILED(context->Map((ID3D11Resource*)buffer, 0, mode, 0, &_mapped)) || _mapped.pData == nullptr) {
      _mapped.pData = nullptr;
      return false;
    }
    return true;
  }

#endif
