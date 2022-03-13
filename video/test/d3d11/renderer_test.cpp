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
# include <video/d3d11/sampler.h>

  using namespace pandora::video::d3d11;

  class D3d11RendererTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- create rendering device/context --

  TEST_F(D3d11RendererTest, createRendererTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    EXPECT_TRUE(renderer.device() != nullptr);
    EXPECT_TRUE(renderer.context() != nullptr);
    EXPECT_TRUE(renderer.dxgiLevel() >= (uint32_t)1u);
    EXPECT_TRUE((uint32_t)renderer.featureLevel() >= (uint32_t)D3D_FEATURE_LEVEL_11_0);

    auto colorSpace = renderer.getMonitorColorSpace(monitor);
    bool isHdrMonitor = (colorSpace == ColorSpace::hdr10_bt2084 || colorSpace == ColorSpace::scRgb);
    uint32_t maxColorSamples = 64u, maxDepthSamples = 64u, maxStencilSamples = 64u;
    while (!renderer.isColorSampleCountAvailable(DataFormat::rgba8_sRGB, maxColorSamples)
          && maxColorSamples > 1) { maxColorSamples >>= 1; }
    while (!renderer.isDepthSampleCountAvailable(DepthStencilFormat::d32_f, maxDepthSamples)
          && maxDepthSamples > 1) { maxDepthSamples >>= 1; }
    while (!renderer.isStencilSampleCountAvailable(DepthStencilFormat::d24_unorm_s8_ui, maxStencilSamples)
          && maxStencilSamples > 1) { maxStencilSamples >>= 1; }

    size_t dedicatedRam = 0, sharedRam = 0;
    EXPECT_TRUE(renderer.getAdapterVramSize(dedicatedRam, sharedRam));
    EXPECT_TRUE(sharedRam > 0); // VRAM may be 0 on headless servers, but not shared RAM

    const char* trueVal = "true";
    const char* falseVal = "false";
    printf("Direct3D context:\n > DXGI level: %u\n > Feature level: 11.%s\n > VRAM: %.3f MB\n > Shared RAM: %.3f MB\n"
           " > Max render views: %u\n > Max sampler/filter states: %u\n > Max anisotropy: %u\n"
           " > Max color samples: %u\n > Max depth samples: %u\n > Max stencil samples: %u\n"
           " > Monitor HDR capable: %s\n > Tearing available: %s\n", 
           renderer.dxgiLevel(), ((uint32_t)renderer.featureLevel() > 0) ? "1+" : "0",
           (float)dedicatedRam/1048576.0f, (float)sharedRam/1048576.0f,
           (uint32_t)renderer.maxRenderTargets(), (uint32_t)renderer.maxSamplerStateSlots(), 
           renderer.maxSamplerAnisotropy(),
           maxColorSamples, maxDepthSamples, maxStencilSamples,
           isHdrMonitor ? trueVal : falseVal,
           renderer.isTearingAvailable() ? trueVal : falseVal);

    DataFormat colorFormats[]{
      DataFormat::rgba16_f_scRGB,
      DataFormat::rgba8_sRGB,
      DataFormat::bgra8_sRGB,
      DataFormat::rgba8_unorm
    };
    EXPECT_TRUE(renderer.findSupportedDataFormat(colorFormats, sizeof(colorFormats)/sizeof(*colorFormats),
                                                 FormatAttachment::color) != DataFormat::unknown);
    EXPECT_TRUE(renderer.findSupportedDataFormat(colorFormats, sizeof(colorFormats)/sizeof(*colorFormats),
                                                 FormatAttachment::colorBlend) != DataFormat::unknown);
    DepthStencilFormat depthFormats[]{
      DepthStencilFormat::d16_unorm,
      DepthStencilFormat::d24_unorm_s8_ui,
      DepthStencilFormat::d32_f
    };
    EXPECT_TRUE((uint32_t)renderer.findSupportedDepthStencilFormat(depthFormats, sizeof(depthFormats)/sizeof(*depthFormats)) != 0);

    ColorFloat gammaCorrectWhite[4];
    float white[] { 1.f, 1.f, 1.f, 0.5f };
    renderer.sRgbToGammaCorrectColor(white, gammaCorrectWhite);
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
  }

  TEST_F(D3d11RendererTest, createSetRendererStatesTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    ASSERT_TRUE(renderer.device() != nullptr);

    GraphicsPipeline::Builder builder(renderer);
    Sampler::Builder samplerBuilder(renderer.resourceManager());

    RasterizerState emptyRaster = nullptr;
    renderer.setRasterizerState(emptyRaster);
    RasterizerState value1 = builder.createRasterizerState(RasterizerParams(CullMode::none, FillMode::fill, true, false));
    RasterizerState value2 = builder.createRasterizerState(RasterizerParams(CullMode::none, FillMode::lines, false, false));
    RasterizerState value3 = builder.createRasterizerState(RasterizerParams(CullMode::cullFront, FillMode::fill, false, true));
    RasterizerState value4 = builder.createRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::linesAA, true, true));
    renderer.setRasterizerState(value1);
    renderer.setRasterizerState(value2);
    renderer.setRasterizerState(value3);
    renderer.setRasterizerState(value4);
    
    SamplerHandle emptyFilter = nullptr;
    renderer.setFragmentSamplerStates(0, nullptr, 0);
    renderer.setFragmentSamplerStates(1, &emptyFilter, 1);
    renderer.setFragmentSamplerStates(2, &emptyFilter, 1);
    renderer.clearFragmentSamplerStates();
    
    TextureWrap addrModes[3] { 
      TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat 
    };
    Sampler sampler = samplerBuilder.createSampler(SamplerParams(TextureFilter::linear, TextureFilter::linear,
                                                                 TextureFilter::linear, addrModes, 0.f,0.f));
    renderer.setFragmentSamplerStates(1, sampler.handlePtr(), 1);
    renderer.clearFragmentSamplerStates();
    renderer.setVertexSamplerStates(1, sampler.handlePtr(), 1);
    renderer.clearVertexSamplerStates();
    renderer.setGeometrySamplerStates(1, sampler.handlePtr(), 1);
    renderer.clearGeometrySamplerStates();
    renderer.setComputeSamplerStates(1, sampler.handlePtr(), 1);
    renderer.clearComputeSamplerStates();
    renderer.setTesselControlSamplerStates(1, sampler.handlePtr(), 1);
    renderer.clearTesselControlSamplerStates();
    renderer.setTesselEvalSamplerStates(1, sampler.handlePtr(), 1);
    renderer.clearTesselEvalSamplerStates();

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

#endif
