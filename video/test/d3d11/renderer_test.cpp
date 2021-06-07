#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <video/d3d11/renderer.h>

  using namespace pandora::video::d3d11;

  class RendererTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- create rendering device/context --

  TEST_F(RendererTest, createRendererInvalidParams) {
    pandora::hardware::DisplayMonitor monitor;
    EXPECT_ANY_THROW(Renderer(monitor, (Renderer::DeviceLevel)1000));
  }

  TEST_F(RendererTest, createRenderer) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor, Renderer::DeviceLevel::direct3D_11_0);
    EXPECT_TRUE(renderer.device() != nullptr);
    EXPECT_TRUE(renderer.context() != nullptr);
    EXPECT_TRUE(renderer.dxgiLevel() >= (uint32_t)1u);
    EXPECT_TRUE((uint32_t)renderer.featureLevel() >= (uint32_t)Renderer::DeviceLevel::direct3D_11_0);

    EXPECT_TRUE((int)renderer.isHdrAvailable() >= (int)renderer.isTearingAvailable());
    EXPECT_TRUE((int)renderer.isFlipSwapAvailable() >= (int)renderer.isTearingAvailable());

    bool hasMSAA1 = renderer.isMultisampleSupported(1, pandora::video::ComponentFormat::rgba8_sRGB);
    EXPECT_TRUE(hasMSAA1);
    bool hasMSAA2 = renderer.isMultisampleSupported(2, pandora::video::ComponentFormat::rgba8_sRGB);
    bool hasMSAA4 = renderer.isMultisampleSupported(4, pandora::video::ComponentFormat::rgba8_sRGB);
    bool hasMSAA8 = renderer.isMultisampleSupported(8, pandora::video::ComponentFormat::rgba8_sRGB);
    EXPECT_FALSE(renderer.isMultisampleSupported(53, pandora::video::ComponentFormat::rgba8_sRGB));

    bool isMonitorHdr = renderer.isMonitorHdrCapable(monitor);
    if (isMonitorHdr) { EXPECT_TRUE(renderer.isHdrAvailable()); }
    size_t dedicatedRam = 0, sharedRam = 0;
    EXPECT_TRUE(renderer.getAdapterVramSize(dedicatedRam, sharedRam));
    EXPECT_TRUE(sharedRam > 0); // VRAM may be 0 on headless servers, but not shared RAM

    const char* trueVal = "true";
    const char* falseVal = "false";
    printf("Direct3D context:\n > DXGI level: %u\n > Feature level: 11.%s\n > VRAM: %.3f MB\n > Shared RAM: %.3f MB\n"
           " > MSAA 1x: %s\n > MSAA 2x: %s\n > MSAA 4x: %s\n > MSAA 8x: %s\n"
           " > Max render views: %u\n > Max sampler/filter states: %u\n > Monitor HDR capable: %s\n"
           " > HDR API available: %s\n > Flip swap available: %s\n > Tearing available: %s\n > Local display limit available: %s\n", 
           renderer.dxgiLevel(), ((uint32_t)renderer.featureLevel() > 0) ? "1+" : "0",
           (float)dedicatedRam/1048576.0f, (float)sharedRam/1048576.0f,
           hasMSAA1 ? trueVal : falseVal, hasMSAA2 ? trueVal : falseVal, hasMSAA4 ? trueVal : falseVal, hasMSAA8 ? trueVal : falseVal,
           (uint32_t)renderer.maxSimultaneousRenderViews(), (uint32_t)renderer.maxFilterStates(),
           isMonitorHdr ? trueVal : falseVal,
           renderer.isHdrAvailable() ? trueVal : falseVal,
           renderer.isFlipSwapAvailable() ? trueVal : falseVal,
           renderer.isTearingAvailable() ? trueVal : falseVal,
           renderer.isLocalDisplayRestrictionAvailable() ? trueVal : falseVal);

    auto handle = renderer.device();
    auto dxgiLevel = renderer.dxgiLevel();
    auto featLevel = renderer.featureLevel();
    Renderer moved = std::move(renderer);
    EXPECT_EQ(handle, moved.device());
    EXPECT_TRUE(moved.context() != nullptr);
    EXPECT_EQ(dxgiLevel, moved.dxgiLevel());
    EXPECT_EQ(featLevel, moved.featureLevel());
    renderer = std::move(moved);
    EXPECT_EQ(handle, renderer.device());
    EXPECT_TRUE(renderer.context() != nullptr);
    EXPECT_EQ(dxgiLevel, renderer.dxgiLevel());
    EXPECT_EQ(featLevel, renderer.featureLevel());
    EXPECT_EQ(isMonitorHdr, renderer.isMonitorHdrCapable(monitor));
  }

  TEST_F(RendererTest, createSetRendererStates) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor, Renderer::DeviceLevel::direct3D_11_0);
    ASSERT_TRUE(renderer.device() != nullptr);

    RasterizerState emptyRaster = nullptr;
    renderer.setRasterizerState(emptyRaster);
    RasterizerState value1 = renderer.createRasterizerState(pandora::video::CullMode::none, true, 
                                                            pandora::video::DepthBias{ 0, 0.f, 0.f, false }, true, false);
    RasterizerState value2 = renderer.createRasterizerState(pandora::video::CullMode::wireFrame, false, 
                                                            pandora::video::DepthBias{ 100, 0.5f, 0.5f, true }, false, false);
    RasterizerState value3 = renderer.createRasterizerState(pandora::video::CullMode::cullFront, false, 
                                                            pandora::video::DepthBias{ -100, -0.5f, 0.1f, false }, false, true);
    RasterizerState value4 = renderer.createRasterizerState(pandora::video::CullMode::cullBack, true, 
                                                            pandora::video::DepthBias{ 0, -0.25f, 1.f, true }, true, true);
    renderer.setRasterizerState(value1);
    renderer.setRasterizerState(value2);
    renderer.setRasterizerState(value3);
    renderer.setRasterizerState(value4);

    FilterStates::State emptyFilter = nullptr;
    renderer.setFilterStates(0, nullptr, 0);
    renderer.setFilterStates(1, &emptyFilter, 1);
    renderer.setFilterStates(2, &emptyFilter, 1);
    renderer.setFilterStates(999999, &emptyFilter, 1);
    renderer.clearFilterStates();
    FilterStates valueContainer;
    renderer.createFilterState(valueContainer);
    renderer.setFilterStates(1, &emptyFilter, 1);
    renderer.clearFilterStates();

    Renderer::RenderTargetViewHandle emptyView = nullptr;
    renderer.setActiveRenderTargets(nullptr, 0, nullptr);
    renderer.setActiveRenderTargets(&emptyView, 1, nullptr);
    renderer.setActiveRenderTarget(nullptr, nullptr);
    renderer.setActiveRenderTarget(emptyView, nullptr);
    renderer.setActiveRenderTargets(nullptr, 0, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    renderer.setActiveRenderTargets(&emptyView, 1, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    renderer.setActiveRenderTarget(nullptr, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    renderer.setActiveRenderTarget(emptyView, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    renderer.clear(nullptr, 0, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    renderer.clear(&emptyView, 1, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    renderer.clear(nullptr, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    renderer.clear(emptyView, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
  }

#endif
