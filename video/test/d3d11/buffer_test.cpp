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
# include <system/align.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/buffer.h>

  using namespace pandora::video::d3d11;

  class D3d11BufferTest : public testing::Test {
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


  // -- constant and vertex buffer creation/binding --

  TEST_F(D3d11BufferTest, constantBufferCreationTest) {
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
      renderer.bindVertexUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindTessCtrlUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindTessEvalUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindGeometryUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindFragmentUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindComputeUniforms(0, buffer1.handlePtr(), size_t{1u});
      EXPECT_TRUE(buffer1.write((const void*)data1b));
      renderer.clearVertexUniforms();
      renderer.clearTessCtrlUniforms();
      renderer.clearTessEvalUniforms();
      renderer.clearGeometryUniforms();
      renderer.clearFragmentUniforms();
      renderer.clearComputeUniforms();
      
      Buffer<ResourceUsage::staticGpu> buffer2(renderer, BufferType::uniform, sizeof(data1a), (const void*)data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BufferType::uniform, buffer2.type());
      renderer.bindVertexUniforms(1, buffer2.handlePtr(), size_t{1u});
      renderer.bindFragmentUniforms(1, buffer2.handlePtr(), size_t{1u});
      EXPECT_TRUE(buffer2.write((const void*)data1b));
      renderer.clearVertexUniforms();
      renderer.clearFragmentUniforms();
      
      Buffer<ResourceUsage::immutable> buffer3(renderer, BufferType::uniform, sizeof(data1b), (const void*)data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexUniforms(2, buffer3.handlePtr(), size_t{1u});
      renderer.clearVertexUniforms();
      
      Buffer<ResourceUsage::staticGpu> buffer4(renderer, BufferType::uniform, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_TRUE(buffer4.write((const void*)&data2a));
      renderer.bindVertexUniforms(0, buffer4.handlePtr(), size_t{1u});
      EXPECT_TRUE(buffer4.write((const void*)&data2b));
      renderer.clearVertexUniforms();
      
      Buffer<ResourceUsage::staticGpu> buffer5(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindFragmentUniforms(0, buffer5.handlePtr(), size_t{1u});
      renderer.clearFragmentUniforms();
      EXPECT_TRUE(buffer5.write((const void*)&data2b));
      
      Buffer<ResourceUsage::immutable> buffer6(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexUniforms(0, buffer6.handlePtr(), size_t{1u});
      renderer.clearVertexUniforms();
      EXPECT_FALSE(buffer6.write((const void*)&data2b));

      BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      renderer.bindFragmentUniforms(1, &buffers[0], size_t{2u});
      renderer.clearFragmentUniforms();
      
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
      renderer.bindVertexUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindTessCtrlUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindTessEvalUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindGeometryUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindFragmentUniforms(0, buffer1.handlePtr(), size_t{1u});
      renderer.bindComputeUniforms(0, buffer1.handlePtr(), size_t{1u});
      buffer1.write((const void*)data1b);
      renderer.clearVertexUniforms();
      renderer.clearTessCtrlUniforms();
      renderer.clearTessEvalUniforms();
      renderer.clearGeometryUniforms();
      renderer.clearFragmentUniforms();
      renderer.clearComputeUniforms();
      
      Buffer<ResourceUsage::dynamicCpu> buffer2(renderer, BufferType::uniform, sizeof(data1a), (const void*)data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BufferType::uniform, buffer2.type());
      renderer.bindVertexUniforms(1, buffer2.handlePtr(), size_t{1u});
      EXPECT_TRUE(buffer2.write((const void*)data1b));
      renderer.clearVertexUniforms();
      
      Buffer<ResourceUsage::dynamicCpu> buffer3(renderer, BufferType::uniform, sizeof(data1b), (const void*)data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindFragmentUniforms(2, buffer3.handlePtr(), size_t{1u});
      renderer.clearFragmentUniforms();
      
      Buffer<ResourceUsage::dynamicCpu> buffer4(renderer, BufferType::uniform, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_TRUE(buffer4.write((const void*)&data2a));
      renderer.bindVertexUniforms(0, buffer4.handlePtr(), size_t{1u});
      EXPECT_TRUE(buffer4.write((const void*)&data2b));
      renderer.clearVertexUniforms();
      
      Buffer<ResourceUsage::dynamicCpu> buffer5(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexUniforms(0, buffer5.handlePtr(), size_t{1u});
      renderer.bindFragmentUniforms(0, buffer5.handlePtr(), size_t{1u});
      renderer.clearVertexUniforms();
      renderer.clearFragmentUniforms();
      EXPECT_TRUE(buffer5.write((const void*)&data2b));
      
      Buffer<ResourceUsage::dynamicCpu> buffer6(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexUniforms(0, buffer6.handlePtr(), size_t{1u});
      renderer.clearVertexUniforms();

      BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      renderer.bindFragmentUniforms(1, &buffers[0], size_t{2u});
      renderer.clearFragmentUniforms();
      
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

  TEST_F(D3d11BufferTest, vertexBufferCreationTest) {
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
      renderer.bindVertexArrayBuffer(0, buffer1.handle(), (unsigned int)buffer1.size(), 0);
      renderer.bindVertexArrayBuffer(1, buffer1.handle(), (unsigned int)buffer1.size()/2u, (unsigned int)buffer1.size()/2u);
      unsigned int bufferSize = (unsigned int)buffer1.size(), offset = 0;
      renderer.bindVertexArrayBuffers(2, size_t{ 1u }, buffer1.handlePtr(), &bufferSize, &offset);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      Buffer<ResourceUsage::staticGpu> buffer2(renderer, BufferType::vertexIndex, sizeof(data1a), (const void*)data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BufferType::vertexIndex, buffer2.type());
      renderer.bindVertexIndexBuffer(buffer2.handle(), VertexIndexFormat::r32_ui, 0);
      EXPECT_TRUE(buffer2.write((const void*)data1b));
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui, 0);

      Buffer<ResourceUsage::immutable> buffer3(renderer, BufferType::vertex, sizeof(data1b), (const void*)data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexArrayBuffer(2, buffer3.handle(), (unsigned int)buffer3.size(), 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      Buffer<ResourceUsage::staticGpu> buffer4(renderer, BufferType::vertex, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_TRUE(buffer4.write((const void*)&data2a));
      renderer.bindVertexArrayBuffer(0, buffer4.handle(), (unsigned int)buffer4.size(), 0);
      EXPECT_TRUE(buffer4.write((const void*)&data2b));
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      Buffer<ResourceUsage::staticGpu> buffer5(renderer, BufferType::vertexIndex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexIndexBuffer(buffer5.handle(), VertexIndexFormat::r16_ui, 0);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r16_ui, 0);
      EXPECT_TRUE(buffer5.write((const void*)&data2b));

      Buffer<ResourceUsage::immutable> buffer6(renderer, BufferType::vertex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexArrayBuffer(0, buffer6.handle(), (unsigned int)buffer6.size(), 0);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      BufferHandle buffers[] = { buffer3.handle(), buffer6.handle() };
      unsigned int sizes[] = { (unsigned int)buffer3.size(), (unsigned int)buffer6.size() };
      unsigned int offsets[] = { 0,0 };
      renderer.bindVertexArrayBuffers(1, size_t{2u}, &buffers[0], &sizes[0], &offsets[0]);
      renderer.bindVertexIndexBuffer(buffer2.handle(), VertexIndexFormat::r32_ui, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui, 0);

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
      renderer.bindVertexArrayBuffer(0, buffer1.handle(), (unsigned int)buffer1.size(), 0);
      renderer.bindVertexArrayBuffer(1, buffer1.handle(), (unsigned int)buffer1.size()/2u, (unsigned int)buffer1.size()/2u);
      unsigned int bufferSize = (unsigned int)buffer1.size(), offset = 0;
      renderer.bindVertexArrayBuffers(2, size_t{ 1u }, buffer1.handlePtr(), &bufferSize, &offset);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      Buffer<ResourceUsage::dynamicCpu> buffer2(renderer, BufferType::vertexIndex, sizeof(data1a), (const void*)data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BufferType::vertexIndex, buffer2.type());
      renderer.bindVertexIndexBuffer(buffer2.handle(), VertexIndexFormat::r32_ui, 0);
      EXPECT_TRUE(buffer2.write((const void*)data1b));
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui, 0);

      Buffer<ResourceUsage::dynamicCpu> buffer3(renderer, BufferType::vertex, sizeof(data1b), (const void*)data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexArrayBuffer(2, buffer3.handle(), (unsigned int)buffer3.size(), 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      Buffer<ResourceUsage::dynamicCpu> buffer4(renderer, BufferType::vertex, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      EXPECT_TRUE(buffer4.write((const void*)&data2a));
      renderer.bindVertexArrayBuffer(0, buffer4.handle(), (unsigned int)buffer4.size(), 0);
      EXPECT_TRUE(buffer4.write((const void*)&data2b));
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      Buffer<ResourceUsage::dynamicCpu> buffer5(renderer, BufferType::vertexIndex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexIndexBuffer(buffer5.handle(), VertexIndexFormat::r16_ui, 0);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r16_ui, 0);
      EXPECT_TRUE(buffer5.write((const void*)&data2b));

      Buffer<ResourceUsage::dynamicCpu> buffer6(renderer, BufferType::vertex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexArrayBuffer(0, buffer6.handle(), (unsigned int)buffer6.size(), 0);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      BufferHandle buffers[] = { buffer3.handle(), buffer6.handle() };
      unsigned int sizes[] = { (unsigned int)buffer3.size(), (unsigned int)buffer6.size() };
      unsigned int offsets[] = { 0,0 };
      renderer.bindVertexArrayBuffers(1, size_t{2u}, &buffers[0], &sizes[0], &offsets[0]);
      renderer.bindVertexIndexBuffer(buffer2.handle(), VertexIndexFormat::r32_ui, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui, 0);

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

  TEST_F(D3d11BufferTest, multiTypeBufferCreationTest) {
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
      Buffer<ResourceUsage::staticGpu> buffer1(renderer, BufferType::vertex | BufferType::vertexIndex, vertexSize + indexSize);
      EXPECT_EQ(vertexSize + indexSize, buffer1.size());
      EXPECT_EQ((BufferType::vertex | BufferType::vertexIndex), buffer1.type());

      uint8_t vertexIndexData1[vertexSize + indexSize]{};
      memcpy(vertexIndexData1, data1a, sizeof(data1a));
      memcpy(vertexIndexData1 + vertexSize, &data2a, sizeof(data2a));
      EXPECT_TRUE(buffer1.write((const void*)vertexIndexData1));

      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::vertex | BufferType::vertexIndex, vertexSize + indexSize);
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

      renderer.bindVertexArrayBuffer(0, buffer1.handle(), (unsigned int)sizeof(data1a), 0);
      renderer.bindVertexIndexBuffer(buffer1.handle(), VertexIndexFormat::r32_ui, vertexSize);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui, 0);

      // uniform + uniform
#     if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
        if (renderer.featureLevel() >= D3D_FEATURE_LEVEL_11_1) {
          constexpr const size_t uniform1Size = __alignUniformSize(sizeof(data1a));
          constexpr const size_t uniform2Size = __alignUniformSize(sizeof(data2b));
          Buffer<ResourceUsage::staticGpu> buffer2(renderer, BufferType::uniform, uniform1Size + uniform2Size);
          EXPECT_EQ(uniform1Size + uniform2Size, buffer2.size());
          EXPECT_EQ((BufferType::uniform), buffer2.type());

          uint8_t uniformData2[uniform1Size + uniform2Size]{};
          memcpy(uniformData2, data1a, sizeof(data1a));
          memcpy(uniformData2 + uniform1Size, &data2b, sizeof(data2b));
          EXPECT_TRUE(buffer2.write((const void*)uniformData2));

          Buffer<ResourceUsage::staging> staging2(renderer, BufferType::uniform, uniform1Size + uniform2Size);
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
          renderer.bindVertexUniforms(0, uniforms, 2, byte16Offsets, byte16Sizes);
          renderer.bindVertexUniforms(0, nullptr, 0);
          renderer.bindVertexUniforms(1, nullptr, 0);
        }
#     endif
    }{
      // vertex + index
      constexpr const size_t vertexSize = __alignMemorySize(sizeof(data1a));
      constexpr const size_t indexSize = __alignMemorySize(sizeof(data2a));
      Buffer<ResourceUsage::dynamicCpu> buffer1(renderer, BufferType::vertex | BufferType::vertexIndex, vertexSize + indexSize);
      EXPECT_EQ(vertexSize + indexSize, buffer1.size());
      EXPECT_EQ((BufferType::vertex | BufferType::vertexIndex), buffer1.type());

      uint8_t vertexIndexData1[vertexSize + indexSize]{};
      memcpy(vertexIndexData1, data1a, sizeof(data1a));
      memcpy(vertexIndexData1 + vertexSize, &data2a, sizeof(data2a));
      EXPECT_TRUE(buffer1.write((const void*)vertexIndexData1));

      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::vertex | BufferType::vertexIndex, vertexSize + indexSize);
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

      renderer.bindVertexArrayBuffer(0, buffer1.handle(), (unsigned int)sizeof(data1a), 0);
      renderer.bindVertexIndexBuffer(buffer1.handle(), VertexIndexFormat::r32_ui, vertexSize);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui, 0);

      // uniform + uniform
#     if !defined(_VIDEO_D3D11_VERSION) || _VIDEO_D3D11_VERSION != 110
        if (renderer.featureLevel() >= D3D_FEATURE_LEVEL_11_1) {
          constexpr const size_t uniform1Size = __alignUniformSize(sizeof(data1a));
          constexpr const size_t uniform2Size = __alignUniformSize(sizeof(data2b));
          Buffer<ResourceUsage::dynamicCpu> buffer2(renderer, BufferType::uniform, uniform1Size + uniform2Size);
          EXPECT_EQ(uniform1Size + uniform2Size, buffer2.size());
          EXPECT_EQ((BufferType::uniform), buffer2.type());

          uint8_t uniformData2[uniform1Size + uniform2Size]{};
          memcpy(uniformData2, data1a, sizeof(data1a));
          memcpy(uniformData2 + uniform1Size, &data2b, sizeof(data2b));
          EXPECT_TRUE(buffer2.write((const void*)uniformData2));

          Buffer<ResourceUsage::staging> staging2(renderer, BufferType::uniform, uniform1Size + uniform2Size);
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
          renderer.bindVertexUniforms(0, uniforms, 2, byte16Offsets, byte16Sizes);
          renderer.bindVertexUniforms(0, nullptr, 0);
          renderer.bindVertexUniforms(1, nullptr, 0);
        }
#     endif
    }
  }

  TEST_F(D3d11BufferTest, bufferCopyTest) {
    uint64_t data1a[2] = { 42, 8 };
    uint64_t data1b[2] = { 10, 22 };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      MappedBufferIO reader;

      Buffer<ResourceUsage::immutable> bufferI1(renderer, BufferType::uniform, sizeof(data1b), (const void*)data1b);
      Buffer<ResourceUsage::staticGpu> bufferS1(renderer, BufferType::uniform, sizeof(data1a));
      Buffer<ResourceUsage::dynamicCpu> bufferD1(renderer, BufferType::uniform, sizeof(data1a));
      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::uniform, sizeof(data1a));

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

  TEST_F(D3d11BufferTest, mappedBufferReadWriteTest) {
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      Buffer<ResourceUsage::dynamicCpu> bufferD1(renderer, BufferType::uniform, sizeof(data2a));
      Buffer<ResourceUsage::staging> staging1(renderer, BufferType::uniform, sizeof(data2a), (const void*)&data2b);

      MappedBufferIO reader(staging1, StagedMapping::read);
      ASSERT_TRUE(reader.isOpen());
      EXPECT_EQ(data2b.rgba[0], ((const _TestColor*)reader.data())->rgba[0]);
      EXPECT_EQ(data2b.rgba[1], ((const _TestColor*)reader.data())->rgba[1]);
      EXPECT_EQ(data2b.rgba[2], ((const _TestColor*)reader.data())->rgba[2]);
      EXPECT_EQ(data2b.rgba[3], ((const _TestColor*)reader.data())->rgba[3]);
      reader.close();
      EXPECT_FALSE(reader.isOpen());

      MappedBufferIO writer(bufferD1, DynamicMapping::discard);
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

      writer.open(bufferD1, DynamicMapping::subsequent);
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

#endif
