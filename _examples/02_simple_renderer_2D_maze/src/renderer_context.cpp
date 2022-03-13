/*******************************************************************************
Pandora Toolbox examples by Romain Vinders

To the extent possible under law, the person who associated CC0 with
Pandora Toolbox examples has waived all copyright and related or neighboring rights
to Pandora Toolbox examples.
CC0 legalcode: see <http://creativecommons.org/publicdomain/zero/1.0/>.
--------------------------------------------------------------------------------
Description : Example - renderer context
*******************************************************************************/
#include <cassert>
#include <stdexcept>
#include "renderer_context.h"

#define __COLOR_FORMAT  DataFormat::rgba8_sRGB
#define __DEPTH_FORMAT  DepthStencilFormat::d24_unorm_s8_ui

using namespace video_api;

// Initialize renderer (throws on failure)
RendererContext::RendererContext(const pandora::hardware::DisplayMonitor& monitor, pandora::video::WindowHandle window,
                                 uint32_t width, uint32_t height, pandora::video::RefreshRate&& rate, bool useVsync)
  : _renderer(monitor),
    _viewport(width, height),
    _scissor(0, 0, width, height),
    _timer(pandora::time::Rate(rate.numerator(), rate.denominator())),
    _rate(rate),
    _useVsync(useVsync) {
  // create framebuffer + targets
  const auto presentMode = useVsync ? pandora::video::PresentMode::fifo : pandora::video::PresentMode::immediate;
  _swapChain = SwapChain(DisplaySurface(_renderer, window),
                         SwapChain::Descriptor(width, height, 2u, presentMode, rate), __COLOR_FORMAT);

  // create texture sampler (bilinear)
  Sampler::Builder samplerBuilder(_renderer.resourceManager());
  TextureWrap texWrap[3] = { TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat };
  _sampler = samplerBuilder.createSampler(SamplerParams(TextureFilter::nearest, TextureFilter::nearest,
                                                        TextureFilter::nearest, texWrap));

  // enable sampler + viewports
  _renderer.flush();
  _renderer.setFragmentSamplerStates(0, _sampler.handlePtr(), 1);
  _renderer.setViewport(_viewport);
  _renderer.setScissorRectangle(_scissor);
}

void RendererContext::release() noexcept {
  _sampler.release();
  _swapChain.release();
  _renderer.release();
}

// -- operations--

void RendererContext::beginDrawing() {
  _renderer.setCleanActiveRenderTarget(_swapChain.getRenderTargetView(), nullptr);

  _renderer.setViewport(_viewport);
  _renderer.setScissorRectangle(_scissor);
}

void RendererContext::swapBuffers() {
  _swapChain.swapBuffers();
  if (!_useVsync) {
    _renderer.flush();
    _timer.waitPeriod();
  }
}
