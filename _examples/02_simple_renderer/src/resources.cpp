/*******************************************************************************
Description : Example - rendering scene
*******************************************************************************/
#include <string>
#include <stdexcept>
#include <io/file_system_io.h>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <DDSTextureLoader.h>
#else
# include <gli/gli.hpp>
#endif
#include "resources.h"

using namespace __RENDER_API_NS;

static const char* __getResourceDir() noexcept {
  static const char* dir = (!pandora::io::verifyFileSystemAccessMode("logo_big.png", pandora::io::FileSystemAccessMode::read)
                          && pandora::io::verifyFileSystemAccessMode("../logo_big.png", pandora::io::FileSystemAccessMode::read))
                         ? "../" : "";
  return dir;
}
static const wchar_t* __getWideResourceDir() noexcept {
  static const wchar_t* dir = (!pandora::io::verifyFileSystemAccessMode("logo_big.png", pandora::io::FileSystemAccessMode::read)
                             && pandora::io::verifyFileSystemAccessMode("../logo_big.png", pandora::io::FileSystemAccessMode::read))
                            ? L"../" : L"";
  return dir;
}


// -- resource file loaders --

// --> shader compilation is too specific to have a cross-API way to do it
// D3D11 note: using D3DReflect to guess layout is much better than hardcoding it. It wasn't used here for the sake of simplicity.
void loadShader(Renderer& renderer, ShaderProgramId id, ResourceStorage& out) {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
    std::wstring vsPath = std::wstring(__getWideResourceDir()) + L"shaders/" + toWideString(id);
    std::wstring fsPath = vsPath + L".ps.hlsl";
    vsPath += L".vs.hlsl";

    D3D11_INPUT_ELEMENT_DESC layoutTextured[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    D3D11_INPUT_ELEMENT_DESC layoutShaded[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    D3D11_INPUT_ELEMENT_DESC* targetLayout = (id == ShaderProgramId::textured) ? layoutTextured : layoutShaded;
    size_t length = (id == ShaderProgramId::textured) ? sizeof(layoutTextured)/sizeof(*layoutTextured) : sizeof(layoutShaded)/sizeof(*layoutShaded);
# else
    //vulkan/openGL: not yet implemented
    //...
# endif

  auto vsBuilder = Shader::Builder::compileFromFile(ShaderType::vertex, vsPath.c_str(), "main", true);
  auto fsBuilder = Shader::Builder::compileFromFile(ShaderType::fragment, fsPath.c_str(), "main", true);
  out.shaders[id].layout = vsBuilder.createInputLayout(renderer.device(), targetLayout, length);
  out.shaders[id].vertex = vsBuilder.createShader(renderer.device());
  out.shaders[id].fragment = fsBuilder.createShader(renderer.device());
  out.shaders[id].strideBytes = (id == ShaderProgramId::textured) ? (12 + 12 + 8) : (12 + 12);
}

// ---

// --> material generator
// --> improvement: use a text file containing all material definitions for a scene/level, or read material from model files
void loadMaterial(Renderer&, MaterialId id, ResourceStorage& out) {
  switch (id) {
    case MaterialId::floor:
      out.materials[id] = Material{
        {0.01f, 0.011f, 0.012f},   // diffuse
        {0.f, 0.f, 0.f},         // ambient
        {0.5f, 0.5f, 0.5f}, 32.f // specular, shininess
      };
      break;
    default:
      out.materials[id] = Material{
        {1.f, 1.f, 1.f},     // diffuse
        {0.f, 0.f, 0.f},     // ambient
        {0.f, 0.f, 0.f}, 0.f // specular, shininess
      };
      break;
  }
}

// ---

// --> texture file loaders differ between APIs
void loadTexture(Renderer& renderer, TextureMapId id, ResourceStorage& out) {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  std::wstring texturePath = std::wstring(__getWideResourceDir()) + L"textures/" + toWideString(id);
  std::wstring normalMapPath = texturePath + L"_normal.dds";
  std::wstring specularMapPath = texturePath + L"_specular.dds";
  texturePath += L"_diffuse.dds";
  auto alphaMode = DirectX::DDS_ALPHA_MODE::DDS_ALPHA_MODE_STRAIGHT;

  ID3D11Resource* diffuse = nullptr;
  ID3D11ShaderResourceView* diffuseView = nullptr;
  if (FAILED(DirectX::CreateDDSTextureFromFileEx(renderer.device(), texturePath.c_str(), 0, D3D11_USAGE_DEFAULT,
                                                 D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &diffuse, &diffuseView, &alphaMode)))
    throw std::runtime_error(std::string("Could not load diffuse texture for ID ") + toString(id));

  ID3D11Resource* normal = nullptr;
  ID3D11ShaderResourceView* normalView = nullptr;
  if (FAILED(DirectX::CreateDDSTextureFromFileEx(renderer.device(), normalMapPath.c_str(), 0, D3D11_USAGE_DEFAULT,
                                                 D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &normal, &normalView, &alphaMode)))
    throw std::runtime_error(std::string("Could not load diffuse texture for ID ") + toString(id));

  ID3D11Resource* specular = nullptr;
  ID3D11ShaderResourceView* specularView = nullptr;
  if (FAILED(DirectX::CreateDDSTextureFromFileEx(renderer.device(), specularMapPath.c_str(), 0, D3D11_USAGE_DEFAULT,
                                                 D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &specular, &specularView, &alphaMode)))
    throw std::runtime_error(std::string("Could not load diffuse texture for ID ") + toString(id));

# else
    //vulkan/openGL: not yet implemented
    //...
# endif

  out.textureMaps[id] = { Texture2D(diffuse, diffuseView, 128*4, 128, 1),
                          Texture2D(normal, normalView, 128*4, 128, 1),
                          Texture2D(specular, specularView, 128*4, 128, 1) };
}


// -- world matrix --

MatrixFloat4 getWorldMatrix(float modelPosition[3], float yaw) {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(modelPosition[0], modelPosition[1], modelPosition[2]);
    if (yaw) {
      DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f), DirectX::XMConvertToRadians(yaw));
      return translation * rotation;
    }
    return translation;
# else
    //...
# endif
}
