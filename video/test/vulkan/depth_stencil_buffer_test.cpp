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
# include <system/align.h>
# include <video/vulkan/renderer.h>
# include <video/vulkan/buffer.h>

  using namespace pandora::video::vulkan;

  class VulkanDepthStencilBufferTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };

  __align_type(16,
    struct _TestColor final {
    float rgba[4];
  });


  // -- depth/stencil buffer params --

  TEST_F(VulkanDepthStencilBufferTest, vkDepthStencilBufferParamsTest) {
    DepthStencilBufferParams emptyParams;
    EXPECT_TRUE(emptyParams.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, emptyParams.descriptor().sType);
    EXPECT_TRUE(emptyParams.descriptor().pNext == nullptr);
    EXPECT_EQ((VkImageType)VK_IMAGE_TYPE_2D, emptyParams.descriptor().imageType);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().extent.width);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().extent.height);
    EXPECT_EQ((uint32_t)1, emptyParams.descriptor().extent.depth);
    EXPECT_EQ((uint32_t)1, emptyParams.descriptor().mipLevels);
    EXPECT_EQ((uint32_t)1, emptyParams.descriptor().arrayLayers);
    EXPECT_EQ((VkFormat)VK_FORMAT_D32_SFLOAT, emptyParams.descriptor().format);
    EXPECT_EQ((VkImageTiling)VK_IMAGE_TILING_OPTIMAL, emptyParams.descriptor().tiling);
    EXPECT_EQ((VkImageLayout)VK_IMAGE_LAYOUT_UNDEFINED, emptyParams.descriptor().initialLayout);
    EXPECT_EQ((VkImageUsageFlags)VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, emptyParams.descriptor().usage);
    EXPECT_EQ((VkSampleCountFlagBits)VK_SAMPLE_COUNT_1_BIT, emptyParams.descriptor().samples);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, emptyParams.descriptor().sharingMode);
    EXPECT_TRUE(emptyParams.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().queueFamilyIndexCount);

    DepthStencilBufferParams params1(DepthStencilFormat::d24_unorm_s8_ui, 800, 600, 4);
    EXPECT_TRUE(params1.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, params1.descriptor().sType);
    EXPECT_TRUE(params1.descriptor().pNext == nullptr);
    EXPECT_EQ((VkImageType)VK_IMAGE_TYPE_2D, params1.descriptor().imageType);
    EXPECT_EQ((uint32_t)800, params1.descriptor().extent.width);
    EXPECT_EQ((uint32_t)600, params1.descriptor().extent.height);
    EXPECT_EQ((uint32_t)1, params1.descriptor().extent.depth);
    EXPECT_EQ((uint32_t)1, params1.descriptor().mipLevels);
    EXPECT_EQ((uint32_t)1, params1.descriptor().arrayLayers);
    EXPECT_EQ((VkFormat)VK_FORMAT_D24_UNORM_S8_UINT, params1.descriptor().format);
    EXPECT_EQ((VkImageTiling)VK_IMAGE_TILING_OPTIMAL, params1.descriptor().tiling);
    EXPECT_EQ((VkImageLayout)VK_IMAGE_LAYOUT_UNDEFINED, params1.descriptor().initialLayout);
    EXPECT_EQ((VkImageUsageFlags)VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, params1.descriptor().usage);
    EXPECT_EQ((VkSampleCountFlagBits)VK_SAMPLE_COUNT_4_BIT, params1.descriptor().samples);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params1.descriptor().queueFamilyIndexCount);

    params1.sharingMode(nullptr, 0);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params1.descriptor().queueFamilyIndexCount);
    uint32_t concurrentQueueFamilies[]{ 1, 2, 3 };
    uint32_t queueCount = (uint32_t)sizeof(concurrentQueueFamilies) / sizeof(*concurrentQueueFamilies);
    params1.sharingMode(concurrentQueueFamilies, queueCount);
    EXPECT_EQ((VkImageTiling)VK_IMAGE_TILING_OPTIMAL, params1.descriptor().tiling);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_CONCURRENT, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == &concurrentQueueFamilies[0]);
    EXPECT_EQ(queueCount, params1.descriptor().queueFamilyIndexCount);

    emptyParams.format(DepthStencilFormat::d16_unorm).size(640,480).sampleCount(8);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, emptyParams.descriptor().sType);
    EXPECT_TRUE(emptyParams.descriptor().pNext == nullptr);
    EXPECT_EQ((VkImageType)VK_IMAGE_TYPE_2D, emptyParams.descriptor().imageType);
    EXPECT_EQ((uint32_t)640, emptyParams.descriptor().extent.width);
    EXPECT_EQ((uint32_t)480, emptyParams.descriptor().extent.height);
    EXPECT_EQ((uint32_t)1, emptyParams.descriptor().extent.depth);
    EXPECT_EQ((uint32_t)1, emptyParams.descriptor().mipLevels);
    EXPECT_EQ((uint32_t)1, emptyParams.descriptor().arrayLayers);
    EXPECT_EQ((VkFormat)VK_FORMAT_D16_UNORM, emptyParams.descriptor().format);
    EXPECT_EQ((VkImageTiling)VK_IMAGE_TILING_OPTIMAL, emptyParams.descriptor().tiling);
    EXPECT_EQ((VkImageLayout)VK_IMAGE_LAYOUT_UNDEFINED, emptyParams.descriptor().initialLayout);
    EXPECT_EQ((VkImageUsageFlags)VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, emptyParams.descriptor().usage);
    EXPECT_EQ((VkSampleCountFlagBits)VK_SAMPLE_COUNT_8_BIT, emptyParams.descriptor().samples);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, emptyParams.descriptor().sharingMode);
    EXPECT_TRUE(emptyParams.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().queueFamilyIndexCount);
  }


  // -- depth/stencil buffer creation/binding --

  TEST_F(VulkanDepthStencilBufferTest, vkDepthStencilBufferCreationTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    
    DepthStencilBuffer emptyBuffer;
    EXPECT_TRUE(emptyBuffer.isEmpty());
    EXPECT_TRUE(emptyBuffer.handle() == nullptr);
    EXPECT_TRUE(emptyBuffer.bufferView() == nullptr);

    DepthStencilBufferParams emptyParams{};
    EXPECT_ANY_THROW(DepthStencilBuffer::Builder(renderer.resourceManager(), emptyParams));
    DepthStencilBufferParams invalidParams1(DepthStencilFormat::d32_f, 0, 600, 1);
    EXPECT_ANY_THROW(DepthStencilBuffer::Builder(renderer.resourceManager(), invalidParams1));
    DepthStencilBufferParams invalidParams2(DepthStencilFormat::d32_f, 600, 0, 1);
    EXPECT_ANY_THROW(DepthStencilBuffer::Builder(renderer.resourceManager(), invalidParams2));
    
    DepthStencilBufferParams params1(DepthStencilFormat::d32_f, 800, 600);
    DepthStencilBuffer buffer1 = DepthStencilBuffer::Builder(renderer.resourceManager(), params1).build();
    EXPECT_FALSE(buffer1.isEmpty());
    EXPECT_TRUE(buffer1.handle() != nullptr);
    EXPECT_TRUE(buffer1.bufferView() != nullptr);
    EXPECT_EQ((uint32_t)800, buffer1.width());
    EXPECT_EQ((uint32_t)600, buffer1.height());
    EXPECT_EQ(DepthStencilFormat::d32_f, buffer1.format());
    
    DepthStencilBufferParams params2(DepthStencilFormat::d32_f, 492, 226);
    DepthStencilBuffer buffer2 = DepthStencilBuffer::Builder(renderer.resourceManager(), params2).build();
    EXPECT_FALSE(buffer2.isEmpty());
    EXPECT_TRUE(buffer2.handle() != nullptr);
    EXPECT_TRUE(buffer2.bufferView() != nullptr);
    EXPECT_EQ((uint32_t)492, buffer2.width());
    EXPECT_EQ((uint32_t)226, buffer2.height());
    EXPECT_EQ(DepthStencilFormat::d32_f, buffer2.format());
    
    DepthStencilBufferParams params3(DepthStencilFormat::d16_unorm, 640, 480);
    DepthStencilBuffer buffer3 = DepthStencilBuffer::Builder(renderer.resourceManager(), params3).build();
    EXPECT_FALSE(buffer3.isEmpty());
    EXPECT_TRUE(buffer3.handle() != nullptr);
    EXPECT_TRUE(buffer3.bufferView() != nullptr);
    EXPECT_EQ((uint32_t)640, buffer3.width());
    EXPECT_EQ((uint32_t)480, buffer3.height());
    EXPECT_EQ(DepthStencilFormat::d16_unorm, buffer3.format());
    
    DepthStencilBufferParams params4(DepthStencilFormat::d32_f_s8_ui, 64, 32);
    DepthStencilBuffer buffer4 = DepthStencilBuffer::Builder(renderer.resourceManager(), params4).build();
    EXPECT_FALSE(buffer4.isEmpty());
    EXPECT_TRUE(buffer4.handle() != nullptr);
    EXPECT_TRUE(buffer4.bufferView() != nullptr);
    EXPECT_EQ((uint32_t)64, buffer4.width());
    EXPECT_EQ((uint32_t)32, buffer4.height());
    EXPECT_EQ(DepthStencilFormat::d32_f_s8_ui, buffer4.format());
    
    DepthStencilBufferParams params5(DepthStencilFormat::d24_unorm_s8_ui, 300, 200);
    DepthStencilBuffer buffer5 = DepthStencilBuffer::Builder(renderer.resourceManager(), params5).build();
    EXPECT_FALSE(buffer5.isEmpty());
    EXPECT_TRUE(buffer5.handle() != nullptr);
    EXPECT_TRUE(buffer5.bufferView() != nullptr);
    EXPECT_EQ((uint32_t)300, buffer5.width());
    EXPECT_EQ((uint32_t)200, buffer5.height());
    EXPECT_EQ(DepthStencilFormat::d24_unorm_s8_ui, buffer5.format());
    
    DepthStencilBufferParams params6(DepthStencilFormat::d16_unorm, 1024, 768);
    auto builder6 = DepthStencilBuffer::Builder(renderer.resourceManager(), params6);
    DepthStencilBuffer buffer6 = builder6.build((builder6.dedicatedAllocRequirement() != Requirement::none) ? FeatureMode::force : FeatureMode::disable);
    EXPECT_FALSE(buffer6.isEmpty());
    EXPECT_TRUE(buffer6.handle() != nullptr);
    EXPECT_TRUE(buffer6.bufferView() != nullptr);
    EXPECT_EQ((uint32_t)1024, buffer6.width());
    EXPECT_EQ((uint32_t)768, buffer6.height());
    EXPECT_EQ(DepthStencilFormat::d16_unorm, buffer6.format());
    
    auto buffer1Handle = buffer1.handle();
    auto buffer1View = buffer1.bufferView();
    DepthStencilBuffer moved(std::move(buffer1));
    EXPECT_FALSE(moved.isEmpty());
    EXPECT_TRUE(buffer1.isEmpty());
    EXPECT_TRUE(moved.handle() == buffer1Handle);
    EXPECT_TRUE(buffer1.handle() == nullptr);
    EXPECT_TRUE(moved.bufferView() == buffer1View);
    EXPECT_TRUE(buffer1.bufferView() == nullptr);
    buffer1 = std::move(moved);
    EXPECT_FALSE(buffer1.isEmpty());
    EXPECT_TRUE(moved.isEmpty());
    EXPECT_TRUE(buffer1.handle() == buffer1Handle);
    EXPECT_TRUE(moved.handle() == nullptr);
    EXPECT_TRUE(buffer1.bufferView() == buffer1View);
    EXPECT_TRUE(moved.bufferView() == nullptr);
  }

#endif
