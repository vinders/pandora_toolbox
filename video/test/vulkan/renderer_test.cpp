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
#     pragma warning(disable: 4100)  // disable warnings about unused params
#   endif
# endif
# include <gtest/gtest.h>
# include <video/vulkan/renderer.h>

  using namespace pandora::video::vulkan;

  class VulkanRendererTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  TEST_F(VulkanRendererTest, vulkanInstanceTest) {
    {
      auto defaultInstance = VulkanInstance::create();
      EXPECT_TRUE(defaultInstance->vkInstance() != VK_NULL_HANDLE);
      EXPECT_TRUE(defaultInstance->featureLevel() == VK_API_VERSION_1_2);
    }
    {
      const char* addedExt = "VK_KHR_get_surface_capabilities2";
      auto customInstance = VulkanInstance::create("ABC -  Test1", VK_MAKE_VERSION(2,3,4), VK_API_VERSION_1_1, NULL, 1);
      EXPECT_TRUE(customInstance->vkInstance() != VK_NULL_HANDLE);
      EXPECT_TRUE(customInstance->featureLevel() == VK_API_VERSION_1_1);
    }
  }

# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
