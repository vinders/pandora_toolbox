#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <video/window.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/swap_chain.h>
# include <video/d3d11/depth_stencil_buffer.h>

  using namespace pandora::video::d3d11;

  class SwapChainTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- create/manage swap-chain --

  TEST_F(SwapChainTest, invalidSwapChain) {
    EXPECT_ANY_THROW(SwapChain(nullptr, nullptr, SwapChain::Descriptor{}, DataFormat::rgba8_sRGB));

    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor);
    EXPECT_ANY_THROW(SwapChain(renderer, nullptr, SwapChain::Descriptor{}, DataFormat::rgba8_sRGB));

    SwapChain defaultInit;
    EXPECT_TRUE(defaultInit.isEmpty());
  }

  TEST_F(SwapChainTest, createSwapChain) {
    auto window = pandora::video::Window::Builder{}
                    .setDisplayMode(pandora::video::WindowType::window, pandora::video::WindowBehavior::globalContext, 
                                    pandora::video::ResizeMode::fixed)
                    .setSize(600,400)
                    .create(L"_SWAPCHAIN_TEST0", L"Test");

    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor);

    SwapChain::Descriptor params{ 600, 400 };
    {
      SwapChain chain1(renderer, window->handle(), params);
      EXPECT_FALSE(chain1.isEmpty());
      EXPECT_TRUE(chain1.handle() != nullptr);
      EXPECT_TRUE(chain1.handleHigherLevel() == nullptr || chain1.handleHigherLevel() == chain1.handle());
      EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
      EXPECT_FALSE(chain1.isHdrEnabled());
      EXPECT_EQ((uint32_t)600, chain1.width());
      EXPECT_EQ((uint32_t)400, chain1.height());
      renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
      EXPECT_NO_THROW(chain1.swapBuffers(false));
      EXPECT_NO_THROW(chain1.swapBuffersDiscard(false, nullptr));
      EXPECT_NO_THROW(chain1.swapBuffers(true));
      auto handle1 = chain1.handle();
      auto target1 = chain1.getRenderTargetView();

      SwapChain moved(std::move(chain1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(moved.handle() != nullptr);
      EXPECT_TRUE(moved.handleHigherLevel() == nullptr || moved.handleHigherLevel() == moved.handle());
      EXPECT_EQ(handle1, moved.handle());
      EXPECT_TRUE(moved.getRenderTargetView() != nullptr);
      EXPECT_EQ(target1, moved.getRenderTargetView());
      EXPECT_FALSE(moved.isHdrEnabled());
      EXPECT_TRUE(chain1.isEmpty());
      EXPECT_TRUE(chain1.handle() == nullptr);
      EXPECT_TRUE(chain1.getRenderTargetView() == nullptr);

      chain1 = std::move(moved);
      EXPECT_FALSE(chain1.isEmpty());
      EXPECT_TRUE(chain1.handle() != nullptr);
      EXPECT_TRUE(chain1.handleHigherLevel() == nullptr || chain1.handleHigherLevel() == chain1.handle());
      EXPECT_EQ(handle1, chain1.handle());
      EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
      EXPECT_EQ(target1, chain1.getRenderTargetView());
      EXPECT_FALSE(chain1.isHdrEnabled());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(moved.handle() == nullptr);
      EXPECT_TRUE(moved.getRenderTargetView() == nullptr);
      EXPECT_NO_THROW(chain1.swapBuffers(true));
    } // destroy "unique" swap-chain before creating other swap-chain
    {
      params.framebufferCount = 1u;
      params.outputFlags = (SwapChain::OutputFlag::hdrPreferred | SwapChain::OutputFlag::partialOutput);
      params.refreshRate = pandora::video::RefreshRate(60000u, 1001u);
      SwapChain chain2(renderer, window->handle(), params, DataFormat::rgb10a2_unorm_hdr10);
      EXPECT_TRUE(chain2.handle() != nullptr);
      EXPECT_TRUE(chain2.handleHigherLevel() == nullptr || chain2.handleHigherLevel() == chain2.handle());
      EXPECT_TRUE(chain2.getRenderTargetView() != nullptr);
      EXPECT_TRUE(chain2.isHdrEnabled() || !renderer->isMonitorHdrCapable(monitor));
      EXPECT_EQ((uint32_t)600, chain2.width());
      EXPECT_EQ((uint32_t)400, chain2.height());
      renderer->setActiveRenderTarget(chain2.getRenderTargetView(), nullptr);
      EXPECT_NO_THROW(chain2.swapBuffers(false));
      EXPECT_NO_THROW(chain2.swapBuffersDiscard(false, nullptr));
      EXPECT_NO_THROW(chain2.swapBuffers(true));

      params.framebufferCount = 2u;
      params.outputFlags = (SwapChain::OutputFlag::hdrPreferred | SwapChain::OutputFlag::partialOutput);
      params.refreshRate = pandora::video::RefreshRate(60u, 1u);
      SwapChain chain3(renderer, window->handle(), params, DataFormat::rgba16_f_scRGB);
      EXPECT_TRUE(chain3.handle() != nullptr);
      EXPECT_TRUE(chain3.handleHigherLevel() == nullptr || chain3.handleHigherLevel() == chain3.handle());
      EXPECT_TRUE(chain3.getRenderTargetView() != nullptr);
      EXPECT_TRUE(chain3.isHdrEnabled() || !renderer->isMonitorHdrCapable(monitor));
      SwapChain chain3B(renderer, window->handle(), params, DataFormat::rgba16_f_scRGB);
      EXPECT_TRUE(chain3B.handle() != nullptr);
      EXPECT_TRUE(chain3B.handleHigherLevel() == nullptr || chain3B.handleHigherLevel() == chain3B.handle());
      EXPECT_TRUE(chain3B.getRenderTargetView() != nullptr);
      EXPECT_TRUE(chain3B.isHdrEnabled() || !renderer->isMonitorHdrCapable(monitor));

      RenderTargetView views[]{ chain3.getRenderTargetView(), chain3B.getRenderTargetView() };
      renderer->setActiveRenderTargets(views, (size_t)2u, nullptr);
      EXPECT_NO_THROW(chain3.swapBuffers(false));
      EXPECT_NO_THROW(chain3.swapBuffers(true));
      EXPECT_NO_THROW(chain3.swapBuffersDiscard(true, nullptr));
    } // destroy to create new "unique" swap-chain

    params.framebufferCount = 2u;
    params.outputFlags = SwapChain::OutputFlag::none;
    params.refreshRate = pandora::video::RefreshRate(60u, 1u);
    SwapChain chain4(renderer, window->handle(), params, DataFormat::rgba8_unorm);
    EXPECT_TRUE(chain4.handle() != nullptr);
    EXPECT_TRUE(chain4.handleHigherLevel() == nullptr || chain4.handleHigherLevel() == chain4.handle());
    EXPECT_TRUE(chain4.getRenderTargetView() != nullptr);
    EXPECT_FALSE(chain4.isHdrEnabled());
    renderer->setActiveRenderTarget(chain4.getRenderTargetView(), nullptr);
    EXPECT_NO_THROW(chain4.swapBuffers(false));
    EXPECT_NO_THROW(chain4.swapBuffers(true));
    EXPECT_NO_THROW(chain4.swapBuffersDiscard(true, nullptr));
    renderer->setActiveRenderTarget(nullptr, nullptr);
    
    DepthStencilBuffer depthBuffer(*renderer, DepthStencilFormat::d32_f, 600, 400);
    EXPECT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthBuffer.handle() != nullptr);
    EXPECT_TRUE(depthBuffer.getDepthStencilView() != nullptr);
    renderer->setActiveRenderTarget(chain4.getRenderTargetView(), depthBuffer.getDepthStencilView());
  }

  TEST_F(SwapChainTest, createSwapChainResized) {
    auto window = pandora::video::Window::Builder{}
                    .setDisplayMode(pandora::video::WindowType::window, pandora::video::WindowBehavior::globalContext, 
                                    pandora::video::ResizeMode::fixed)
                    .setSize(600,400)
                    .create(L"_SWAPCHAIN_TEST1", L"Test");

    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor);

    SwapChain chain1(renderer, window->handle(), SwapChain::Descriptor(600,400));
    EXPECT_TRUE(chain1.handle() != nullptr);
    EXPECT_TRUE(chain1.handleHigherLevel() == nullptr || chain1.handleHigherLevel() == chain1.handle());
    EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
    EXPECT_FALSE(chain1.isHdrEnabled());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    EXPECT_NO_THROW(chain1.swapBuffers(false));
    EXPECT_NO_THROW(chain1.swapBuffersDiscard(false, nullptr));
    EXPECT_NO_THROW(chain1.swapBuffers(true));

    window->resize(640, 480);
    chain1.resize(640, 480);
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);

    EXPECT_TRUE(chain1.handle() != nullptr);
    EXPECT_TRUE(chain1.handleHigherLevel() == nullptr || chain1.handleHigherLevel() == chain1.handle());
    EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
    EXPECT_FALSE(chain1.isHdrEnabled());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    EXPECT_NO_THROW(chain1.swapBuffers(false));
    EXPECT_NO_THROW(chain1.swapBuffersDiscard(false, nullptr));
    EXPECT_NO_THROW(chain1.swapBuffers(true));
    
    Viewport viewport1(0,0, 640u,480u,0.,1.);
    renderer->setViewports(&viewport1, size_t{1u});
    renderer->setViewport(viewport1);
    Viewport viewports2[] = { Viewport(0,0, 320u,480u,0.,1.), Viewport(320,0, 320u,480u,0.,1.)};
    renderer->setViewport(viewports2[0]);
    renderer->setViewport(viewports2[1]);
    renderer->setViewports(viewports2, size_t{1u});
  }

#endif
