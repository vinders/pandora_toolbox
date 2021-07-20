/*******************************************************************************
Description : Example - rendering scene
*******************************************************************************/
#pragma once

#include "display_pipeline.h"
#include "resources.h"
#include "camera.h"

// Rendering scene: scenery and entities
// --> contains camera view, entities/models, textures/materials/shaders...
class Scene final {
public:
  // Create rendering scene components
  Scene(DisplayPipeline& renderer, uint32_t width, uint32_t height, float sensitivity)
    : _camera(width, height, sensitivity) {
    init(renderer, width, height);
  }

  Scene() = default;
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
  ~Scene() noexcept { release(); }

  // Re-initialize rendering scene components (must be called if the DisplayPipeline instance is re-created)
  // --> throws on scene creation failure
  void init(DisplayPipeline& renderer, uint32_t width, uint32_t height);
  // Destroy scene components (should be called before destroying associated DisplayPipeline)
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

  bool isUpdated() const noexcept { return _isUpdated; }
  void refreshScreen() noexcept { _isUpdated = true; }

  // Compute and draw scene
  // --> throws if device lost -> recreate DisplayPipeline + call init)
  void render();


private:
  void _setCameraViewProjection(bool isInit, __RENDER_API(Renderer)& renderer, MatrixFloat4 modelWorldMatrix);

private:
  DisplayPipeline* _renderer = nullptr;
  ResourceStorage _resources;
  Camera _camera;
  bool _isUpdated = false;
};
