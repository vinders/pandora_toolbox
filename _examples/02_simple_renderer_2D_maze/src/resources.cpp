/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - rendering resources (materials, textures, meshes)
*******************************************************************************/
#include "file_io.h"
#include "resources.h"

using namespace video_api;

// -- resource file loaders --

// --> shader input layouts: specific to each API
static InputLayout __loadInputLayout(DeviceResourceManager context,
                                     Shader::Builder& vsBuilder, uint32_t& outStrideBytes) {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
    static D3D11_INPUT_ELEMENT_DESC layout2D[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    outStrideBytes = 8 + 8;
    return vsBuilder.createInputLayout(context, layout2D, sizeof(layout2D)/sizeof(*layout2D));
# else
    // vulkan: not yet implemented...
    return nullptr;
# endif
}

// ---

// --> graphics pipeline build
void loadPipeline(Renderer& renderer, ResourceStorage& out) {
  GraphicsPipeline::Builder builder(renderer);
  builder.setRasterizerState(RasterizerParams(CullMode::none, FillMode::fill, true, false, false));
  builder.setDepthStencilState(DepthStencilParams(false, false));
  builder.setBlendState(BlendParams(BlendFactor::sourceAlpha, BlendFactor::sourceInvAlpha, BlendOp::add,
                                    BlendFactor::one, BlendFactor::zero, BlendOp::add));
  builder.setViewports(nullptr, 1, nullptr, 1, false); // dynamic viewports

  // load shaders
  if (out.shaders.vertex.isEmpty()) {
    auto vsPath = getShaderDirectory() + __VERT_SHADER;
    auto fsPath = getShaderDirectory() + __FRAG_SHADER;
    auto vsBuilder = Shader::Builder::compileFromFile(ShaderType::vertex, vsPath.c_str(), "main");
    auto fsBuilder = Shader::Builder::compileFromFile(ShaderType::fragment, fsPath.c_str(), "main");
    out.shaders.layout = __loadInputLayout(renderer.resourceManager(), vsBuilder, out.shaders.strideBytes);
    out.shaders.vertex = vsBuilder.createShader(renderer.resourceManager());
    out.shaders.fragment = fsBuilder.createShader(renderer.resourceManager());
  }

  // bind shaders
  builder.setVertexTopology(VertexTopology::triangles);
  builder.setInputLayout(out.shaders.layout);
  builder.attachShaderStage(out.shaders.vertex);
  builder.attachShaderStage(out.shaders.fragment);

  out.pipeline = builder.build();
}

// ---

// --> texture/sprite image file loaders differ between APIs
void loadImage(Renderer& renderer, ImageId id, uint32_t width, uint32_t height, ResourceStorage& out) {
  TextureHandle image = nullptr;
  TextureView imageView = nullptr;

  auto imagePath = getResourceDirectory() + _RESOURCE_PATH("images/")
                   + _RESOURCE_STRING(id) + _RESOURCE_PATH(".png");
  readImageFile(imagePath.c_str(), id, renderer.device(), &image, &imageView);
  out.images[id] = Texture2D(image, imageView, width*4, height, 1);
}
