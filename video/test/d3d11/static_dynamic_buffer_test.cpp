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
# include <video/d3d11/static_buffer.h>
# include <video/d3d11/dynamic_buffer.h>

  using namespace pandora::video::d3d11;

  class D3d11StaticDynamicBufferTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };
  
  __align_type(16, struct _TestColor final {
    float rgba[4];
  });


  // -- constant buffer creation/binding --

  TEST_F(D3d11StaticDynamicBufferTest, constantBufferCreationTest) {
    uint64_t data1a[2] = { 42, 0 };
    uint64_t data1b[2] = { 0, 22 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };
    
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      StaticBuffer emptyBuffer;
      EXPECT_TRUE(emptyBuffer.isEmpty());
      EXPECT_TRUE(emptyBuffer.handle() == nullptr);
      
      EXPECT_ANY_THROW(StaticBuffer(renderer, BaseBufferType::uniform, 0));
      EXPECT_ANY_THROW(StaticBuffer(renderer, BaseBufferType::uniform, 0, nullptr, false));
      EXPECT_ANY_THROW(StaticBuffer(renderer, BaseBufferType::uniform, size_t{1u}, nullptr, true));

      StaticBuffer buffer1(renderer, BaseBufferType::uniform, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BaseBufferType::uniform, buffer1.type());
      buffer1.write(renderer, (const void*)&data1a);
      renderer.bindVertexUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindTessCtrlUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindTessEvalUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindGeometryUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindFragmentUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindComputeUniforms(0, buffer1.handleArray(), size_t{1u});
      buffer1.write(renderer, (const void*)&data1b);
      renderer.clearVertexUniforms();
      renderer.clearTessCtrlUniforms();
      renderer.clearTessEvalUniforms();
      renderer.clearGeometryUniforms();
      renderer.clearFragmentUniforms();
      renderer.clearComputeUniforms();
      
      StaticBuffer buffer2(renderer, BaseBufferType::uniform, sizeof(data1a), (const void*)&data1a, false);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BaseBufferType::uniform, buffer2.type());
      renderer.bindVertexUniforms(1, buffer2.handleArray(), size_t{1u});
      renderer.bindFragmentUniforms(1, buffer2.handleArray(), size_t{1u});
      buffer2.write(renderer, (const void*)&data1b);
      renderer.clearVertexUniforms();
      renderer.clearFragmentUniforms();
      
      StaticBuffer buffer3(renderer, BaseBufferType::uniform, sizeof(data1b), (const void*)&data1b, true);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexUniforms(2, buffer3.handleArray(), size_t{1u});
      renderer.clearVertexUniforms();
      
      StaticBuffer buffer4(renderer, BaseBufferType::uniform, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      buffer4.write(renderer, (const void*)&data2a);
      renderer.bindVertexUniforms(0, buffer4.handleArray(), size_t{1u});
      buffer4.write(renderer, (const void*)&data2b);
      renderer.clearVertexUniforms();
      
      StaticBuffer buffer5(renderer, BaseBufferType::uniform, sizeof(data2a), (const void*)&data2a, false);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindFragmentUniforms(0, buffer5.handleArray(), size_t{1u});
      renderer.clearFragmentUniforms();
      buffer5.write(renderer, (const void*)&data2b);
      
      StaticBuffer buffer6(renderer, BaseBufferType::uniform, sizeof(data2a), (const void*)&data2a, true);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexUniforms(0, buffer6.handleArray(), size_t{1u});
      renderer.clearVertexUniforms();

      BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      renderer.bindFragmentUniforms(1, &buffers[0], size_t{2u});
      renderer.clearFragmentUniforms();
      
      auto buffer1Handle = buffer1.handle();
      StaticBuffer moved(std::move(buffer1));
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
      DynamicBuffer emptyBuffer;
      EXPECT_TRUE(emptyBuffer.isEmpty());
      EXPECT_TRUE(emptyBuffer.handle() == nullptr);
      
      EXPECT_ANY_THROW(DynamicBuffer(renderer, BaseBufferType::uniform, 0));
      EXPECT_ANY_THROW(DynamicBuffer(renderer, BaseBufferType::uniform, 0, nullptr));

      DynamicBuffer buffer1(renderer, BaseBufferType::uniform, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BaseBufferType::uniform, buffer1.type());
      buffer1.writeDiscard(renderer, (const void*)&data1a);
      renderer.bindVertexUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindTessCtrlUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindTessEvalUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindGeometryUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindFragmentUniforms(0, buffer1.handleArray(), size_t{1u});
      renderer.bindComputeUniforms(0, buffer1.handleArray(), size_t{1u});
      buffer1.write(renderer, (const void*)&data1b);
      renderer.clearVertexUniforms();
      renderer.clearTessCtrlUniforms();
      renderer.clearTessEvalUniforms();
      renderer.clearGeometryUniforms();
      renderer.clearFragmentUniforms();
      renderer.clearComputeUniforms();
      
      DynamicBuffer buffer2(renderer, BaseBufferType::uniform, sizeof(data1a), (const void*)&data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BaseBufferType::uniform, buffer2.type());
      renderer.bindVertexUniforms(1, buffer2.handleArray(), size_t{1u});
      buffer2.write(renderer, (const void*)&data1b);
      renderer.clearVertexUniforms();
      
      DynamicBuffer buffer3(renderer, BaseBufferType::uniform, sizeof(data1b), (const void*)&data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindFragmentUniforms(2, buffer3.handleArray(), size_t{1u});
      renderer.clearFragmentUniforms();
      
      DynamicBuffer buffer4(renderer, BaseBufferType::uniform, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      buffer4.writeDiscard(renderer, (const void*)&data2a);
      renderer.bindVertexUniforms(0, buffer4.handleArray(), size_t{1u});
      buffer4.write(renderer, (const void*)&data2b);
      renderer.clearVertexUniforms();
      
      DynamicBuffer buffer5(renderer, BaseBufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexUniforms(0, buffer5.handleArray(), size_t{1u});
      renderer.bindFragmentUniforms(0, buffer5.handleArray(), size_t{1u});
      renderer.clearVertexUniforms();
      renderer.clearFragmentUniforms();
      buffer5.write(renderer, (const void*)&data2b);
      
      DynamicBuffer buffer6(renderer, BaseBufferType::uniform, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexUniforms(0, buffer6.handleArray(), size_t{1u});
      renderer.clearVertexUniforms();

      BufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      renderer.bindFragmentUniforms(1, &buffers[0], size_t{2u});
      renderer.clearFragmentUniforms();
      
      auto buffer1Handle = buffer1.handle();
      DynamicBuffer moved(std::move(buffer1));
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

  TEST_F(D3d11StaticDynamicBufferTest, vertexBufferCreationTest) {
    uint64_t data1a[4] = { 42, 0, 42, 0 };
    uint64_t data1b[2] = { 0, 22 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };

    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    {
      StaticBuffer emptyBuffer;
      EXPECT_TRUE(emptyBuffer.isEmpty());
      EXPECT_TRUE(emptyBuffer.handle() == nullptr);

      EXPECT_ANY_THROW(StaticBuffer(renderer, BaseBufferType::vertex, 0));
      EXPECT_ANY_THROW(StaticBuffer(renderer, BaseBufferType::vertexIndex, 0, nullptr, false));
      EXPECT_ANY_THROW(StaticBuffer(renderer, BaseBufferType::vertex, size_t{1u}, nullptr, true));

      StaticBuffer buffer1(renderer, BaseBufferType::vertex, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BaseBufferType::vertex, buffer1.type());
      buffer1.write(renderer, (const void*)&data1a);
      renderer.bindVertexArrayBuffer(0, buffer1.handle(), (unsigned int)buffer1.size(), 0);
      renderer.bindVertexArrayBuffer(1, buffer1.handle(), (unsigned int)buffer1.size()/2u, (unsigned int)buffer1.size()/2u);
      unsigned int bufferSize = (unsigned int)buffer1.size(), offset = 0;
      renderer.bindVertexArrayBuffers(2, size_t{ 1u }, buffer1.handleArray(), &bufferSize, &offset);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      StaticBuffer buffer2(renderer, BaseBufferType::vertexIndex, sizeof(data1a), (const void*)&data1a, false);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BaseBufferType::vertexIndex, buffer2.type());
      renderer.bindVertexIndexBuffer(buffer2.handle(), VertexIndexFormat::r32_ui, 0);
      buffer2.write(renderer, (const void*)&data1b);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui, 0);

      StaticBuffer buffer3(renderer, BaseBufferType::vertex, sizeof(data1b), (const void*)&data1b, true);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexArrayBuffer(2, buffer3.handle(), (unsigned int)buffer3.size(), 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      StaticBuffer buffer4(renderer, BaseBufferType::vertex, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      buffer4.write(renderer, (const void*)&data2a);
      renderer.bindVertexArrayBuffer(0, buffer4.handle(), (unsigned int)buffer4.size(), 0);
      buffer4.write(renderer, (const void*)&data2b);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      StaticBuffer buffer5(renderer, BaseBufferType::vertexIndex, sizeof(data2a), (const void*)&data2a, false);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexIndexBuffer(buffer5.handle(), VertexIndexFormat::r16_ui, 0);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r16_ui, 0);
      buffer5.write(renderer, (const void*)&data2b);

      StaticBuffer buffer6(renderer, BaseBufferType::vertex, sizeof(data2a), (const void*)&data2a, true);
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
      StaticBuffer moved(std::move(buffer1));
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
      DynamicBuffer emptyBuffer;
      EXPECT_TRUE(emptyBuffer.isEmpty());
      EXPECT_TRUE(emptyBuffer.handle() == nullptr);

      EXPECT_ANY_THROW(DynamicBuffer(renderer, BaseBufferType::vertex, 0));
      EXPECT_ANY_THROW(DynamicBuffer(renderer, BaseBufferType::vertexIndex, 0, nullptr));

      DynamicBuffer buffer1(renderer, BaseBufferType::vertex, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(BaseBufferType::vertex, buffer1.type());
      buffer1.writeDiscard(renderer, (const void*)&data1a);
      renderer.bindVertexArrayBuffer(0, buffer1.handle(), (unsigned int)buffer1.size(), 0);
      renderer.bindVertexArrayBuffer(1, buffer1.handle(), (unsigned int)buffer1.size()/2u, (unsigned int)buffer1.size()/2u);
      unsigned int bufferSize = (unsigned int)buffer1.size(), offset = 0;
      renderer.bindVertexArrayBuffers(2, size_t{ 1u }, buffer1.handleArray(), &bufferSize, &offset);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      DynamicBuffer buffer2(renderer, BaseBufferType::vertexIndex, sizeof(data1a), (const void*)&data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(BaseBufferType::vertexIndex, buffer2.type());
      renderer.bindVertexIndexBuffer(buffer2.handle(), VertexIndexFormat::r32_ui, 0);
      buffer2.writeDiscard(renderer, (const void*)&data1b);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r32_ui, 0);

      DynamicBuffer buffer3(renderer, BaseBufferType::vertex, sizeof(data1b), (const void*)&data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexArrayBuffer(2, buffer3.handle(), (unsigned int)buffer3.size(), 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      DynamicBuffer buffer4(renderer, BaseBufferType::vertex, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      buffer4.writeDiscard(renderer, (const void*)&data2a);
      renderer.bindVertexArrayBuffer(0, buffer4.handle(), (unsigned int)buffer4.size(), 0);
      buffer4.write(renderer, (const void*)&data2b);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      DynamicBuffer buffer5(renderer, BaseBufferType::vertexIndex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexIndexBuffer(buffer5.handle(), VertexIndexFormat::r16_ui, 0);
      renderer.bindVertexIndexBuffer(nullptr, VertexIndexFormat::r16_ui, 0);
      buffer5.write(renderer, (const void*)&data2b);

      DynamicBuffer buffer6(renderer, BaseBufferType::vertex, sizeof(data2a), (const void*)&data2a);
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
      DynamicBuffer moved(std::move(buffer1));
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

#endif
