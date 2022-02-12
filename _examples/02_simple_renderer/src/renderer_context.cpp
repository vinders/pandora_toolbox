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
                                 uint32_t width, uint32_t height, pandora::video::RefreshRate&& rate,
                                 bool useAntialiasing, bool useAnisotropy, bool useVsync)
  : _renderer(std::make_shared<Renderer>(monitor)),
    _viewport(width, height),
    _scissor(0, 0, width, height),
    _timer(pandora::time::Rate(rate.numerator(), rate.denominator())),
    _rate(rate),
    _useAntialiasing(useAntialiasing),
    _useAnisotropy(useAnisotropy),
    _useVsync(useVsync) {
  // render target params
  const auto presentMode = useVsync ? pandora::video::PresentMode::fifo : pandora::video::PresentMode::immediate;
  _aaSamples = 16;
  while ((!_renderer->isColorSampleCountAvailable(__COLOR_FORMAT, _aaSamples) // find highest available sample count
       || !_renderer->isDepthSampleCountAvailable(__DEPTH_FORMAT, _aaSamples)) && _aaSamples > 1) {
    _aaSamples >>= 1; // divide by 2
  }

  // create framebuffer + targets
  _swapChain = SwapChain(DisplaySurface(_renderer, window),
                         SwapChain::Descriptor(width, height, 2u, presentMode, rate), __COLOR_FORMAT);
  _depthBuffer = DepthStencilBuffer(*_renderer, __DEPTH_FORMAT, width, height, 1);
  _depthBufferMsaa = DepthStencilBuffer(*_renderer, __DEPTH_FORMAT, width, height, _aaSamples);

  Texture2DParams msaaParams(width, height, __COLOR_FORMAT, 1, 1, 0, ResourceUsage::staticGpu, _aaSamples);
  _msaaTarget = TextureTarget2D(*_renderer, msaaParams);

  // create samplers
  SamplerBuilder samplerBuilder(*_renderer);
  TextureWrap texWrap[3] = { TextureWrap::repeatMirror, TextureWrap::repeatMirror, TextureWrap::repeatMirror };
  _samplers.append(samplerBuilder.create(SamplerParams(TextureFilter::linear, TextureFilter::linear,
                                                       TextureFilter::linear, texWrap)));
  _samplers.append(samplerBuilder.create(SamplerParams(SamplerParams::maxAnisotropy()/2, texWrap)));

  // enable sampler + viewports
  _renderer->flush();
  _renderer->setFragmentSamplerStates(0, _samplers.getFrom(_useAnisotropy ? 1 : 0), 1);
  _renderer->setViewport(_viewport);
  _renderer->setScissorRectangle(_scissor);
}

void RendererContext::release() noexcept {
  if (_renderer) {
    if (!_samplers.empty())
      _samplers.clear();
    if (!_msaaTarget.isEmpty())
      _msaaTarget.release();
    if (!_depthBufferMsaa.isEmpty())
      _depthBufferMsaa.release();
    if (!_depthBuffer.isEmpty())
      _depthBuffer.release();
    if (!_swapChain.isEmpty())
      _swapChain.release();
    _renderer.reset();
  }
}

// -- settings --

void RendererContext::resize(pandora::video::WindowHandle window, uint32_t width, uint32_t height) {
  _renderer->bindGraphicsPipeline(nullptr);
  try {
    _swapChain.resize(width, height);
  }
  catch (const std::runtime_error&) { // resize failure -> re-create SwapChain (don't catch domain_error -> let Renderer be re-created)
    const auto presentMode = _useVsync ? pandora::video::PresentMode::fifo : pandora::video::PresentMode::immediate;
    _swapChain.release();
    _swapChain = SwapChain(DisplaySurface(_renderer, window),
                           SwapChain::Descriptor(width, height, 2u, presentMode, _rate), __COLOR_FORMAT);
  }
  _viewport.resize(0, 0, (float)width, (float)height);
  _scissor = ScissorRectangle(0, 0, width, height);

  _depthBuffer = DepthStencilBuffer(*_renderer, __DEPTH_FORMAT, width, height, 1);
  _depthBufferMsaa = DepthStencilBuffer(*_renderer, __DEPTH_FORMAT, width, height, _aaSamples);
  Texture2DParams msaaParams(width, height, __COLOR_FORMAT, 1, 1, 0, ResourceUsage::staticGpu, _aaSamples);
  _msaaTarget.release();
  _msaaTarget = TextureTarget2D(*_renderer, msaaParams);
  
  _renderer->flush();
  _renderer->setFragmentSamplerStates(0, _samplers.getFrom(_useAnisotropy ? 1 : 0), 1);
  _renderer->setViewport(_viewport);
  _renderer->setScissorRectangle(_scissor);
}

// -- operations--

void RendererContext::beginDrawing() {
  if (_useAntialiasing)
    _renderer->setCleanActiveRenderTarget(_msaaTarget.getRenderTargetView(), _depthBufferMsaa.getDepthStencilView());
  else
    _renderer->setCleanActiveRenderTarget(_swapChain.getRenderTargetView(), _depthBuffer.getDepthStencilView());

  _renderer->setViewport(_viewport);
  _renderer->setScissorRectangle(_scissor);
  _isMsaaPending = _useAntialiasing;
}

void RendererContext::setDrawMode2D() {
  if (_useAntialiasing) {
    _swapChain.resolve(_msaaTarget.handle(), 0);
    _msaaTarget.discard(_depthBufferMsaa.getDepthStencilView());
    _isMsaaPending = false;
  }

  _renderer->setActiveRenderTarget(_swapChain.getRenderTargetView()); // UI drawing (always on top) -> no depth buffer
  _renderer->setViewport(_viewport);
  _renderer->setScissorRectangle(_scissor);
}

void RendererContext::swapBuffers() {
  if (_isMsaaPending && _useAntialiasing) {
    _swapChain.resolve(_msaaTarget.handle(), 0);
    _msaaTarget.discard(_depthBufferMsaa.getDepthStencilView());
  }
  _isMsaaPending = false;

  _swapChain.swapBuffers(_depthBuffer.getDepthStencilView());
  if (!_useVsync) {
    _renderer->flush();
    _timer.waitPeriod();
  }
}
