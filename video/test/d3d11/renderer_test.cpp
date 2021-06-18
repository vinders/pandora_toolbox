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

    bool isMonitorHdr = renderer.isMonitorHdrCapable(monitor);
    if (isMonitorHdr) { EXPECT_TRUE(renderer.isHdrAvailable()); }
    size_t dedicatedRam = 0, sharedRam = 0;
    EXPECT_TRUE(renderer.getAdapterVramSize(dedicatedRam, sharedRam));
    EXPECT_TRUE(sharedRam > 0); // VRAM may be 0 on headless servers, but not shared RAM

    const char* trueVal = "true";
    const char* falseVal = "false";
    printf("Direct3D context:\n > DXGI level: %u\n > Feature level: 11.%s\n > VRAM: %.3f MB\n > Shared RAM: %.3f MB\n"
           " > Max render views: %u\n > Max sampler/filter states: %u\n > Max anisotropy: %u\n > Monitor HDR capable: %s\n"
           " > HDR API available: %s\n > Flip swap available: %s\n > Tearing available: %s\n > Local display limit available: %s\n", 
           renderer.dxgiLevel(), ((uint32_t)renderer.featureLevel() > 0) ? "1+" : "0",
           (float)dedicatedRam/1048576.0f, (float)sharedRam/1048576.0f,
           (uint32_t)renderer.maxSimultaneousRenderViews(), (uint32_t)renderer.maxFilterStateSlots(), 
           renderer.maxAnisotropy(), 
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
                                                            pandora::video::DepthBias{ 0, 0.f, 0.f, false }, false);
    RasterizerState value2 = renderer.createRasterizerState(pandora::video::CullMode::wireFrame, false, 
                                                            pandora::video::DepthBias{ 100, 0.5f, 0.5f, true }, false);
    RasterizerState value3 = renderer.createRasterizerState(pandora::video::CullMode::cullFront, false, 
                                                            pandora::video::DepthBias{ -100, -0.5f, 0.1f, false }, true);
    RasterizerState value4 = renderer.createRasterizerState(pandora::video::CullMode::cullBack, true, 
                                                            pandora::video::DepthBias{ 0, -0.25f, 1.f, true }, true);
    renderer.setRasterizerState(value1);
    renderer.setRasterizerState(value2);
    renderer.setRasterizerState(value3);
    renderer.setRasterizerState(value4);
    
    FilterStates::State emptyFilter = nullptr;
    renderer.setFragmentFilterStates(0, nullptr, 0);
    renderer.setFragmentFilterStates(1, &emptyFilter, 1);
    renderer.setFragmentFilterStates(2, &emptyFilter, 1);
    renderer.setFragmentFilterStates(999999, &emptyFilter, 1);
    renderer.clearFragmentFilterStates();
    
    FilterStates valueContainer;
    pandora::video::TextureAddressMode addrModes[3] { 
      pandora::video::TextureAddressMode::repeat, pandora::video::TextureAddressMode::repeat, pandora::video::TextureAddressMode::repeat 
    };
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::linear, 
                          pandora::video::MagnificationFilter::linear, addrModes, 0.,0.);
    renderer.setFragmentFilterStates(1, valueContainer.get(), 1);
    renderer.clearFragmentFilterStates();
    renderer.setVertexFilterStates(1, valueContainer.get(), 1);
    renderer.clearVertexFilterStates();
    renderer.setGeometryFilterStates(1, valueContainer.get(), 1);
    renderer.clearGeometryFilterStates();
    renderer.setComputeFilterStates(1, valueContainer.get(), 1);
    renderer.clearComputeFilterStates();
    renderer.setTesselControlFilterStates(1, valueContainer.get(), 1);
    renderer.clearTesselControlFilterStates();
    renderer.setTesselEvalFilterStates(1, valueContainer.get(), 1);
    renderer.clearTesselEvalFilterStates();

    Renderer::RenderTargetViewHandle emptyView = nullptr;
    renderer.setActiveRenderTargets(nullptr, 0, nullptr);
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.setActiveRenderTargets(&emptyView, 1, nullptr);
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.setActiveRenderTarget(nullptr, nullptr);
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.setActiveRenderTarget(emptyView, nullptr);
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.setActiveRenderTargets(nullptr, 0, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.setActiveRenderTargets(&emptyView, 1, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.setActiveRenderTarget(nullptr, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.setActiveRenderTarget(emptyView, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.clearViews(nullptr, 0, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.clearViews(&emptyView, 1, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.clearView(nullptr, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    renderer.clearView(emptyView, nullptr, pandora::video::ComponentVector128{{{ 0.f,0.f,0.f,0.f }}});
    EXPECT_EQ(size_t{0}, renderer.activeRenderViews());
    
    pandora::video::Viewport viewport1(0,0, 640u,480u,0.,1.);
    renderer.setViewports(&viewport1, size_t{1u});
    renderer.setViewport(viewport1);
  }
  
  TEST_F(RendererTest, createFilterStateParams) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor, Renderer::DeviceLevel::direct3D_11_0);
    ASSERT_TRUE(renderer.device() != nullptr);

    FilterStates valueContainer;
    pandora::video::TextureAddressMode addrModes[3] { 
      pandora::video::TextureAddressMode::repeat, pandora::video::TextureAddressMode::repeat, pandora::video::TextureAddressMode::repeat 
    };
    float borderColor[4] = { 0.,0.,0.,1. };
    
    // base filters
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::nearest,
                          pandora::video::MagnificationFilter::nearest, addrModes, 0.,0., 0.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::border;
    borderColor[0] = borderColor[1] = borderColor[2] = 1.;
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::linear,
                          pandora::video::MagnificationFilter::nearest, addrModes, 0.,1., 1.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::repeatMirror;
    borderColor[0] = borderColor[1] = borderColor[2] = 0.5;
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::nearest,
                          pandora::video::MagnificationFilter::linear, addrModes, 1.,8., -1.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::clamp;
    borderColor[0] = 0.; borderColor[1] = 0.5; borderColor[2] = 1.; borderColor[3] = 0.5;
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::linear,
                          pandora::video::MagnificationFilter::linear, addrModes, 8.,8., 2.0, borderColor);
    valueContainer.clear();
    
    addrModes[0] = pandora::video::TextureAddressMode::repeat;
    addrModes[1] = pandora::video::TextureAddressMode::border;
    addrModes[2] = pandora::video::TextureAddressMode::clamp;
    borderColor[0] = 1.; borderColor[1] = 1.; borderColor[2] = 1.; borderColor[3] = 0.;
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::linear_mipLinear,
                          pandora::video::MagnificationFilter::linear, addrModes, 0.,4., 0.0, borderColor);
    borderColor[0] = 0.; borderColor[1] = 0.; borderColor[2] = 0.; borderColor[3] = 1.;
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::nearest_mipNearest,
                          pandora::video::MagnificationFilter::nearest, addrModes, 0.,0., 1.0, borderColor);
    borderColor[0] = 1.; borderColor[1] = 1.; borderColor[2] = 1.; borderColor[3] = 1.;
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::nearest_mipLinear,
                          pandora::video::MagnificationFilter::nearest, addrModes, 1.,1., 0.0, borderColor);
    borderColor[0] = 0.; borderColor[1] = 0.5; borderColor[2] = 0.; borderColor[3] = 0.5;
    renderer.createFilter(valueContainer, pandora::video::MinificationFilter::linear_mipNearest,
                          pandora::video::MagnificationFilter::linear, addrModes, 0.,1., -1.0, borderColor);
    valueContainer.clear();
    
    // comparison filters
    borderColor[0] = borderColor[1] = borderColor[2] = 0.; borderColor[3] = 1.;
    renderer.createComparedFilter(valueContainer, pandora::video::MinificationFilter::nearest, pandora::video::MagnificationFilter::nearest,
                                  addrModes, pandora::video::DepthComparison::always, 0.,0., 0.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::border;
    borderColor[0] = borderColor[1] = borderColor[2] = 1.;
    renderer.createComparedFilter(valueContainer, pandora::video::MinificationFilter::linear, pandora::video::MagnificationFilter::nearest,
                                  addrModes, pandora::video::DepthComparison::greater, 0.,1., 1.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::repeatMirror;
    borderColor[0] = borderColor[1] = borderColor[2] = 0.5;
    renderer.createComparedFilter(valueContainer, pandora::video::MinificationFilter::nearest, pandora::video::MagnificationFilter::linear,
                                  addrModes, pandora::video::DepthComparison::equal, 1.,8., -1.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::clamp;
    borderColor[0] = 0.; borderColor[1] = 0.5; borderColor[2] = 1.; borderColor[3] = 0.5;
    renderer.createComparedFilter(valueContainer, pandora::video::MinificationFilter::linear, pandora::video::MagnificationFilter::linear,
                                  addrModes, pandora::video::DepthComparison::notEqual, 8.,8., 2.0, borderColor);
    valueContainer.clear();
    
    addrModes[0] = pandora::video::TextureAddressMode::repeat;
    addrModes[1] = pandora::video::TextureAddressMode::border;
    addrModes[2] = pandora::video::TextureAddressMode::clamp;
    borderColor[0] = 1.; borderColor[1] = 1.; borderColor[2] = 1.; borderColor[3] = 0.;
    renderer.createComparedFilter(valueContainer, pandora::video::MinificationFilter::linear_mipLinear, pandora::video::MagnificationFilter::linear,
                                  addrModes, pandora::video::DepthComparison::never, 0.,4., 0.0, borderColor);
    borderColor[0] = 0.; borderColor[1] = 0.; borderColor[2] = 0.; borderColor[3] = 1.;
    renderer.createComparedFilter(valueContainer, pandora::video::MinificationFilter::nearest_mipNearest, pandora::video::MagnificationFilter::nearest,
                                  addrModes, pandora::video::DepthComparison::less, 0.,0., 1.0, borderColor);
    borderColor[0] = 1.; borderColor[1] = 1.; borderColor[2] = 1.; borderColor[3] = 1.;
    renderer.createComparedFilter(valueContainer, pandora::video::MinificationFilter::nearest_mipLinear, pandora::video::MagnificationFilter::nearest,
                                  addrModes, pandora::video::DepthComparison::lessEqual, 1.,1., 0.0, borderColor);
    borderColor[0] = 0.; borderColor[1] = 0.5; borderColor[2] = 0.; borderColor[3] = 0.5;
    renderer.createComparedFilter(valueContainer, pandora::video::MinificationFilter::linear_mipNearest, pandora::video::MagnificationFilter::linear,
                                  addrModes, pandora::video::DepthComparison::always, 0.,1., -1.0, borderColor);
    valueContainer.clear();
    
    // anisotropic filters
    borderColor[0] = borderColor[1] = borderColor[2] = 0.; borderColor[3] = 1.;
    renderer.createAnisotropicFilter(valueContainer, 1u, addrModes, 0.,0., 0.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::border;
    borderColor[0] = borderColor[1] = borderColor[2] = 1.;
    renderer.createAnisotropicFilter(valueContainer, 2u, addrModes, 0.,1., 1.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::repeatMirror;
    borderColor[0] = borderColor[1] = borderColor[2] = 0.5;
    renderer.createAnisotropicFilter(valueContainer, 4u, addrModes, 1.,8., -1.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::clamp;
    borderColor[0] = 0.; borderColor[1] = 0.5; borderColor[2] = 1.; borderColor[3] = 0.5;
    renderer.createAnisotropicFilter(valueContainer, 8u, addrModes, 8.,8., 2.0, borderColor);
    valueContainer.clear();
    
    addrModes[0] = pandora::video::TextureAddressMode::repeat;
    addrModes[1] = pandora::video::TextureAddressMode::border;
    addrModes[2] = pandora::video::TextureAddressMode::clamp;
    borderColor[0] = 1.; borderColor[1] = 1.; borderColor[2] = 1.; borderColor[3] = 0.;
    renderer.createAnisotropicFilter(valueContainer, renderer.maxAnisotropy(), addrModes, 0.,4., 0.0, borderColor);
    borderColor[0] = 0.; borderColor[1] = 0.; borderColor[2] = 0.; borderColor[3] = 1.;
    renderer.createAnisotropicFilter(valueContainer, renderer.maxAnisotropy()/2u, addrModes, 0.,0., 1.0, borderColor);
    valueContainer.clear();
    
    // comparison anisotropic filters
    borderColor[0] = borderColor[1] = borderColor[2] = 0.; borderColor[3] = 1.;
    renderer.createComparedAnisotropicFilter(valueContainer, 1u, addrModes, pandora::video::DepthComparison::always, 0.,0., 0.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::border;
    borderColor[0] = borderColor[1] = borderColor[2] = 1.;
    renderer.createComparedAnisotropicFilter(valueContainer, 2u, addrModes, pandora::video::DepthComparison::never, 0.,1., 1.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::repeatMirror;
    borderColor[0] = borderColor[1] = borderColor[2] = 0.5;
    renderer.createComparedAnisotropicFilter(valueContainer, 4u, addrModes, pandora::video::DepthComparison::equal, 1.,8., -1.0, borderColor);
    addrModes[0] = addrModes[1] = addrModes[2] = pandora::video::TextureAddressMode::clamp;
    borderColor[0] = 0.; borderColor[1] = 0.5; borderColor[2] = 1.; borderColor[3] = 0.5;
    renderer.createComparedAnisotropicFilter(valueContainer, 8u, addrModes, pandora::video::DepthComparison::notEqual, 8.,8., 2.0, borderColor);
    valueContainer.clear();
    
    addrModes[0] = pandora::video::TextureAddressMode::repeat;
    addrModes[1] = pandora::video::TextureAddressMode::border;
    addrModes[2] = pandora::video::TextureAddressMode::clamp;
    borderColor[0] = 1.; borderColor[1] = 1.; borderColor[2] = 1.; borderColor[3] = 0.;
    renderer.createComparedAnisotropicFilter(valueContainer, renderer.maxAnisotropy(), addrModes, pandora::video::DepthComparison::less, 0.,4., 0.0, borderColor);
    borderColor[0] = 0.; borderColor[1] = 0.; borderColor[2] = 0.; borderColor[3] = 1.;
    renderer.createComparedAnisotropicFilter(valueContainer, renderer.maxAnisotropy()/2u, addrModes, pandora::video::DepthComparison::greater, 0.,0., 1.0, borderColor);
    valueContainer.clear();
  }

#endif
