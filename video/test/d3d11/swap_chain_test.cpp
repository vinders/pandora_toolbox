/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <video/window.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/swap_chain.h>
# include <video/d3d11/buffer.h>

  using namespace pandora::video::d3d11;

  class D3d11SwapChainTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- create/manage swap-chain --

  TEST_F(D3d11SwapChainTest, invalidSwapChain) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    EXPECT_ANY_THROW(SwapChain(DisplaySurface(renderer, nullptr), SwapChain::Descriptor{}, DataFormat::rgba8_sRGB));

    SwapChain defaultInit;
    EXPECT_TRUE(defaultInit.isEmpty());
  }

  TEST_F(D3d11SwapChainTest, createSwapChain) {
    auto window = pandora::video::Window::Builder{}
                    .setDisplayMode(pandora::video::WindowType::window, pandora::video::WindowBehavior::globalContext, 
                                    pandora::video::ResizeMode::fixed)
                    .setSize(600,400)
                    .create(L"_SWAPCHAIN_TEST0", L"Test");

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    SwapChain::Descriptor params{ 600, 400 };
    {
      DisplaySurface surface1(renderer, window->handle());
      EXPECT_TRUE(surface1.isFormatSupported(DataFormat::rgba8_sRGB));
      EXPECT_FALSE(surface1.isFormatSupported(DataFormat::d32_f_s8_ui));

      params.presentMode = pandora::video::PresentMode::immediate;
      SwapChain chain1(std::move(surface1), params);
      EXPECT_FALSE(chain1.isEmpty());
      EXPECT_TRUE(chain1.handle() != nullptr);
      EXPECT_TRUE(chain1.handleExt() == nullptr || chain1.handleExt() == chain1.handle());
      EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
      EXPECT_EQ((uint32_t)600, chain1.width());
      EXPECT_EQ((uint32_t)400, chain1.height());
      renderer.setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
      EXPECT_NO_THROW(chain1.swapBuffers());
      chain1.setPresentMode(pandora::video::PresentMode::fifo);
      EXPECT_NO_THROW(chain1.swapBuffers());
      auto handle1 = chain1.handle();
      auto target1 = chain1.getRenderTargetView();

      SwapChain moved(std::move(chain1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(moved.handle() != nullptr);
      EXPECT_TRUE(moved.handleExt() == nullptr || moved.handleExt() == moved.handle());
      EXPECT_EQ(handle1, moved.handle());
      EXPECT_TRUE(moved.getRenderTargetView() != nullptr);
      EXPECT_EQ(target1, moved.getRenderTargetView());
      EXPECT_TRUE(chain1.isEmpty());
      EXPECT_TRUE(chain1.handle() == nullptr);
      EXPECT_TRUE(chain1.getRenderTargetView() == nullptr);

      chain1 = std::move(moved);
      EXPECT_FALSE(chain1.isEmpty());
      EXPECT_TRUE(chain1.handle() != nullptr);
      EXPECT_TRUE(chain1.handleExt() == nullptr || chain1.handleExt() == chain1.handle());
      EXPECT_EQ(handle1, chain1.handle());
      EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
      EXPECT_EQ(target1, chain1.getRenderTargetView());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(moved.handle() == nullptr);
      EXPECT_TRUE(moved.getRenderTargetView() == nullptr);
      EXPECT_NO_THROW(chain1.swapBuffers());
    } // destroy "unique" swap-chain before creating other swap-chain
    {
      params.framebufferCount = 1u;
      params.presentMode = pandora::video::PresentMode::fifo;
      params.outputFlags = (SwapChain::OutputFlag::disableFlipSwap | SwapChain::OutputFlag::swapNoDiscard);
      params.refreshRate = pandora::video::RefreshRate(60000u, 1001u);
      DisplaySurface surface2(renderer, window->handle());
      DisplaySurface surfaceMoved2(std::move(surface2));
      SwapChain chain2(std::move(surfaceMoved2), params, DataFormat::rgb10a2_unorm_hdr10);
      EXPECT_TRUE(chain2.handle() != nullptr);
      EXPECT_TRUE(chain2.handleExt() == nullptr || chain2.handleExt() == chain2.handle());
      EXPECT_TRUE(chain2.getRenderTargetView() != nullptr);
      EXPECT_EQ((uint32_t)600, chain2.width());
      EXPECT_EQ((uint32_t)400, chain2.height());
      renderer.setActiveRenderTarget(chain2.getRenderTargetView(), nullptr);
      EXPECT_NO_THROW(chain2.swapBuffers());

      params.framebufferCount = 2u;
      params.outputFlags = (SwapChain::OutputFlag::disableFlipSwap);
      params.refreshRate = pandora::video::RefreshRate(60u, 1u);
      SwapChain chain3(DisplaySurface(renderer, window->handle()), params, DataFormat::rgba16_f_scRGB);
      EXPECT_TRUE(chain3.handle() != nullptr);
      EXPECT_TRUE(chain3.handleExt() == nullptr || chain3.handleExt() == chain3.handle());
      EXPECT_TRUE(chain3.getRenderTargetView() != nullptr);
      SwapChain chain3B(DisplaySurface(renderer, window->handle()), params, DataFormat::rgba16_f_scRGB);
      EXPECT_TRUE(chain3B.handle() != nullptr);
      EXPECT_TRUE(chain3B.handleExt() == nullptr || chain3B.handleExt() == chain3B.handle());
      EXPECT_TRUE(chain3B.getRenderTargetView() != nullptr);

      RenderTargetView views[]{ chain3.getRenderTargetView(), chain3B.getRenderTargetView() };
      renderer.setActiveRenderTargets(views, (size_t)2u, nullptr);
      EXPECT_NO_THROW(chain3.swapBuffers());
    } // destroy to create new "unique" swap-chain

    params.framebufferCount = 2u;
    params.outputFlags = SwapChain::OutputFlag::none;
    params.refreshRate = pandora::video::RefreshRate(60u, 1u);
    SwapChain chain4(DisplaySurface(renderer, window->handle()), params, DataFormat::rgba8_unorm);
    EXPECT_TRUE(chain4.handle() != nullptr);
    EXPECT_TRUE(chain4.handleExt() == nullptr || chain4.handleExt() == chain4.handle());
    EXPECT_TRUE(chain4.getRenderTargetView() != nullptr);
    renderer.setActiveRenderTarget(chain4.getRenderTargetView(), nullptr);
    EXPECT_NO_THROW(chain4.swapBuffers());
    renderer.setActiveRenderTarget(nullptr, nullptr);
    
    DepthStencilBuffer depthBuffer(renderer.resourceManager(), DepthStencilFormat::d32_f, 600, 400);
    EXPECT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthBuffer.handle() != nullptr);
    EXPECT_TRUE(depthBuffer.getDepthStencilView() != nullptr);
    renderer.setActiveRenderTarget(chain4.getRenderTargetView(), depthBuffer.getDepthStencilView());
  }

  TEST_F(D3d11SwapChainTest, createSwapChainResized) {
    auto window = pandora::video::Window::Builder{}
                    .setDisplayMode(pandora::video::WindowType::window, pandora::video::WindowBehavior::globalContext, 
                                    pandora::video::ResizeMode::fixed)
                    .setSize(600,400)
                    .create(L"_SWAPCHAIN_TEST1", L"Test");

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    SwapChain chain1(DisplaySurface(renderer, window->handle()), SwapChain::Descriptor(600,400));
    EXPECT_TRUE(chain1.handle() != nullptr);
    EXPECT_TRUE(chain1.handleExt() == nullptr || chain1.handleExt() == chain1.handle());
    EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
    renderer.setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    EXPECT_NO_THROW(chain1.swapBuffers());

    window->resize(640, 480);
    chain1.resize(640, 480);
    renderer.setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);

    EXPECT_TRUE(chain1.handle() != nullptr);
    EXPECT_TRUE(chain1.handleExt() == nullptr || chain1.handleExt() == chain1.handle());
    EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
    renderer.setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    EXPECT_NO_THROW(chain1.swapBuffers());
    
    Viewport viewport1(0,0, 640u,480u,0.,1.);
    renderer.setViewports(&viewport1, size_t{1u});
    renderer.setViewport(viewport1);
    Viewport viewports2[] = { Viewport(0,0, 320u,480u,0.,1.), Viewport(320,0, 320u,480u,0.,1.)};
    renderer.setViewport(viewports2[0]);
    renderer.setViewport(viewports2[1]);
    renderer.setViewports(viewports2, size_t{1u});
  }

#endif
