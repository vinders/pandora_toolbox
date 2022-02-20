/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - renderer context
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory>
#include <time/timer.h>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/renderer.h>
# include <video/d3d11/buffer.h>
# include <video/d3d11/texture.h>
  namespace video_api = pandora::video::d3d11;
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/renderer.h>
# include <video/vulkan/buffer.h>
# include <video/vulkan/texture.h>
  namespace video_api = pandora::video::vulkan;
#endif


// Rendering context
// --> device context, framebuffers, rendering states/samplers, viewport
class RendererContext final {
public:
  // Initialize renderer (throws on failure)
  RendererContext(const pandora::hardware::DisplayMonitor& monitor, pandora::video::WindowHandle window,
                  uint32_t width, uint32_t height, pandora::video::RefreshRate&& rate,
                  bool useAntialiasing, bool useAnisotropy, bool useVsync);
  RendererContext() = default;
  ~RendererContext() noexcept { release(); }

  // Close pipeline
  void release() noexcept;

  RendererContext(RendererContext&&) noexcept = default;
  RendererContext& operator=(RendererContext&&) noexcept = default;


  // -- settings --

  // Resize pipeline (throws on device error -> recreate pipeline) -- warning: do NOT while a frame is still drawing!
  void resize(pandora::video::WindowHandle window, uint32_t width, uint32_t height);

  // Toggle anti-aliasing (MSAA / none) -- warning: do NOT while a frame is still drawing!
  void toggleAntiAliasing() noexcept { _useAntialiasing ^= true; }
  bool isAntiAliasingEnabled() const noexcept { return _useAntialiasing; }
  bool antiAliasingSamples() const noexcept { return _aaSamples; }

  // Toggle texture sampler (anisotropic / trilinear)
  void toggleSampler() noexcept {
    _useAnisotropy ^= true;
    _renderer->setFragmentSamplerStates(0, _samplers.getFrom(_useAnisotropy ? 1 : 0), 1);
  }
  bool isAnisotropicSampler() const noexcept { return _useAnisotropy; }

  bool hasVsync() const noexcept { return _useVsync; }


  // -- operations --

  // Get renderer to bind resources (shaders, layout, textures, uniform/constant buffers...) or draw (draw, clear...)
  std::shared_ptr<video_api::Renderer> renderer() noexcept { return _renderer; }

  // Cleanup and enable render target for 3D drawing
  void beginDrawing();
  // Toggle draw mode to 2D / UI -- must be called after drawing all 3D entities
  void setDrawMode2D();
  // Swap framebuffers to display on screen (throws if device lost -> recreate pipeline)
  void swapBuffers();

  // Adjust timer to skip current frame
  void skipFrame() noexcept { _timer.reset(); }


private:
  std::shared_ptr<video_api::Renderer> _renderer = nullptr;
  video_api::SwapChain _swapChain;
  video_api::TextureTarget2D _msaaTarget;
  video_api::DepthStencilBuffer _depthBuffer;
  video_api::DepthStencilBuffer _depthBufferMsaa;
  video_api::SamplerStateArray _samplers;

  video_api::Viewport _viewport;
  video_api::ScissorRectangle _scissor;

  pandora::time::Timer<pandora::time::HighResolutionClock,
                       pandora::time::HighResolutionAuxClock,
                       pandora::time::DelayHandling::none, true> _timer;
  pandora::video::RefreshRate _rate;
  uint32_t _aaSamples = 1u;
  bool _useAntialiasing = false;
  bool _useAnisotropy = false;
  bool _useVsync = false;
  bool _isMsaaPending = false;
};
