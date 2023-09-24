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
#include <vector>
#include <system/align.h>
#include <system/preprocessor_tools.h>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/graphics_pipeline.h>
# include <video/d3d11/texture.h>
# include <video/d3d11/buffer.h>
  namespace video_api = pandora::video::d3d11;
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/graphics_pipeline.h>
# include <video/vulkan/texture.h>
# include <video/vulkan/buffer.h>
  namespace video_api = pandora::video::vulkan;
#endif


// -- identifiers --

enum class ImageId : int {
  title = 0,
  hedge,
  grass,
  player
};
_P_SERIALIZABLE_ENUM(ImageId, title, hedge, grass, player);


// -- types --

// vertex/fragment shader program
struct ShaderProgram final {
  video_api::InputLayout layout;
  video_api::Shader vertex;
  video_api::Shader fragment;
  uint32_t strideBytes = 0;
};

// 2D entity
struct Entity2D final {
  video_api::ImmutableBuffer vertices; // list of tiles with same image, triangle per triangle
  video_api::ImmutableBuffer indices;
  uint32_t indexCount = 0;
  ImageId image; // image associated with each tile
};


// ---

// Display resource storage
// --> Graphics pipelines, materials, texture maps to bind to renderer + existing model entities
struct ResourceStorage final {
  std::map<ImageId, video_api::Texture2D> images;
  ShaderProgram shaders;

  video_api::GraphicsPipeline pipeline;
  std::vector<Entity2D> entities;

  video_api::ImmutableBuffer worldOffset; // --> set to static if world can be updated (e.g.: scroll)
  video_api::StaticBuffer playerOffset;
  video_api::StagingBuffer playerOffsetStaging;

  void clear() {
    entities.clear();
    worldOffset.release();
    playerOffset.release();
    playerOffsetStaging.release();
    images.clear();
    pipeline.release();
    shaders.layout.release();
    shaders.vertex.release();
    shaders.fragment.release();
  }
};

// ---------------------------------------------------------------------------------------

// -- resource loaders --

void loadPipeline(video_api::Renderer& renderer, ResourceStorage& out);
void loadImage(video_api::Renderer& renderer, ImageId id, ResourceStorage& out);
