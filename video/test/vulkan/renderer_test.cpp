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
#   define __SYSTEM_STR(str) L"" str
# else
#   define __SYSTEM_STR(str) str
# endif
# include <gtest/gtest.h>
# include <video/window.h>
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
    try {
      auto defaultInstance = VulkanInstance::create();
      ASSERT_TRUE(defaultInstance != nullptr);
      EXPECT_TRUE(defaultInstance->vkInstance() != VK_NULL_HANDLE);
      EXPECT_TRUE(defaultInstance->featureLevel() == VK_API_VERSION_1_2);
      defaultInstance = nullptr;

      const char* addedExt = "VK_KHR_get_surface_capabilities2";
      auto customInstance = VulkanInstance::create("ABC -  Test1", VK_MAKE_VERSION(2, 3, 4), VK_API_VERSION_1_1, &addedExt, 1);
      ASSERT_TRUE(customInstance != nullptr);
      EXPECT_TRUE(customInstance->vkInstance() != VK_NULL_HANDLE);
      EXPECT_TRUE(customInstance->featureLevel() == VK_API_VERSION_1_1);
    }
    catch (const std::exception& exc) { ASSERT_STREQ("ok", exc.what()); }
  }

  TEST_F(VulkanRendererTest, createDefaultRenderer) {
    try {
      pandora::hardware::DisplayMonitor monitor;
      Renderer renderer(monitor);
      EXPECT_TRUE(renderer.device() != VK_NULL_HANDLE);
      EXPECT_TRUE(renderer.context() != VK_NULL_HANDLE);
      EXPECT_TRUE(renderer.vkInstance() != VK_NULL_HANDLE);
      EXPECT_TRUE(renderer.featureLevel() >= (uint32_t)VK_API_VERSION_1_2);

      auto window = pandora::video::Window::Builder{}.create(__SYSTEM_STR("VK_TEST"), __SYSTEM_STR("Test"));
      window->show();

      auto colorSpace = renderer.getMonitorColorSpace(monitor);
      size_t dedicatedRam = 0, sharedRam = 0;
      EXPECT_TRUE(renderer.getAdapterVramSize(dedicatedRam, sharedRam));
      EXPECT_TRUE(dedicatedRam > 0 || sharedRam > 0); // VRAM may be 0 on headless servers, but not shared RAM

      const char* trueVal = "true";
      const char* falseVal = "false";
      printf("Vulkan context:\n > API level: %u.%u\n > VRAM: %.3f MB\n > Shared RAM: %.3f MB\n"
             " > Max render views: %u\n > Tearing available: %s\n", 
             VK_API_VERSION_MAJOR(renderer.featureLevel()), VK_API_VERSION_MINOR(renderer.featureLevel()),
             (float)dedicatedRam/1048576.0f, (float)sharedRam/1048576.0f,
             (uint32_t)renderer.maxRenderTargets(),
             renderer.isTearingAvailable() ? trueVal : falseVal);

      ColorChannel gammaCorrectWhite[4];
      float white[] { 1.f, 1.f, 1.f, 0.5f };
      renderer.sRgbToGammaCorrectColor(white, gammaCorrectWhite);
      EXPECT_TRUE(gammaCorrectWhite[0] >= 0.9f && gammaCorrectWhite[0] <= 1.2f);
      EXPECT_TRUE(gammaCorrectWhite[1] >= 0.9f && gammaCorrectWhite[1] <= 1.2f);
      EXPECT_TRUE(gammaCorrectWhite[2] >= 0.9f && gammaCorrectWhite[2] <= 1.2f);
      EXPECT_EQ(0.5f, gammaCorrectWhite[3]);

      auto instance = renderer.vkInstance();
      auto physical = renderer.device();
      auto device = renderer.context();
      auto featLevel = renderer.featureLevel();
      Renderer moved = std::move(renderer);
      EXPECT_EQ(instance, moved.vkInstance());
      EXPECT_EQ(physical, moved.device());
      EXPECT_TRUE(moved.context() != nullptr);
      EXPECT_EQ(device, moved.context());
      EXPECT_EQ(featLevel, moved.featureLevel());
      renderer = std::move(moved);
      EXPECT_EQ(instance, renderer.vkInstance());
      EXPECT_EQ(physical, renderer.device());
      EXPECT_TRUE(renderer.context() != nullptr);
      EXPECT_EQ(device, renderer.context());
      EXPECT_EQ(featLevel, renderer.featureLevel());
    }
    catch (const std::exception& exc) { ASSERT_STREQ("ok", exc.what()); }
  }

  TEST_F(VulkanRendererTest, createCustomRenderer) {
    try {
      auto customInstance = VulkanInstance::create("TestRenderer", VK_MAKE_VERSION(2,3,4), VK_API_VERSION_1_1, nullptr, 0);
      ASSERT_TRUE(customInstance != nullptr);
      EXPECT_TRUE(customInstance->vkInstance() != VK_NULL_HANDLE);
      EXPECT_TRUE(customInstance->featureLevel() == VK_API_VERSION_1_1);

      pandora::hardware::DisplayMonitor monitor;
      const char* extensions[] { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
      Renderer renderer(monitor, customInstance, Renderer::defaultFeatures(), false, extensions, sizeof(extensions)/sizeof(*extensions));
      EXPECT_TRUE(renderer.device() != VK_NULL_HANDLE);
      EXPECT_TRUE(renderer.context() != VK_NULL_HANDLE);
      EXPECT_TRUE(renderer.vkInstance() != VK_NULL_HANDLE);
      EXPECT_TRUE(renderer.featureLevel() == (uint32_t)VK_API_VERSION_1_1);
    }
    catch (const std::exception& exc) { ASSERT_STREQ("ok", exc.what()); }
  }

# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
