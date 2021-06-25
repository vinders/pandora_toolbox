#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/depth_stencil_buffer.h>

  using namespace pandora::video::d3d11;

  class DepthStencilBufferTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- depth/stencil buffer creation/binding --

  TEST_F(DepthStencilBufferTest, depthStencilBufferCreationTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    
    DepthStencilBuffer emptyBuffer;
    EXPECT_TRUE(emptyBuffer.isEmpty());
    EXPECT_TRUE(emptyBuffer.handle() == nullptr);
    EXPECT_TRUE(emptyBuffer.getDepthStencilView() == nullptr);
    
    EXPECT_ANY_THROW(DepthStencilBuffer(renderer, pandora::video::ComponentFormat::d32_f, 0, 600));
    EXPECT_ANY_THROW(DepthStencilBuffer(renderer, pandora::video::ComponentFormat::d32_f, 600, 0));
    
    DepthStencilBuffer buffer1(renderer, pandora::video::ComponentFormat::d32_f, 800, 600);
    EXPECT_FALSE(buffer1.isEmpty());
    EXPECT_TRUE(buffer1.handle() != nullptr);
    EXPECT_TRUE(buffer1.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)800, buffer1.width());
    EXPECT_EQ((uint32_t)600, buffer1.height());
    EXPECT_EQ(pandora::video::ComponentFormat::d32_f, buffer1.getFormat());
    
    DepthStencilBuffer buffer2(renderer, pandora::video::ComponentFormat::d32_f, 492, 226);
    EXPECT_FALSE(buffer2.isEmpty());
    EXPECT_TRUE(buffer2.handle() != nullptr);
    EXPECT_TRUE(buffer2.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)492, buffer2.width());
    EXPECT_EQ((uint32_t)226, buffer2.height());
    EXPECT_EQ(pandora::video::ComponentFormat::d32_f, buffer2.getFormat());
    
    DepthStencilBuffer buffer3(renderer, pandora::video::ComponentFormat::d16_unorm, 640, 480);
    EXPECT_FALSE(buffer3.isEmpty());
    EXPECT_TRUE(buffer3.handle() != nullptr);
    EXPECT_TRUE(buffer3.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)640, buffer3.width());
    EXPECT_EQ((uint32_t)480, buffer3.height());
    EXPECT_EQ(pandora::video::ComponentFormat::d16_unorm, buffer3.getFormat());
    
    DepthStencilBuffer buffer4(renderer, pandora::video::ComponentFormat::d32_f_s8_ui, 64, 32);
    EXPECT_FALSE(buffer4.isEmpty());
    EXPECT_TRUE(buffer4.handle() != nullptr);
    EXPECT_TRUE(buffer4.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)64, buffer4.width());
    EXPECT_EQ((uint32_t)32, buffer4.height());
    EXPECT_EQ(pandora::video::ComponentFormat::d32_f_s8_ui, buffer4.getFormat());
    
    DepthStencilBuffer buffer5(renderer, pandora::video::ComponentFormat::d24_unorm_s8_ui, 300, 200);
    EXPECT_FALSE(buffer5.isEmpty());
    EXPECT_TRUE(buffer5.handle() != nullptr);
    EXPECT_TRUE(buffer5.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)300, buffer5.width());
    EXPECT_EQ((uint32_t)200, buffer5.height());
    EXPECT_EQ(pandora::video::ComponentFormat::d24_unorm_s8_ui, buffer5.getFormat());
    
    DepthStencilBuffer buffer6(renderer, pandora::video::ComponentFormat::d16_unorm, 1024, 768);
    EXPECT_FALSE(buffer6.isEmpty());
    EXPECT_TRUE(buffer6.handle() != nullptr);
    EXPECT_TRUE(buffer6.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)1024, buffer6.width());
    EXPECT_EQ((uint32_t)768, buffer6.height());
    EXPECT_EQ(pandora::video::ComponentFormat::d16_unorm, buffer6.getFormat());
    
    auto buffer1Handle = buffer1.handle();
    auto buffer1View = buffer1.getDepthStencilView();
    DepthStencilBuffer moved(std::move(buffer1));
    EXPECT_FALSE(moved.isEmpty());
    EXPECT_TRUE(buffer1.isEmpty());
    EXPECT_TRUE(moved.handle() == buffer1Handle);
    EXPECT_TRUE(buffer1.handle() == nullptr);
    EXPECT_TRUE(moved.getDepthStencilView() == buffer1View);
    EXPECT_TRUE(buffer1.getDepthStencilView() == nullptr);
    buffer1 = std::move(moved);
    EXPECT_FALSE(buffer1.isEmpty());
    EXPECT_TRUE(moved.isEmpty());
    EXPECT_TRUE(buffer1.handle() == buffer1Handle);
    EXPECT_TRUE(moved.handle() == nullptr);
    EXPECT_TRUE(buffer1.getDepthStencilView() == buffer1View);
    EXPECT_TRUE(moved.getDepthStencilView() == nullptr);
  }

#endif
