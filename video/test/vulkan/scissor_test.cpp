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
#if defined(_VIDEO_VULKAN_SUPPORT)
# if defined(_WINDOWS)
#   ifndef __MINGW32__
#     pragma warning(push)
#     pragma warning(disable: 26812) // disable warnings about vulkan enums
#   endif
# endif
# include <gtest/gtest.h>
# include <video/vulkan/scissor.h>

  using namespace pandora::video::vulkan;

  class VulkanScissorTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- scissor test rectangle container --

  TEST_F(VulkanScissorTest, createCopyMoveTest) {
    ScissorRectangle sr0;
    EXPECT_EQ((int32_t)0, sr0.x());
    EXPECT_EQ((int32_t)0, sr0.y());
    EXPECT_EQ((uint32_t)0, sr0.width());
    EXPECT_EQ((uint32_t)0, sr0.height());

    ScissorRectangle sr1(0,0, 640u,480u);
    EXPECT_EQ((int32_t)0, sr1.x());
    EXPECT_EQ((int32_t)0, sr1.y());
    EXPECT_EQ((uint32_t)640u, sr1.width());
    EXPECT_EQ((uint32_t)480u, sr1.height());
    
    ScissorRectangle sr2(0,0, 640u,480u);
    EXPECT_EQ((int32_t)0, sr2.x());
    EXPECT_EQ((int32_t)0, sr2.y());
    EXPECT_EQ((uint32_t)640u, sr2.width());
    EXPECT_EQ((uint32_t)480u, sr2.height());
    EXPECT_TRUE(sr1 == sr2);
    EXPECT_FALSE(sr1 != sr2);
    EXPECT_FALSE(sr0 == sr2);
    
    ScissorRectangle sr3(320,240, 640u,480u);
    EXPECT_EQ((int32_t)320, sr3.x());
    EXPECT_EQ((int32_t)240, sr3.y());
    EXPECT_EQ((uint32_t)640u, sr3.width());
    EXPECT_EQ((uint32_t)480u, sr3.height());
    
    ScissorRectangle copied(sr3);
    EXPECT_EQ((int32_t)320, copied.x());
    EXPECT_EQ((int32_t)240, copied.y());
    EXPECT_EQ((uint32_t)640u, copied.width());
    EXPECT_EQ((uint32_t)480u, copied.height());
    
    ScissorRectangle moved(std::move(sr3));
    EXPECT_EQ((int32_t)320, moved.x());
    EXPECT_EQ((int32_t)240, moved.y());
    EXPECT_EQ((uint32_t)640u, moved.width());
    EXPECT_EQ((uint32_t)480u, moved.height());
    
    sr3 = std::move(sr2);
    EXPECT_EQ((int32_t)0, sr3.x());
    EXPECT_EQ((int32_t)0, sr3.y());
    EXPECT_EQ((uint32_t)640u, sr3.width());
    EXPECT_EQ((uint32_t)480u, sr3.height());
    
    sr3 = copied;
    EXPECT_EQ((int32_t)320, sr3.x());
    EXPECT_EQ((int32_t)240, sr3.y());
    EXPECT_EQ((uint32_t)640u, sr3.width());
    EXPECT_EQ((uint32_t)480u, sr3.height());
  }

  // -- scissor rectangle builder --

  TEST_F(VulkanScissorTest, scissorRectangleBuildersTest) {
    ScissorRectangle srRef1(320,240, 640u,480u);
    EXPECT_TRUE(ScissorRectangle::fromTopLeft(800u, 320,240, 640u,480u) == srRef1);

    auto reverted1 = ScissorRectangle::fromBottomLeft(800u, 320,240, 640u,480u);
    auto reverted2 = ScissorRectangle::fromTopLeft(800u, 320,80, 640u,480u);
    EXPECT_TRUE(reverted1 == reverted2);
    EXPECT_EQ(320.f, reverted1.x());
    EXPECT_EQ(80.f, reverted1.y());
    EXPECT_EQ(640.f, reverted1.width());
    EXPECT_EQ(480.f, reverted1.height());
  }

# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
