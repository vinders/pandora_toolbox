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
#include <time/timer.h>
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/renderer.h>
# include <video/d3d11/sampler.h>
  namespace video_api = pandora::video::d3d11;
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/renderer.h>
# include <video/vulkan/sampler.h>
  namespace video_api = pandora::video::vulkan;
#endif


// Rendering context
// --> device context, framebuffers, rendering states/samplers, viewport
class RendererContext final {
public:
  // Initialize renderer (throws on failure)
  RendererContext(const pandora::hardware::DisplayMonitor& monitor, pandora::video::WindowHandle window,
                  uint32_t width, uint32_t height, pandora::video::RefreshRate&& rate, bool useVsync);
  RendererContext() = default;
  ~RendererContext() noexcept { release(); }

  // Close pipeline
  void release() noexcept;

  RendererContext(RendererContext&&) noexcept = default;
  RendererContext& operator=(RendererContext&&) noexcept = default;


  // -- settings --

  bool hasVsync() const noexcept { return _useVsync; }


  // -- operations --

  // Get renderer to bind resources (shaders, layout, textures, uniform/constant buffers...) or draw (draw, clear...)
  video_api::Renderer& renderer() noexcept { return _renderer; }

  // Cleanup and enable render target for 2D drawing
  void beginDrawing();
  // Swap framebuffers to display on screen (throws if device lost -> recreate renderer)
  void swapBuffers();

  // Adjust timer to skip current frame
  void skipFrame() noexcept { _timer.reset(); }


private:
  video_api::Renderer _renderer;
  video_api::SwapChain _swapChain;
  video_api::SamplerState _sampler;

  video_api::Viewport _viewport;
  video_api::ScissorRectangle _scissor;

  pandora::time::Timer<pandora::time::HighResolutionClock,
                       pandora::time::HighResolutionAuxClock,
                       pandora::time::DelayHandling::none, true> _timer;
  pandora::video::RefreshRate _rate;
  bool _useVsync = false;
};
