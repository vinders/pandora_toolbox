/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - rendering resources (materials, textures, meshes)
*******************************************************************************/
#include <cassert>
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

using namespace video_api;

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

// --> graphics pipeline states generator
static void __loadPipelineStates(PipelineStateId id, GraphicsPipeline::Builder& inOutBuilder, uint32_t aaSamples) {
  switch (id) {
    case PipelineStateId::entities3D: {
      inOutBuilder.setRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::fill, true, true, false, aaSamples));
      inOutBuilder.setDepthStencilState(DepthStencilParams(StencilCompare::less, StencilOp::incrementWrap, StencilOp::keep,
                                                           StencilOp::decrementWrap, StencilOp::keep, 1));
      inOutBuilder.setBlendState(BlendParams(BlendFactor::sourceAlpha, BlendFactor::sourceInvAlpha, BlendOp::add,
                                              BlendFactor::one, BlendFactor::zero, BlendOp::add));
      inOutBuilder.setViewports(nullptr, 1, nullptr, 1, false);
      break;
    }
    case PipelineStateId::entities2D:
    default: {
      inOutBuilder.setRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::fill, true, false, false));
      inOutBuilder.setDepthStencilState(DepthStencilParams(StencilCompare::less, StencilOp::incrementWrap, StencilOp::keep,
                                                           StencilOp::decrementWrap, StencilOp::keep, 1));
      inOutBuilder.setBlendState(BlendParams(BlendFactor::sourceAlpha, BlendFactor::sourceInvAlpha, BlendOp::add,
                                              BlendFactor::one, BlendFactor::zero, BlendOp::add));
      inOutBuilder.setViewports(nullptr, 1, nullptr, 1, false);
      break;
    }
  }
}

// --> shader input layouts: very specific to each API -> separate implementations
// D3D11 note: using D3DReflect to guess layout is much better than hardcoding it.
//             It wasn't used here for the sake of simplicity.
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
  static const D3D11_INPUT_ELEMENT_DESC* __loadInputLayout(ShaderProgramId id, size_t& outLength, uint32_t& outStrideBytes) {
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
void loadShaders(Renderer& renderer, ShaderProgramId programId, ResourceStorage& out) {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
    std::wstring vsPath = std::wstring(__getWideResourceDir()) + L"shaders/" + toWideString(programId);
    std::wstring fsPath = vsPath + L".ps.hlsl";
    vsPath += L".vs.hlsl";
# else
    std::wstring vsPath = std::wstring(__getResourceDir()) + L"shaders/" + toString(programId);
    std::wstring fsPath = vsPath + L".frag";
    vsPath += L".vert";
# endif

  auto vsBuilder = Shader::Builder::compileFromFile(ShaderType::vertex, vsPath.c_str(), "main", true);
  auto fsBuilder = Shader::Builder::compileFromFile(ShaderType::fragment, fsPath.c_str(), "main", true);

  auto& outEntry = out.shaders[programId];
  size_t layoutSize = 0;
  auto layoutDesc = __loadInputLayout(programId, layoutSize, outEntry.strideBytes);
  outEntry.layout = vsBuilder.createInputLayout(renderer.resourceManager(), layoutDesc, layoutSize);
  outEntry.vertex = vsBuilder.createShader(renderer.resourceManager());
  outEntry.fragment = fsBuilder.createShader(renderer.resourceManager());
}

// ---

// --> graphics pipeline build
void loadPipeline(std::shared_ptr<Renderer>& renderer, PipelineStateId stateId,
                  ShaderProgramId programId, uint32_t aaSamples, ResourceStorage& out) {
  GraphicsPipeline::Builder builder(renderer);
  __loadPipelineStates(stateId, builder, aaSamples);

  auto shaderProgram = out.shaders.find(programId);
  if (shaderProgram == out.shaders.end()) {
    loadShaders(*renderer, programId, out);
    shaderProgram = out.shaders.find(programId);
    assert(shaderProgram != out.shaders.end());
  }

  builder.setVertexTopology(VertexTopology::triangles);
  builder.setInputLayout(shaderProgram->second.layout);
  builder.attachShaderStage(shaderProgram->second.vertex);
  builder.attachShaderStage(shaderProgram->second.fragment);

  GraphicsPipelineId pipelineId = toGraphicsPipelineId(stateId, programId);
  out.pipelines[pipelineId].pipeline = builder.build();
  out.pipelines[pipelineId].strideBytes = shaderProgram->second.strideBytes;
}

// ---

// --> material generator
// --> improvement: use a text file containing all material definitions for a scene/level, or read material from model files
void loadMaterial(Renderer& renderer, MaterialId id, ResourceStorage& out) {
  switch (id) {
    case MaterialId::floor: {
      Material materialData{
        {0.012f, 0.014f, 0.017f},// diffuse
        {0.f, 0.f, 0.f},         // ambient
        {0.4f, 0.4f, 0.4f}, 16.f // specular, shininess
      };
      out.materials[id] = ImmutableBuffer(renderer, BufferType::uniform, sizeof(Material), &materialData);
      break;
    }
    default: {
      Material materialData{
        {1.f, 1.f, 1.f},     // diffuse
        {0.f, 0.f, 0.f},     // ambient
        {0.f, 0.f, 0.f}, 0.f // specular, shininess
      };
      out.materials[id] = ImmutableBuffer(renderer, BufferType::uniform, sizeof(Material), &materialData);
      break;
    }
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
                                                 D3D11_BIND_SHADER_RESOURCE, 0, 0,
                                                 DirectX::WIC_LOADER_FLAGS::WIC_LOADER_FORCE_RGBA32,
                                                 &image, &imageView)))
    throw std::runtime_error(std::string("Could not load sprite image for ID ") + toString(id));
# else
  //vulkan/openGL: not yet implemented
  //...
# endif
  out.sprites[id] = Texture2D(image, imageView, width*4, height, 1);
}
