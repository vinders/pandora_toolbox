/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - rendering resources (materials, textures, meshes)
*******************************************************************************/
#pragma once

#include <map>
#include <memory>
#include <vector>
#include <system/align.h>
#include <system/preprocessor_tools.h>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/renderer.h>
# include <video/d3d11/shader.h>
# include <video/d3d11/texture.h>
# include <video/d3d11/static_buffer.h>
  namespace video_api = pandora::video::d3d11;
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/renderer.h>
# include <video/vulkan/shader.h>
# include <video/vulkan/texture.h>
# include <video/vulkan/static_buffer.h>
  namespace video_api = pandora::video::vulkan;
#else
# include <video/openGL4/renderer.h>
# include <video/openGL4/shader.h>
# include <video/openGL4/texture.h>
# include <video/openGL4/static_buffer.h>
  namespace video_api = pandora::video::openGL4;
#endif
#include "camera.h"


// -- identifiers --

enum class ShaderProgramId : int {
  tx2d,
  shaded,
  textured
};
_P_SERIALIZABLE_ENUM(ShaderProgramId, tx2d, shaded, textured);

enum class MaterialId : int {
  none = 0,
  floor
};
enum class TextureMapId : int {
  none = 0,
  woodCrate
};
_P_SERIALIZABLE_ENUM(TextureMapId, none, woodCrate);

enum class SpriteId : int {
  title,
  commands
};
_P_SERIALIZABLE_ENUM(SpriteId, title, commands);


// -- types --

// material color/shininess
__align_type(16,
struct Material final {
  float diffuse[4];  // base color (when fully illuminated)
  float ambient[4];  // ambient color
  float specular[3]; // specular color
  float shininess;   // specular shininess
});

// material texture maps
struct TextureMap final {
  video_api::Texture2D diffuseMap;  // base texture
  video_api::Texture2D normalMap;   // texel normal map
  video_api::Texture2D specularMap; // specular light map
};

// vertex/fragment shader program
struct ShaderProgram final {
  video_api::InputLayout layout;
  video_api::Shader vertex;
  video_api::Shader fragment;
  uint32_t strideBytes = 0;
};

// point omni-directional light
__align_type(16,
struct PointLight final {
  float diffuse[4];
  float ambient[4];
  float specular[4];
  float position[4];
});
// uni-directional light
__align_type(16,
struct DirectionalLight final {
  float diffuse[4];
  float ambient[4];
  float specular[4];
  float direction[4];
});

// ---

// group of joined vertices (with same material)
struct Mesh final {
  video_api::StaticBuffer vertices;
  video_api::StaticBuffer indices;
  uint32_t indexCount = 0;
  MaterialId material = MaterialId::none;
  TextureMapId texture = TextureMapId::none;
  ShaderProgramId shaders = ShaderProgramId::textured;

  Mesh() = default;
  Mesh(video_api::StaticBuffer&& vertices, video_api::StaticBuffer&& indices, uint32_t indexCount,
    MaterialId material, TextureMapId texture, ShaderProgramId shaders)
    : vertices(std::move(vertices)), indices(std::move(indices)), indexCount(indexCount),
      material(material), texture(texture), shaders(shaders) {}
  Mesh(Mesh&&) noexcept = default;
  Mesh& operator=(Mesh&&) noexcept = default;
  ~Mesh() noexcept {
    vertices.release();
    indices.release();
  }
};

// model entity (meshes + position/angle)
struct Entity final {
  std::vector<std::shared_ptr<Mesh> > meshes;
  float position[3];
  float yaw;
};

// 2D/UI sprite entity
struct SpriteEntity final {
  std::shared_ptr<video_api::StaticBuffer> vertices;
  SpriteId image;
};

// ---

// Display resource storage
// --> Shader programs, materials, texture maps to bind to renderer + existing model entities
struct ResourceStorage final {
  std::map<MaterialId, Material> materials;
  std::map<TextureMapId, TextureMap> textureMaps;
  std::map<SpriteId, video_api::Texture2D> sprites;
  std::map<ShaderProgramId, ShaderProgram> shaders;
  std::vector<Entity> entities;
  std::vector<SpriteEntity> spriteEntities;
  ShaderProgram spriteShader;
  video_api::StaticBuffer cameraViewProjection;
  video_api::StaticBuffer activeMaterial;
  video_api::StaticBuffer activeLights;

  void clear() {
    entities.clear();
    sprites.clear();
    materials.clear();
    textureMaps.clear();
    shaders.clear();
    spriteShader.vertex.release();
    spriteShader.fragment.release();
    spriteShader.layout.release();
    cameraViewProjection.release();
    activeMaterial.release();
    activeLights.release();
  }
};

// ---------------------------------------------------------------------------------------

// -- resource loaders --

void loadShader(video_api::Renderer& renderer, ShaderProgramId id, ResourceStorage& out);
void loadMaterial(video_api::Renderer& renderer, MaterialId id, ResourceStorage& out);
void loadTexture(video_api::Renderer& renderer, TextureMapId id, ResourceStorage& out);
void loadSprite(video_api::Renderer& renderer, SpriteId id, uint32_t width, uint32_t height, ResourceStorage& out);
