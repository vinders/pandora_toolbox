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
# include <video/vulkan/api/vulkan_loader.h>

  using namespace pandora::video::vulkan;

  class VulkanLoaderTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  TEST_F(VulkanLoaderTest, vulkanLoaderInitTest) {
    ASSERT_TRUE(VulkanLoader::isSupported());

    VulkanLoader& loader = VulkanLoader::instance();
    EXPECT_TRUE(loader.vk.instance != nullptr);
    EXPECT_TRUE(loader.vk.platformExtension != PlatformExtension::unknown);
    EXPECT_TRUE(loader.vk.GetInstanceProcAddr_ != nullptr);
    EXPECT_TRUE(loader.vk.EnumerateInstanceExtensionProperties_ != nullptr);
    EXPECT_STRNE("", loader.getPlatformSurfaceExtensionId());
    
    bool results1[2];
    const char* ext1[2] = { "VK_KHR_surface", loader.getPlatformSurfaceExtensionId() };
    EXPECT_EQ((size_t)2, loader.findInstanceExtensions(ext1, 2, results1));
    EXPECT_TRUE(results1[0]);
    EXPECT_TRUE(results1[1]);
    
    bool results2[4];
    const char* ext2[4] = { "VK_KHR_surface", "-dummy-1", loader.getPlatformSurfaceExtensionId(), "-dummy-2" };
    EXPECT_EQ((size_t)2, loader.findInstanceExtensions(ext2, 4, results2));
    EXPECT_TRUE(results2[0]);
    EXPECT_FALSE(results2[1]);
    EXPECT_TRUE(results2[2]);
    EXPECT_FALSE(results2[3]);
    
    bool results3[2];
    const char* ext3[2] = { "-dummy-1", "-dummy-2" };
    EXPECT_EQ((size_t)0, loader.findInstanceExtensions(ext3, 2, results3));
    EXPECT_FALSE(results3[0]);
    EXPECT_FALSE(results3[1]);

    EXPECT_TRUE(loader.findLayer("VK_LAYER_KHRONOS_validation"));
    
    loader.shutdown();
  }

# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
