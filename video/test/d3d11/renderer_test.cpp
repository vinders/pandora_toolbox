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
# include <video/d3d11/renderer.h>
# include <video/d3d11/renderer_state_factory.h>

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

  TEST_F(RendererTest, createRenderer) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    EXPECT_TRUE(renderer.device() != nullptr);
    EXPECT_TRUE(renderer.context() != nullptr);
    EXPECT_TRUE(renderer.dxgiLevel() >= (uint32_t)1u);
    EXPECT_TRUE((uint32_t)renderer.featureLevel() >= (uint32_t)D3D_FEATURE_LEVEL_11_0);

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
           " > HDR API available: %s\n > Flip swap available: %s\n > Tearing available: %s\n", 
           renderer.dxgiLevel(), ((uint32_t)renderer.featureLevel() > 0) ? "1+" : "0",
           (float)dedicatedRam/1048576.0f, (float)sharedRam/1048576.0f,
           (uint32_t)renderer.maxRenderTargets(), (uint32_t)renderer.maxFilterStateSlots(), 
           FilterParams::maxAnisotropy(), 
           isMonitorHdr ? trueVal : falseVal,
           renderer.isHdrAvailable() ? trueVal : falseVal,
           renderer.isFlipSwapAvailable() ? trueVal : falseVal,
           renderer.isTearingAvailable() ? trueVal : falseVal);

    ColorChannel gammaCorrectWhite[4];
    float white[] { 1.f, 1.f, 1.f, 0.5f };
    renderer.toGammaCorrectColor(white, gammaCorrectWhite);
    EXPECT_TRUE(gammaCorrectWhite[0] >= 0.9f && gammaCorrectWhite[0] <= 1.2f);
    EXPECT_TRUE(gammaCorrectWhite[1] >= 0.9f && gammaCorrectWhite[1] <= 1.2f);
    EXPECT_TRUE(gammaCorrectWhite[2] >= 0.9f && gammaCorrectWhite[2] <= 1.2f);
    EXPECT_EQ(0.5f, gammaCorrectWhite[3]);

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
    Renderer renderer(monitor);
    ASSERT_TRUE(renderer.device() != nullptr);

    RendererStateFactory factory(renderer);

    RasterizerState emptyRaster = nullptr;
    renderer.setRasterizerState(emptyRaster);
    RasterizerState value1 = factory.createRasterizerState(RasterizerParams(CullMode::none, FillMode::fill, true, false));
    RasterizerState value2 = factory.createRasterizerState(RasterizerParams(CullMode::none, FillMode::lines, false, false));
    RasterizerState value3 = factory.createRasterizerState(RasterizerParams(CullMode::cullFront, FillMode::fill, false, true));
    RasterizerState value4 = factory.createRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::linesAA, true, true));
    renderer.setRasterizerState(value1);
    renderer.setRasterizerState(value2);
    renderer.setRasterizerState(value3);
    renderer.setRasterizerState(value4);
    
    FilterStateArray::State emptyFilter = nullptr;
    renderer.setFragmentFilterStates(0, nullptr, 0);
    renderer.setFragmentFilterStates(1, &emptyFilter, 1);
    renderer.setFragmentFilterStates(2, &emptyFilter, 1);
    renderer.clearFragmentFilterStates();
    
    FilterStateArray valueContainer;
    TextureWrap addrModes[3] { 
      TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat 
    };
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear, addrModes, 0.f,0.f)));
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

    RenderTargetView emptyView = nullptr;
    renderer.setActiveRenderTargets(nullptr, 0, nullptr);
    renderer.setActiveRenderTargets(&emptyView, 1, nullptr);
    renderer.setActiveRenderTarget(nullptr, nullptr);
    renderer.setActiveRenderTarget(emptyView, nullptr);
    renderer.setCleanActiveRenderTargets(nullptr, 0, nullptr, nullptr);
    renderer.setCleanActiveRenderTargets(&emptyView, 1, nullptr, nullptr);
    renderer.setCleanActiveRenderTarget(nullptr, nullptr, nullptr);
    renderer.setCleanActiveRenderTarget(emptyView, nullptr, nullptr);
    renderer.clearViews(nullptr, 0, nullptr, nullptr);
    renderer.clearViews(&emptyView, 1, nullptr, nullptr);
    renderer.clearView(nullptr, nullptr, nullptr);
    renderer.clearView(emptyView, nullptr, nullptr);
    
    Viewport viewport1(0,0, 640u,480u,0.,1.);
    renderer.setViewports(&viewport1, size_t{1u});
    renderer.setViewport(viewport1);
  }
  
  TEST_F(RendererTest, createFilterStateParams) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    ASSERT_TRUE(renderer.device() != nullptr);

    RendererStateFactory factory(renderer);

    FilterStateArray valueContainer;
    TextureWrap addrModes[3] { 
      TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat 
    };
    
    // base filters
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                                 addrModes, 0.f,0.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToBorder;
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::nearest, TextureFilter::nearest,
                                                                 addrModes, 0.f,1.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::repeatMirror;
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest,
                                                                 addrModes, 1.f,8.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToEdge;
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                                 addrModes, 8.f,8.f)));
    valueContainer.clear();
    
    addrModes[0] = TextureWrap::repeat;
    addrModes[1] = TextureWrap::clampToBorder;
    addrModes[2] = TextureWrap::clampToEdge;
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear,
                                                                 addrModes, 0.f,4.f)));
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                                 addrModes, 0.f,0.f)));
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::linear,
                                                                 addrModes, 1.f,1.f)));
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                                 addrModes, 0.f,1.f)));
    valueContainer.clear();
    
    // comparison filters
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                                 addrModes, 0.f,0.f, StencilCompare::always)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToBorder;
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::nearest, TextureFilter::nearest,
                                                                 addrModes, 0.f,1.f, StencilCompare::greater)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::repeatMirror;
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest,
                                                                 addrModes, 1.f,8.f, StencilCompare::equal)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToEdge;
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                                 addrModes, 8.f,8.f, StencilCompare::notEqual)));
    valueContainer.clear();
    
    addrModes[0] = TextureWrap::repeat;
    addrModes[1] = TextureWrap::clampToBorder;
    addrModes[2] = TextureWrap::clampToEdge;
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear,
                                                                 addrModes, 0.f,4.f, StencilCompare::never)));
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                                 addrModes, 0.f,0.f, StencilCompare::less)));
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::linear,
                                                                 addrModes, 1.f,1.f, StencilCompare::lessEqual)));
    valueContainer.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                                 addrModes, 0.f,1.f, StencilCompare::always)));
    valueContainer.clear();
    
    // anisotropic filters
    valueContainer.append(factory.createFilterState(FilterParams(1u, addrModes, 0.f,0.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToBorder;
    valueContainer.append(factory.createFilterState(FilterParams(2u, addrModes, 0.f,1.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::repeatMirror;
    valueContainer.append(factory.createFilterState(FilterParams(4u, addrModes, 1.f,8.f)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToEdge;
    valueContainer.append(factory.createFilterState(FilterParams(8u, addrModes, 8.f,8.f)));
    valueContainer.clear();
    
    addrModes[0] = TextureWrap::repeat;
    addrModes[1] = TextureWrap::clampToBorder;
    addrModes[2] = TextureWrap::clampToEdge;
    valueContainer.append(factory.createFilterState(FilterParams(FilterParams::maxAnisotropy(), addrModes, 0.f,4.f)));
    valueContainer.append(factory.createFilterState(FilterParams(FilterParams::maxAnisotropy()/2u, addrModes, 0.f,0.f)));
    valueContainer.clear();
    
    // comparison anisotropic filters
    valueContainer.append(factory.createFilterState(FilterParams(1u, addrModes, 0.f,0.f, StencilCompare::always)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToBorder;
    valueContainer.append(factory.createFilterState(FilterParams(2u, addrModes, 0.f,1.f, StencilCompare::never)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::repeatMirror;
    valueContainer.append(factory.createFilterState(FilterParams(4u, addrModes, 1.f,8.f, StencilCompare::equal)));
    addrModes[0] = addrModes[1] = addrModes[2] = TextureWrap::clampToEdge;
    valueContainer.append(factory.createFilterState(FilterParams(8u, addrModes, 8.f,8.f, StencilCompare::notEqual)));
    valueContainer.clear();
    
    addrModes[0] = TextureWrap::repeat;
    addrModes[1] = TextureWrap::clampToBorder;
    addrModes[2] = TextureWrap::clampToEdge;
    valueContainer.append(factory.createFilterState(FilterParams(FilterParams::maxAnisotropy(), addrModes, 0.f,4.f, StencilCompare::less)));
    valueContainer.append(factory.createFilterState(FilterParams(FilterParams::maxAnisotropy()/2u, addrModes, 0.f,0.f, StencilCompare::greater)));
    valueContainer.clear();
  }

#endif
