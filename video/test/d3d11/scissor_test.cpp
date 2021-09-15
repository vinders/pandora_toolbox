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
# include <video/d3d11/scissor.h>

  using namespace pandora::video::d3d11;

  class D3d11ScissorTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- scissor test rectangle container --

  TEST_F(D3d11ScissorTest, createCopyMoveTest) {
    ScissorRectangle vp0;
    EXPECT_EQ((int32_t)0, vp0.x());
    EXPECT_EQ((int32_t)0, vp0.y());
    EXPECT_EQ((uint32_t)0, vp0.width());
    EXPECT_EQ((uint32_t)0, vp0.height());

    ScissorRectangle vp1(0,0, 640u,480u);
    EXPECT_EQ((int32_t)0, vp1.x());
    EXPECT_EQ((int32_t)0, vp1.y());
    EXPECT_EQ((uint32_t)640u, vp1.width());
    EXPECT_EQ((uint32_t)480u, vp1.height());
    
    ScissorRectangle vp2(0,0, 640u,480u);
    EXPECT_EQ((int32_t)0, vp2.x());
    EXPECT_EQ((int32_t)0, vp2.y());
    EXPECT_EQ((uint32_t)640u, vp2.width());
    EXPECT_EQ((uint32_t)480u, vp2.height());
    EXPECT_TRUE(vp1 == vp2);
    EXPECT_FALSE(vp1 != vp2);
    EXPECT_FALSE(vp0 == vp2);
    
    ScissorRectangle vp3(320,240, 640u,480u);
    EXPECT_EQ((int32_t)320, vp3.x());
    EXPECT_EQ((int32_t)240, vp3.y());
    EXPECT_EQ((uint32_t)640u, vp3.width());
    EXPECT_EQ((uint32_t)480u, vp3.height());
    
    ScissorRectangle copied(vp3);
    EXPECT_EQ((int32_t)320, copied.x());
    EXPECT_EQ((int32_t)240, copied.y());
    EXPECT_EQ((uint32_t)640u, copied.width());
    EXPECT_EQ((uint32_t)480u, copied.height());
    
    ScissorRectangle moved(std::move(vp3));
    EXPECT_EQ((int32_t)320, moved.x());
    EXPECT_EQ((int32_t)240, moved.y());
    EXPECT_EQ((uint32_t)640u, moved.width());
    EXPECT_EQ((uint32_t)480u, moved.height());
    
    vp3 = std::move(vp2);
    EXPECT_EQ((int32_t)0, vp3.x());
    EXPECT_EQ((int32_t)0, vp3.y());
    EXPECT_EQ((uint32_t)640u, vp3.width());
    EXPECT_EQ((uint32_t)480u, vp3.height());
    
    vp3 = copied;
    EXPECT_EQ((int32_t)320, vp3.x());
    EXPECT_EQ((int32_t)240, vp3.y());
    EXPECT_EQ((uint32_t)640u, vp3.width());
    EXPECT_EQ((uint32_t)480u, vp3.height());
  }

#endif
