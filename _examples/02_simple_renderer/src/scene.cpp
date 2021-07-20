/*******************************************************************************
Description : Example - rendering scene
*******************************************************************************/
#include <cstdint>
#include "resources.h"
#include "scene.h"

using namespace __RENDER_API_NS;

// directional vectors (to simply vertex normal/tangent/bitangent declaration)
#define LEFT   -1.f,0.f,0.f
#define UP     0.f,1.f,0.f
#define BACK   0.f,0.f,-1.f
#define RIGHT  1.f,0.f,0.f
#define DOWN   0.f,-1.f,0.f
#define FORWD  0.f,0.f,1.f


// -- geometry generators -- ---------------------------------------------------

static void __generateUi(Renderer& renderer, uint32_t clientWidth, uint32_t clientHeight, ResourceStorage& outStorage) {
  if (outStorage.shaders.find(ShaderProgramId::tx2d) == outStorage.shaders.end())
    loadShader(renderer, ShaderProgramId::tx2d, outStorage);
  if (outStorage.sprites.find(SpriteId::title) == outStorage.sprites.end())
    loadSprite(renderer, SpriteId::title, 160, 32, outStorage);
  if (outStorage.sprites.find(SpriteId::commands) == outStorage.sprites.end())
    loadSprite(renderer, SpriteId::commands, 128, 48, outStorage);

  float texelWidth  = 2.f/(float)clientWidth;
  float texelHeight = 2.f/(float)clientHeight;

  const float titleArea[] = { -0.9f, -0.9f + 32.f*texelHeight, -0.9f + 160.f*texelWidth, -0.9f };
  const float titleVertices[] = { titleArea[0],titleArea[1],0.f,0.f, titleArea[2],titleArea[1],1.f,0.f, titleArea[0],titleArea[3],0.f,1.f,
                                  titleArea[2],titleArea[1],1.f,0.f, titleArea[2],titleArea[3],1.f,1.f, titleArea[0],titleArea[3],0.f,1.f };
  outStorage.spriteEntities.push_back(SpriteEntity{
    std::make_shared<StaticBuffer>(renderer, BaseBufferType::vertex, sizeof(titleVertices), &titleVertices[0], true),
    SpriteId::title
  });
  const float commandsArea[] = { 0.9f - 128.f*texelWidth, -0.9f + 48.f*texelHeight, 0.9f, -0.9f };
  const float commandsVertices[] = { commandsArea[0],commandsArea[1],0.f,0.f, commandsArea[2],commandsArea[1],1.f,0.f, commandsArea[0],commandsArea[3],0.f,1.f,
                                     commandsArea[2],commandsArea[1],1.f,0.f, commandsArea[2],commandsArea[3],1.f,1.f, commandsArea[0],commandsArea[3],0.f,1.f };
  outStorage.spriteEntities.push_back(SpriteEntity{
    std::make_shared<StaticBuffer>(renderer, BaseBufferType::vertex, sizeof(commandsVertices), &commandsVertices[0], true),
    SpriteId::commands
  });
}

static void __generateCrate(Renderer& renderer, ResourceStorage& outStorage) {
  if (outStorage.shaders.find(ShaderProgramId::textured) == outStorage.shaders.end())
    loadShader(renderer, ShaderProgramId::textured, outStorage);
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

  outStorage.entities.push_back(Entity{
    {std::make_shared<Mesh>(
      StaticBuffer(renderer, BaseBufferType::vertex, sizeof(vertices), vertices, true),
      StaticBuffer(renderer, BaseBufferType::vertexIndex, sizeof(indices), indices, true),
      static_cast<uint32_t>(sizeof(indices)/sizeof(*indices)),
      MaterialId::none, TextureMapId::woodCrate, ShaderProgramId::textured
    )},
    {0.f,0.25f,0.f}, // position
    30.f              // rotation
  });
}

static void __generateFloor(Renderer& renderer, ResourceStorage& outStorage) {
  if (outStorage.shaders.find(ShaderProgramId::shaded) == outStorage.shaders.end())
    loadShader(renderer, ShaderProgramId::shaded, outStorage);
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

  outStorage.entities.push_back(Entity{
    {std::make_shared<Mesh>(
      StaticBuffer(renderer, BaseBufferType::vertex, vertices.size() * sizeof(float), &vertices[0], true),
      StaticBuffer(renderer, BaseBufferType::vertexIndex, sizeof(indices), indices, true),
      static_cast<uint32_t>(sizeof(indices)/sizeof(*indices)),
      MaterialId::floor, TextureMapId::none, ShaderProgramId::shaded
    )},
    {0.f,0.f,0.f}, // position
    0.f            // rotation
  });
}


// -- rendering scene components -- --------------------------------------------

void Scene::init(DisplayPipeline& renderer, uint32_t width, uint32_t height) {
  release();
  _renderer = &renderer;
  _isUpdated = true;

  __generateUi(renderer.renderer(), width, height, _resources);
  __generateFloor(renderer.renderer(), _resources);
  __generateCrate(renderer.renderer(), _resources);

  _setCameraViewProjection(true, renderer.renderer(), MatrixFloat4{});
  _resources.activeMaterial = StaticBuffer(renderer.renderer(), BaseBufferType::uniform, sizeof(Material));

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
  _resources.activeLights = StaticBuffer(renderer.renderer(), BaseBufferType::uniform, sizeof(lights), &lights, true);
}

void Scene::release() noexcept {
  if (_renderer != nullptr) {
    _resources.clear();
    _renderer = nullptr;
  }
}

void Scene::resizeScreen(uint32_t width, uint32_t height) noexcept {
  if (_renderer != nullptr) {
    _resources.spriteEntities.clear();
    __generateUi(_renderer->renderer(), width, height, _resources);
    _isUpdated = true;
  }
}

// ---

void Scene::_setCameraViewProjection(bool isInit, Renderer& renderer, MatrixFloat4 modelWorldMatrix) {
  struct {
    MatrixFloat4 world;
    MatrixFloat4 view;
    MatrixFloat4 projection;
    float position[4];
  } cameraBuffer;

  cameraBuffer.world = modelWorldMatrix;
  cameraBuffer.view = _camera.viewMatrix();
  cameraBuffer.projection = _camera.projectionMatrix();
  memcpy(cameraBuffer.position, _camera.position(), 4*sizeof(float));
  if (isInit)
    _resources.cameraViewProjection = StaticBuffer(renderer, BaseBufferType::uniform, sizeof(cameraBuffer), &cameraBuffer, false);
  else
    _resources.cameraViewProjection.write(renderer, &cameraBuffer);
}

// --> draw scene
// --> optimization: order meshes by shader, then by texture, then by material (to limit resource changes).
// --> optimization: order polygons of a mesh from closest to farthest.
void Scene::render() {
  if (_renderer != nullptr) {
    _isUpdated = false;
    auto& renderer = _renderer->renderer();

    ShaderProgramId curShader = (ShaderProgramId)-1;
    MaterialId curMaterial = (MaterialId)-1;
    TextureMapId curTexture = (TextureMapId)-1;
    renderer.setVertexTopology(VertexTopology::triangles);

    // meshes
    for (auto& model : _resources.entities) {
      _setCameraViewProjection(false, renderer, getWorldMatrix(model.position, model.yaw));

      for (auto& mesh : model.meshes) {
        auto& shaders = _resources.shaders[mesh->shaders];

        // bind mesh shader
        if (mesh->shaders != curShader) {
          curShader = mesh->shaders;
          renderer.bindInputLayout(shaders.layout.handle());
          renderer.bindVertexShader(shaders.vertex.handle());
          renderer.bindFragmentShader(shaders.fragment.handle());
        }
        // update mesh material
        if (mesh->material != curMaterial) {
          curMaterial = mesh->material;
          if (curMaterial != MaterialId::none) {
            auto& material = _resources.materials[mesh->material];
            _resources.activeMaterial.write(renderer, &material);
            renderer.bindFragmentUniforms(1, _resources.activeMaterial.handleArray(), 1);
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
        renderer.bindVertexUniforms(0, _resources.cameraViewProjection.handleArray(), 1);
        renderer.bindFragmentUniforms(0, _resources.cameraViewProjection.handleArray(), 1);
        renderer.bindFragmentUniforms(2, _resources.activeLights.handleArray(), 1);

        // draw mesh
        renderer.bindVertexArrayBuffer(0, mesh->vertices.handle(), shaders.strideBytes);
        renderer.bindVertexIndexBuffer(mesh->indices.handle(), VertexIndexFormat::r32_ui);
        renderer.drawIndexed(mesh->indexCount);
      }
    }

    // UI/sprites
    renderer.clearVertexUniforms();
    renderer.clearFragmentUniforms();
    renderer.clearFragmentTextures();
    renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui);

    auto& uiShaders = _resources.shaders[ShaderProgramId::tx2d];
    renderer.bindInputLayout(uiShaders.layout.handle());
    renderer.bindVertexShader(uiShaders.vertex.handle());
    renderer.bindFragmentShader(uiShaders.fragment.handle());

    SpriteId curSprite = (SpriteId)-1;
    for (auto& sprite : _resources.spriteEntities) {
      if (sprite.image != curSprite) {
        curSprite = sprite.image;
        TextureView spriteView = _resources.sprites[curSprite].resourceView();
        renderer.bindFragmentTextures(0, &spriteView, 1);
      }

      renderer.bindVertexArrayBuffer(0, sprite.vertices->handle(), uiShaders.strideBytes);
      renderer.draw(6);
    }
  }
}
