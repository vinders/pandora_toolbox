/*******************************************************************************
Description : Example - renderer pipeline
*******************************************************************************/
#include <cassert>
#include <stdexcept>

#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/renderer_state_factory.h>
#elif defined(_VIDEO_VULKAN_SUPPORT)
# include <video/vulkan/renderer_state_factory.h>
#else
# include <video/openGL4/renderer_state_factory.h>
#endif
#include "display_pipeline.h"

using namespace __RENDER_API_NS;

// Initialize pipeline (throws on failure)
DisplayPipeline::DisplayPipeline(const pandora::hardware::DisplayMonitor& monitor, pandora::video::WindowHandle window,
                                 uint32_t clientWidth, uint32_t clientHeight, pandora::video::RefreshRate&& rate,
                                 bool useAnisotropy, bool useVsync)
  : _renderer(std::make_shared<Renderer>(monitor)),
    _swapChain(_renderer, window, SwapChain::Descriptor(clientWidth, clientHeight, 2u, rate)),
    _depthBuffer(*_renderer, DepthStencilFormat::d24_unorm_s8_ui, clientWidth, clientHeight),
    _viewport(0, 0, clientWidth, clientHeight),
    _timer(pandora::time::Rate(rate.numerator(), rate.denominator())),
    _useAnisotropy(useAnisotropy),
    _useVsync(useVsync) {

  RendererStateFactory factory(*_renderer);
  _depthTests.append(factory.createDepthStencilTestState(DepthStencilParams(StencilCompare::less, StencilOp::incrementWrap, StencilOp::keep,
                                                                            StencilOp::decrementWrap, StencilOp::keep)));
  _depthTests.append(factory.createDepthStencilTestState(DepthStencilParams(StencilCompare::always, StencilCompare::always,
                                                                            StencilOp::keep, StencilOp::replace,
                                                                            StencilOp::keep, StencilOp::replace)));
  TextureWrap addressMode[3] = { TextureWrap::repeatMirror, TextureWrap::repeatMirror, TextureWrap::repeatMirror };
  _samplers.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear, addressMode)));
  _samplers.append(factory.createFilterState(FilterParams(FilterParams::maxAnisotropy()/2, addressMode)));
  _rasterizer = factory.createRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::fill, true, true, false));
  _blend = factory.createBlendState(BlendParams(BlendFactor::sourceAlpha, BlendFactor::sourceInvAlpha, BlendOp::add,
                                                BlendFactor::one, BlendFactor::zero, BlendOp::add));

  _renderer->setCleanActiveRenderTarget(_swapChain.getRenderTargetView(), _depthBuffer.getDepthStencilView());
  _renderer->setRasterizerState(_rasterizer);
  _renderer->setBlendState(_blend);
  _renderer->setDepthStencilState(_depthTests.at(0));
  _renderer->setFragmentFilterStates(0, _samplers.getFrom(useAnisotropy ? 1 : 0), 1);
  _renderer->setViewport(_viewport);
}

void DisplayPipeline::release() noexcept {
  if (_renderer) {
    if (_rasterizer.isValid())
      _rasterizer.release();
    if (!_samplers.empty())
      _samplers.clear();
    if (!_depthTests.empty())
      _depthTests.clear();

    if (!_depthBuffer.isEmpty())
      _depthBuffer.release();
    if (!_swapChain.isEmpty())
      _swapChain.release();
    _renderer.reset();
  }
}

// -- settings --

void DisplayPipeline::resize(uint32_t clientWidth, uint32_t clientHeight) {
  _swapChain.resize(clientWidth, clientHeight);
  _depthBuffer = DepthStencilBuffer(*_renderer, DepthStencilFormat::d24_unorm_s8_ui, clientWidth, clientHeight);
  _viewport.resize(0, 0, clientWidth, clientHeight);

  _renderer->setViewport(_viewport);
  _renderer->setCleanActiveRenderTarget(_swapChain.getRenderTargetView(), _depthBuffer.getDepthStencilView());
}

// -- operations--

void DisplayPipeline::enableRenderTarget(bool isCleaned) {
  if (isCleaned)
    _renderer->setCleanActiveRenderTarget(_swapChain.getRenderTargetView(), _depthBuffer.getDepthStencilView());
  else
    _renderer->setActiveRenderTarget(_swapChain.getRenderTargetView(), _depthBuffer.getDepthStencilView());
}

void DisplayPipeline::swapBuffers() {
  _swapChain.swapBuffersDiscard(_useVsync, _depthBuffer.getDepthStencilView());
  if (!_useVsync)
    _timer.waitPeriod();
}
