/*******************************************************************************
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
# ifndef __RENDER_API
#   define __RENDER_API(className) pandora::video::d3d11:: className
#   define __RENDER_API_NS pandora::video::d3d11
# endif

#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/renderer.h>
# include <video/vulkan/shader.h>
# include <video/vulkan/texture.h>
# include <video/vulkan/static_buffer.h>
# ifndef __RENDER_API
#   define __RENDER_API(className) pandora::video::vulkan:: className
#   define __RENDER_API_NS pandora::video::vulkan
# endif

#else
# include <video/openGL4/renderer.h>
# include <video/openGL4/shader.h>
# include <video/openGL4/texture.h>
# include <video/openGL4/static_buffer.h>
# ifndef __RENDER_API
#   define __RENDER_API(className) pandora::video::openGL4:: className
#   define __RENDER_API_NS pandora::video::openGL4
# endif
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
  __RENDER_API(Texture2D) diffuseMap;  // base texture
  __RENDER_API(Texture2D) normalMap;   // texel normal map
  __RENDER_API(Texture2D) specularMap; // specular light map
};

// vertex/fragment shader program
struct ShaderProgram final {
  __RENDER_API(InputLayout) layout;
  __RENDER_API(Shader) vertex;
  __RENDER_API(Shader) fragment;
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
  __RENDER_API(StaticBuffer) vertices;
  __RENDER_API(StaticBuffer) indices;
  uint32_t indexCount = 0;
  MaterialId material = MaterialId::none;
  TextureMapId texture = TextureMapId::none;
  ShaderProgramId shaders = ShaderProgramId::textured;

  Mesh() = default;
  Mesh(__RENDER_API(StaticBuffer)&& vertices, __RENDER_API(StaticBuffer)&& indices, uint32_t indexCount,
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
  std::shared_ptr<__RENDER_API(StaticBuffer)> vertices;
  SpriteId image;
};

// ---

// Display resource storage
// --> Shader programs, materials, texture maps to bind to renderer + existing model entities
struct ResourceStorage final {
  std::map<MaterialId, Material> materials;
  std::map<TextureMapId, TextureMap> textureMaps;
  std::map<SpriteId, __RENDER_API(Texture2D)> sprites;
  std::map<ShaderProgramId, ShaderProgram> shaders;
  std::vector<Entity> entities;
  std::vector<SpriteEntity> spriteEntities;
  ShaderProgram spriteShader;
  __RENDER_API(StaticBuffer) cameraViewProjection;
  __RENDER_API(StaticBuffer) activeMaterial;
  __RENDER_API(StaticBuffer) activeLights;

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

void loadShader(__RENDER_API(Renderer)& renderer, ShaderProgramId id, ResourceStorage& out);
void loadMaterial(__RENDER_API(Renderer)& renderer, MaterialId id, ResourceStorage& out);
void loadTexture(__RENDER_API(Renderer)& renderer, TextureMapId id, ResourceStorage& out);
void loadSprite(__RENDER_API(Renderer)& renderer, SpriteId id, uint32_t width, uint32_t height, ResourceStorage& out);

// -- world matrix --

MatrixFloat4 getWorldMatrix(float modelPosition[3], float yaw);
