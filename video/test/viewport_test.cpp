#include <gtest/gtest.h>
#include <video/viewport.h>

using namespace pandora::video;

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
  Viewport vp1(640u,480u);
  EXPECT_EQ((int32_t)0, vp1.x());
  EXPECT_EQ((int32_t)0, vp1.y());
  EXPECT_EQ((uint32_t)640u, vp1.width());
  EXPECT_EQ((uint32_t)480u, vp1.height());
  EXPECT_EQ((double)0., vp1.nearClipping());
  EXPECT_EQ((double)1., vp1.farClipping());
  
  Viewport vp2(0,0, 640u,480u);
  EXPECT_EQ((int32_t)0, vp2.x());
  EXPECT_EQ((int32_t)0, vp2.y());
  EXPECT_EQ((uint32_t)640u, vp2.width());
  EXPECT_EQ((uint32_t)480u, vp2.height());
  EXPECT_EQ((double)0., vp2.nearClipping());
  EXPECT_EQ((double)1., vp2.farClipping());
  
  Viewport vp3(320,240, 640u,480u);
  EXPECT_EQ((int32_t)320, vp3.x());
  EXPECT_EQ((int32_t)240, vp3.y());
  EXPECT_EQ((uint32_t)640u, vp3.width());
  EXPECT_EQ((uint32_t)480u, vp3.height());
  EXPECT_EQ((double)0., vp3.nearClipping());
  EXPECT_EQ((double)1., vp3.farClipping());

  Viewport vp4(0,0, 640u,480u, 0.5,1.2);
  EXPECT_EQ((int32_t)0, vp4.x());
  EXPECT_EQ((int32_t)0, vp4.y());
  EXPECT_EQ((uint32_t)640u, vp4.width());
  EXPECT_EQ((uint32_t)480u, vp4.height());
  EXPECT_EQ((double)0.5, vp4.nearClipping());
  EXPECT_EQ((double)1.2, vp4.farClipping());
  
  Viewport vp5(320,240, 640u,480u, 0.1,1.2);
  EXPECT_EQ((int32_t)320, vp5.x());
  EXPECT_EQ((int32_t)240, vp5.y());
  EXPECT_EQ((uint32_t)640u, vp5.width());
  EXPECT_EQ((uint32_t)480u, vp5.height());
  EXPECT_EQ((double)0.1, vp5.nearClipping());
  EXPECT_EQ((double)1.2, vp5.farClipping());
  
  Viewport copied(vp5);
  EXPECT_EQ((int32_t)320, copied.x());
  EXPECT_EQ((int32_t)240, copied.y());
  EXPECT_EQ((uint32_t)640u, copied.width());
  EXPECT_EQ((uint32_t)480u, copied.height());
  EXPECT_EQ((double)0.1, copied.nearClipping());
  EXPECT_EQ((double)1.2, copied.farClipping());
  
  Viewport moved(std::move(vp5));
  EXPECT_EQ((int32_t)320, moved.x());
  EXPECT_EQ((int32_t)240, moved.y());
  EXPECT_EQ((uint32_t)640u, moved.width());
  EXPECT_EQ((uint32_t)480u, moved.height());
  EXPECT_EQ((double)0.1, moved.nearClipping());
  EXPECT_EQ((double)1.2, moved.farClipping());
  
  vp5 = std::move(vp4);
  EXPECT_EQ((int32_t)0, vp5.x());
  EXPECT_EQ((int32_t)0, vp5.y());
  EXPECT_EQ((uint32_t)640u, vp5.width());
  EXPECT_EQ((uint32_t)480u, vp5.height());
  EXPECT_EQ((double)0.5, vp5.nearClipping());
  EXPECT_EQ((double)1.2, vp5.farClipping());
  
  vp5 = vp3;
  EXPECT_EQ((int32_t)320, vp5.x());
  EXPECT_EQ((int32_t)240, vp5.y());
  EXPECT_EQ((uint32_t)640u, vp5.width());
  EXPECT_EQ((uint32_t)480u, vp5.height());
  EXPECT_EQ((double)0., vp5.nearClipping());
  EXPECT_EQ((double)1., vp5.farClipping());
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
  EXPECT_EQ((double)0., vp1.nearClipping());
  EXPECT_EQ((double)1., vp1.farClipping());
  
  vp1.move(10,20);
  EXPECT_EQ((int32_t)10, vp1.x());
  EXPECT_EQ((int32_t)20, vp1.y());
  EXPECT_EQ((uint32_t)640u, vp1.width());
  EXPECT_EQ((uint32_t)480u, vp1.height());
  EXPECT_EQ((double)0., vp1.nearClipping());
  EXPECT_EQ((double)1., vp1.farClipping());
  
  vp1.resize(240u,180u);
  EXPECT_EQ((int32_t)10, vp1.x());
  EXPECT_EQ((int32_t)20, vp1.y());
  EXPECT_EQ((uint32_t)240u, vp1.width());
  EXPECT_EQ((uint32_t)180u, vp1.height());
  EXPECT_EQ((double)0., vp1.nearClipping());
  EXPECT_EQ((double)1., vp1.farClipping());
  
  vp1.resize(40,50, 260u,120u);
  EXPECT_EQ((int32_t)40, vp1.x());
  EXPECT_EQ((int32_t)50, vp1.y());
  EXPECT_EQ((uint32_t)260u, vp1.width());
  EXPECT_EQ((uint32_t)120u, vp1.height());
  EXPECT_EQ((double)0., vp1.nearClipping());
  EXPECT_EQ((double)1., vp1.farClipping());
  
  vp1.setDepthRange(0.4, 1.6);
  EXPECT_EQ((int32_t)40, vp1.x());
  EXPECT_EQ((int32_t)50, vp1.y());
  EXPECT_EQ((uint32_t)260u, vp1.width());
  EXPECT_EQ((uint32_t)120u, vp1.height());
  EXPECT_EQ((double)0.4, vp1.nearClipping());
  EXPECT_EQ((double)1.6, vp1.farClipping());
}


// -- viewport builder --

TEST_F(ViewportTest, viewportBuildersTest) {
  Viewport vpRef1(320,240, 640u,480u, 0.1,1.2);
  EXPECT_TRUE(TopBasedViewportBuilder::fromTopLeft(320,240, 640u,480u, 800u, 0.1,1.2) == vpRef1);
  EXPECT_TRUE(BottomBasedViewportBuilder::fromBottomLeft(320,240, 640u,480u, 800u, 0.1,1.2) == vpRef1);
  
  auto reverted1 = TopBasedViewportBuilder::fromBottomLeft(320,240, 640u,480u, 800u, 0.1,1.2);
  auto reverted2 = BottomBasedViewportBuilder::fromTopLeft(320,240, 640u,480u, 800u, 0.1,1.2);
  EXPECT_TRUE(reverted1 == reverted2);
  EXPECT_EQ((int32_t)320, reverted1.x());
  EXPECT_EQ((int32_t)80, reverted1.y());
  EXPECT_EQ((uint32_t)640u, reverted1.width());
  EXPECT_EQ((uint32_t)480u, reverted1.height());
  EXPECT_EQ((double)0.1, reverted1.nearClipping());
  EXPECT_EQ((double)1.2, reverted1.farClipping());
}
