/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - rendering scene
*******************************************************************************/
#pragma once

#include "renderer_context.h"
#include "resources.h"
#include "camera.h"

// Rendering scene: scenery and entities
// --> contains camera view, entities/models, textures/materials/shaders...
class Scene final {
public:
  // Create rendering scene components
  Scene(RendererContext& renderer, uint32_t width, uint32_t height, uint32_t aaSamples, float sensitivity)
    : _camera(width, height, sensitivity) {
    init(renderer, width, height, aaSamples);
  }

  Scene() = default;
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
  ~Scene() noexcept { release(); }

  // Re-initialize rendering scene components (must be called if the RendererContext instance is re-created)
  // --> throws on scene creation failure
  void init(RendererContext& renderer, uint32_t width, uint32_t height, uint32_t aaSamples);
  // Destroy scene components (should be called before destroying associated RendererContext)
  void release() noexcept;

  // Reconfigure camera
  void setCamera(uint32_t width, uint32_t height, float sensitivity) noexcept {
    _camera.configure(width, height, sensitivity);
    _isUpdated = true;
  }


  // -- operations --

  // Report mouse move -> move camera view
  void moveCamera(int32_t deltaX, int32_t deltaY) noexcept {
    _camera.move((float)deltaX, (float)deltaY);
    _isUpdated = true;
  }
  // Report size change -> update UI
  void resizeScreen(uint32_t width, uint32_t height) noexcept;
  // Report MSAA change -> update pipelines
  void rebuildPipelines(uint32_t width, uint32_t height, uint32_t aaSamples);

  bool isUpdated() const noexcept { return _isUpdated; }
  void refreshScreen() noexcept { _isUpdated = true; }

  // Compute and draw scene 3D entities
  // --> throws if device lost -> recreate RendererContext + call init)
  void render3D();
  // Compute and draw scene 2D entities
  // --> throws if device lost -> recreate RendererContext + call init)
  void render2D();


private:
  void _setCameraViewProjection(bool isInit, video_api::Renderer& renderer, MatrixFloat4x4 modelWorldMatrix);

private:
  RendererContext* _renderer = nullptr;
  ResourceStorage _resources;
  Camera _camera;
  bool _isUpdated = false;
};
