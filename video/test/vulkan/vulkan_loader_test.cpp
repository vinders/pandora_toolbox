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
    loader.shutdown();
  }

#endif
