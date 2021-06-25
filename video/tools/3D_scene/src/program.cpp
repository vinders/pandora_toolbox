/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#include <cassert>
#include "program.h"
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <video/d3d11/renderer.h>
#else
# include <video/component_format.h>
# include <video/render_options.h>
# include <video/swap_chain_params.h>
#endif

using namespace pandora::video;

// -- common implementation --

template <typename _Renderer>
static inline void __renderFrame(_Renderer& renderer, typename _Renderer::SwapChain& swapChain, bool useVsync) {
  renderer.setCleanActiveRenderTarget(swapChain.getRenderTargetView(), nullptr, nullptr);

  //...

  swapChain.swapBuffersDiscard(useVsync, nullptr);
}


// -- renderer classes --

namespace scene {
# if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
    class D3d11Program final : scene::Program {
    public:
      D3d11Program(std::shared_ptr<d3d11::Renderer> renderer, SwapChainParams& params,
                   std::shared_ptr<scene::MenuManager> menu, Window& window)
        : Program(), _menu(menu), _renderer(renderer), 
          _swapChain(renderer, params, window.handle(), window.getClientSize().width, window.getClientSize().height) {
        //init shaders + filters
      }
      virtual ~D3d11Program() noexcept = default;

      void onFilterChange() override {}
      void onSizeChange(uint32_t width, uint32_t height) override { this->_swapChain.resize(width, height); }
      void onViewportChange() override {}

      void renderFrame() override { __renderFrame<d3d11::Renderer>(*_renderer, _swapChain, _menu->settings().useVsync); }

    private:
      std::shared_ptr<scene::MenuManager> _menu = nullptr;
      std::shared_ptr<d3d11::Renderer> _renderer = nullptr;
      d3d11::Renderer::SwapChain _swapChain;
    };
# endif
}


// -- builder --

std::unique_ptr<scene::Program> scene::Program::createProgram(std::shared_ptr<scene::MenuManager> menu, pandora::video::Window& window) {
  pandora::hardware::DisplayMonitor primaryMonitor;
  SwapChainParams params;
  params.setBackBufferFormat(pandora::video::ComponentFormat::rgba8_sRGB)
        .setFrameBufferNumber(2u).setRenderTargetMode(SwapChainTargetMode::uniqueOutput);

  switch (menu->settings().api) {
#   if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
      case scene::RenderingApi::d3d11:
        return std::unique_ptr<scene::Program>((scene::Program*)new scene::D3d11Program(std::make_shared<d3d11::Renderer>(primaryMonitor), params, menu, window));
#   endif
    case scene::RenderingApi::openGL4: break;
    case scene::RenderingApi::openGLES3: break;
    default: break;
  }
  return nullptr;
}
