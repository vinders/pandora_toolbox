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


  // -- depth/stencil buffer creation/binding --

  TEST_F(VulkanBufferTest, vkDepthStencilBufferCreationTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    
    DepthStencilBuffer emptyBuffer;
    EXPECT_TRUE(emptyBuffer.isEmpty());
    EXPECT_TRUE(emptyBuffer.handle() == nullptr);
    EXPECT_TRUE(emptyBuffer.getDepthStencilView() == nullptr);
    
    EXPECT_ANY_THROW(DepthStencilBuffer(renderer, DepthStencilFormat::d32_f, 0, 600));
    EXPECT_ANY_THROW(DepthStencilBuffer(renderer, DepthStencilFormat::d32_f, 600, 0));
    
    DepthStencilBuffer buffer1(renderer, DepthStencilFormat::d32_f, 800, 600);
    EXPECT_FALSE(buffer1.isEmpty());
    EXPECT_TRUE(buffer1.handle() != nullptr);
    EXPECT_TRUE(buffer1.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)800, buffer1.width());
    EXPECT_EQ((uint32_t)600, buffer1.height());
    EXPECT_EQ(DepthStencilFormat::d32_f, buffer1.getFormat());
    
    DepthStencilBuffer buffer2(renderer, DepthStencilFormat::d32_f, 492, 226);
    EXPECT_FALSE(buffer2.isEmpty());
    EXPECT_TRUE(buffer2.handle() != nullptr);
    EXPECT_TRUE(buffer2.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)492, buffer2.width());
    EXPECT_EQ((uint32_t)226, buffer2.height());
    EXPECT_EQ(DepthStencilFormat::d32_f, buffer2.getFormat());
    
    DepthStencilBuffer buffer3(renderer, DepthStencilFormat::d16_unorm, 640, 480);
    EXPECT_FALSE(buffer3.isEmpty());
    EXPECT_TRUE(buffer3.handle() != nullptr);
    EXPECT_TRUE(buffer3.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)640, buffer3.width());
    EXPECT_EQ((uint32_t)480, buffer3.height());
    EXPECT_EQ(DepthStencilFormat::d16_unorm, buffer3.getFormat());
    
    DepthStencilBuffer buffer4(renderer, DepthStencilFormat::d32_f_s8_ui, 64, 32);
    EXPECT_FALSE(buffer4.isEmpty());
    EXPECT_TRUE(buffer4.handle() != nullptr);
    EXPECT_TRUE(buffer4.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)64, buffer4.width());
    EXPECT_EQ((uint32_t)32, buffer4.height());
    EXPECT_EQ(DepthStencilFormat::d32_f_s8_ui, buffer4.getFormat());
    
    DepthStencilBuffer buffer5(renderer, DepthStencilFormat::d24_unorm_s8_ui, 300, 200);
    EXPECT_FALSE(buffer5.isEmpty());
    EXPECT_TRUE(buffer5.handle() != nullptr);
    EXPECT_TRUE(buffer5.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)300, buffer5.width());
    EXPECT_EQ((uint32_t)200, buffer5.height());
    EXPECT_EQ(DepthStencilFormat::d24_unorm_s8_ui, buffer5.getFormat());
    
    DepthStencilBuffer buffer6(renderer, DepthStencilFormat::d16_unorm, 1024, 768);
    EXPECT_FALSE(buffer6.isEmpty());
    EXPECT_TRUE(buffer6.handle() != nullptr);
    EXPECT_TRUE(buffer6.getDepthStencilView() != nullptr);
    EXPECT_EQ((uint32_t)1024, buffer6.width());
    EXPECT_EQ((uint32_t)768, buffer6.height());
    EXPECT_EQ(DepthStencilFormat::d16_unorm, buffer6.getFormat());
    
    auto buffer1Handle = buffer1.handle();
    auto buffer1View = buffer1.getDepthStencilView();
    DepthStencilBuffer moved(std::move(buffer1));
    EXPECT_FALSE(moved.isEmpty());
    EXPECT_TRUE(buffer1.isEmpty());
    EXPECT_TRUE(moved.handle() == buffer1Handle);
    EXPECT_TRUE(buffer1.handle() == nullptr);
    EXPECT_TRUE(moved.getDepthStencilView() == buffer1View);
    EXPECT_TRUE(buffer1.getDepthStencilView() == nullptr);
    buffer1 = std::move(moved);
    EXPECT_FALSE(buffer1.isEmpty());
    EXPECT_TRUE(moved.isEmpty());
    EXPECT_TRUE(buffer1.handle() == buffer1Handle);
    EXPECT_TRUE(moved.handle() == nullptr);
    EXPECT_TRUE(buffer1.getDepthStencilView() == buffer1View);
    EXPECT_TRUE(moved.getDepthStencilView() == nullptr);
  }

  // -- constant and vertex buffer creation/binding --

  TEST_F(VulkanBufferTest, vkConstantBufferCreationTest) {
    uint64_t data1a[2] = { 42, 0 };
    uint64_t data1b[2] = { 0, 22 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      Buffer<ResourceUsage::staticGpu> emptyBuffer;
      EXPECT_TRUE(emptyBuffer.isEmpty());
      EXPECT_TRUE(emptyBuffer.handle() == nullptr);

      EXPECT_ANY_THROW(Buffer<ResourceUsage::staticGpu>(renderer, BufferType::uniform, 0));
      EXPECT_ANY_THROW(Buffer<ResourceUsage::staticGpu>(renderer, BufferType::uniform, 0, nullptr));

      Buffer<ResourceUsage::staticGpu> buffer1(renderer, BufferType::uniform, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BufferType::uniform, buffer1.type());
      EXPECT_TRUE(buffer1.write((const void*)data1a));
      //TODO... bind
      EXPECT_TRUE(buffer1.write((const void*)data1b));
      //TODO... clear bindings

      Buffer<ResourceUsage::staticGpu> buffer2(renderer, BufferType::uniform, sizeof(data1a), (const void*)data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BufferType::uniform, buffer2.type());
      //TODO... bind
      EXPECT_TRUE(buffer2.write((const void*)data1b));
      //TODO... clear bindings

      Buffer<ResourceUsage::immutable> buffer3(renderer, BufferType::uniform, sizeof(data1b), (const void*)data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings

      Buffer<ResourceUsage::staticGpu> buffer4(renderer, BufferType::uniform, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_TRUE(buffer4.write((const void*)&data2a));
      //TODO... bind
      EXPECT_TRUE(buffer4.write((const void*)&data2b));
      //TODO... clear bindings

      Buffer<ResourceUsage::staticGpu> buffer5(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings
      EXPECT_TRUE(buffer5.write((const void*)&data2b));

      Buffer<ResourceUsage::immutable> buffer6(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings
      EXPECT_FALSE(buffer6.write((const void*)&data2b));

      BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      //TODO... bind
      //TODO... clear bindings

      auto buffer1Handle = buffer1.handle();
      Buffer<ResourceUsage::staticGpu> moved(std::move(buffer1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(buffer1.isEmpty());
      EXPECT_TRUE(moved.handle() == buffer1Handle);
      EXPECT_TRUE(buffer1.handle() == nullptr);
      buffer1 = std::move(moved);
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(buffer1.handle() == buffer1Handle);
      EXPECT_TRUE(moved.handle() == nullptr);
    }{
      Buffer<ResourceUsage::dynamicCpu> emptyBuffer;
      EXPECT_TRUE(emptyBuffer.isEmpty());
      EXPECT_TRUE(emptyBuffer.handle() == nullptr);

      EXPECT_ANY_THROW(Buffer<ResourceUsage::dynamicCpu>(renderer, BufferType::uniform, 0));
      EXPECT_ANY_THROW(Buffer<ResourceUsage::dynamicCpu>(renderer, BufferType::uniform, 0, nullptr));

      Buffer<ResourceUsage::dynamicCpu> buffer1(renderer, BufferType::uniform, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BufferType::uniform, buffer1.type());
      buffer1.write((const void*)data1a);
      //TODO... bind
      buffer1.write((const void*)data1b);
      //TODO... clear bindings

      Buffer<ResourceUsage::dynamicCpu> buffer2(renderer, BufferType::uniform, sizeof(data1a), (const void*)data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BufferType::uniform, buffer2.type());
      //TODO... bind
      EXPECT_TRUE(buffer2.write((const void*)data1b));
      //TODO... clear bindings

      Buffer<ResourceUsage::dynamicCpu> buffer3(renderer, BufferType::uniform, sizeof(data1b), (const void*)data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings

      Buffer<ResourceUsage::dynamicCpu> buffer4(renderer, BufferType::uniform, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_TRUE(buffer4.write((const void*)&data2a));
      //TODO... bind
      EXPECT_TRUE(buffer4.write((const void*)&data2b));
      //TODO... clear bindings

      Buffer<ResourceUsage::dynamicCpu> buffer5(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings
      EXPECT_TRUE(buffer5.write((const void*)&data2b));

      Buffer<ResourceUsage::dynamicCpu> buffer6(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings

      BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      //TODO... bind
      //TODO... clear bindings

      auto buffer1Handle = buffer1.handle();
      Buffer<ResourceUsage::dynamicCpu> moved(std::move(buffer1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(buffer1.isEmpty());
      EXPECT_TRUE(moved.handle() == buffer1Handle);
      EXPECT_TRUE(buffer1.handle() == nullptr);
      buffer1 = std::move(moved);
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(buffer1.handle() == buffer1Handle);
      EXPECT_TRUE(moved.handle() == nullptr);
    }
  }

  TEST_F(VulkanBufferTest, vkVertexBufferCreationTest) {
    uint64_t data1a[4] = { 42, 0, 42, 0 };
    uint64_t data1b[2] = { 0, 22 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      Buffer<ResourceUsage::staticGpu> emptyBuffer;
      EXPECT_TRUE(emptyBuffer.isEmpty());
      EXPECT_TRUE(emptyBuffer.handle() == nullptr);

      EXPECT_ANY_THROW(Buffer<ResourceUsage::staticGpu>(renderer, BufferType::vertex, 0));
      EXPECT_ANY_THROW(Buffer<ResourceUsage::staticGpu>(renderer, BufferType::vertexIndex, 0, nullptr));

      Buffer<ResourceUsage::staticGpu> buffer1(renderer, BufferType::vertex, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BufferType::vertex, buffer1.type());
      EXPECT_TRUE(buffer1.write((const void*)data1a));
      //TODO... bind
      //TODO... clear bindings

      Buffer<ResourceUsage::staticGpu> buffer2(renderer, BufferType::vertexIndex, sizeof(data1a), (const void*)data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BufferType::vertexIndex, buffer2.type());
      //TODO... bind
      EXPECT_TRUE(buffer2.write((const void*)data1b));
      //TODO... clear bindings

      Buffer<ResourceUsage::immutable> buffer3(renderer, BufferType::vertex, sizeof(data1b), (const void*)data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings

      Buffer<ResourceUsage::staticGpu> buffer4(renderer, BufferType::vertex, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_TRUE(buffer4.write((const void*)&data2a));
      //TODO... bind
      EXPECT_TRUE(buffer4.write((const void*)&data2b));
      //TODO... clear bindings

      Buffer<ResourceUsage::staticGpu> buffer5(renderer, BufferType::vertexIndex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings
      EXPECT_TRUE(buffer5.write((const void*)&data2b));

      Buffer<ResourceUsage::immutable> buffer6(renderer, BufferType::vertex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings

      BufferHandle buffers[] = { buffer3.handle(), buffer6.handle() };
      unsigned int sizes[] = { (unsigned int)buffer3.size(), (unsigned int)buffer6.size() };
      unsigned int offsets[] = { 0,0 };
      //TODO... bind
      //TODO... clear bindings

      auto buffer1Handle = buffer1.handle();
      Buffer<ResourceUsage::staticGpu> moved(std::move(buffer1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(buffer1.isEmpty());
      EXPECT_TRUE(moved.handle() == buffer1Handle);
      EXPECT_TRUE(buffer1.handle() == nullptr);
      buffer1 = std::move(moved);
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(buffer1.handle() == buffer1Handle);
      EXPECT_TRUE(moved.handle() == nullptr);
    }{
      Buffer<ResourceUsage::dynamicCpu> emptyBuffer;
      EXPECT_TRUE(emptyBuffer.isEmpty());
      EXPECT_TRUE(emptyBuffer.handle() == nullptr);

      EXPECT_ANY_THROW(Buffer<ResourceUsage::dynamicCpu>(renderer, BufferType::vertex, 0));
      EXPECT_ANY_THROW(Buffer<ResourceUsage::dynamicCpu>(renderer, BufferType::vertexIndex, 0, nullptr));

      Buffer<ResourceUsage::dynamicCpu> buffer1(renderer, BufferType::vertex, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BufferType::vertex, buffer1.type());
      EXPECT_TRUE(buffer1.write((const void*)data1a));
      //TODO... bind
      //TODO... clear bindings

      Buffer<ResourceUsage::dynamicCpu> buffer2(renderer, BufferType::vertexIndex, sizeof(data1a), (const void*)data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BufferType::vertexIndex, buffer2.type());
      //TODO... bind
      EXPECT_TRUE(buffer2.write((const void*)data1b));
      //TODO... clear bindings

      Buffer<ResourceUsage::dynamicCpu> buffer3(renderer, BufferType::vertex, sizeof(data1b), (const void*)data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings

      Buffer<ResourceUsage::dynamicCpu> buffer4(renderer, BufferType::vertex, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_TRUE(buffer4.write((const void*)&data2a));
      //TODO... bind
      EXPECT_TRUE(buffer4.write((const void*)&data2b));
      //TODO... clear bindings

      Buffer<ResourceUsage::dynamicCpu> buffer5(renderer, BufferType::vertexIndex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings
      EXPECT_TRUE(buffer5.write((const void*)&data2b));

      Buffer<ResourceUsage::dynamicCpu> buffer6(renderer, BufferType::vertex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      //TODO... bind
      //TODO... clear bindings

      BufferHandle buffers[] = { buffer3.handle(), buffer6.handle() };
      unsigned int sizes[] = { (unsigned int)buffer3.size(), (unsigned int)buffer6.size() };
      unsigned int offsets[] = { 0,0 };
      //TODO... bind
      //TODO... clear bindings

      auto buffer1Handle = buffer1.handle();
      Buffer<ResourceUsage::dynamicCpu> moved(std::move(buffer1));
      EXPECT_FALSE(moved.isEmpty());
      EXPECT_TRUE(buffer1.isEmpty());
      EXPECT_TRUE(moved.handle() == buffer1Handle);
      EXPECT_TRUE(buffer1.handle() == nullptr);
      buffer1 = std::move(moved);
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(moved.isEmpty());
      EXPECT_TRUE(buffer1.handle() == buffer1Handle);
      EXPECT_TRUE(moved.handle() == nullptr);
    }
  }

  static constexpr inline size_t __alignMemorySize(size_t length) noexcept {
    return (((length + 15u) >> 4) << 4);
  }
  static constexpr inline size_t __alignUniformSize(size_t length) noexcept {
    return (((length + 255u) >> 8) << 8);
  }
  static constexpr inline size_t __alignSuballocSize(size_t length) noexcept {
    return (((length + 255u) >> 8) << 8);
  }

  TEST_F(VulkanBufferTest, vkMultiTypeBufferCreationTest) {
    uint64_t data1a[4] = { 42, 0, 42, 0 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 0.5f,0.6f,0.7f,0.8f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    MappedBufferIO reader;
    {
      // vertex + index
      constexpr const size_t vertexSize = __alignMemorySize(sizeof(data1a));
      constexpr const size_t indexSize = __alignMemorySize(sizeof(data2a));
      Buffer<ResourceUsage::staticGpu> buffer1(renderer, BufferType::vertex | BufferType::vertexIndex, vertexSize + indexSize, true);
      EXPECT_EQ(vertexSize + indexSize, buffer1.size());
      EXPECT_EQ((BufferType::vertex | BufferType::vertexIndex), buffer1.type());

      uint8_t vertexIndexData1[vertexSize + indexSize]{};
      memcpy(vertexIndexData1, data1a, sizeof(data1a));
      memcpy(vertexIndexData1 + vertexSize, &data2a, sizeof(data2a));
      EXPECT_TRUE(buffer1.write((const void*)vertexIndexData1));

      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::vertex | BufferType::vertexIndex, vertexSize + indexSize, true);
      staging1.copy(buffer1);
      reader.open(staging1, StagedMapping::readWrite);
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
        Buffer<ResourceUsage::staticGpu> buffer2(renderer, BufferType::uniform, uniform1Size + uniform2Size, true);
        EXPECT_EQ(uniform1Size + uniform2Size, buffer2.size());
        EXPECT_EQ((BufferType::uniform), buffer2.type());

        uint8_t uniformData2[uniform1Size + uniform2Size]{};
        memcpy(uniformData2, data1a, sizeof(data1a));
        memcpy(uniformData2 + uniform1Size, &data2b, sizeof(data2b));
        EXPECT_TRUE(buffer2.write((const void*)uniformData2));

        Buffer<ResourceUsage::staging> staging2(renderer, BufferType::uniform, uniform1Size + uniform2Size, true);
        staging2.copy(buffer2);
        reader.open(staging2, StagedMapping::read);
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

        BufferHandle uniforms[2]{ buffer2.handle(), buffer2.handle() };
        unsigned int byte16Offsets[2]{ 0, (unsigned int)uniform1Size >> 4 };
        unsigned int byte16Sizes[2]{ (unsigned int)uniform1Size >> 4, (unsigned int)uniform2Size >> 4 };
        //TODO... bind + unbind
      }
    }{
      // vertex + index
      constexpr const size_t vertexSize = __alignMemorySize(sizeof(data1a));
      constexpr const size_t indexSize = __alignMemorySize(sizeof(data2a));
      Buffer<ResourceUsage::dynamicCpu> buffer1(renderer, BufferType::vertex | BufferType::vertexIndex, vertexSize + indexSize, true);
      EXPECT_EQ(vertexSize + indexSize, buffer1.size());
      EXPECT_EQ((BufferType::vertex | BufferType::vertexIndex), buffer1.type());

      uint8_t vertexIndexData1[vertexSize + indexSize]{};
      memcpy(vertexIndexData1, data1a, sizeof(data1a));
      memcpy(vertexIndexData1 + vertexSize, &data2a, sizeof(data2a));
      EXPECT_TRUE(buffer1.write((const void*)vertexIndexData1));

      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::vertex | BufferType::vertexIndex, vertexSize + indexSize, true);
      staging1.copy(buffer1);
      reader.open(staging1, StagedMapping::readWrite);
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
        Buffer<ResourceUsage::dynamicCpu> buffer2(renderer, BufferType::uniform, uniform1Size + uniform2Size, true);
        EXPECT_EQ(uniform1Size + uniform2Size, buffer2.size());
        EXPECT_EQ((BufferType::uniform), buffer2.type());

        uint8_t uniformData2[uniform1Size + uniform2Size]{};
        memcpy(uniformData2, data1a, sizeof(data1a));
        memcpy(uniformData2 + uniform1Size, &data2b, sizeof(data2b));
        EXPECT_TRUE(buffer2.write((const void*)uniformData2));

        Buffer<ResourceUsage::staging> staging2(renderer, BufferType::uniform, uniform1Size + uniform2Size, true);
        staging2.copy(buffer2);
        reader.open(staging2, StagedMapping::read);
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

        BufferHandle uniforms[2]{ buffer2.handle(), buffer2.handle() };
        unsigned int byte16Offsets[2]{ 0, (unsigned int)uniform1Size >> 4 };
        unsigned int byte16Sizes[2]{ (unsigned int)uniform1Size >> 4, (unsigned int)uniform2Size >> 4 };
        //TODO... bind + unbind
      }
    }
  }

  TEST_F(VulkanBufferTest, vkBufferCopyTest) {
    uint64_t data1a[2] = { 42, 8 };
    uint64_t data1b[2] = { 10, 22 };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      MappedBufferIO reader;

      Buffer<ResourceUsage::immutable> bufferI1(renderer, BufferType::uniform, sizeof(data1b), (const void*)data1b, true);
      Buffer<ResourceUsage::staticGpu> bufferS1(renderer, BufferType::uniform, sizeof(data1a), true);
      Buffer<ResourceUsage::dynamicCpu> bufferD1(renderer, BufferType::uniform, sizeof(data1a), true);
      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::uniform, sizeof(data1a), true);

      EXPECT_TRUE(staging1.write((const void*)data1a));
      bufferS1.copy(staging1);
      bufferS1.copy(bufferD1);
      reader.open(staging1, StagedMapping::readWrite);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1a[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1a[1], ((const uint64_t*)reader.data())[1]);
      reader.close();

      EXPECT_TRUE(bufferD1.write((const void*)data1b));
      staging1.copy(bufferD1);
      reader.open(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1b[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1b[1], ((const uint64_t*)reader.data())[1]);
      reader.close();

      EXPECT_TRUE(bufferD1.write((const void*)data1a));
      bufferS1.copy(bufferD1);
      staging1.copy(bufferS1);
      reader.open(staging1, StagedMapping::readWrite);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1a[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1a[1], ((const uint64_t*)reader.data())[1]);
      reader.close();

      staging1.copy(bufferI1);
      reader.open(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1b[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1b[1], ((const uint64_t*)reader.data())[1]);
      reader.close();
    }
  }

  TEST_F(VulkanBufferTest, vkMappedBufferReadWriteTest) {
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      Buffer<ResourceUsage::dynamicCpu> bufferD1(renderer, BufferType::uniform, sizeof(data2a));
      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2b, true);

      MappedBufferIO reader(staging1, StagedMapping::read);
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
      staging1.copy(bufferD1);
      reader.open(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2a.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2a.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2a.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2a.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());

      writer.open(staging1, StagedMapping::readWrite);
      ASSERT_TRUE(writer.isOpen());
      memcpy(writer.data(), &data2b, sizeof(data2b));
      EXPECT_EQ(data2b.rgba[0], ((const _TestColor*)writer.data())->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], ((const _TestColor*)writer.data())->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], ((const _TestColor*)writer.data())->rgba[2]);
      EXPECT_EQ(data2b.rgba[3], ((const _TestColor*)writer.data())->rgba[3]);
      writer.close();
      EXPECT_FALSE(writer.isOpen());
      reader.open(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2b.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2b.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());

      writer.open(staging1, StagedMapping::write);
      ASSERT_TRUE(writer.isOpen());
      memcpy(writer.data(), &data2a, sizeof(data2a));
      writer.close();
      EXPECT_FALSE(writer.isOpen());
      reader.open(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2a.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2a.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2a.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2a.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());

      writer.open(bufferD1);
      ASSERT_TRUE(writer.isOpen());
      ((_TestColor*)writer.data())->rgba[1] = data2b.rgba[1];
      ((_TestColor*)writer.data())->rgba[2] = data2b.rgba[2];
      writer.close();
      EXPECT_FALSE(writer.isOpen());
      staging1.copy(bufferD1);
      reader.open(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2a.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2a.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());
    }
  }

  TEST_F(VulkanBufferTest, vkSuballocatedBufferCreationTest) {
    uint64_t data1a[4] = { 42, 0, 42, 0 };
    uint64_t data1b[4] = { 1, 2, 3, 4 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 0.5f,0.6f,0.7f,0.8f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    MappedBufferIO reader;
    {
      // vertex + index + uniform + uniform
      constexpr const size_t vertexSize = __alignSuballocSize(sizeof(data1a));
      constexpr const size_t indexSize = __alignSuballocSize(sizeof(data2a));
      constexpr const size_t uniform1Size = __alignSuballocSize(sizeof(data1b));
      constexpr const size_t uniform2Size = __alignSuballocSize(sizeof(data2b));

      auto buffer1 = Buffer<ResourceUsage::staticGpu>::createUnallocatedBuffer(renderer, BufferType::vertex, sizeof(data1a), true);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BufferType::vertex, buffer1.type());
      EXPECT_TRUE(buffer1.allocation() == VK_NULL_HANDLE);
      auto buffer2 = Buffer<ResourceUsage::staticGpu>::createUnallocatedBuffer(renderer, BufferType::vertexIndex, sizeof(data2a), true);
      EXPECT_EQ(sizeof(data2a), buffer2.size());
      EXPECT_EQ(BufferType::vertexIndex, buffer2.type());
      EXPECT_TRUE(buffer2.allocation() == VK_NULL_HANDLE);
      auto buffer3 = Buffer<ResourceUsage::staticGpu>::createUnallocatedBuffer(renderer, BufferType::uniform, sizeof(data1b), true);
      EXPECT_EQ(sizeof(data1b), buffer3.size());
      EXPECT_EQ(BufferType::uniform, buffer3.type());
      EXPECT_TRUE(buffer3.allocation() == VK_NULL_HANDLE);
      auto buffer4 = Buffer<ResourceUsage::staticGpu>::createUnallocatedBuffer(renderer, BufferType::uniform, sizeof(data2b), false);
      EXPECT_EQ(sizeof(data2b), buffer4.size());
      EXPECT_EQ(BufferType::uniform, buffer4.type());
      EXPECT_TRUE(buffer4.allocation() == VK_NULL_HANDLE);

      BufferHandle bufferTypes[] = { buffer1.handle(), buffer2.handle(), buffer3.handle(), buffer4.handle() };
      BufferMemory bufferMemory = BufferMemory::create(renderer, ResourceUsage::staticGpu, vertexSize + indexSize + uniform1Size + uniform2Size,
                                                       bufferTypes, sizeof(bufferTypes)/sizeof(*bufferTypes));
      ASSERT_TRUE(bufferMemory.handle() != VK_NULL_HANDLE);

      bufferMemory.bind(buffer1, 0);
      bufferMemory.bind(buffer2, vertexSize);
      bufferMemory.bind(buffer3, vertexSize + indexSize);
      bufferMemory.bind(buffer4, vertexSize + indexSize + uniform1Size);
      EXPECT_TRUE(buffer1.allocation() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffer2.allocation() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffer3.allocation() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffer4.allocation() != VK_NULL_HANDLE);

      EXPECT_TRUE(buffer1.write(data1a));
      EXPECT_TRUE(buffer2.write(&data2a));
      EXPECT_TRUE(buffer3.write(data1b));
      EXPECT_TRUE(buffer4.write(&data2b));

      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::vertex, sizeof(data1a), true);
      Buffer<ResourceUsage::staging> staging2(renderer, BufferType::vertexIndex, sizeof(data2a), true);
      Buffer<ResourceUsage::staging> staging3(renderer, BufferType::uniform, sizeof(data1b), true);
      // no staging4: buffer4 not bidirectional -> can't be copied
      EXPECT_TRUE(staging1.copy(buffer1));
      EXPECT_TRUE(staging2.copy(buffer2));
      EXPECT_TRUE(staging3.copy(buffer3));

      reader.open(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1a[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1a[1], ((const uint64_t*)reader.data())[1]);
      EXPECT_EQ(data1a[2], ((const uint64_t*)reader.data())[2]);
      EXPECT_EQ(data1a[3], ((const uint64_t*)reader.data())[3]);
      reader.close();

      reader.open(staging2, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2a.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2a.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2a.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2a.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();

      reader.open(staging3, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1b[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1b[1], ((const uint64_t*)reader.data())[1]);
      EXPECT_EQ(data1b[2], ((const uint64_t*)reader.data())[2]);
      EXPECT_EQ(data1b[3], ((const uint64_t*)reader.data())[3]);
      reader.close();

      buffer1.release(); // release buffers before memory
      buffer2.release();
      buffer3.release();
      buffer4.release();
      bufferMemory.release();
    }{
      // vertex + index + uniform + uniform
      constexpr const size_t vertexSize = __alignSuballocSize(sizeof(data1a));
      constexpr const size_t indexSize = __alignSuballocSize(sizeof(data2a));
      constexpr const size_t uniform1Size = __alignSuballocSize(sizeof(data1b));
      constexpr const size_t uniform2Size = __alignSuballocSize(sizeof(data2b));

      auto buffer1 = Buffer<ResourceUsage::dynamicCpu>::createUnallocatedBuffer(renderer, BufferType::vertex, sizeof(data1a), true);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BufferType::vertex, buffer1.type());
      EXPECT_TRUE(buffer1.allocation() == VK_NULL_HANDLE);
      auto buffer2 = Buffer<ResourceUsage::dynamicCpu>::createUnallocatedBuffer(renderer, BufferType::vertexIndex, sizeof(data2a), true);
      EXPECT_EQ(sizeof(data2a), buffer2.size());
      EXPECT_EQ(BufferType::vertexIndex, buffer2.type());
      EXPECT_TRUE(buffer2.allocation() == VK_NULL_HANDLE);
      auto buffer3 = Buffer<ResourceUsage::dynamicCpu>::createUnallocatedBuffer(renderer, BufferType::uniform, sizeof(data1b), true);
      EXPECT_EQ(sizeof(data1b), buffer3.size());
      EXPECT_EQ(BufferType::uniform, buffer3.type());
      EXPECT_TRUE(buffer3.allocation() == VK_NULL_HANDLE);
      auto buffer4 = Buffer<ResourceUsage::dynamicCpu>::createUnallocatedBuffer(renderer, BufferType::uniform, sizeof(data2b), false);
      EXPECT_EQ(sizeof(data2b), buffer4.size());
      EXPECT_EQ(BufferType::uniform, buffer4.type());
      EXPECT_TRUE(buffer4.allocation() == VK_NULL_HANDLE);

      BufferHandle bufferTypes[] = { buffer1.handle(), buffer2.handle(), buffer3.handle(), buffer4.handle() };
      BufferMemory bufferMemory = BufferMemory::create(renderer, ResourceUsage::dynamicCpu, vertexSize + indexSize + uniform1Size + uniform2Size,
        bufferTypes, sizeof(bufferTypes)/sizeof(*bufferTypes));
      ASSERT_TRUE(bufferMemory.handle() != VK_NULL_HANDLE);

      bufferMemory.bind(buffer1, 0);
      bufferMemory.bind(buffer2, vertexSize);
      bufferMemory.bind(buffer3, vertexSize + indexSize);
      bufferMemory.bind(buffer4, vertexSize + indexSize + uniform1Size);
      EXPECT_TRUE(buffer1.allocation() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffer2.allocation() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffer3.allocation() != VK_NULL_HANDLE);
      EXPECT_TRUE(buffer4.allocation() != VK_NULL_HANDLE);

      EXPECT_TRUE(buffer1.write(data1a));
      EXPECT_TRUE(buffer2.write(&data2a));
      EXPECT_TRUE(buffer3.write(data1b));
      EXPECT_TRUE(buffer4.write(&data2b));

      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::vertex, sizeof(data1a), true);
      Buffer<ResourceUsage::staging> staging2(renderer, BufferType::vertexIndex, sizeof(data2a), true);
      Buffer<ResourceUsage::staging> staging3(renderer, BufferType::uniform, sizeof(data1b), true);
      // no staging4: buffer4 not bidirectional -> can't be copied
      EXPECT_TRUE(staging1.copy(buffer1));
      EXPECT_TRUE(staging2.copy(buffer2));
      EXPECT_TRUE(staging3.copy(buffer3));

      reader.open(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1a[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1a[1], ((const uint64_t*)reader.data())[1]);
      EXPECT_EQ(data1a[2], ((const uint64_t*)reader.data())[2]);
      EXPECT_EQ(data1a[3], ((const uint64_t*)reader.data())[3]);
      reader.close();

      reader.open(staging2, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2a.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2a.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2a.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2a.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();

      reader.open(staging3, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data1b[0], ((const uint64_t*)reader.data())[0]);
      EXPECT_EQ(data1b[1], ((const uint64_t*)reader.data())[1]);
      EXPECT_EQ(data1b[2], ((const uint64_t*)reader.data())[2]);
      EXPECT_EQ(data1b[3], ((const uint64_t*)reader.data())[3]);
      reader.close();

      buffer1.release(); // release buffers before memory
      buffer2.release();
      buffer3.release();
      buffer4.release();
      bufferMemory.release();
    }
  }

#endif
