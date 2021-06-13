#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <video/window.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/swap_chain.h>

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
    EXPECT_ANY_THROW(SwapChain(nullptr, pandora::video::SwapChainParams{}, nullptr, 0, 0));

    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor, Renderer::DeviceLevel::direct3D_11_0);
    EXPECT_ANY_THROW(SwapChain(renderer, pandora::video::SwapChainParams{}, nullptr, 0, 0));
    EXPECT_ANY_THROW(SwapChain(renderer, pandora::video::SwapChainParams{}, nullptr, 600, 400));

    auto window = pandora::video::Window::Builder{}
                    .setDisplayMode(pandora::video::WindowType::window, pandora::video::WindowBehavior::globalContext, 
                                    pandora::video::ResizeMode::fixed)
                    .setSize(600,400)
                    .create(L"_INVALID_TEST0", L"Test");
    window->show();
    EXPECT_ANY_THROW(SwapChain(renderer, pandora::video::SwapChainParams{}, window->handle(), 0, 0));
    
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
    auto renderer = std::make_shared<Renderer>(monitor, Renderer::DeviceLevel::direct3D_11_0);

    pandora::video::SwapChainParams params{};
    {
      SwapChain chain1(renderer, params, window->handle(), 600, 400);
      EXPECT_FALSE(chain1.isEmpty());
      EXPECT_TRUE(chain1.handle() != nullptr);
      EXPECT_TRUE(chain1.handleLevel1() == nullptr || chain1.handleLevel1() == chain1.handle());
      EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
      EXPECT_FALSE(chain1.isHdrEnabled());
      EXPECT_EQ((uint32_t)600, chain1.width());
      EXPECT_EQ((uint32_t)400, chain1.height());
      renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
      EXPECT_EQ(size_t{1u}, renderer->activeRenderViews());
      EXPECT_NO_THROW(chain1.swapBuffers(false));
      EXPECT_NO_THROW(chain1.swapBuffersDiscard(false, nullptr));
      EXPECT_NO_THROW(chain1.swapBuffers(true));
      auto handle1 = chain1.handle();
      auto target1 = chain1.getRenderTargetView();

      SwapChain moved(std::move(chain1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(moved.handle() != nullptr);
      EXPECT_TRUE(moved.handleLevel1() == nullptr || moved.handleLevel1() == moved.handle());
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
      EXPECT_TRUE(chain1.handleLevel1() == nullptr || chain1.handleLevel1() == chain1.handle());
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
      params.setBackBufferFormat(pandora::video::ComponentFormat::rgb10a2_unorm_hdr10)
            .setFrameBufferNumber(1u).setHdrPreferred(true)
            .setRefreshRate(60000u, 1001u).setRenderTargetMode(pandora::video::SwapChainTargetMode::partialOutput);
      SwapChain chain2(renderer, params, window->handle(), 600, 400);
      EXPECT_TRUE(chain2.handle() != nullptr);
      EXPECT_TRUE(chain2.handleLevel1() == nullptr || chain2.handleLevel1() == chain2.handle());
      EXPECT_TRUE(chain2.getRenderTargetView() != nullptr);
      EXPECT_TRUE(chain2.isHdrEnabled() || !renderer->isMonitorHdrCapable(monitor));
      EXPECT_EQ((uint32_t)600, chain2.width());
      EXPECT_EQ((uint32_t)400, chain2.height());
      renderer->setActiveRenderTarget(chain2.getRenderTargetView(), nullptr);
      EXPECT_EQ(size_t{1u}, renderer->activeRenderViews());
      EXPECT_NO_THROW(chain2.swapBuffers(false));
      EXPECT_NO_THROW(chain2.swapBuffersDiscard(false, nullptr));
      EXPECT_NO_THROW(chain2.swapBuffers(true));

      params.setBackBufferFormat(pandora::video::ComponentFormat::rgba16_f_hdr_scRGB)
            .setFrameBufferNumber(2u).setHdrPreferred(true)
            .setRefreshRate(60u, 1u).setRenderTargetMode(pandora::video::SwapChainTargetMode::partialOutput);
      SwapChain chain3(renderer, params, window->handle(), 600, 400);
      EXPECT_TRUE(chain3.handle() != nullptr);
      EXPECT_TRUE(chain3.handleLevel1() == nullptr || chain3.handleLevel1() == chain3.handle());
      EXPECT_TRUE(chain3.getRenderTargetView() != nullptr);
      EXPECT_TRUE(chain3.isHdrEnabled() || !renderer->isMonitorHdrCapable(monitor));
      SwapChain chain3B(renderer, params, window->handle(), 600, 400);
      EXPECT_TRUE(chain3B.handle() != nullptr);
      EXPECT_TRUE(chain3B.handleLevel1() == nullptr || chain3B.handleLevel1() == chain3B.handle());
      EXPECT_TRUE(chain3B.getRenderTargetView() != nullptr);
      EXPECT_TRUE(chain3B.isHdrEnabled() || !renderer->isMonitorHdrCapable(monitor));

      Renderer::RenderTargetViewHandle views[]{ chain3.getRenderTargetView(), chain3B.getRenderTargetView() };
      renderer->setActiveRenderTargets(views, (size_t)2u, nullptr);
      EXPECT_EQ(size_t{2u}, renderer->activeRenderViews());
      EXPECT_NO_THROW(chain3.swapBuffers(false));
      EXPECT_NO_THROW(chain3.swapBuffers(true));
      EXPECT_NO_THROW(chain3.swapBuffersDiscard(true, nullptr));
    } // destroy to create new "unique" swap-chain

    params.setBackBufferFormat(pandora::video::ComponentFormat::rgba8_unorm)
          .setFrameBufferNumber(2u).setHdrPreferred(false)
          .setRefreshRate(60u, 1u).setRenderTargetMode(pandora::video::SwapChainTargetMode::uniqueOutput);
    SwapChain chain4(renderer, params, window->handle(), 600, 400);
    EXPECT_TRUE(chain4.handle() != nullptr);
    EXPECT_TRUE(chain4.handleLevel1() == nullptr || chain4.handleLevel1() == chain4.handle());
    EXPECT_TRUE(chain4.getRenderTargetView() != nullptr);
    EXPECT_FALSE(chain4.isHdrEnabled());
    renderer->setActiveRenderTarget(chain4.getRenderTargetView(), nullptr);
    EXPECT_EQ(size_t{1u}, renderer->activeRenderViews());
    EXPECT_NO_THROW(chain4.swapBuffers(false));
    EXPECT_NO_THROW(chain4.swapBuffers(true));
    EXPECT_NO_THROW(chain4.swapBuffersDiscard(true, nullptr));
    renderer->setActiveRenderTarget(nullptr, nullptr);
    
    DepthStencilBuffer depthBuffer(*renderer, pandora::video::ComponentFormat::d32_f, 600, 400);
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
    auto renderer = std::make_shared<Renderer>(monitor, Renderer::DeviceLevel::direct3D_11_0);

    pandora::video::SwapChainParams params{};
    SwapChain chain1(renderer, params, window->handle(), 600, 400);
    EXPECT_TRUE(chain1.handle() != nullptr);
    EXPECT_TRUE(chain1.handleLevel1() == nullptr || chain1.handleLevel1() == chain1.handle());
    EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
    EXPECT_FALSE(chain1.isHdrEnabled());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    EXPECT_EQ(size_t{1u}, renderer->activeRenderViews());
    EXPECT_NO_THROW(chain1.swapBuffers(false));
    EXPECT_NO_THROW(chain1.swapBuffersDiscard(false, nullptr));
    EXPECT_NO_THROW(chain1.swapBuffers(true));

    window->resize(640, 480);
    chain1.resize(640, 480);
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    EXPECT_EQ(size_t{1u}, renderer->activeRenderViews());

    EXPECT_TRUE(chain1.handle() != nullptr);
    EXPECT_TRUE(chain1.handleLevel1() == nullptr || chain1.handleLevel1() == chain1.handle());
    EXPECT_TRUE(chain1.getRenderTargetView() != nullptr);
    EXPECT_FALSE(chain1.isHdrEnabled());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    EXPECT_EQ(size_t{1u}, renderer->activeRenderViews());
    EXPECT_NO_THROW(chain1.swapBuffers(false));
    EXPECT_NO_THROW(chain1.swapBuffersDiscard(false, nullptr));
    EXPECT_NO_THROW(chain1.swapBuffers(true));
    
    pandora::video::Viewport viewport1(0,0, 640u,480u,0.,1.);
    renderer->setViewports(&viewport1, size_t{1u});
    renderer->setViewport(viewport1);
    pandora::video::Viewport viewports2[] = { pandora::video::Viewport(0,0, 320u,480u,0.,1.), pandora::video::Viewport(320,0, 320u,480u,0.,1.)};
    renderer->setViewport(viewports2[0]);
    renderer->setViewport(viewports2[1]);
    renderer->setViewports(viewports2, size_t{1u});
  }

#endif
