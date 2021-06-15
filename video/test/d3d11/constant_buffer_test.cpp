#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <system/align.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/constant_buffer.h>

  using namespace pandora::video::d3d11;

  class ConstantBufferTest : public testing::Test {
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

  TEST_F(ConstantBufferTest, constantBufferCreationTest) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor, Renderer::DeviceLevel::direct3D_11_0);
    
    ConstantBuffer emptyBuffer;
    EXPECT_TRUE(emptyBuffer.isEmpty());
    EXPECT_TRUE(emptyBuffer.handle() == nullptr);
    
    EXPECT_ANY_THROW(ConstantBuffer(renderer, 0));
    EXPECT_ANY_THROW(ConstantBuffer(renderer, 0, nullptr, false));
    EXPECT_ANY_THROW(ConstantBuffer(renderer, size_t{1u}, nullptr, true));
    
    uint64_t data1a[2] = { 42, 0 };
    uint64_t data1b[2] = { 0, 22 };
    _TestColor data2a{ 0.1f,0.2f,0.3f,0.4f };
    _TestColor data2b{ 1.0f,0.8f,0.6f,0.4f };
    
    ConstantBuffer buffer1(renderer, sizeof(data1a));
    EXPECT_FALSE(buffer1.isEmpty());
    EXPECT_TRUE(buffer1.handle() != nullptr);
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
    
    ConstantBuffer buffer2(renderer, sizeof(data1a), (const void*)&data1a, false);
    EXPECT_FALSE(buffer2.isEmpty());
    EXPECT_TRUE(buffer2.handle() != nullptr);
    renderer.bindVertexFragmentConstantBuffers(1, buffer2.handleArray(), size_t{1u});
    buffer2.write(renderer, (const void*)&data1b);
    renderer.clearVertexFragmentConstantBuffers();
    
    ConstantBuffer buffer3(renderer, sizeof(data1b), (const void*)&data1b, true);
    EXPECT_FALSE(buffer3.isEmpty());
    EXPECT_TRUE(buffer3.handle() != nullptr);
    renderer.bindVertexFragmentConstantBuffers(2, buffer3.handleArray(), size_t{1u});
    renderer.clearVertexFragmentConstantBuffers();
    
    ConstantBuffer buffer4(renderer, sizeof(data2a));
    EXPECT_FALSE(buffer4.isEmpty());
    EXPECT_TRUE(buffer4.handle() != nullptr);
    buffer4.write(renderer, (const void*)&data2a);
    renderer.bindVertexConstantBuffers(0, buffer4.handleArray(), size_t{1u});
    buffer4.write(renderer, (const void*)&data2b);
    renderer.clearVertexConstantBuffers();
    
    ConstantBuffer buffer5(renderer, sizeof(data2a), (const void*)&data2a, false);
    EXPECT_FALSE(buffer5.isEmpty());
    EXPECT_TRUE(buffer5.handle() != nullptr);
    renderer.bindVertexFragmentConstantBuffers(0, buffer5.handleArray(), size_t{1u});
    renderer.clearVertexFragmentConstantBuffers();
    buffer5.write(renderer, (const void*)&data2b);
    
    ConstantBuffer buffer6(renderer, sizeof(data2a), (const void*)&data2a, true);
    EXPECT_FALSE(buffer6.isEmpty());
    EXPECT_TRUE(buffer6.handle() != nullptr);
    renderer.bindVertexFragmentConstantBuffers(0, buffer6.handleArray(), size_t{1u});
    renderer.clearVertexFragmentConstantBuffers();

    Renderer::ConstantBufferHandle buffers[] = { buffer2.handle(), buffer5.handle() };
    renderer.bindVertexFragmentConstantBuffers(1, &buffers[0], size_t{2u});
    renderer.clearVertexFragmentConstantBuffers();
    
    auto buffer1Handle = buffer1.handle();
    ConstantBuffer moved(std::move(buffer1));
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

#endif
