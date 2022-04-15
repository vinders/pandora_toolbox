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
# include <video/vulkan/renderer.h>
# include <video/vulkan/buffer.h>
# include <video/vulkan/device_memory_pool.h>

  using namespace pandora::video::vulkan;

  class VulkanDeviceMemoryPoolTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- alignment helpers --

  TEST_F(VulkanDeviceMemoryPoolTest, vkMemoryPoolAlignmentTest) {
    EXPECT_EQ((size_t)0, DeviceMemoryPool::alignPoolSize((size_t)0));
    EXPECT_EQ((size_t)256, DeviceMemoryPool::alignPoolSize((size_t)1));
    EXPECT_EQ((size_t)256, DeviceMemoryPool::alignPoolSize((size_t)42));
    EXPECT_EQ((size_t)256, DeviceMemoryPool::alignPoolSize((size_t)255));
    EXPECT_EQ((size_t)256, DeviceMemoryPool::alignPoolSize((size_t)256));
    EXPECT_EQ((size_t)512, DeviceMemoryPool::alignPoolSize((size_t)257));
    EXPECT_EQ((size_t)512, DeviceMemoryPool::alignPoolSize((size_t)421));
    EXPECT_EQ((size_t)512, DeviceMemoryPool::alignPoolSize((size_t)511));
    EXPECT_EQ((size_t)512, DeviceMemoryPool::alignPoolSize((size_t)512));
    EXPECT_EQ((size_t)768, DeviceMemoryPool::alignPoolSize((size_t)513));
    EXPECT_EQ((size_t)768, DeviceMemoryPool::alignPoolSize((size_t)750));
    EXPECT_EQ((size_t)768, DeviceMemoryPool::alignPoolSize((size_t)768));
    EXPECT_EQ((size_t)1024, DeviceMemoryPool::alignPoolSize((size_t)1000));
    EXPECT_EQ((size_t)1024, DeviceMemoryPool::alignPoolSize((size_t)1023));
    EXPECT_EQ((size_t)1024, DeviceMemoryPool::alignPoolSize((size_t)1024));
    EXPECT_EQ((size_t)1234688, DeviceMemoryPool::alignPoolSize((size_t)1234567));

    EXPECT_EQ((size_t)42, DeviceMemoryPool::align((size_t)1, (size_t)42));
    EXPECT_EQ((size_t)256, DeviceMemoryPool::align((size_t)1, (size_t)256));
    EXPECT_EQ((size_t)320, DeviceMemoryPool::align((size_t)1, (size_t)320));
    EXPECT_EQ((size_t)42, DeviceMemoryPool::align((size_t)41, (size_t)42));
    EXPECT_EQ((size_t)256, DeviceMemoryPool::align((size_t)255, (size_t)256));
    EXPECT_EQ((size_t)320, DeviceMemoryPool::align((size_t)319, (size_t)320));
    EXPECT_EQ((size_t)42, DeviceMemoryPool::align((size_t)42, (size_t)42));
    EXPECT_EQ((size_t)256, DeviceMemoryPool::align((size_t)256, (size_t)256));
    EXPECT_EQ((size_t)320, DeviceMemoryPool::align((size_t)320, (size_t)320));
    EXPECT_EQ((size_t)84, DeviceMemoryPool::align((size_t)43, (size_t)42));
    EXPECT_EQ((size_t)512, DeviceMemoryPool::align((size_t)257, (size_t)256));
    EXPECT_EQ((size_t)640, DeviceMemoryPool::align((size_t)321, (size_t)320));
    EXPECT_EQ((size_t)1234590, DeviceMemoryPool::align((size_t)1234567, (size_t)42));
    EXPECT_EQ((size_t)1234688, DeviceMemoryPool::align((size_t)1234567, (size_t)256));
    EXPECT_EQ((size_t)1234880, DeviceMemoryPool::align((size_t)1234567, (size_t)320));
  }


  // -- device memory pool creation --

  TEST_F(VulkanDeviceMemoryPoolTest, vkMemoryPoolCreationTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      DeviceMemoryPool emptyPool;
      EXPECT_TRUE(emptyPool.handle() == VK_NULL_HANDLE);
      EXPECT_TRUE(emptyPool.isEmpty());
      EXPECT_EQ((size_t)0, emptyPool.size());
      emptyPool.release();
      EXPECT_TRUE(emptyPool.handle() == VK_NULL_HANDLE);
      EXPECT_TRUE(emptyPool.isEmpty());

      // per index
      uint32_t memoryTypeIndex = renderer.resourceManager()->memoryProps().findMemoryTypeIndex(0xFFFFFFFFu,
                                                                           (VkMemoryPropertyFlags)0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      EXPECT_NE(MemoryProps::indexNotFound(), memoryTypeIndex);
      DeviceMemoryPool poolPerIndex(renderer.resourceManager(), (size_t)1024, memoryTypeIndex);
      EXPECT_TRUE(poolPerIndex.handle() != VK_NULL_HANDLE);
      EXPECT_FALSE(poolPerIndex.isEmpty());
      EXPECT_EQ((size_t)1024, poolPerIndex.size());

      auto poolPerIndexHandle = poolPerIndex.handle();
      DeviceMemoryPool moved(std::move(poolPerIndex));
      EXPECT_TRUE(moved.handle() != VK_NULL_HANDLE);
      EXPECT_EQ(poolPerIndexHandle, moved.handle());
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_EQ((size_t)1024, moved.size());
      EXPECT_TRUE(poolPerIndex.handle() == VK_NULL_HANDLE);
      EXPECT_TRUE(poolPerIndex.isEmpty());
      poolPerIndex = std::move(moved);
      EXPECT_TRUE(poolPerIndex.handle() != VK_NULL_HANDLE);
      EXPECT_EQ(poolPerIndexHandle, poolPerIndex.handle());
      EXPECT_FALSE(poolPerIndex.isEmpty());
      EXPECT_EQ((size_t)1024, poolPerIndex.size());
      EXPECT_TRUE(moved.handle() == VK_NULL_HANDLE);
      EXPECT_TRUE(moved.isEmpty());
      poolPerIndex.release();
      EXPECT_TRUE(poolPerIndex.handle() == VK_NULL_HANDLE);
      EXPECT_TRUE(poolPerIndex.isEmpty());

      // per builder
      StaticBufferParams params1(BufferType::vertex, (size_t)1024);
      StaticBufferParams params2(BufferType::vertexIndex, (size_t)512);
      StaticBuffer::Builder builders[]{ StaticBuffer::Builder(renderer.resourceManager(), params1),
                                        StaticBuffer::Builder(renderer.resourceManager(), params2) };
      EXPECT_NE((uint32_t)0, builders[0].memoryTypeBits());
      EXPECT_NE((uint32_t)0, builders[1].memoryTypeBits());
      ASSERT_TRUE(builders[0].dedicatedAllocRequirement() != Requirement::required && builders[1].dedicatedAllocRequirement() != Requirement::required);
      size_t buffer2Offset = DeviceMemoryPool::align(builders[0].requiredMemorySize(), builders[1].requiredAlignment());

      size_t perSingleBuilderSize = DeviceMemoryPool::alignPoolSize(builders[0].requiredMemorySize());
      DeviceMemoryPool perSingleBuilder(renderer.resourceManager(), perSingleBuilderSize, &builders[0], 1);
      EXPECT_TRUE(perSingleBuilder.handle() != VK_NULL_HANDLE);
      EXPECT_FALSE(perSingleBuilder.isEmpty());
      EXPECT_EQ(perSingleBuilderSize, perSingleBuilder.size());
      EXPECT_NE(MemoryProps::indexNotFound(), perSingleBuilder.memoryTypeIndex());

      size_t perBuildersSize = DeviceMemoryPool::alignPoolSize(buffer2Offset + builders[1].requiredMemorySize());
      DeviceMemoryPool perBuilders(renderer.resourceManager(), perBuildersSize, builders, 2);
      EXPECT_TRUE(perBuilders.handle() != VK_NULL_HANDLE);
      EXPECT_FALSE(perBuilders.isEmpty());
      EXPECT_EQ(perBuildersSize, perBuilders.size());
      EXPECT_NE(MemoryProps::indexNotFound(), perBuilders.memoryTypeIndex());

      auto buffer0 = builders[0].build(perSingleBuilder, 0);
      EXPECT_TRUE(buffer0.handle() != VK_NULL_HANDLE);
      auto buffer1 = builders[0].build(perBuilders, 0);
      EXPECT_TRUE(buffer1.handle() != VK_NULL_HANDLE);
      auto buffer2 = builders[1].build(perBuilders, buffer2Offset);
      EXPECT_TRUE(buffer2.handle() != VK_NULL_HANDLE);

      buffer0.release();
      buffer1.release();
      buffer2.release();
    }
  }

#endif
