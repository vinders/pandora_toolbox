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
    EXPECT_ANY_THROW(Renderer((RendererDeviceLevel)1000));
  }

  TEST_F(RendererTest, createRenderer) {
    Renderer renderer(RendererDeviceLevel::direct3D_11_0);
    EXPECT_TRUE(renderer.device() != nullptr);
    EXPECT_TRUE(renderer.context() != nullptr);
    EXPECT_TRUE(renderer.dxgiLevel() >= (uint32_t)1u);
    EXPECT_TRUE((uint32_t)renderer.featureLevel() >= (uint32_t)RendererDeviceLevel::direct3D_11_0);

    EXPECT_TRUE((int)renderer.isHdrAvailable() <= (int)renderer.isTearingAvailable());
    EXPECT_TRUE((int)renderer.isFlipSwapAvailable() >= (int)renderer.isTearingAvailable());

    uint32_t qualityMSAA1 = 0, qualityMSAA2 = 0, qualityMSAA4 = 0, qualityMSAA8 = 0, qualityInvalid = 0;
    if (renderer.isMultisampleSupported(1, qualityMSAA1)) { EXPECT_NE((uint32_t)0, qualityMSAA1); }
    if (renderer.isMultisampleSupported(2, qualityMSAA2)) { EXPECT_NE((uint32_t)0, qualityMSAA2); }
    if (renderer.isMultisampleSupported(4, qualityMSAA4)) { EXPECT_NE((uint32_t)0, qualityMSAA4); }
    if (renderer.isMultisampleSupported(8, qualityMSAA8)) { EXPECT_NE((uint32_t)0, qualityMSAA8); }
    EXPECT_FALSE(renderer.isMultisampleSupported(53, qualityInvalid));

    pandora::hardware::DisplayMonitor primaryDisplay;
    if (renderer.isMonitorHdrCapable(primaryDisplay)) { EXPECT_TRUE(renderer.isHdrAvailable()); }

    size_t dedicatedRam = 0, sharedRam = 0;
    EXPECT_TRUE(renderer.getAdapterVramSize(dedicatedRam, sharedRam));
    EXPECT_TRUE(sharedRam > 0); // VRAM may be 0 on headless servers, but not shared RAM

    printf("Direct3D context:\n > DXGI level: %u\n > Feature level: 11.%u\n > VRAM: %.3f MB\n > Shared RAM: %.3f MB\n"
           " > MSAA 1x quality: %u\n > MSAA 2x quality: %u\n > MSAA 4x quality: %u\n > MSAA 8x quality: %u\n"
           " > HDR available: %s\n > Flip swap available: %s\n > Tearing available: %s\n > Local display limit available: %s\n", 
           renderer.dxgiLevel(), (uint32_t)renderer.featureLevel(), (float)dedicatedRam/1000000.0, (float)sharedRam/1000000.0,
           qualityMSAA1, qualityMSAA2, qualityMSAA4, qualityMSAA8,
           renderer.isHdrAvailable() ? "true" : "false",
           renderer.isFlipSwapAvailable() ? "true" : "false",
           renderer.isTearingAvailable() ? "true" : "false",
           renderer.isLocalOnlyOutputAvailable() ? "true" : "false");

    auto handle = renderer.device();
    auto dxgiLevel = renderer.dxgiLevel();
    auto featLevel = renderer.featureLevel();
    Renderer moved = std::move(renderer);
    EXPECT_EQ(handle, moved.device());
    EXPECT_TRUE(moved.context() != nullptr);
    EXPECT_EQ(dxgiLevel, moved.dxgiLevel());
    EXPECT_EQ(featLevel, moved.featureLevel());
  }

#endif
