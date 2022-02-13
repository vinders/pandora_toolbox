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
#include "file_io.h"
#include "resources.h"

using namespace video_api;

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

// --> shader input layouts: specific to each API
// D3D11 note: using D3DReflect to guess layout is much better than hardcoding it.
//             It wasn't used here for the sake of simplicity.
static InputLayout __loadInputLayout(ShaderProgramId id, DeviceResourceManager context,
                                     Shader::Builder& vsBuilder, uint32_t& outStrideBytes) {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
    switch (id) {
      case ShaderProgramId::tx2d: {
        static D3D11_INPUT_ELEMENT_DESC layout2D[] = {
          { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        outStrideBytes = 8 + 8;
        return vsBuilder.createInputLayout(context, layout2D, sizeof(layout2D)/sizeof(*layout2D));
      }
      case ShaderProgramId::shaded: {
        static D3D11_INPUT_ELEMENT_DESC layoutShaded[] = {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        outStrideBytes = 12 + 12;
        return vsBuilder.createInputLayout(context, layoutShaded, sizeof(layoutShaded)/sizeof(*layoutShaded));
      }
      case ShaderProgramId::textured: {
        static D3D11_INPUT_ELEMENT_DESC layoutTextured[] = {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "BITANGENT",0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        outStrideBytes = 12 + 12 + 12 + 12 + 8;
        return vsBuilder.createInputLayout(context, layoutTextured, sizeof(layoutTextured)/sizeof(*layoutTextured));
      }
      default: return nullptr;
    }
# else
    // vulkan: not yet implemented...
    return nullptr;
# endif
}

// --> shader compilation
static void __loadShaders(Renderer& renderer, ShaderProgramId programId, ResourceStorage& out) {
  auto vsPath = getShaderDirectory() + _RESOURCE_STRING(programId);
  auto fsPath = vsPath + __FRAG_SHADER_SUFFIX;
  vsPath += __VERT_SHADER_SUFFIX;

  auto& outEntry = out.shaders[programId];
# ifdef _P_SHADER_PRECOMPILED
    std::unique_ptr<uint8_t[]> fileBuffer = nullptr;
    size_t fileLength = 0;

    readBinaryShaderFile(vsPath.c_str(), fileBuffer, fileLength);
    auto vsBuilder = Shader::Builder(ShaderType::vertex, fileBuffer.get(), fileLength);
    outEntry.layout = __loadInputLayout(programId, renderer.resourceManager(), vsBuilder, outEntry.strideBytes);
    outEntry.vertex = vsBuilder.createShader(renderer.resourceManager());

    readBinaryShaderFile(fsPath.c_str(), fileBuffer, fileLength); // overwrite buffer -> vsBuilder can no longer be used
    auto fsBuilder = Shader::Builder(ShaderType::fragment, fileBuffer.get(), fileLength);
    outEntry.fragment = fsBuilder.createShader(renderer.resourceManager());
# else
    auto vsBuilder = Shader::Builder::compileFromFile(ShaderType::vertex, vsPath.c_str(), "main");
    auto fsBuilder = Shader::Builder::compileFromFile(ShaderType::fragment, fsPath.c_str(), "main");
    outEntry.layout = __loadInputLayout(programId, renderer.resourceManager(), vsBuilder, outEntry.strideBytes);
    outEntry.vertex = vsBuilder.createShader(renderer.resourceManager());
    outEntry.fragment = fsBuilder.createShader(renderer.resourceManager());
# endif
}

// ---

// --> graphics pipeline build
void loadPipeline(std::shared_ptr<Renderer>& renderer, PipelineStateId stateId,
                  ShaderProgramId programId, uint32_t aaSamples, ResourceStorage& out) {
  GraphicsPipeline::Builder builder(renderer);
  __loadPipelineStates(stateId, builder, aaSamples);

  auto shaderProgram = out.shaders.find(programId);
  if (shaderProgram == out.shaders.end()) {
    __loadShaders(*renderer, programId, out);
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
  TextureHandle diffuse = nullptr,   normal = nullptr,     specular = nullptr;
  TextureView diffuseView = nullptr, normalView = nullptr, specularView = nullptr;

  try {
    auto textureBasePath = getResourceDirectory() + _RESOURCE_PATH("textures/") + _RESOURCE_STRING(id);
    readTextureFile(textureBasePath, _RESOURCE_PATH("_diffuse.dds"), id, renderer.device(), &diffuse, &diffuseView);
    out.textureMaps[id].diffuseMap = Texture2D(diffuse, diffuseView, 128 * 4, 128, 1);
    readTextureFile(textureBasePath, _RESOURCE_PATH("_normal.dds"), id, renderer.device(), &normal, &normalView);
    out.textureMaps[id].normalMap = Texture2D(normal, normalView, 128 * 4, 128, 1);
    readTextureFile(textureBasePath, _RESOURCE_PATH("_specular.dds"), id, renderer.device(), &specular, &specularView);
    out.textureMaps[id].specularMap = Texture2D(specular, specularView, 128 * 4, 128, 1);
  }
  catch (...) { out.textureMaps.erase(id); throw; }
}

// --> sprite image file loaders differ between APIs
void loadSprite(Renderer& renderer, SpriteId id, uint32_t width, uint32_t height, ResourceStorage& out) {
  TextureHandle image = nullptr;
  TextureView imageView = nullptr;

  auto imagePath = getResourceDirectory() + _RESOURCE_PATH("sprites/") + _RESOURCE_STRING(id) + _RESOURCE_PATH(".png");
  readSpriteFile(imagePath.c_str(), id, renderer.device(), &image, &imageView);
  out.sprites[id] = Texture2D(image, imageView, width*4, height, 1);
}
