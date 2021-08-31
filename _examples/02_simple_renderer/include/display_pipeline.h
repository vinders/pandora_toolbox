/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - renderer pipeline
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory>
#include <time/timer.h>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/renderer.h>
# include <video/d3d11/depth_stencil_buffer.h>
# include <video/d3d11/texture.h>
  namespace video_api = pandora::video::d3d11;
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/renderer.h>
# include <video/vulkan/depth_stencil_buffer.h>
# include <video/vulkan/texture.h>
  namespace video_api = pandora::video::vulkan;
#else
# include <video/openGL4/renderer.h>
# include <video/openGL4/depth_stencil_buffer.h>
# include <video/openGL4/texture.h>
  namespace video_api = pandora::video::openGL4;
#endif


// Rendering pipeline
// --> device context, framebuffers, rendering states/samplers, viewport, drawing calls
class DisplayPipeline final {
public:
  // Initialize pipeline (throws on failure)
  DisplayPipeline(const pandora::hardware::DisplayMonitor& monitor, pandora::video::WindowHandle window,
                  uint32_t clientWidth, uint32_t clientHeight, pandora::video::RefreshRate&& rate,
                  bool useAnisotropy, bool useVsync);
  DisplayPipeline() = default;
  ~DisplayPipeline() noexcept { release(); }

  // Close pipeline
  void release() noexcept;

  DisplayPipeline(DisplayPipeline&&) noexcept = default;
  DisplayPipeline& operator=(DisplayPipeline&&) noexcept = default;


  // -- settings --

  // Resize pipeline (throws on device error -> recreate pipeline)
  void resize(uint32_t clientWidth, uint32_t clientHeight);

  // Toggle vertical sync
  void toggleVsync() noexcept {
    _useVsync ^= true;
    _timer.reset();
  }
  bool hasVsync() const noexcept { return _useVsync; }

  // Toggle texture sampler (anisotropic / trilinear)
  void toggleSampler() noexcept {
    _useAnisotropy ^= true;
    _renderer->setFragmentFilterStates(0, _samplers.getFrom(_useAnisotropy ? 1 : 0), 1);
  }
  bool hasAnisotropy() const noexcept { return _useAnisotropy; }

  // Set depth/stencil test (depth/stencil: true / depth only: false)
  void setDepthStencilTest(bool enableStencilTest) noexcept {
    _renderer->setDepthStencilState(_depthTests.at(enableStencilTest ? 1 : 0));
  }


  // -- operations --

  // Get renderer context to bind resources (shaders, layout, textures, uniform/constant buffers...) or draw (draw, clear...)
  video_api::Renderer& renderer() noexcept { return *_renderer; }

  // Enable render target for drawing
  // --> isCleaned is only useful if some areas have no polygons
  void enableRenderTarget(bool isCleaned);
  // Swap framebuffers to display on screen (throws if device lost -> recreate pipeline)
  void swapBuffers();

  // Adjust timer to skip current frame
  void skipFrame() noexcept { _timer.reset(); }


private:
  std::shared_ptr<video_api::Renderer> _renderer = nullptr;
  video_api::SwapChain _swapChain;
  video_api::DepthStencilBuffer _depthBuffer;
  video_api::DepthStencilStateArray<2> _depthTests;
  video_api::FilterStateArray _samplers;
  video_api::RasterizerState _rasterizer;
  video_api::BlendState _blend;
  video_api::Viewport _viewport;
  pandora::time::Timer<pandora::time::HighResolutionClock,
                       pandora::time::HighResolutionAuxClock,
                       pandora::time::DelayHandling::none, true> _timer;
  bool _useAnisotropy = false;
  bool _useVsync = false;
};
