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


// -- create/compile shaders -- ------------------------------------------------

# ifdef _P_VIDEO_SHADER_COMPILERS
    // Get D3D11 shader model ID
    static const char* __getShaderModel(ShaderType type) noexcept {
      switch (type) {
        case ShaderType::vertex:   return "vs_5_0";
#       ifndef __P_DISABLE_TESSELLATION_STAGE
          case ShaderType::tessCtrl: return "hs_5_0";
          case ShaderType::tessEval: return "ds_5_0";
#       endif
#       ifndef __P_DISABLE_GEOMETRY_STAGE
          case ShaderType::geometry: return "gs_5_0";
#       endif
        case ShaderType::fragment: return "ps_5_0";
        case ShaderType::compute:  return "cs_5_0";
        default: return "";
      }
    }

    // Compile shader from text content
    Shader::Builder Shader::Builder::compile(ShaderType type, const char* textContent, size_t length,
                                             const char* entryPoint, bool isStrict) {
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
    Shader::Builder Shader::Builder::compileFromFile(ShaderType type, const wchar_t* filePath,
                                                     const char* entryPoint, bool isStrict) {
      ID3DBlob* errorMessage = nullptr;
      ID3DBlob* shaderBuffer = nullptr;
      const char* shaderModel = __getShaderModel(type);
      HRESULT result = D3DCompileFromFile(filePath, nullptr, nullptr, entryPoint, shaderModel, 
                                          isStrict ? D3DCOMPILE_ENABLE_STRICTNESS : 0, 0, &shaderBuffer, &errorMessage);
      if (FAILED(result))
        throwShaderError(errorMessage, "Shader: file/compile error", shaderModel);
      return Shader::Builder(type, shaderBuffer);
    }
# endif


// -- create shader objects -- -------------------------------------------------

  // Create shader object
  Shader Shader::Builder::createShader(DeviceResourceManager device) const {
    HRESULT result;
    Shader::Handle handle = nullptr;
    switch (this->_type) {
      case ShaderType::vertex:
        result = device->CreateVertexShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11VertexShader**)&handle);
        break;
#     ifndef __P_DISABLE_TESSELLATION_STAGE
        case ShaderType::tessCtrl:
          result = device->CreateHullShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11HullShader**)&handle);
          break;
        case ShaderType::tessEval:
          result = device->CreateDomainShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11DomainShader**)&handle);
          break;
#     endif
#     ifndef __P_DISABLE_GEOMETRY_STAGE
        case ShaderType::geometry:
          result = device->CreateGeometryShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11GeometryShader**)&handle);
          break;
#     endif
      case ShaderType::fragment:
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
        ((ID3D11DeviceChild*)this->_handle)->Release();
        this->_handle = nullptr;
      }
      catch (...) {}
    }
  }

  Shader& Shader::operator=(const Shader& rhs) noexcept {
    release();
    this->_handle = rhs._handle;
    this->_type = rhs._type;
    if (rhs._handle != nullptr)
      ((ID3D11DeviceChild*)rhs._handle)->AddRef();
    return *this;
  }
  Shader& Shader::operator=(Shader&& rhs) noexcept {
    release();
    this->_handle = rhs._handle;
    this->_type = rhs._type;
    rhs._handle = nullptr;
    return *this; 
  }
  
  // ---

  // Create input layout for shader object
  InputLayout Shader::Builder::createInputLayout(DeviceResourceManager device, D3D11_INPUT_ELEMENT_DESC* layoutElements, size_t length) const {
    ID3D11InputLayout* inputLayout = nullptr;
    HRESULT result = device->CreateInputLayout((D3D11_INPUT_ELEMENT_DESC*)layoutElements, (UINT)length, 
                                               (const void*)this->_data, (SIZE_T)this->_length, &inputLayout);
    if (FAILED(result) || inputLayout == nullptr)
      throwError(result, "Shader: layout creation error");
    return InputLayout((InputLayoutHandle)inputLayout);
  }

#endif
