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
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

  using namespace pandora::video::vulkan;

  class VulkanBufferTest : public testing::Test {
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


  // -- constant and vertex buffer params --

  TEST_F(VulkanBufferTest, vkStaticBufferParamsTest) {
    StaticBufferParams emptyParams;
    EXPECT_TRUE(emptyParams.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, emptyParams.descriptor().sType);
    EXPECT_TRUE(emptyParams.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)0, emptyParams.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)0, emptyParams.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, emptyParams.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, emptyParams.descriptor().sharingMode);
    EXPECT_TRUE(emptyParams.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, emptyParams._descPreferredUsage());

    StaticBufferParams params1(BufferType::vertex, (size_t)16, false, TransferMode::standard);
    EXPECT_TRUE(params1.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, params1.descriptor().sType);
    EXPECT_TRUE(params1.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), params1.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)16uLL, params1.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, params1.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params1.descriptor().queueFamilyIndexCount);
    params1.sharingMode(nullptr, 0);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params1.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, params1._descPreferredUsage());

    uint32_t concurrentQueueFamilies[]{ 1, 2, 3 };
    uint32_t queueCount = (uint32_t)sizeof(concurrentQueueFamilies) / sizeof(*concurrentQueueFamilies);
    StaticBufferParams params2(BufferType::uniform, (size_t)256, true, TransferMode::bidirectional);
    EXPECT_TRUE(params2.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, params2.descriptor().sType);
    EXPECT_TRUE(params2.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), params2.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)256uLL, params2.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, params2.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params2.descriptor().sharingMode);
    EXPECT_TRUE(params2.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params2.descriptor().queueFamilyIndexCount);
    params2.specialFlags(VK_BUFFER_CREATE_SPARSE_BINDING_BIT);
    params2.sharingMode(concurrentQueueFamilies, queueCount);
    EXPECT_EQ((VkBufferCreateFlags)VK_BUFFER_CREATE_SPARSE_BINDING_BIT, params2.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_CONCURRENT, params2.descriptor().sharingMode);
    EXPECT_TRUE(params2.descriptor().pQueueFamilyIndices == &concurrentQueueFamilies[0]);
    EXPECT_EQ(queueCount, params2.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, params2._descPreferredUsage());

    emptyParams.type(BufferType::vertexIndex, true, TransferMode::standard).size((size_t)32).specialFlags(VK_BUFFER_CREATE_PROTECTED_BIT);
    EXPECT_TRUE(emptyParams.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, emptyParams.descriptor().sType);
    EXPECT_TRUE(emptyParams.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), emptyParams.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)32uLL, emptyParams.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)VK_BUFFER_CREATE_PROTECTED_BIT, emptyParams.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, emptyParams.descriptor().sharingMode);
    EXPECT_TRUE(emptyParams.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, emptyParams._descPreferredUsage());
  }

  TEST_F(VulkanBufferTest, vkDynamicBufferParamsTest) {
    DynamicBufferParams emptyParams;
    EXPECT_TRUE(emptyParams.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, emptyParams.descriptor().sType);
    EXPECT_TRUE(emptyParams.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)0, emptyParams.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)0, emptyParams.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, emptyParams.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, emptyParams.descriptor().sharingMode);
    EXPECT_TRUE(emptyParams.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), (emptyParams._descPreferredUsage()));

    DynamicBufferParams params1(BufferType::vertex, (size_t)16, false, TransferMode::standard);
    EXPECT_TRUE(params1.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, params1.descriptor().sType);
    EXPECT_TRUE(params1.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, params1.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)16uLL, params1.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, params1.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params1.descriptor().queueFamilyIndexCount);
    params1.sharingMode(nullptr, 0);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params1.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), (params1._descPreferredUsage()));

    uint32_t concurrentQueueFamilies[]{ 1, 2, 3 };
    uint32_t queueCount = (uint32_t)sizeof(concurrentQueueFamilies) / sizeof(*concurrentQueueFamilies);
    DynamicBufferParams params2(BufferType::uniform, (size_t)256, true, TransferMode::bidirectional);
    EXPECT_TRUE(params2.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, params2.descriptor().sType);
    EXPECT_TRUE(params2.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), params2.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)256uLL, params2.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, params2.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params2.descriptor().sharingMode);
    EXPECT_TRUE(params2.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params2.descriptor().queueFamilyIndexCount);
    params2.specialFlags(VK_BUFFER_CREATE_SPARSE_BINDING_BIT);
    params2.sharingMode(concurrentQueueFamilies, queueCount);
    EXPECT_EQ((VkBufferCreateFlags)VK_BUFFER_CREATE_SPARSE_BINDING_BIT, params2.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_CONCURRENT, params2.descriptor().sharingMode);
    EXPECT_TRUE(params2.descriptor().pQueueFamilyIndices == &concurrentQueueFamilies[0]);
    EXPECT_EQ(queueCount, params2.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
              (params2._descPreferredUsage() & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));

    emptyParams.type(BufferType::vertexIndex, true, TransferMode::standard).size((size_t)32).specialFlags(VK_BUFFER_CREATE_PROTECTED_BIT);
    EXPECT_TRUE(emptyParams.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, emptyParams.descriptor().sType);
    EXPECT_TRUE(emptyParams.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)VK_BUFFER_USAGE_INDEX_BUFFER_BIT, emptyParams.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)32uLL, emptyParams.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)VK_BUFFER_CREATE_PROTECTED_BIT, emptyParams.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, emptyParams.descriptor().sharingMode);
    EXPECT_TRUE(emptyParams.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
              (emptyParams._descPreferredUsage() & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
  }

  TEST_F(VulkanBufferTest, vkStagingBufferParamsTest) {
    StagingBufferParams emptyParams;
    EXPECT_TRUE(emptyParams.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, emptyParams.descriptor().sType);
    EXPECT_TRUE(emptyParams.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)0, emptyParams.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)0, emptyParams.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, emptyParams.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, emptyParams.descriptor().sharingMode);
    EXPECT_TRUE(emptyParams.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), (emptyParams._descPreferredUsage()));

    StagingBufferParams params1(BufferType::vertex, (size_t)16, false, TransferMode::standard);
    EXPECT_TRUE(params1.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, params1.descriptor().sType);
    EXPECT_TRUE(params1.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)VK_BUFFER_USAGE_TRANSFER_SRC_BIT, params1.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)16uLL, params1.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, params1.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params1.descriptor().queueFamilyIndexCount);
    params1.sharingMode(nullptr, 0);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params1.descriptor().sharingMode);
    EXPECT_TRUE(params1.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params1.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), (params1._descPreferredUsage()));

    uint32_t concurrentQueueFamilies[]{ 1, 2, 3 };
    uint32_t queueCount = (uint32_t)sizeof(concurrentQueueFamilies) / sizeof(*concurrentQueueFamilies);
    StagingBufferParams params2(BufferType::uniform, (size_t)256, true, TransferMode::bidirectional);
    EXPECT_TRUE(params2.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, params2.descriptor().sType);
    EXPECT_TRUE(params2.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), params2.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)256uLL, params2.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)0, params2.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, params2.descriptor().sharingMode);
    EXPECT_TRUE(params2.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, params2.descriptor().queueFamilyIndexCount);
    params2.specialFlags(VK_BUFFER_CREATE_SPARSE_BINDING_BIT);
    params2.sharingMode(concurrentQueueFamilies, queueCount);
    EXPECT_EQ((VkBufferCreateFlags)VK_BUFFER_CREATE_SPARSE_BINDING_BIT, params2.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_CONCURRENT, params2.descriptor().sharingMode);
    EXPECT_TRUE(params2.descriptor().pQueueFamilyIndices == &concurrentQueueFamilies[0]);
    EXPECT_EQ(queueCount, params2.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_CACHED_BIT), (params2._descPreferredUsage()));

    emptyParams.type(BufferType::vertexIndex, true, TransferMode::standard).size((size_t)32).specialFlags(VK_BUFFER_CREATE_PROTECTED_BIT);
    EXPECT_TRUE(emptyParams.descriptorPtr() != nullptr);
    EXPECT_EQ((VkStructureType)VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, emptyParams.descriptor().sType);
    EXPECT_TRUE(emptyParams.descriptor().pNext == nullptr);
    EXPECT_EQ((VkBufferUsageFlags)VK_BUFFER_USAGE_TRANSFER_SRC_BIT, emptyParams.descriptor().usage);
    EXPECT_EQ((VkDeviceSize)32uLL, emptyParams.descriptor().size);
    EXPECT_EQ((VkBufferCreateFlags)VK_BUFFER_CREATE_PROTECTED_BIT, emptyParams.descriptor().flags);
    EXPECT_EQ((VkSharingMode)VK_SHARING_MODE_EXCLUSIVE, emptyParams.descriptor().sharingMode);
    EXPECT_TRUE(emptyParams.descriptor().pQueueFamilyIndices == nullptr);
    EXPECT_EQ((uint32_t)0, emptyParams.descriptor().queueFamilyIndexCount);
    EXPECT_EQ((VkMemoryPropertyFlags)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_CACHED_BIT), (emptyParams._descPreferredUsage()));
  }


  // -- buffer creation/binding --

  TEST_F(VulkanBufferTest, vkStaticBufferCreationTest) {
    uint64_t data1a[2] = { 42, 0 };
    uint64_t data1b[2] = { 0, 22 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    StaticBuffer emptyBuffer;
    EXPECT_TRUE(emptyBuffer.isEmpty());
    EXPECT_TRUE(emptyBuffer.handle() == VK_NULL_HANDLE);

    StaticBufferParams emptyParams{};
    EXPECT_ANY_THROW(StaticBuffer::Builder(renderer.resourceManager(), emptyParams));
    StaticBufferParams invalidParams(BufferType::uniform, (size_t)0);
    EXPECT_ANY_THROW(StaticBuffer::Builder(renderer.resourceManager(), invalidParams));

    const BufferType bufferTypes[]{ BufferType::uniform, BufferType::vertex, BufferType::vertexIndex };
    for (size_t i = 0; i < sizeof(bufferTypes)/sizeof(*bufferTypes); ++i) {
      const BufferType bufferType = bufferTypes[i];

      StaticBufferParams params1(bufferType, sizeof(data1a), false, TransferMode::standard);
      StaticBuffer::Builder builder1(renderer.resourceManager(), params1);
      EXPECT_LE(sizeof(data1a), builder1.requiredMemorySize());
      EXPECT_LE((size_t)16, builder1.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder1.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::local, builder1.memoryUsage());
      EXPECT_NE(Requirement::required, builder1.dedicatedAllocRequirement());
      StaticBuffer buffer1(builder1.build());
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(bufferType, buffer1.type());
      EXPECT_EQ(TransferMode::standard, buffer1.transferMode());
      EXPECT_EQ(ResourceUsage::local, buffer1.memoryUsage());
      EXPECT_TRUE((buffer1.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0);
      EXPECT_TRUE(buffer1.write(renderer.resourceManager(), data1a));
      //TODO... bind
      EXPECT_TRUE(buffer1.write(renderer.resourceManager(), data1b));
      //TODO... clear bindings

      StaticBuffer buffer2(builder1.build(data1a));
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(bufferType, buffer2.type());
      EXPECT_EQ(TransferMode::standard, buffer2.transferMode());
      EXPECT_EQ(ResourceUsage::local, buffer2.memoryUsage());
      EXPECT_TRUE((buffer2.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0);
      //TODO... bind
      EXPECT_TRUE(buffer2.write(renderer.resourceManager(), data1b));
      //TODO... clear bindings

      StaticBufferParams params3(bufferType, sizeof(data1b), true, TransferMode::standard);
      StaticBuffer::Builder builder3(renderer.resourceManager(), params3);
      EXPECT_LE(sizeof(data1b), builder3.requiredMemorySize());
      EXPECT_LE((size_t)16, builder3.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder3.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::local, builder3.memoryUsage());
      EXPECT_NE(Requirement::required, builder3.dedicatedAllocRequirement());
      StaticBuffer buffer3(builder3.build(data1b)); // immutable
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      EXPECT_EQ(sizeof(data1b), buffer3.size());
      EXPECT_EQ(bufferType, buffer3.type());
      EXPECT_EQ(TransferMode::standard, buffer3.transferMode());
      EXPECT_EQ(ResourceUsage::local, buffer3.memoryUsage());
      EXPECT_TRUE((buffer3.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0);
      //TODO... bind
      //TODO... clear bindings

      StaticBufferParams params4(bufferType, sizeof(data2a), false, TransferMode::bidirectional);
      StaticBuffer::Builder builder4(renderer.resourceManager(), params4);
      EXPECT_LE(sizeof(data2a), builder4.requiredMemorySize());
      EXPECT_LE((size_t)16, builder4.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder4.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::local, builder4.memoryUsage());
      EXPECT_NE(Requirement::required, builder4.dedicatedAllocRequirement());
      auto useDedicatedAlloc = (builder4.dedicatedAllocRequirement() != Requirement::none) ? FeatureMode::force : FeatureMode::disable;
      StaticBuffer buffer4(builder4.build(useDedicatedAlloc));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_EQ(sizeof(data2a), buffer4.size());
      EXPECT_EQ(bufferType, buffer4.type());
      EXPECT_EQ(TransferMode::bidirectional, buffer4.transferMode());
      EXPECT_EQ(ResourceUsage::local, buffer4.memoryUsage());
      EXPECT_TRUE((buffer4.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0);
      EXPECT_TRUE(buffer4.write(renderer.resourceManager(), &data2a));
      //TODO... bind
      EXPECT_TRUE(buffer4.write(renderer.resourceManager(), &data2b));
      //TODO... clear bindings

      StaticBuffer buffer5(builder4.build(&data2a, useDedicatedAlloc));
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer5.size());
      EXPECT_EQ(bufferType, buffer5.type());
      EXPECT_EQ(TransferMode::bidirectional, buffer5.transferMode());
      EXPECT_EQ(ResourceUsage::local, buffer5.memoryUsage());
      EXPECT_TRUE((buffer5.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0);
      //TODO... bind
      EXPECT_TRUE(buffer5.write(renderer.resourceManager(), &data2b));
      //TODO... clear bindings

      StaticBufferParams params6(bufferType, sizeof(data2a), true, TransferMode::standard);
      StaticBuffer::Builder builder6(renderer.resourceManager(), params6);
      EXPECT_LE(sizeof(data1b), builder6.requiredMemorySize());
      EXPECT_LE((size_t)16, builder6.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder6.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::local, builder6.memoryUsage());
      EXPECT_NE(Requirement::required, builder6.dedicatedAllocRequirement());
      StaticBuffer buffer6(builder6.build(&data2a)); // immutable
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      EXPECT_EQ(sizeof(data1b), buffer6.size());
      EXPECT_EQ(bufferType, buffer6.type());
      EXPECT_EQ(TransferMode::standard, buffer6.transferMode());
      EXPECT_EQ(ResourceUsage::local, buffer6.memoryUsage());
      EXPECT_TRUE((buffer6.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0);
      //TODO... bind
      //TODO... clear bindings

      //BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      //TODO... bind
      //TODO... clear bindings

      auto buffer1Handle = buffer1.handle();
      StaticBuffer moved(std::move(buffer1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(buffer1.isEmpty());
      EXPECT_TRUE(moved.handle() == buffer1Handle);
      EXPECT_TRUE(buffer1.handle() == nullptr);
      EXPECT_EQ(sizeof(data1a), moved.size());
      EXPECT_EQ(bufferType, moved.type());
      EXPECT_EQ(TransferMode::standard, moved.transferMode());
      EXPECT_EQ(ResourceUsage::local, moved.memoryUsage());
      EXPECT_TRUE((moved.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0);
      buffer1 = std::move(moved);
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(buffer1.handle() == buffer1Handle);
      EXPECT_TRUE(moved.handle() == nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(bufferType, buffer1.type());
      EXPECT_EQ(TransferMode::standard, buffer1.transferMode());
      EXPECT_EQ(ResourceUsage::local, buffer1.memoryUsage());
      EXPECT_TRUE((buffer1.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0);
    }
  }

  TEST_F(VulkanBufferTest, vkDynamicBufferCreationTest) {
    uint64_t data1a[2] = { 42, 0 };
    uint64_t data1b[2] = { 0, 22 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    DynamicBuffer emptyBuffer;
    EXPECT_TRUE(emptyBuffer.isEmpty());
    EXPECT_TRUE(emptyBuffer.handle() == VK_NULL_HANDLE);

    DynamicBufferParams emptyParams{};
    EXPECT_ANY_THROW(DynamicBuffer::Builder(renderer.resourceManager(), emptyParams));
    DynamicBufferParams invalidParams(BufferType::uniform, (size_t)0);
    EXPECT_ANY_THROW(DynamicBuffer::Builder(renderer.resourceManager(), invalidParams));

    const BufferType bufferTypes[]{ BufferType::uniform, BufferType::vertex, BufferType::vertexIndex };
    for (size_t i = 0; i < sizeof(bufferTypes)/sizeof(*bufferTypes); ++i) {
      const BufferType bufferType = bufferTypes[i];

      DynamicBufferParams params1(bufferType, sizeof(data1a), false, TransferMode::standard);
      DynamicBuffer::Builder builder1(renderer.resourceManager(), params1);
      EXPECT_LE(sizeof(data1a), builder1.requiredMemorySize());
      EXPECT_LE((size_t)16, builder1.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder1.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::dynamic, builder1.memoryUsage());
      EXPECT_NE(Requirement::required, builder1.dedicatedAllocRequirement());
      DynamicBuffer buffer1(builder1.build());
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(bufferType, buffer1.type());
      EXPECT_EQ(TransferMode::standard, buffer1.transferMode());
      EXPECT_EQ(ResourceUsage::dynamic, buffer1.memoryUsage());
      EXPECT_TRUE((buffer1.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      EXPECT_TRUE(buffer1.write(data1a));
      //TODO... bind
      EXPECT_TRUE(buffer1.write(data1b));
      //TODO... clear bindings

      DynamicBuffer buffer2(builder1.build(data1a));
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(bufferType, buffer2.type());
      EXPECT_EQ(TransferMode::standard, buffer2.transferMode());
      EXPECT_EQ(ResourceUsage::dynamic, buffer2.memoryUsage());
      EXPECT_TRUE((buffer2.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      //TODO... bind
      EXPECT_TRUE(buffer2.write(data1b));
      //TODO... clear bindings

      DynamicBufferParams params3(bufferType, sizeof(data1b), true, TransferMode::standard);
      DynamicBuffer::Builder builder3(renderer.resourceManager(), params3);
      EXPECT_LE(sizeof(data1b), builder3.requiredMemorySize());
      EXPECT_LE((size_t)16, builder3.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder3.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::dynamic, builder3.memoryUsage());
      EXPECT_NE(Requirement::required, builder3.dedicatedAllocRequirement());
      DynamicBuffer buffer3(builder3.build(data1b)); // DMA
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      EXPECT_EQ(sizeof(data1b), buffer3.size());
      EXPECT_EQ(bufferType, buffer3.type());
      EXPECT_EQ(TransferMode::standard, buffer3.transferMode());
      EXPECT_EQ(ResourceUsage::dynamic, buffer3.memoryUsage());
      EXPECT_TRUE((buffer3.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      //TODO... bind
      //TODO... clear bindings

      DynamicBufferParams params4(bufferType, sizeof(data2a), false, TransferMode::bidirectional);
      DynamicBuffer::Builder builder4(renderer.resourceManager(), params4);
      EXPECT_LE(sizeof(data2a), builder4.requiredMemorySize());
      EXPECT_LE((size_t)16, builder4.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder4.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::dynamic, builder4.memoryUsage());
      EXPECT_NE(Requirement::required, builder4.dedicatedAllocRequirement());
      auto useDedicatedAlloc = (builder4.dedicatedAllocRequirement() != Requirement::none) ? FeatureMode::force : FeatureMode::disable;
      DynamicBuffer buffer4(builder4.build(useDedicatedAlloc));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_EQ(sizeof(data2a), buffer4.size());
      EXPECT_EQ(bufferType, buffer4.type());
      EXPECT_EQ(TransferMode::bidirectional, buffer4.transferMode());
      EXPECT_EQ(ResourceUsage::dynamic, buffer4.memoryUsage());
      EXPECT_TRUE((buffer4.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      EXPECT_TRUE(buffer4.write(&data2a));
      //TODO... bind
      EXPECT_TRUE(buffer4.write(&data2b));
      //TODO... clear bindings

      DynamicBuffer buffer5(builder4.build(&data2a, useDedicatedAlloc));
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer5.size());
      EXPECT_EQ(bufferType, buffer5.type());
      EXPECT_EQ(TransferMode::bidirectional, buffer5.transferMode());
      EXPECT_EQ(ResourceUsage::dynamic, buffer5.memoryUsage());
      EXPECT_TRUE((buffer5.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      //TODO... bind
      EXPECT_TRUE(buffer5.write(&data2b));
      //TODO... clear bindings

      DynamicBufferParams params6(bufferType, sizeof(data2a), true, TransferMode::standard);
      DynamicBuffer::Builder builder6(renderer.resourceManager(), params6);
      EXPECT_LE(sizeof(data1b), builder6.requiredMemorySize());
      EXPECT_LE((size_t)16, builder6.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder6.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::dynamic, builder6.memoryUsage());
      EXPECT_NE(Requirement::required, builder6.dedicatedAllocRequirement());
      DynamicBuffer buffer6(builder6.build(&data2a)); // DMA
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      EXPECT_EQ(sizeof(data1b), buffer6.size());
      EXPECT_EQ(bufferType, buffer6.type());
      EXPECT_EQ(TransferMode::standard, buffer6.transferMode());
      EXPECT_EQ(ResourceUsage::dynamic, buffer6.memoryUsage());
      EXPECT_TRUE((buffer6.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      //TODO... bind
      //TODO... clear bindings

      //BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      //TODO... bind
      //TODO... clear bindings

      auto buffer1Handle = buffer1.handle();
      DynamicBuffer moved(std::move(buffer1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(buffer1.isEmpty());
      EXPECT_TRUE(moved.handle() == buffer1Handle);
      EXPECT_TRUE(buffer1.handle() == nullptr);
      EXPECT_EQ(sizeof(data1a), moved.size());
      EXPECT_EQ(bufferType, moved.type());
      EXPECT_EQ(TransferMode::standard, moved.transferMode());
      EXPECT_EQ(ResourceUsage::dynamic, moved.memoryUsage());
      EXPECT_TRUE((moved.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      buffer1 = std::move(moved);
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(buffer1.handle() == buffer1Handle);
      EXPECT_TRUE(moved.handle() == nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(bufferType, buffer1.type());
      EXPECT_EQ(TransferMode::standard, buffer1.transferMode());
      EXPECT_EQ(ResourceUsage::dynamic, buffer1.memoryUsage());
      EXPECT_TRUE((buffer1.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    }
  }

  TEST_F(VulkanBufferTest, vkStagingBufferCreationTest) {
    uint64_t data1a[2] = { 42, 0 };
    uint64_t data1b[2] = { 0, 22 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    StagingBuffer emptyBuffer;
    EXPECT_TRUE(emptyBuffer.isEmpty());
    EXPECT_TRUE(emptyBuffer.handle() == VK_NULL_HANDLE);

    StagingBufferParams emptyParams{};
    EXPECT_ANY_THROW(StagingBuffer::Builder(renderer.resourceManager(), emptyParams));
    StagingBufferParams invalidParams(BufferType::uniform, (size_t)0);
    EXPECT_ANY_THROW(StagingBuffer::Builder(renderer.resourceManager(), invalidParams));

    const BufferType bufferTypes[]{ BufferType::uniform, BufferType::vertex, BufferType::vertexIndex };
    for (size_t i = 0; i < sizeof(bufferTypes)/sizeof(*bufferTypes); ++i) {
      const BufferType bufferType = bufferTypes[i];

      StagingBufferParams params1(bufferType, sizeof(data1a), false, TransferMode::standard);
      StagingBuffer::Builder builder1(renderer.resourceManager(), params1);
      EXPECT_LE(sizeof(data1a), builder1.requiredMemorySize());
      EXPECT_LE((size_t)16, builder1.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder1.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::staging, builder1.memoryUsage());
      EXPECT_NE(Requirement::required, builder1.dedicatedAllocRequirement());
      StagingBuffer buffer1(builder1.build());
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ((BufferType)0, buffer1.type());
      EXPECT_EQ(TransferMode::standard, buffer1.transferMode());
      EXPECT_EQ(ResourceUsage::staging, buffer1.memoryUsage());
      EXPECT_TRUE((buffer1.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      EXPECT_TRUE(buffer1.write(data1a));
      //TODO... bind
      EXPECT_TRUE(buffer1.write(data1b));
      //TODO... clear bindings

      StagingBuffer buffer2(builder1.build(data1a));
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ((BufferType)0, buffer2.type());
      EXPECT_EQ(TransferMode::standard, buffer2.transferMode());
      EXPECT_EQ(ResourceUsage::staging, buffer2.memoryUsage());
      EXPECT_TRUE((buffer2.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      //TODO... bind
      EXPECT_TRUE(buffer2.write(data1b));
      //TODO... clear bindings

      StagingBufferParams params3(bufferType, sizeof(data1b), true, TransferMode::standard);
      StagingBuffer::Builder builder3(renderer.resourceManager(), params3);
      EXPECT_LE(sizeof(data1b), builder3.requiredMemorySize());
      EXPECT_LE((size_t)16, builder3.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder3.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::staging, builder3.memoryUsage());
      EXPECT_NE(Requirement::required, builder3.dedicatedAllocRequirement());
      StagingBuffer buffer3(builder3.build(data1b)); // cache
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      EXPECT_EQ(sizeof(data1b), buffer3.size());
      EXPECT_EQ((BufferType)0, buffer3.type());
      EXPECT_EQ(TransferMode::standard, buffer3.transferMode());
      EXPECT_EQ(ResourceUsage::staging, buffer3.memoryUsage());
      EXPECT_TRUE((buffer3.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      //TODO... bind
      //TODO... clear bindings

      StagingBufferParams params4(bufferType, sizeof(data2a), false, TransferMode::bidirectional);
      StagingBuffer::Builder builder4(renderer.resourceManager(), params4);
      EXPECT_LE(sizeof(data2a), builder4.requiredMemorySize());
      EXPECT_LE((size_t)16, builder4.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder4.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::staging, builder4.memoryUsage());
      EXPECT_NE(Requirement::required, builder4.dedicatedAllocRequirement());
      auto useDedicatedAlloc = (builder4.dedicatedAllocRequirement() != Requirement::none) ? FeatureMode::force : FeatureMode::disable;
      StagingBuffer buffer4(builder4.build(useDedicatedAlloc));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_EQ(sizeof(data2a), buffer4.size());
      EXPECT_EQ((BufferType)0, buffer4.type());
      EXPECT_EQ(TransferMode::bidirectional, buffer4.transferMode());
      EXPECT_EQ(ResourceUsage::staging, buffer4.memoryUsage());
      EXPECT_TRUE((buffer4.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      EXPECT_TRUE(buffer4.write(&data2a));
      //TODO... bind
      EXPECT_TRUE(buffer4.write(&data2b));
      //TODO... clear bindings

      StagingBuffer buffer5(builder4.build(&data2a, useDedicatedAlloc));
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer5.size());
      EXPECT_EQ((BufferType)0, buffer5.type());
      EXPECT_EQ(TransferMode::bidirectional, buffer5.transferMode());
      EXPECT_EQ(ResourceUsage::staging, buffer5.memoryUsage());
      EXPECT_TRUE((buffer5.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      //TODO... bind
      EXPECT_TRUE(buffer5.write(&data2b));
      //TODO... clear bindings

      StagingBufferParams params6(bufferType, sizeof(data2a), true, TransferMode::standard);
      StagingBuffer::Builder builder6(renderer.resourceManager(), params6);
      EXPECT_LE(sizeof(data1b), builder6.requiredMemorySize());
      EXPECT_LE((size_t)16, builder6.requiredAlignment());
      EXPECT_NE((uint32_t)0, builder6.memoryTypeBits());
      EXPECT_EQ(ResourceUsage::staging, builder6.memoryUsage());
      EXPECT_NE(Requirement::required, builder6.dedicatedAllocRequirement());
      StagingBuffer buffer6(builder6.build(&data2a)); // cache
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      EXPECT_EQ(sizeof(data1b), buffer6.size());
      EXPECT_EQ((BufferType)0, buffer6.type());
      EXPECT_EQ(TransferMode::standard, buffer6.transferMode());
      EXPECT_EQ(ResourceUsage::staging, buffer6.memoryUsage());
      EXPECT_TRUE((buffer6.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      //TODO... bind
      //TODO... clear bindings

      //BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      //TODO... bind
      //TODO... clear bindings

      auto buffer1Handle = buffer1.handle();
      StagingBuffer moved(std::move(buffer1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(buffer1.isEmpty());
      EXPECT_TRUE(moved.handle() == buffer1Handle);
      EXPECT_TRUE(buffer1.handle() == nullptr);
      EXPECT_EQ(sizeof(data1a), moved.size());
      EXPECT_EQ((BufferType)0, moved.type());
      EXPECT_EQ(TransferMode::standard, moved.transferMode());
      EXPECT_EQ(ResourceUsage::staging, moved.memoryUsage());
      EXPECT_TRUE((moved.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      buffer1 = std::move(moved);
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(buffer1.handle() == buffer1Handle);
      EXPECT_TRUE(moved.handle() == nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ((BufferType)0, buffer1.type());
      EXPECT_EQ(TransferMode::standard, buffer1.transferMode());
      EXPECT_EQ(ResourceUsage::staging, buffer1.memoryUsage());
      EXPECT_TRUE((buffer1.memoryUsageFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    }
  }


  // -- virtual-buffers & suballocation --

  static constexpr inline size_t __alignMemorySize(size_t length) noexcept {
    return (((length + 15u) >> 4) << 4);
  }
  static constexpr inline size_t __alignUniformSize(size_t length) noexcept {
    return (((length + 255u) >> 8) << 8);
  }

  static bool __vkCopyFromBuffer(DeviceResourceManager device, const StaticBuffer& src, StagingBuffer& dest) {
    return dest.copyFrom(device, src);
  }
  static bool __vkCopyFromBuffer(DeviceResourceManager, const DynamicBuffer& src, StagingBuffer& dest) {
    return dest.copyFrom(src);
  }
  static bool __vkCopyFromBuffer(DeviceResourceManager, const StagingBuffer& src, StagingBuffer& dest) {
    return dest.copyFrom(src);
  }

  static bool __vkWriteBuffer(DeviceResourceManager device, StaticBuffer& dest, const void* data) {
    return dest.write(device, data);
  }
  static bool __vkWriteBuffer(DeviceResourceManager, DynamicBuffer& dest, const void* data) {
    return dest.write(data);
  }
  static bool __vkWriteBuffer(DeviceResourceManager, StagingBuffer& dest, const void* data) {
    return dest.write(data);
  }

  template <ResourceUsage _Usage>
  static void __vkCreateMultiTypeBuffers(Renderer& renderer) {
    uint64_t data1a[4] = { 42, 0, 42, 0 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 0.5f,0.6f,0.7f,0.8f };

    // vertex + index
    constexpr const size_t vertexSize = __alignMemorySize(sizeof(data1a));
    constexpr const size_t indexSize = __alignMemorySize(sizeof(data2a));
    BufferParams<_Usage> params1((BufferType::vertex | BufferType::vertexIndex), vertexSize + indexSize, false, TransferMode::bidirectional);
    Buffer<_Usage> buffer1 = typename Buffer<_Usage>::Builder(renderer.resourceManager(), params1).build();
    EXPECT_EQ(vertexSize + indexSize, buffer1.size());
    EXPECT_EQ((BufferType::vertex | BufferType::vertexIndex), buffer1.type());

    uint8_t vertexIndexData1[vertexSize + indexSize]{};
    memcpy(vertexIndexData1, data1a, sizeof(data1a));
    memcpy(vertexIndexData1 + vertexSize, &data2a, sizeof(data2a));
    EXPECT_TRUE(__vkWriteBuffer(renderer.resourceManager(), buffer1, vertexIndexData1));

    MappedBufferIO reader;
    BufferParams<ResourceUsage::staging> paramsStaging1((BufferType::vertex | BufferType::vertexIndex),
                                                         vertexSize + indexSize, false, TransferMode::bidirectional);
    StagingBuffer staging1 = Buffer<ResourceUsage::staging>::Builder(renderer.resourceManager(), paramsStaging1).build();
    ASSERT_TRUE(__vkCopyFromBuffer(renderer.resourceManager(), buffer1, staging1));
    EXPECT_TRUE(reader.open(staging1, IOMode::readWrite));
    ASSERT_TRUE(reader.isOpen());
    const uint64_t* vertexStaging = (const uint64_t*)reader.data();
    const _TestColor* indexStaging = (const _TestColor*)(((const uint8_t*)reader.data()) + vertexSize);
    EXPECT_EQ(data1a[0], vertexStaging[0]);
    EXPECT_EQ(data1a[1], vertexStaging[1]);
    EXPECT_EQ(data2a.rgba[0], indexStaging->rgba[0]);
    EXPECT_EQ(data2a.rgba[1], indexStaging->rgba[1]);
    EXPECT_EQ(data2a.rgba[2], indexStaging->rgba[2]);
    EXPECT_EQ(data2a.rgba[3], indexStaging->rgba[3]);
    reader.close();

    //TODO... bind + unbind

    // uniform + uniform
    if (renderer.featureLevel() >= VK_API_VERSION_1_2) {
      constexpr const size_t uniform1Size = __alignUniformSize(sizeof(data1a));
      constexpr const size_t uniform2Size = __alignUniformSize(sizeof(data2b));
      BufferParams<_Usage> params2(BufferType::uniform, uniform1Size + uniform2Size, false, TransferMode::bidirectional);
      Buffer<_Usage> buffer2 = typename Buffer<_Usage>::Builder(renderer.resourceManager(), params2).build();
      EXPECT_EQ(uniform1Size + uniform2Size, buffer2.size());
      EXPECT_EQ((BufferType::uniform), buffer2.type());

      uint8_t uniformData2[uniform1Size + uniform2Size]{};
      memcpy(uniformData2, data1a, sizeof(data1a));
      memcpy(uniformData2 + uniform1Size, &data2b, sizeof(data2b));
      EXPECT_TRUE(__vkWriteBuffer(renderer.resourceManager(), buffer2, uniformData2));

      BufferParams<ResourceUsage::staging> paramsStaging2(BufferType::uniform, uniform1Size + uniform2Size, false, TransferMode::bidirectional);
      StagingBuffer staging2 = Buffer<ResourceUsage::staging>::Builder(renderer.resourceManager(), paramsStaging2).build();
      ASSERT_TRUE(__vkCopyFromBuffer(renderer.resourceManager(), buffer2, staging2));
      EXPECT_TRUE(reader.open(staging2, IOMode::read));
      ASSERT_TRUE(reader.isOpen());
      const uint64_t* uniform1Staging = (const uint64_t*)reader.data();
      const _TestColor* uniform2Staging = (const _TestColor*)(((const uint8_t*)reader.data()) + uniform1Size);
      EXPECT_EQ(data1a[0], uniform1Staging[0]);
      EXPECT_EQ(data1a[1], uniform1Staging[1]);
      EXPECT_EQ(data2b.rgba[0], uniform2Staging->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], uniform2Staging->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], uniform2Staging->rgba[2]);
      EXPECT_EQ(data2b.rgba[3], uniform2Staging->rgba[3]);
      reader.close();

      //BufferHandle uniforms[2]{ buffer2.handle(), buffer2.handle() };
      //unsigned int byte16Offsets[2]{ 0, (unsigned int)uniform1Size >> 4 };
      //unsigned int byte16Sizes[2]{ (unsigned int)uniform1Size >> 4, (unsigned int)uniform2Size >> 4 };
      //TODO... bind + unbind
    }
  }

  template <ResourceUsage _Usage>
  static void __vkCreateSuballocatedBuffers(Renderer& renderer) {
    uint64_t data1a[4] = { 42, 0, 42, 0 };
    uint64_t data1b[4] = { 1, 2, 3, 4 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 0.5f,0.6f,0.7f,0.8f };

    struct {
      BufferType type;
      TransferMode transfer;
      size_t size;
      size_t alignedOffset;
      const void* data;
      bool isDataTestColor;
    } bufferInfo[] {
      { BufferType::vertex, TransferMode::bidirectional, sizeof(data1a), 0, data1a, false },
      { BufferType::vertexIndex, TransferMode::bidirectional, sizeof(data2a), 0, &data2a, true },
      { BufferType::uniform, TransferMode::bidirectional, sizeof(data1b), 0, data1b, false },
      { BufferType::uniform, TransferMode::standard, sizeof(data2b), 0, &data2b, true }
    };
    Buffer<_Usage> buffers[sizeof(bufferInfo)/sizeof(*bufferInfo)]{};
    Buffer<_Usage> buffersWithInit[sizeof(bufferInfo)/sizeof(*bufferInfo)]{};
    typename Buffer<_Usage>::Builder builders[sizeof(bufferInfo)/sizeof(*bufferInfo)]{};
    BufferParams<_Usage> params[sizeof(bufferInfo)/sizeof(*bufferInfo)]{};
    int bufferCount = (int)sizeof(bufferInfo)/sizeof(*bufferInfo);

    // vertex + index + uniform + uniform
    size_t totalSize = 0;
    for (int i = 0; i < bufferCount; ++i) {
      auto& current = bufferInfo[i];
      params[i] = BufferParams<_Usage>(current.type, current.size, false, current.transfer);
      builders[i] = typename Buffer<_Usage>::Builder(renderer.resourceManager(), params[i]);
      totalSize = DeviceMemoryPool::align(totalSize, builders[i].requiredAlignment());
      current.alignedOffset = totalSize;
      totalSize += builders[i].requiredMemorySize();
    }

    totalSize = DeviceMemoryPool::alignPoolSize(totalSize);
    DeviceMemoryPool memoryPool(renderer.resourceManager(), totalSize, builders, (uint32_t)(sizeof(bufferInfo)/sizeof(*bufferInfo)));
    ASSERT_TRUE(memoryPool.handle() != VK_NULL_HANDLE);

    for (int i = 0; i < bufferCount; ++i) {
      const auto& current = bufferInfo[i];
      buffers[i] = builders[i].build(memoryPool, current.alignedOffset);
      buffersWithInit[i] = builders[i].build(bufferInfo[i].data, memoryPool, current.alignedOffset);
      __if_constexpr (_Usage != ResourceUsage::staging) {
        EXPECT_EQ(current.type, buffers[i].type());
        EXPECT_EQ(current.type, buffersWithInit[i].type());
      }
      EXPECT_EQ(current.size, buffers[i].size());
      EXPECT_EQ(current.size, buffersWithInit[i].size());
      EXPECT_EQ(current.transfer, buffers[i].transferMode());
      EXPECT_EQ(current.transfer, buffersWithInit[i].transferMode());
      EXPECT_TRUE(buffers[i].handle() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffersWithInit[i].handle() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffers[i].allocation() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffersWithInit[i].allocation() != VK_NULL_HANDLE);

      EXPECT_TRUE(__vkWriteBuffer(renderer.resourceManager(), buffers[i], bufferInfo[i].data));
    }

    MappedBufferIO reader;
    for (int i = 0; i < 2*bufferCount; ++i) {
      const auto& current = bufferInfo[(i < bufferCount) ? i : i - bufferCount];
      if (current.transfer == TransferMode::bidirectional) { // only check content of readable buffers
        StagingBufferParams paramsSt(current.type, current.size, false, TransferMode::bidirectional);
        auto staging = StagingBuffer::Builder(renderer.resourceManager(), paramsSt).build();
        EXPECT_TRUE(__vkCopyFromBuffer(renderer.resourceManager(), (i < bufferCount) ? buffers[i] : buffersWithInit[i - bufferCount], staging));

        reader.open(staging, IOMode::read);
        ASSERT_TRUE(reader.isOpen());
        if (current.isDataTestColor) {
          EXPECT_EQ(((const _TestColor*)current.data)->rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
          EXPECT_EQ(((const _TestColor*)current.data)->rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
          EXPECT_EQ(((const _TestColor*)current.data)->rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
          EXPECT_EQ(((const _TestColor*)current.data)->rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
        }
        else {
          EXPECT_EQ(((const uint64_t*)current.data)[0], ((const uint64_t*)reader.data())[0]);
          EXPECT_EQ(((const uint64_t*)current.data)[1], ((const uint64_t*)reader.data())[1]);
          EXPECT_EQ(((const uint64_t*)current.data)[2], ((const uint64_t*)reader.data())[2]);
          EXPECT_EQ(((const uint64_t*)current.data)[3], ((const uint64_t*)reader.data())[3]);
        }
        reader.close();
      }
    }

    for (int i = 0; i < bufferCount; ++i)
      buffers[i].release();
    memoryPool.release();
  }

  TEST_F(VulkanBufferTest, vkMultiTypeBufferCreationTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    __vkCreateMultiTypeBuffers<ResourceUsage::local>(renderer);
    __vkCreateMultiTypeBuffers<ResourceUsage::dynamic>(renderer);
  }
  TEST_F(VulkanBufferTest, vkSuballocatedBufferCreationTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    __vkCreateSuballocatedBuffers<ResourceUsage::local>(renderer);
    __vkCreateSuballocatedBuffers<ResourceUsage::dynamic>(renderer);
    __vkCreateSuballocatedBuffers<ResourceUsage::staging>(renderer);
  }


  // -- buffer copy/mapping --

  TEST_F(VulkanBufferTest, vkBufferCopyTest) {
    uint64_t data1a[2] = { 42, 8 };
    uint64_t data1b[2] = { 10, 22 };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      MappedBufferIO reader;
      StagingBufferParams paramsSt1(BufferType::uniform, sizeof(data1a), true, TransferMode::bidirectional);
      StagingBuffer staging1 = StagingBuffer::Builder(renderer.resourceManager(), paramsSt1).build();

      StaticBufferParams paramsI1(BufferType::uniform, sizeof(data1b), true, TransferMode::bidirectional);
      StaticBuffer bufferI1 = StaticBuffer::Builder(renderer.resourceManager(), paramsI1).build(data1b);
      EXPECT_TRUE(staging1.copyFrom(renderer.resourceManager(), bufferI1));
      EXPECT_TRUE(reader.open(staging1, IOMode::read));
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1b[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1b[1], ((const uint64_t*)reader.data())[1]);
      reader.close();
      bufferI1.release();

      StaticBufferParams paramsSP(BufferType::uniform, sizeof(data1a), true, TransferMode::bidirectional);
      StaticBuffer::Builder builderSP(renderer.resourceManager(), paramsSP);
      DeviceMemoryPool memoryPool(renderer.resourceManager(), (size_t)256u, &builderSP, 1u);
      StaticBuffer bufferSP = builderSP.build(memoryPool, 0);
      EXPECT_TRUE(bufferSP.write(renderer.resourceManager(), data1a));
      EXPECT_TRUE(staging1.copyFrom(renderer.resourceManager(), bufferSP));
      EXPECT_TRUE(reader.open(staging1, IOMode::read));
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1a[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1a[1], ((const uint64_t*)reader.data())[1]);
      reader.close();
      bufferSP.release();
      memoryPool.release();

      StaticBufferParams paramsS1(BufferType::uniform, sizeof(data1a), true, TransferMode::bidirectional);
      StaticBuffer bufferS1 = StaticBuffer::Builder(renderer.resourceManager(), paramsS1).build();
      DynamicBufferParams paramsD1(BufferType::uniform, sizeof(data1a), true, TransferMode::bidirectional);
      DynamicBuffer bufferD1 = DynamicBuffer::Builder(renderer.resourceManager(), paramsD1).build();
      EXPECT_TRUE(staging1.write(data1a));
      EXPECT_TRUE(bufferS1.copyFrom(renderer.resourceManager(), staging1));
      EXPECT_TRUE(bufferD1.copyFrom(renderer.resourceManager(), bufferS1));
      EXPECT_TRUE(reader.open(staging1, IOMode::readWrite));
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1a[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1a[1], ((const uint64_t*)reader.data())[1]);
      reader.close();

      EXPECT_TRUE(bufferD1.write(data1b));
      EXPECT_TRUE(staging1.copyFrom(bufferD1));
      EXPECT_TRUE(reader.open(staging1, IOMode::read));
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1b[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1b[1], ((const uint64_t*)reader.data())[1]);
      reader.close();

      EXPECT_TRUE(bufferD1.write(data1a));
      EXPECT_TRUE(bufferS1.copyFrom(renderer.resourceManager(), bufferD1));
      EXPECT_TRUE(staging1.copyFrom(renderer.resourceManager(), bufferS1));
      EXPECT_TRUE(reader.open(staging1, IOMode::readWrite));
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1a[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1a[1], ((const uint64_t*)reader.data())[1]);
      reader.close();
    }
  }

  TEST_F(VulkanBufferTest, vkMappedBufferReadWriteTest) {
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      DynamicBufferParams paramsD1(BufferType::uniform, sizeof(data2a), true, TransferMode::standard);
      DynamicBuffer bufferD1 = DynamicBuffer::Builder(renderer.resourceManager(), paramsD1).build();
      StagingBufferParams paramsSt1(BufferType::uniform, sizeof(data2a), true, TransferMode::bidirectional);
      StagingBuffer staging1 = StagingBuffer::Builder(renderer.resourceManager(), paramsSt1).build(&data2b);

      MappedBufferIO reader(staging1, IOMode::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2b.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2b.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());

      MappedBufferIO writer(bufferD1);
      ASSERT_TRUE(writer.isOpen());
      memcpy(writer.data(), &data2a, sizeof(data2a));
      writer.close();
      EXPECT_FALSE(writer.isOpen());
      EXPECT_TRUE(staging1.copyFrom(bufferD1));
      EXPECT_TRUE(reader.open(staging1, IOMode::read));
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2a.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2a.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2a.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2a.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());

      EXPECT_TRUE(writer.open(staging1, IOMode::readWrite));
      ASSERT_TRUE(writer.isOpen());
      memcpy(writer.data(), &data2b, sizeof(data2b));
      EXPECT_EQ(data2b.rgba[0], ((const _TestColor*)writer.data())->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], ((const _TestColor*)writer.data())->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], ((const _TestColor*)writer.data())->rgba[2]);
      EXPECT_EQ(data2b.rgba[3], ((const _TestColor*)writer.data())->rgba[3]);
      writer.close();
      EXPECT_FALSE(writer.isOpen());
      EXPECT_TRUE(reader.open(staging1, IOMode::read));
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2b.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2b.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());

      EXPECT_TRUE(writer.open(staging1, IOMode::write));
      ASSERT_TRUE(writer.isOpen());
      memcpy(writer.data(), &data2a, sizeof(data2a));
      writer.close();
      EXPECT_FALSE(writer.isOpen());
      reader.open(staging1, IOMode::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2a.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2a.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2a.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2a.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());

      EXPECT_TRUE(writer.open(bufferD1));
      ASSERT_TRUE(writer.isOpen());
      ((_TestColor*)writer.data())->rgba[1] = data2b.rgba[1];
      ((_TestColor*)writer.data())->rgba[2] = data2b.rgba[2];
      writer.close();
      EXPECT_FALSE(writer.isOpen());
      EXPECT_TRUE(staging1.copyFrom(bufferD1));
      EXPECT_TRUE(reader.open(staging1, IOMode::read));
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2a.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2a.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());
    }
  }

# undef __if_constexpr
#endif
