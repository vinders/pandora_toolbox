/*******************************************************************************
Description : Example - rendering scene
*******************************************************************************/
#include <string>
#include <stdexcept>
#include <io/file_system_io.h>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <DDSTextureLoader.h>
# include <WICTextureLoader.h>
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

// --> shader input layouts are very specific to each API -> separate implementations
// D3D11 note: using D3DReflect to guess layout is much better than hardcoding it. It wasn't used here for the sake of simplicity.
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  static D3D11_INPUT_ELEMENT_DESC* __loadInputLayout(ShaderProgramId id, size_t& outLength, uint32_t& outStrideBytes) {
    switch (id) {
      case ShaderProgramId::tx2d: {
        static D3D11_INPUT_ELEMENT_DESC layout2D[] = {
          { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        outLength = sizeof(layout2D) / sizeof(*layout2D);
        outStrideBytes = 8 + 8;
        return layout2D;
      }
      case ShaderProgramId::shaded: {
        static D3D11_INPUT_ELEMENT_DESC layoutShaded[] = {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        outLength = sizeof(layoutShaded) / sizeof(*layoutShaded);
        outStrideBytes = 12 + 12;
        return layoutShaded;
      }
      case ShaderProgramId::textured: {
        static D3D11_INPUT_ELEMENT_DESC layoutTextured[] = {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "BITANGENT",0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        outLength = sizeof(layoutTextured) / sizeof(*layoutTextured);
        outStrideBytes = 12 + 12 + 12 + 12 + 8;
        return layoutTextured;
      }
      default: return nullptr;
    }
  }
# else
  static void* __loadInputLayout(ShaderProgramId id, size_t& outLength) {
    //vulkan/openGL: not yet implemented
    //...
    outLength = 0;
    return nullptr;
  }
# endif

// --> shader compilation
void loadShader(Renderer& renderer, ShaderProgramId id, ResourceStorage& out) {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
    std::wstring vsPath = std::wstring(__getWideResourceDir()) + L"shaders/" + toWideString(id);
    std::wstring fsPath = vsPath + L".ps.hlsl";
    vsPath += L".vs.hlsl";
# else
    //vulkan/openGL: not yet implemented
    //...
# endif
  
  size_t length = 0;
  uint32_t strideBytes = 0;
  auto* layout = __loadInputLayout(id, length, strideBytes);
  auto vsBuilder = Shader::Builder::compileFromFile(ShaderType::vertex, vsPath.c_str(), "main", true);
  auto fsBuilder = Shader::Builder::compileFromFile(ShaderType::fragment, fsPath.c_str(), "main", true);
  out.shaders[id].layout = vsBuilder.createInputLayout(renderer.device(), layout, length);
  out.shaders[id].vertex = vsBuilder.createShader(renderer.device());
  out.shaders[id].fragment = fsBuilder.createShader(renderer.device());
  out.shaders[id].strideBytes = strideBytes;
}

// ---

// --> material generator
// --> improvement: use a text file containing all material definitions for a scene/level, or read material from model files
void loadMaterial(Renderer&, MaterialId id, ResourceStorage& out) {
  switch (id) {
    case MaterialId::floor:
      out.materials[id] = Material{
        {0.012f, 0.014f, 0.017f},// diffuse
        {0.f, 0.f, 0.f},         // ambient
        {0.4f, 0.4f, 0.4f}, 16.f // specular, shininess
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
  if (FAILED(DirectX::CreateDDSTextureFromFileEx(renderer.device(), texturePath.c_str(), 0, D3D11_USAGE_IMMUTABLE,
                                                 D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &diffuse, &diffuseView, &alphaMode)))
    throw std::runtime_error(std::string("Could not load diffuse texture for ID ") + toString(id));

  ID3D11Resource* normal = nullptr;
  ID3D11ShaderResourceView* normalView = nullptr;
  if (FAILED(DirectX::CreateDDSTextureFromFileEx(renderer.device(), normalMapPath.c_str(), 0, D3D11_USAGE_IMMUTABLE,
                                                 D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &normal, &normalView, &alphaMode)))
    throw std::runtime_error(std::string("Could not load diffuse texture for ID ") + toString(id));

  ID3D11Resource* specular = nullptr;
  ID3D11ShaderResourceView* specularView = nullptr;
  if (FAILED(DirectX::CreateDDSTextureFromFileEx(renderer.device(), specularMapPath.c_str(), 0, D3D11_USAGE_IMMUTABLE,
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

// --> sprite image file loaders differ between APIs
void loadSprite(Renderer& renderer, SpriteId id, uint32_t width, uint32_t height, ResourceStorage& out) {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  std::wstring imagePath = std::wstring(__getWideResourceDir()) + L"sprites/" + toWideString(id) + L".png";
  ID3D11Resource* image = nullptr;
  ID3D11ShaderResourceView* imageView = nullptr;
  if (FAILED(DirectX::CreateWICTextureFromFileEx(renderer.device(), imagePath.c_str(), 0, D3D11_USAGE_IMMUTABLE,
                                                 D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::WIC_LOADER_FLAGS::WIC_LOADER_FORCE_RGBA32,
                                                 &image, &imageView)))
    throw std::runtime_error(std::string("Could not load sprite image for ID ") + toString(id));
# else
  //vulkan/openGL: not yet implemented
  //...
# endif
  out.sprites[id] = Texture2D(image, imageView, width*4, height, 1);
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
