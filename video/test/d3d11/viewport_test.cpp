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
# include <video/d3d11/viewport.h>

  using namespace pandora::video::d3d11;

  class ViewportTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- viewport container --

  TEST_F(ViewportTest, createCopyMoveTest) {
    Viewport vp0;
    EXPECT_EQ((int32_t)0, vp0.x());
    EXPECT_EQ((int32_t)0, vp0.y());
    EXPECT_EQ((uint32_t)0, vp0.width());
    EXPECT_EQ((uint32_t)0, vp0.height());
    EXPECT_EQ(0.f, vp0.nearClipping());
    EXPECT_EQ(1.f, vp0.farClipping());

    Viewport vp1(640u,480u);
    EXPECT_EQ((int32_t)0, vp1.x());
    EXPECT_EQ((int32_t)0, vp1.y());
    EXPECT_EQ((uint32_t)640u, vp1.width());
    EXPECT_EQ((uint32_t)480u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());
    
    Viewport vp2(0,0, 640u,480u);
    EXPECT_EQ((int32_t)0, vp2.x());
    EXPECT_EQ((int32_t)0, vp2.y());
    EXPECT_EQ((uint32_t)640u, vp2.width());
    EXPECT_EQ((uint32_t)480u, vp2.height());
    EXPECT_EQ(0.f, vp2.nearClipping());
    EXPECT_EQ(1.f, vp2.farClipping());
    
    Viewport vp3(320,240, 640u,480u);
    EXPECT_EQ((int32_t)320, vp3.x());
    EXPECT_EQ((int32_t)240, vp3.y());
    EXPECT_EQ((uint32_t)640u, vp3.width());
    EXPECT_EQ((uint32_t)480u, vp3.height());
    EXPECT_EQ(0.f, vp3.nearClipping());
    EXPECT_EQ(1.f, vp3.farClipping());

    Viewport vp4(0,0, 640u,480u, 0.5f,1.2f);
    EXPECT_EQ((int32_t)0, vp4.x());
    EXPECT_EQ((int32_t)0, vp4.y());
    EXPECT_EQ((uint32_t)640u, vp4.width());
    EXPECT_EQ((uint32_t)480u, vp4.height());
    EXPECT_EQ(0.5f, vp4.nearClipping());
    EXPECT_EQ(1.2f, vp4.farClipping());
    
    Viewport vp5(320,240, 640u,480u, 0.1f,1.2f);
    EXPECT_EQ((int32_t)320, vp5.x());
    EXPECT_EQ((int32_t)240, vp5.y());
    EXPECT_EQ((uint32_t)640u, vp5.width());
    EXPECT_EQ((uint32_t)480u, vp5.height());
    EXPECT_EQ(0.1f, vp5.nearClipping());
    EXPECT_EQ(1.2f, vp5.farClipping());
    
    Viewport copied(vp5);
    EXPECT_EQ((int32_t)320, copied.x());
    EXPECT_EQ((int32_t)240, copied.y());
    EXPECT_EQ((uint32_t)640u, copied.width());
    EXPECT_EQ((uint32_t)480u, copied.height());
    EXPECT_EQ(0.1f, copied.nearClipping());
    EXPECT_EQ(1.2f, copied.farClipping());
    
    Viewport moved(std::move(vp5));
    EXPECT_EQ((int32_t)320, moved.x());
    EXPECT_EQ((int32_t)240, moved.y());
    EXPECT_EQ((uint32_t)640u, moved.width());
    EXPECT_EQ((uint32_t)480u, moved.height());
    EXPECT_EQ(0.1f, moved.nearClipping());
    EXPECT_EQ(1.2f, moved.farClipping());
    
    vp5 = std::move(vp4);
    EXPECT_EQ((int32_t)0, vp5.x());
    EXPECT_EQ((int32_t)0, vp5.y());
    EXPECT_EQ((uint32_t)640u, vp5.width());
    EXPECT_EQ((uint32_t)480u, vp5.height());
    EXPECT_EQ(0.5f, vp5.nearClipping());
    EXPECT_EQ(1.2f, vp5.farClipping());
    
    vp5 = vp3;
    EXPECT_EQ((int32_t)320, vp5.x());
    EXPECT_EQ((int32_t)240, vp5.y());
    EXPECT_EQ((uint32_t)640u, vp5.width());
    EXPECT_EQ((uint32_t)480u, vp5.height());
    EXPECT_EQ(0.f, vp5.nearClipping());
    EXPECT_EQ(1.f, vp5.farClipping());
    EXPECT_TRUE(vp5 == vp3);
    EXPECT_FALSE(vp5 != vp3);
    EXPECT_FALSE(vp5 == vp1);
    EXPECT_TRUE(vp5 != vp1);
  }

  TEST_F(ViewportTest, viewportSettersTest) {
    Viewport vp1(640u,480u);
    EXPECT_EQ((int32_t)0, vp1.x());
    EXPECT_EQ((int32_t)0, vp1.y());
    EXPECT_EQ((uint32_t)640u, vp1.width());
    EXPECT_EQ((uint32_t)480u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());
    
    vp1.resize(10,20, 240u,180u);
    EXPECT_EQ((int32_t)10, vp1.x());
    EXPECT_EQ((int32_t)20, vp1.y());
    EXPECT_EQ((uint32_t)240u, vp1.width());
    EXPECT_EQ((uint32_t)180u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());

    vp1.resizeFromBottomLeft(200u, 40,50, 260u,120u);
    EXPECT_EQ((int32_t)40, vp1.x());
    EXPECT_EQ((int32_t)30, vp1.y());
    EXPECT_EQ((uint32_t)260u, vp1.width());
    EXPECT_EQ((uint32_t)120u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());
    
    vp1.resizeFromTopLeft(800u, 40,50, 260u,120u);
    EXPECT_EQ((int32_t)40, vp1.x());
    EXPECT_EQ((int32_t)50, vp1.y());
    EXPECT_EQ((uint32_t)260u, vp1.width());
    EXPECT_EQ((uint32_t)120u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());
    
    vp1.setDepthRange(0.4, 1.6);
    EXPECT_EQ((int32_t)40, vp1.x());
    EXPECT_EQ((int32_t)50, vp1.y());
    EXPECT_EQ((uint32_t)260u, vp1.width());
    EXPECT_EQ((uint32_t)120u, vp1.height());
    EXPECT_EQ(0.4f, vp1.nearClipping());
    EXPECT_EQ(1.6f, vp1.farClipping());
  }

  TEST_F(ViewportTest, viewportFPSettersTest) {
    Viewport vp1(0.f,0.f,640.f,480.f);
    EXPECT_EQ((int32_t)0, vp1.x());
    EXPECT_EQ((int32_t)0, vp1.y());
    EXPECT_EQ((uint32_t)640u, vp1.width());
    EXPECT_EQ((uint32_t)480u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());

    vp1.resize(10.f,20.f, 240.f,180.f);
    EXPECT_EQ((int32_t)10, vp1.x());
    EXPECT_EQ((int32_t)20, vp1.y());
    EXPECT_EQ((uint32_t)240u, vp1.width());
    EXPECT_EQ((uint32_t)180u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());

    vp1.resizeFromBottomLeft(200u, 40.f,50.f, 260.f,120.f);
    EXPECT_EQ((int32_t)40, vp1.x());
    EXPECT_EQ((int32_t)30, vp1.y());
    EXPECT_EQ((uint32_t)260u, vp1.width());
    EXPECT_EQ((uint32_t)120u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());

    vp1.resizeFromTopLeft(800u, 40.f,50.f, 260.f,120.f);
    EXPECT_EQ((int32_t)40, vp1.x());
    EXPECT_EQ((int32_t)50, vp1.y());
    EXPECT_EQ((uint32_t)260u, vp1.width());
    EXPECT_EQ((uint32_t)120u, vp1.height());
    EXPECT_EQ(0.f, vp1.nearClipping());
    EXPECT_EQ(1.f, vp1.farClipping());

    vp1.setDepthRange(0.4, 1.6);
    EXPECT_EQ((int32_t)40, vp1.x());
    EXPECT_EQ((int32_t)50, vp1.y());
    EXPECT_EQ((uint32_t)260u, vp1.width());
    EXPECT_EQ((uint32_t)120u, vp1.height());
    EXPECT_EQ(0.4f, vp1.nearClipping());
    EXPECT_EQ(1.6f, vp1.farClipping());
  }


  // -- viewport builder --

  TEST_F(ViewportTest, viewportBuildersTest) {
    Viewport vpRef1(320,240, 640u,480u, 0.1f,1.2f);
    EXPECT_TRUE(Viewport::fromTopLeft(800u, 320,240, 640u,480u, 0.1f,1.2f) == vpRef1);

    auto reverted1 = Viewport::fromBottomLeft(800u, 320,240, 640u,480u, 0.1f,1.2f);
    auto reverted2 = Viewport::fromTopLeft(800u, 320,80, 640u,480u, 0.1f,1.2f);
    EXPECT_TRUE(reverted1 == reverted2);
    EXPECT_EQ((int32_t)320, reverted1.x());
    EXPECT_EQ((int32_t)80, reverted1.y());
    EXPECT_EQ((uint32_t)640u, reverted1.width());
    EXPECT_EQ((uint32_t)480u, reverted1.height());
    EXPECT_EQ(0.1f, reverted1.nearClipping());
    EXPECT_EQ(1.2f, reverted1.farClipping());
  }
  TEST_F(ViewportTest, viewportFPBuildersTest) {
    Viewport vpRef1(320.f,240.f, 640.f,480.f, 0.1f,1.2f);
    EXPECT_TRUE(Viewport::fromTopLeft(800u, 320.f,240.f, 640.f,480.f, 0.1f,1.2f) == vpRef1);

    auto reverted1 = Viewport::fromBottomLeft(800u, 320.f,240.f, 640.f,480.f, 0.1f,1.2f);
    auto reverted2 = Viewport::fromTopLeft(800u, 320.f,80.f, 640.f,480.f, 0.1f,1.2f);
    EXPECT_TRUE(reverted1 == reverted2);
    EXPECT_EQ((int32_t)320, reverted1.x());
    EXPECT_EQ((int32_t)80, reverted1.y());
    EXPECT_EQ((uint32_t)640u, reverted1.width());
    EXPECT_EQ((uint32_t)480u, reverted1.height());
    EXPECT_EQ(0.1f, reverted1.nearClipping());
    EXPECT_EQ(1.2f, reverted1.farClipping());
  }

#endif
