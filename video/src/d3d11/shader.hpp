/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Implementation included in renderer.cpp
(grouped object improves compiler optimizations + greatly reduces executable size)
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
// includes + namespaces: in renderer.cpp


// -- create/compile shaders -- ------------------------------------------------

  // Get D3D11 shader model ID
  static const char* __getShaderModel(pandora::video::ShaderType type) {
    switch (type) {
      case pandora::video::ShaderType::vertex:        return "vs_5_0"; break;
      case pandora::video::ShaderType::tesselControl: return "hs_5_0"; break;
      case pandora::video::ShaderType::tesselEval:    return "ds_5_0"; break;
      case pandora::video::ShaderType::geometry:      return "gs_5_0"; break;
      case pandora::video::ShaderType::fragment:      return "ps_5_0"; break;
      default: return "cs_5_0"; break;
    }
  }

  // Compile shader from text content
  Shader::Builder Shader::Builder::compile(pandora::video::ShaderType type, const char* textContent, size_t length, const char* entryPoint, bool isStrict) {
    ID3DBlob* errorMessage = nullptr;
    ID3DBlob* shaderBuffer = nullptr;
    const char* shaderModel = __getShaderModel(type);
    HRESULT result = D3DCompile((LPCVOID)textContent, (SIZE_T)length, nullptr, nullptr, nullptr, entryPoint, shaderModel, 
                                isStrict ? D3DCOMPILE_ENABLE_STRICTNESS : 0, 0, &shaderBuffer, &errorMessage);
    if (FAILED(result))
      throwShaderError(errorMessage, "Shader: compile error", shaderModel);
    return Shader::Builder(type, shaderBuffer);
  }
  // Compile shader from text file
  Shader::Builder Shader::Builder::compileFromFile(pandora::video::ShaderType type, const wchar_t* filePath, const char* entryPoint, bool isStrict) {
    
    ID3DBlob* errorMessage = nullptr;
    ID3DBlob* shaderBuffer = nullptr;
    const char* shaderModel = __getShaderModel(type);
    HRESULT result = D3DCompileFromFile(filePath, nullptr, nullptr, entryPoint, shaderModel, 
                                        isStrict ? D3DCOMPILE_ENABLE_STRICTNESS : 0, 0, &shaderBuffer, &errorMessage);
    if (FAILED(result))
      throwShaderError(errorMessage, "Shader: file/compile error", shaderModel);
    return Shader::Builder(type, shaderBuffer);
  }


// -- create shader objects -- -------------------------------------------------

  // Create shader object
  Shader Shader::Builder::createShader(Shader::DeviceHandle device) const {
    HRESULT result;
    Shader::Handle handle = nullptr;
    switch (this->_type) {
      case pandora::video::ShaderType::vertex:
        result = device->CreateVertexShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11VertexShader**)&handle);
        break;
      case pandora::video::ShaderType::tesselControl:
        result = device->CreateHullShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11HullShader**)&handle);
        break;
      case pandora::video::ShaderType::tesselEval:
        result = device->CreateDomainShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11DomainShader**)&handle);
        break;
      case pandora::video::ShaderType::geometry:
        result = device->CreateGeometryShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11GeometryShader**)&handle);
        break;
      case pandora::video::ShaderType::fragment:
        result = device->CreatePixelShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11PixelShader**)&handle);
        break;
      default:
        result = device->CreateComputeShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11ComputeShader**)&handle);
        break;
    }

    if (FAILED(result) || handle == nullptr)
      throwError(result, "Shader: creation error");
    return Shader(handle, this->_type);
  }
  
  // Destroy shader object
  void Shader::release() noexcept {
    if (this->_handle != nullptr) {
      try {
        switch (this->_type) {
          case pandora::video::ShaderType::vertex:        ((ID3D11VertexShader*)this->_handle)->Release(); break;
          case pandora::video::ShaderType::tesselControl: ((ID3D11HullShader*)this->_handle)->Release(); break;
          case pandora::video::ShaderType::tesselEval:    ((ID3D11DomainShader*)this->_handle)->Release(); break;
          case pandora::video::ShaderType::geometry:      ((ID3D11GeometryShader*)this->_handle)->Release(); break;
          case pandora::video::ShaderType::fragment:      ((ID3D11PixelShader*)this->_handle)->Release(); break;
          default: ((ID3D11ComputeShader*)this->_handle)->Release(); break;
        }
        this->_handle = nullptr;
      }
      catch (...) {}
    }
  }
  
  // ---

  // Create input layout for shader object
  ShaderInputLayout Shader::Builder::createInputLayout(Shader::DeviceHandle device, D3D11_INPUT_ELEMENT_DESC* layoutElements, size_t length) const {
    ID3D11InputLayout* inputLayout = nullptr;
    HRESULT result = device->CreateInputLayout((D3D11_INPUT_ELEMENT_DESC*)layoutElements, (UINT)length, 
                                                                (const void*)this->_data, (SIZE_T)this->_length, &inputLayout);
    if (FAILED(result) || inputLayout == nullptr)
      throwError(result, "Shader: layout creation error");
    return ShaderInputLayout((ShaderInputLayout::Handle)inputLayout);
  }

#endif
