/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - rendering scene
*******************************************************************************/
#include <cstdint>
#include "resources.h"
#include "scene.h"

using namespace video_api;

// resource identifiers
#define __UI_STATE_ID     PipelineStateId::entities2D
#define __UI_SHADERS_ID   ShaderProgramId::tx2d
#define __UI_PIPELINE_ID  toGraphicsPipelineId(__UI_STATE_ID, __UI_SHADERS_ID)

#define __CRATE_STATE_ID     PipelineStateId::entities3D
#define __CRATE_SHADERS_ID   ShaderProgramId::textured
#define __CRATE_PIPELINE_ID  toGraphicsPipelineId(__CRATE_STATE_ID, __CRATE_SHADERS_ID)

#define __FLOOR_STATE_ID     PipelineStateId::entities3D
#define __FLOOR_SHADERS_ID   ShaderProgramId::shaded
#define __FLOOR_PIPELINE_ID  toGraphicsPipelineId(__FLOOR_STATE_ID, __FLOOR_SHADERS_ID)

// directional vectors (to simplify vertex normal/tangent/bitangent declaration)
#define LEFT   -1.f,0.f,0.f
#define UP     0.f,1.f,0.f
#define BACK   0.f,0.f,-1.f
#define RIGHT  1.f,0.f,0.f
#define DOWN   0.f,-1.f,0.f
#define FORWD  0.f,0.f,1.f


// -- geometry generators -- ---------------------------------------------------

static void __generateUi(Renderer& renderer, uint32_t clientWidth,
                         uint32_t clientHeight, ResourceStorage& outStorage) {
  if (outStorage.pipelines.find(__UI_PIPELINE_ID) == outStorage.pipelines.end())
    loadPipeline(renderer, __UI_STATE_ID, __UI_SHADERS_ID, 1, outStorage);
  if (outStorage.sprites.find(SpriteId::title) == outStorage.sprites.end())
    loadSprite(renderer, SpriteId::title, 160, 32, outStorage);
  if (outStorage.sprites.find(SpriteId::commands) == outStorage.sprites.end())
    loadSprite(renderer, SpriteId::commands, 128, 48, outStorage);

  float texelWidth  = 2.f/(float)clientWidth;
  float texelHeight = 2.f/(float)clientHeight;

  const float titleArea[] = { -0.9f, -0.9f + 32.f*texelHeight, -0.9f + 160.f*texelWidth, -0.9f };
  const float titleVertices[] = { titleArea[0],titleArea[1],0.f,0.f, titleArea[2],titleArea[1],1.f,0.f,
                                  titleArea[0],titleArea[3],0.f,1.f, titleArea[2],titleArea[1],1.f,0.f,
                                  titleArea[2],titleArea[3],1.f,1.f, titleArea[0],titleArea[3],0.f,1.f };
  outStorage.entities2D.push_back(SpriteEntity{
    std::make_shared<ImmutableBuffer>(renderer, BufferType::vertex, sizeof(titleVertices), &titleVertices[0]),
    SpriteId::title
  });
  const float commandsArea[] = { 0.9f - 128.f*texelWidth, -0.9f + 48.f*texelHeight, 0.9f, -0.9f };
  const float commandsVertices[] = { commandsArea[0],commandsArea[1],0.f,0.f, commandsArea[2],commandsArea[1],1.f,0.f,
                                     commandsArea[0],commandsArea[3],0.f,1.f, commandsArea[2],commandsArea[1],1.f,0.f,
                                     commandsArea[2],commandsArea[3],1.f,1.f, commandsArea[0],commandsArea[3],0.f,1.f };
  outStorage.entities2D.push_back(SpriteEntity{
    std::make_shared<ImmutableBuffer>(renderer, BufferType::vertex, sizeof(commandsVertices), &commandsVertices[0]),
    SpriteId::commands
  });
}

static void __generateCrate(Renderer& renderer, uint32_t aaSamples, ResourceStorage& outStorage) {
  if (outStorage.pipelines.find(__CRATE_PIPELINE_ID) == outStorage.pipelines.end())
    loadPipeline(renderer, __CRATE_STATE_ID, __CRATE_SHADERS_ID, aaSamples, outStorage);
  if (outStorage.textureMaps.find(TextureMapId::woodCrate) == outStorage.textureMaps.end())
    loadTexture(renderer, TextureMapId::woodCrate, outStorage);

  const float vertices[] = {
    -0.25f,0.25f,0.25f,   LEFT,BACK,DOWN,  0.f,0.f,  -0.25f,0.25f,-0.25f,  LEFT,BACK,DOWN, 1.f,0.f, // left
    -0.25f,-0.25f,0.25f,  LEFT,BACK,DOWN,  0.f,1.f,  -0.25f,-0.25f,-0.25f, LEFT,BACK,DOWN, 1.f,1.f,
    -0.25f,0.25f,0.25f,   UP,RIGHT,FORWD, 0.f,0.f,    0.25f,0.25f,0.25f,   UP,RIGHT,FORWD, 1.f,0.f, // top
    -0.25f,0.25f,-0.25f,  UP,RIGHT,FORWD, 0.f,1.f,    0.25f,0.25f,-0.25f,  UP,RIGHT,FORWD, 1.f,1.f,
    -0.25f,0.25f,-0.25f,  BACK,LEFT,DOWN,  0.f,0.f,   0.25f,0.25f,-0.25f,  BACK,LEFT,DOWN, 1.f,0.f, // front
    -0.25f,-0.25f,-0.25f, BACK,LEFT,DOWN,  0.f,1.f,   0.25f,-0.25f,-0.25f, BACK,LEFT,DOWN, 1.f,1.f,
    0.25f,0.25f,-0.25f,   RIGHT,FORWD,UP,0.f,0.f,     0.25f,0.25f,0.25f,   RIGHT,FORWD,UP, 1.f,0.f, // right
    0.25f,-0.25f,-0.25f,  RIGHT,FORWD,UP,0.f,1.f,     0.25f,-0.25f,0.25f,  RIGHT,FORWD,UP, 1.f,1.f,
    0.25f,0.25f,0.25f,    FORWD,LEFT,UP,0.f,0.f,     -0.25f,0.25f,0.25f,   FORWD,LEFT,UP,  1.f,0.f, // back
    0.25f,-0.25f,0.25f,   FORWD,LEFT,UP,0.f,1.f,     -0.25f,-0.25f,0.25f,  FORWD,LEFT,UP,  1.f,1.f,
  };
  const uint32_t indices[] = { 0,1,2, 1,3,2,  4,5,6, 5,7,6,  8,9,10, 9,11,10,  12,13,14, 13,15,14,  16,17,18, 17,19,18 };

  outStorage.entities3D.push_back(Entity{
    {std::make_shared<Mesh>(
      ImmutableBuffer(renderer, BufferType::vertex, sizeof(vertices), vertices),
      ImmutableBuffer(renderer, BufferType::vertexIndex, sizeof(indices), indices),
      static_cast<uint32_t>(sizeof(indices)/sizeof(*indices)),
      MaterialId::none, TextureMapId::woodCrate, __CRATE_PIPELINE_ID
    )},
    {0.f,0.25f,0.f}, // position
    30.f             // rotation
  });
}

static void __generateFloor(Renderer& renderer, uint32_t aaSamples, ResourceStorage& outStorage) {
  if (outStorage.pipelines.find(__FLOOR_PIPELINE_ID) == outStorage.pipelines.end())
    loadPipeline(renderer, __FLOOR_STATE_ID, __FLOOR_SHADERS_ID, aaSamples, outStorage);
  if (outStorage.materials.find(MaterialId::floor) == outStorage.materials.end())
    loadMaterial(renderer, MaterialId::floor, outStorage);

  std::vector<float> vertices;
  vertices.reserve(45*6);
  double unit = 2 * 3.14159265359 / 45.;
  for (double i = 0.; i < 2*3.14159265; i += unit) {
    vertices.emplace_back((float)cos(i) * 0.7f);
    vertices.emplace_back(0.f);
    vertices.emplace_back((float)sin(i) * 0.7f);
    vertices.emplace_back(0.f); vertices.emplace_back(1.f); vertices.emplace_back(0.f);

    vertices.emplace_back(0.f); vertices.emplace_back(0.f); vertices.emplace_back(0.f);
    vertices.emplace_back(0.f); vertices.emplace_back(1.f); vertices.emplace_back(0.f);
  }
  const uint32_t indices[] = {
    0,1,2,    2,3,4,    4,5,6,    6,7,8,    8,9,10,   10,11,12, 12,13,14, 14,15,16, 16,17,18, 18,19,20, 20,21,22,
    22,23,24, 24,25,26, 26,27,28, 28,29,30, 30,31,32, 32,33,34, 34,35,36, 36,37,38, 38,39,40, 40,41,42, 42,43,44,
    44,45,46, 46,47,48, 48,49,50, 50,51,52, 52,53,54, 54,55,56, 56,57,58, 58,59,60, 60,61,62, 62,63,64, 64,65,66,
    66,67,68, 68,69,70, 70,71,72, 72,73,74, 74,75,76, 76,77,78, 78,79,80, 80,81,82, 82,83,84, 84,85,86, 86,87,88, 88,89,0
  };

  outStorage.entities3D.push_back(Entity{
    {std::make_shared<Mesh>(
      ImmutableBuffer(renderer, BufferType::vertex, vertices.size() * sizeof(float), &vertices[0]),
      ImmutableBuffer(renderer, BufferType::vertexIndex, sizeof(indices), indices),
      static_cast<uint32_t>(sizeof(indices)/sizeof(*indices)),
      MaterialId::floor, TextureMapId::none, __FLOOR_PIPELINE_ID
    )},
    {0.f,0.f,0.f}, // position
    0.f            // rotation
  });
}


// -- rendering scene components -- --------------------------------------------

void Scene::init(RendererContext& renderer, uint32_t width, uint32_t height, uint32_t aaSamples) {
  release();
  Renderer& rendererContext = renderer.renderer();
  _renderer = &renderer;
  _isUpdated = true;

  __generateUi(rendererContext, width, height, _resources);
  __generateFloor(rendererContext, aaSamples, _resources);
  __generateCrate(rendererContext, aaSamples, _resources);
  _setCameraViewProjection(true, rendererContext, MatrixFloat4x4{});

  struct {
    PointLight pointLight = {
      {0.0f, 0.2f, 0.8f, 1.0f},
      {0.0f, 0.05f, 0.2f, 0.0f},
      {0.0f, 0.2f, 0.8f, 32.0f},
      {-1.0f, 2.5f, -1.5f, 1.0f}
    };
    DirectionalLight directionalLight = {
      {1.0f, 1.0f, 0.75f, 1.0f},
      {0.15f, 0.15f, 0.12f, 0.0f},
      {0.2f, 0.2f, 0.1f, 16.0f},
      {0.2f, -0.6f, 0.6f, 1.0f}
    };
  } lights;
  _resources.activeLights = ImmutableBuffer(rendererContext, BufferType::uniform, sizeof(lights), &lights);
}

void Scene::release() noexcept {
  if (_renderer != nullptr) {
    _resources.clear();
    _renderer = nullptr;
  }
}

// ---

void Scene::resizeScreen(uint32_t width, uint32_t height) noexcept {
  if (_renderer != nullptr) {
    Renderer& rendererContext = _renderer->renderer();
    _resources.entities2D.clear();
    __generateUi(rendererContext, width, height, _resources);
    _isUpdated = true;
  }
}

void Scene::rebuildPipelines(uint32_t width, uint32_t height, uint32_t aaSamples) {
  if (_renderer != nullptr) {
    Renderer& rendererContext = _renderer->renderer();
    _resources.entities3D.clear();
    _resources.entities2D.clear();
    _resources.pipelines.clear();
    __generateUi(rendererContext, width, height, _resources);
    __generateFloor(rendererContext, aaSamples, _resources);
    __generateCrate(rendererContext, aaSamples, _resources);
    _isUpdated = true;
  }
}

// ---

void Scene::_setCameraViewProjection(bool isInit, Renderer& renderer, MatrixFloat4x4 modelWorldMatrix) {
  struct {
    MatrixFloat4x4 world;
    MatrixFloat4x4 view;
    MatrixFloat4x4 projection;
    float position[4];
  } cameraBuffer;

  cameraBuffer.world = modelWorldMatrix;
  cameraBuffer.view = _camera.viewMatrix();
  cameraBuffer.projection = _camera.projectionMatrix();
  memcpy(cameraBuffer.position, _camera.position(), 4*sizeof(float));
  if (isInit)
    _resources.cameraViewProjection = StaticBuffer(renderer, BufferType::uniform,
                                                   sizeof(cameraBuffer), &cameraBuffer);
  else
    _resources.cameraViewProjection.write(&cameraBuffer);
}

// --> draw scene 3D entities
// --> optimization: order meshes by shader, then by texture, then by material (to limit resource changes).
// --> optimization: order polygons of a mesh from closest to farthest.
void Scene::render3D() {
  if (_renderer != nullptr) {
    _isUpdated = false;
    Renderer& renderer = _renderer->renderer();

    MaterialId curMaterial = (MaterialId)-1;
    TextureMapId curTexture = (TextureMapId)-1;
    GraphicsPipelineId curPipeline = (GraphicsPipelineId)-1;
    uint32_t curStrideBytes = 0;

    // meshes
    for (auto& model : _resources.entities3D) {
      _setCameraViewProjection(false, renderer, CameraUtils::computeWorldMatrix(model.position, model.yaw));

      for (auto& mesh : model.meshes) {
        // bind mesh pipeline
        if (mesh->pipeline != curPipeline) {
          auto& pipeline = _resources.pipelines[mesh->pipeline];
          renderer.bindGraphicsPipeline(pipeline.pipeline.handle());
          curStrideBytes = pipeline.strideBytes;
        }

        // update mesh material
        if (mesh->material != curMaterial) {
          curMaterial = mesh->material;
          if (curMaterial != MaterialId::none) {
            renderer.bindFragmentUniforms(1, _resources.materials[mesh->material].handlePtr(), 1);
          }
          else {
            BufferHandle empty = nullptr;
            renderer.bindFragmentUniforms(1, &empty, 1);
          }
        }
        // bind mesh textures
        if (mesh->texture != curTexture) {
          curTexture = mesh->texture;
          if (curTexture != TextureMapId::none) {
            auto& texture = _resources.textureMaps[mesh->texture];
            TextureView textureHandles[3] = { texture.diffuseMap.resourceView(),
                                              texture.normalMap.resourceView(),
                                              texture.specularMap.resourceView() };
            renderer.bindFragmentTextures(0, textureHandles, 3);
          }
          else
            renderer.clearFragmentTextures();
        }
        renderer.bindVertexUniforms(0, _resources.cameraViewProjection.handlePtr(), 1);
        renderer.bindFragmentUniforms(0, _resources.cameraViewProjection.handlePtr(), 1);
        renderer.bindFragmentUniforms(2, _resources.activeLights.handlePtr(), 1);

        // draw mesh
        renderer.bindVertexArrayBuffer(0, mesh->vertices.handle(), curStrideBytes);
        renderer.bindVertexIndexBuffer(mesh->indices.handle(), VertexIndexFormat::r32_ui);
        renderer.drawIndexed(mesh->indexCount);
      }
    }
  }
}

// --> draw scene 2D & UI entities
void Scene::render2D() {
  if (_renderer != nullptr) {
    Renderer& renderer = _renderer->renderer();

    // UI/sprites
    renderer.clearVertexUniforms();
    renderer.clearFragmentUniforms();
    renderer.clearFragmentTextures();
    renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui);

    auto& uiPipeline = _resources.pipelines[__UI_PIPELINE_ID];
    renderer.bindGraphicsPipeline(uiPipeline.pipeline.handle());

    SpriteId curSprite = (SpriteId)-1;
    for (auto& sprite : _resources.entities2D) {
      if (sprite.image != curSprite) {
        curSprite = sprite.image;
        TextureView spriteView = _resources.sprites[curSprite].resourceView();
        renderer.bindFragmentTextures(0, &spriteView, 1);
      }

      renderer.bindVertexArrayBuffer(0, sprite.vertices->handle(), uiPipeline.strideBytes);
      renderer.draw(6);
    }
  }
}
