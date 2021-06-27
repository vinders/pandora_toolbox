#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <system/align.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/static_buffer.h>
# include <video/d3d11/dynamic_buffer.h>

  using namespace pandora::video::d3d11;

  class StaticDynamicBufferTest : public testing::Test {
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

  TEST_F(StaticDynamicBufferTest, constantBufferCreationTest) {
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
      
      EXPECT_ANY_THROW(StaticBuffer(renderer, pandora::video::DataBufferType::constant, 0));
      EXPECT_ANY_THROW(StaticBuffer(renderer, pandora::video::DataBufferType::constant, 0, nullptr, false));
      EXPECT_ANY_THROW(StaticBuffer(renderer, pandora::video::DataBufferType::constant, size_t{1u}, nullptr, true));

      StaticBuffer buffer1(renderer, pandora::video::DataBufferType::constant, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(pandora::video::DataBufferType::constant, buffer1.type());
      buffer1.write(renderer, (const void*)&data1a);
      renderer.bindVertexConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindTesselControlConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindTesselEvalConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindGeometryConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindFragmentConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindComputeConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      buffer1.write(renderer, (const void*)&data1b);
      renderer.clearVertexConstantBuffers();
      renderer.clearTesselControlConstantBuffers();
      renderer.clearTesselEvalConstantBuffers();
      renderer.clearGeometryConstantBuffers();
      renderer.clearFragmentConstantBuffers();
      renderer.clearComputeConstantBuffers();
      
      StaticBuffer buffer2(renderer, pandora::video::DataBufferType::constant, sizeof(data1a), (const void*)&data1a, false);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(pandora::video::DataBufferType::constant, buffer2.type());
      renderer.bindVertexConstantBuffers(1, buffer2.handleArray(), size_t{1u});
      renderer.bindFragmentConstantBuffers(1, buffer2.handleArray(), size_t{1u});
      buffer2.write(renderer, (const void*)&data1b);
      renderer.clearVertexConstantBuffers();
      renderer.clearFragmentConstantBuffers();
      
      StaticBuffer buffer3(renderer, pandora::video::DataBufferType::constant, sizeof(data1b), (const void*)&data1b, true);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexConstantBuffers(2, buffer3.handleArray(), size_t{1u});
      renderer.clearVertexConstantBuffers();
      
      StaticBuffer buffer4(renderer, pandora::video::DataBufferType::constant, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      buffer4.write(renderer, (const void*)&data2a);
      renderer.bindVertexConstantBuffers(0, buffer4.handleArray(), size_t{1u});
      buffer4.write(renderer, (const void*)&data2b);
      renderer.clearVertexConstantBuffers();
      
      StaticBuffer buffer5(renderer, pandora::video::DataBufferType::constant, sizeof(data2a), (const void*)&data2a, false);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindFragmentConstantBuffers(0, buffer5.handleArray(), size_t{1u});
      renderer.clearFragmentConstantBuffers();
      buffer5.write(renderer, (const void*)&data2b);
      
      StaticBuffer buffer6(renderer, pandora::video::DataBufferType::constant, sizeof(data2a), (const void*)&data2a, true);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexConstantBuffers(0, buffer6.handleArray(), size_t{1u});
      renderer.clearVertexConstantBuffers();

      Renderer::DataBufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      renderer.bindFragmentConstantBuffers(1, &buffers[0], size_t{2u});
      renderer.clearFragmentConstantBuffers();
      
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
      
      EXPECT_ANY_THROW(DynamicBuffer(renderer, pandora::video::DataBufferType::constant, 0));
      EXPECT_ANY_THROW(DynamicBuffer(renderer, pandora::video::DataBufferType::constant, 0, nullptr));

      DynamicBuffer buffer1(renderer, pandora::video::DataBufferType::constant, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(pandora::video::DataBufferType::constant, buffer1.type());
      buffer1.writeDiscard(renderer, (const void*)&data1a);
      renderer.bindVertexConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindTesselControlConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindTesselEvalConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindGeometryConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindFragmentConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      renderer.bindComputeConstantBuffers(0, buffer1.handleArray(), size_t{1u});
      buffer1.write(renderer, (const void*)&data1b);
      renderer.clearVertexConstantBuffers();
      renderer.clearTesselControlConstantBuffers();
      renderer.clearTesselEvalConstantBuffers();
      renderer.clearGeometryConstantBuffers();
      renderer.clearFragmentConstantBuffers();
      renderer.clearComputeConstantBuffers();
      
      DynamicBuffer buffer2(renderer, pandora::video::DataBufferType::constant, sizeof(data1a), (const void*)&data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(pandora::video::DataBufferType::constant, buffer2.type());
      renderer.bindVertexConstantBuffers(1, buffer2.handleArray(), size_t{1u});
      buffer2.write(renderer, (const void*)&data1b);
      renderer.clearVertexConstantBuffers();
      
      DynamicBuffer buffer3(renderer, pandora::video::DataBufferType::constant, sizeof(data1b), (const void*)&data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindFragmentConstantBuffers(2, buffer3.handleArray(), size_t{1u});
      renderer.clearFragmentConstantBuffers();
      
      DynamicBuffer buffer4(renderer, pandora::video::DataBufferType::constant, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      buffer4.writeDiscard(renderer, (const void*)&data2a);
      renderer.bindVertexConstantBuffers(0, buffer4.handleArray(), size_t{1u});
      buffer4.write(renderer, (const void*)&data2b);
      renderer.clearVertexConstantBuffers();
      
      DynamicBuffer buffer5(renderer, pandora::video::DataBufferType::constant, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexConstantBuffers(0, buffer5.handleArray(), size_t{1u});
      renderer.bindFragmentConstantBuffers(0, buffer5.handleArray(), size_t{1u});
      renderer.clearVertexConstantBuffers();
      renderer.clearFragmentConstantBuffers();
      buffer5.write(renderer, (const void*)&data2b);
      
      DynamicBuffer buffer6(renderer, pandora::video::DataBufferType::constant, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexConstantBuffers(0, buffer6.handleArray(), size_t{1u});
      renderer.clearVertexConstantBuffers();

      Renderer::DataBufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
      renderer.bindFragmentConstantBuffers(1, &buffers[0], size_t{2u});
      renderer.clearFragmentConstantBuffers();
      
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

  TEST_F(StaticDynamicBufferTest, vertexBufferCreationTest) {
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

      EXPECT_ANY_THROW(StaticBuffer(renderer, pandora::video::DataBufferType::vertexArray, 0));
      EXPECT_ANY_THROW(StaticBuffer(renderer, pandora::video::DataBufferType::vertexIndex, 0, nullptr, false));
      EXPECT_ANY_THROW(StaticBuffer(renderer, pandora::video::DataBufferType::vertexArray, size_t{1u}, nullptr, true));

      StaticBuffer buffer1(renderer, pandora::video::DataBufferType::vertexArray, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(pandora::video::DataBufferType::vertexArray, buffer1.type());
      buffer1.write(renderer, (const void*)&data1a);
      renderer.bindVertexArrayBuffer(0, buffer1.handle(), (unsigned int)buffer1.size(), 0);
      renderer.bindVertexArrayBuffer(1, buffer1.handle(), (unsigned int)buffer1.size()/2u, (unsigned int)buffer1.size()/2u);
      unsigned int bufferSize = (unsigned int)buffer1.size(), offset = 0;
      renderer.bindVertexArrayBuffers(2, size_t{ 1u }, buffer1.handleArray(), &bufferSize, &offset);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      StaticBuffer buffer2(renderer, pandora::video::DataBufferType::vertexIndex, sizeof(data1a), (const void*)&data1a, false);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(pandora::video::DataBufferType::vertexIndex, buffer2.type());
      renderer.bindVertexIndexBuffer(buffer2.handle(), pandora::video::IndexBufferFormat::r32_ui, 0);
      buffer2.write(renderer, (const void*)&data1b);
      renderer.bindVertexIndexBuffer(nullptr, pandora::video::IndexBufferFormat::r32_ui, 0);

      StaticBuffer buffer3(renderer, pandora::video::DataBufferType::vertexArray, sizeof(data1b), (const void*)&data1b, true);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexArrayBuffer(2, buffer3.handle(), (unsigned int)buffer3.size(), 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      StaticBuffer buffer4(renderer, pandora::video::DataBufferType::vertexArray, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      buffer4.write(renderer, (const void*)&data2a);
      renderer.bindVertexArrayBuffer(0, buffer4.handle(), (unsigned int)buffer4.size(), 0);
      buffer4.write(renderer, (const void*)&data2b);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      StaticBuffer buffer5(renderer, pandora::video::DataBufferType::vertexIndex, sizeof(data2a), (const void*)&data2a, false);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexIndexBuffer(buffer5.handle(), pandora::video::IndexBufferFormat::r16_ui, 0);
      renderer.bindVertexIndexBuffer(nullptr, pandora::video::IndexBufferFormat::r16_ui, 0);
      buffer5.write(renderer, (const void*)&data2b);

      StaticBuffer buffer6(renderer, pandora::video::DataBufferType::vertexArray, sizeof(data2a), (const void*)&data2a, true);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexArrayBuffer(0, buffer6.handle(), (unsigned int)buffer6.size(), 0);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      Renderer::DataBufferHandle buffers[] = { buffer3.handle(), buffer6.handle() };
      unsigned int sizes[] = { (unsigned int)buffer3.size(), (unsigned int)buffer6.size() };
      unsigned int offsets[] = { 0,0 };
      renderer.bindVertexArrayBuffers(1, size_t{2u}, &buffers[0], &sizes[0], &offsets[0]);
      renderer.bindVertexIndexBuffer(buffer2.handle(), pandora::video::IndexBufferFormat::r32_ui, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);
      renderer.bindVertexIndexBuffer(nullptr, pandora::video::IndexBufferFormat::r32_ui, 0);

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

      EXPECT_ANY_THROW(DynamicBuffer(renderer, pandora::video::DataBufferType::vertexArray, 0));
      EXPECT_ANY_THROW(DynamicBuffer(renderer, pandora::video::DataBufferType::vertexIndex, 0, nullptr));

      DynamicBuffer buffer1(renderer, pandora::video::DataBufferType::vertexArray, sizeof(data1a));
      EXPECT_FALSE(buffer1.isEmpty());
      EXPECT_TRUE(buffer1.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer1.size());
      EXPECT_EQ(pandora::video::DataBufferType::vertexArray, buffer1.type());
      buffer1.writeDiscard(renderer, (const void*)&data1a);
      renderer.bindVertexArrayBuffer(0, buffer1.handle(), (unsigned int)buffer1.size(), 0);
      renderer.bindVertexArrayBuffer(1, buffer1.handle(), (unsigned int)buffer1.size()/2u, (unsigned int)buffer1.size()/2u);
      unsigned int bufferSize = (unsigned int)buffer1.size(), offset = 0;
      renderer.bindVertexArrayBuffers(2, size_t{ 1u }, buffer1.handleArray(), &bufferSize, &offset);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      DynamicBuffer buffer2(renderer, pandora::video::DataBufferType::vertexIndex, sizeof(data1a), (const void*)&data1a);
      EXPECT_FALSE(buffer2.isEmpty());
      EXPECT_TRUE(buffer2.handle() != nullptr);
      EXPECT_EQ(sizeof(data1a), buffer2.size());
      EXPECT_EQ(pandora::video::DataBufferType::vertexIndex, buffer2.type());
      renderer.bindVertexIndexBuffer(buffer2.handle(), pandora::video::IndexBufferFormat::r32_ui, 0);
      buffer2.writeDiscard(renderer, (const void*)&data1b);
      renderer.bindVertexIndexBuffer(nullptr, pandora::video::IndexBufferFormat::r32_ui, 0);

      DynamicBuffer buffer3(renderer, pandora::video::DataBufferType::vertexArray, sizeof(data1b), (const void*)&data1b);
      EXPECT_FALSE(buffer3.isEmpty());
      EXPECT_TRUE(buffer3.handle() != nullptr);
      renderer.bindVertexArrayBuffer(2, buffer3.handle(), (unsigned int)buffer3.size(), 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);

      DynamicBuffer buffer4(renderer, pandora::video::DataBufferType::vertexArray, sizeof(data2a));
      EXPECT_FALSE(buffer4.isEmpty());
      EXPECT_TRUE(buffer4.handle() != nullptr);
      buffer4.writeDiscard(renderer, (const void*)&data2a);
      renderer.bindVertexArrayBuffer(0, buffer4.handle(), (unsigned int)buffer4.size(), 0);
      buffer4.write(renderer, (const void*)&data2b);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      DynamicBuffer buffer5(renderer, pandora::video::DataBufferType::vertexIndex, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer5.isEmpty());
      EXPECT_TRUE(buffer5.handle() != nullptr);
      renderer.bindVertexIndexBuffer(buffer5.handle(), pandora::video::IndexBufferFormat::r16_ui, 0);
      renderer.bindVertexIndexBuffer(nullptr, pandora::video::IndexBufferFormat::r16_ui, 0);
      buffer5.write(renderer, (const void*)&data2b);

      DynamicBuffer buffer6(renderer, pandora::video::DataBufferType::vertexArray, sizeof(data2a), (const void*)&data2a);
      EXPECT_FALSE(buffer6.isEmpty());
      EXPECT_TRUE(buffer6.handle() != nullptr);
      renderer.bindVertexArrayBuffer(0, buffer6.handle(), (unsigned int)buffer6.size(), 0);
      renderer.bindVertexArrayBuffer(0, nullptr, 0);

      Renderer::DataBufferHandle buffers[] = { buffer3.handle(), buffer6.handle() };
      unsigned int sizes[] = { (unsigned int)buffer3.size(), (unsigned int)buffer6.size() };
      unsigned int offsets[] = { 0,0 };
      renderer.bindVertexArrayBuffers(1, size_t{2u}, &buffers[0], &sizes[0], &offsets[0]);
      renderer.bindVertexIndexBuffer(buffer2.handle(), pandora::video::IndexBufferFormat::r32_ui, 0);
      renderer.bindVertexArrayBuffer(1, nullptr, 0);
      renderer.bindVertexArrayBuffer(2, nullptr, 0);
      renderer.bindVertexIndexBuffer(nullptr, pandora::video::IndexBufferFormat::r32_ui, 0);

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
