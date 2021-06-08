/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <cstddef>
# include <string>
# include <stdexcept>
# include "video/d3d11/shader.h"

# define NOMINMAX
# define NODRAWTEXT
# define NOGDI
# define NOBITMAP
# define NOMCX
# define NOSERVICE
# include "video/d3d11/api/d3d_11.h"
# include <D3DCompiler.h>
# include "video/d3d11/_private/_d3d_resource.h"

  using namespace pandora::video::d3d11;


// -- errors -- ----------------------------------------------------------------

  // Throw shader-specific error message
  static void __throwShaderError(ID3DBlob* errorMessage, const char* messagePrefix, const char* shaderInfo) {
    std::string message(messagePrefix);
    message += " (";
    message += shaderInfo;
    message += "): ";
    if (errorMessage) {
      message += (const char*)errorMessage->GetBufferPointer();
      errorMessage->Release();
    }
    else
      message += "missing or empty shader file/content";
    throw std::runtime_error(std::move(message));
  }


// -- shader -- ----------------------------------------------------------------

  Shader::~Shader() noexcept {
    if (this->_handle != nullptr) {
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
  }


// -- create/compile shaders -- ------------------------------------------------

  // Create shader builder from byte-code buffer
  Shader::Builder::Builder(ShaderType type, void* shaderBuffer)
    : _shaderBuffer(shaderBuffer),
      _data(static_cast<const uint8_t*>(((ID3DBlob*)_shaderBuffer)->GetBufferPointer())),
      _length(static_cast<size_t>(((ID3DBlob*)_shaderBuffer)->GetBufferSize())),
      _type(type) {}
  // Release byte-code buffer
  Shader::Builder::~Builder() noexcept {
    if (_shaderBuffer != nullptr)
      ((ID3DBlob*)_shaderBuffer)->Release();
  }

  // ---

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
      __throwShaderError(errorMessage, "Shader: text compile error", shaderModel);
    return Shader::Builder(type, (void*)shaderBuffer);
  }
  // Compile shader from text file
  Shader::Builder Shader::Builder::compileFromFile(pandora::video::ShaderType type, const wchar_t* filePath, const char* entryPoint, bool isStrict) {
    
    ID3DBlob* errorMessage = nullptr;
    ID3DBlob* shaderBuffer = nullptr;
    const char* shaderModel = __getShaderModel(type);
    HRESULT result = D3DCompileFromFile(filePath, nullptr, nullptr, entryPoint, shaderModel, 
                                        isStrict ? D3DCOMPILE_ENABLE_STRICTNESS : 0, 0, &shaderBuffer, &errorMessage);
    if (FAILED(result))
      __throwShaderError(errorMessage, "Shader: file compile error", shaderModel);
    return Shader::Builder(type, (void*)shaderBuffer);
  }


// -- create shader objects -- -------------------------------------------------

  // Create shader object
  Shader Shader::Builder::createShader(Shader::DeviceHandle device) const {
    HRESULT result;
    Shader::Handle handle = nullptr;
    switch (this->_type) {
      case pandora::video::ShaderType::vertex:
        result = ((ID3D11Device*)device)->CreateVertexShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11VertexShader**)&handle);
        break;
      case pandora::video::ShaderType::tesselControl:
        result = ((ID3D11Device*)device)->CreateHullShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11HullShader**)&handle);
        break;
      case pandora::video::ShaderType::tesselEval:
        result = ((ID3D11Device*)device)->CreateDomainShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11DomainShader**)&handle);
        break;
      case pandora::video::ShaderType::geometry:
        result = ((ID3D11Device*)device)->CreateGeometryShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11GeometryShader**)&handle);
        break;
      case pandora::video::ShaderType::fragment:
        result = ((ID3D11Device*)device)->CreatePixelShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11PixelShader**)&handle);
        break;
      default:
        result = ((ID3D11Device*)device)->CreateComputeShader((const void*)this->_data, (SIZE_T)this->_length, nullptr, (ID3D11ComputeShader**)&handle);
        break;
    }

    if (FAILED(result) || handle == nullptr)
      throwError(result, "Shader.Builder: failed to create shader object");
    return Shader(handle, this->_type);
  }
  
  // ---

  // Create input layout for shader object
  ShaderInputLayout Shader::Builder::createInputLayout(Shader::DeviceHandle device, Shader::InputElementDescArray layoutElements, size_t length) const {
    ID3D11InputLayout* inputLayout = nullptr;
    HRESULT result = ((ID3D11Device*)device)->CreateInputLayout((D3D11_INPUT_ELEMENT_DESC*)layoutElements, (UINT)length, 
                                                                (const void*)this->_data, (SIZE_T)this->_length, &inputLayout);
    if (FAILED(result) || inputLayout == nullptr)
      throwError(result, "Shader.Builder: failed to create specified input layout");
    return ShaderInputLayout((ShaderInputLayout::Handle)inputLayout);
  }
  
  // ---
  
  ShaderInputLayout::~ShaderInputLayout() noexcept {
    if (this->_handle)
      ((ID3D11InputLayout*)this->_handle)->Release();
  }

// ACTIVATION -> à placer dans Renderer
/*
void Shader::Use()
{
	m_Renderer->GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	m_Renderer->GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	m_Renderer->GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);
}
void LightShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}
*/
#endif
