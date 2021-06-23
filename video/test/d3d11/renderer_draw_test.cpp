#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <video/window.h>
# include <video/d3d11/api/d3d_11.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/camera.h>
# include <video/d3d11/static_buffer.h>
# include <video/d3d11/depth_stencil_buffer.h>

  using namespace pandora::video::d3d11;
  using namespace pandora::video;

  class RendererDrawTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };
  
  
  // -- helpers --
  
# define __WIDTH  800
# define __HEIGHT 600
  
  static const char* __vertexBaseShaderText() noexcept {
    return 
      "float4 VSMain(float3 position : POSITION) : SV_POSITION"
      "{"
      "  return float4(position.x, position.y, position.z, 1.0f);"
      "}";
  }
  static const char* __fragmentBaseShaderText() noexcept {
    return 
      "float4 PSMain() : SV_TARGET"
      "{"
      "  return float4(1.0f,1.0f,1.0f,1.0f);"
      "}";
  }
  static const char* __vertexPosColorShaderText() noexcept {
    return 
      "struct VertexInputType {"
      "  float4 position : POSITION;"
      "  float4 color : COLOR;"
      "};"
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float4 color : COLOR;"
      "};"
      "PixelInputType VSMain(VertexInputType input) {"
      "  PixelInputType output;"
      "  output.position = input.position;"
      "  output.color = input.color;"
      "  return output;"
      "}";
  }
  static const char* __fragmentPosColorShaderText() noexcept {
    return 
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float4 color : COLOR;"
      "};"
      "float4 PSMain(PixelInputType input) : SV_TARGET"
      "{"
      "  return input.color;"
      "}";
  }
  static const char* __vertexInstanceShaderText() noexcept {
    return 
      "struct VertexInputType {"
      "  float4 position : POSITION0;"
      "  float4 color : COLOR0;"
      "  float3 instancePos : POSITION1;"
      "  float3 instanceColor : COLOR1;"
      "};"
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float4 color : COLOR0;"
      "};"
      "PixelInputType VSMain(VertexInputType input) {"
      "  PixelInputType output;"
      "  output.position.x = input.position.x + input.instancePos.x;"
      "  output.position.y = input.position.y + input.instancePos.y;"
      "  output.position.z = input.position.z + input.instancePos.z;"
      "  output.position.w = input.position.w;"
      "  output.color.x = input.color.x + input.instanceColor.x;"
      "  output.color.y = input.color.y + input.instanceColor.y;"
      "  output.color.z = input.color.z + input.instanceColor.z;"
      "  output.color.w = input.color.w;"
      "  return output;"
      "}";
  }
  static const char* __fragmentInstanceShaderText() noexcept {
    return 
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float4 color : COLOR0;"
      "};"
      "float4 PSMain(PixelInputType input) : SV_TARGET"
      "{"
      "  return input.color;"
      "}";
  }
  static const char* __vertexInstanceCamShaderText() noexcept {
    return 
      "cbuffer CamBuffer : register(b0)"
      "{"
      "  matrix projection;"
      "}"
      "struct VertexInputType {"
      "  float4 position : POSITION0;"
      "  float4 color : COLOR0;"
      "  float3 instancePos : POSITION1;"
      "  float3 instanceColor : COLOR1;"
      "};"
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float4 color : COLOR0;"
      "};"
      "PixelInputType VSMain(VertexInputType input) {"
      "  PixelInputType output;"
      "  output.position = float4(input.position.x + input.instancePos.x, input.position.y + input.instancePos.y, input.position.z + input.instancePos.z, input.position.w);"
      "  output.position = mul(projection, output.position);"
      "  if (input.position.z <= 0.5) { output.color = float4(input.color.x + input.instanceColor.x, input.color.y + input.instanceColor.y, input.color.z + input.instanceColor.z, input.color.w); }"
      "  else { output.color = float4((input.color.x + input.instanceColor.x)/4, (input.color.y + input.instanceColor.y)/4, (input.color.z + input.instanceColor.z)/4, input.color.w); }"
      "  return output;"
      "}";
  }

  __align_type(16,
  struct VertexPosColorData final {
    float position[4];
    float color[4];
  });
  __align_type(16,
  struct InstanceData final {
    float position[3];
    float color[3];
  });
  __align_type(16,
  struct CamBuffer final {
    DirectX::XMMATRIX projection;
  });


  // -- create window/renderer/swap-chain/buffers + draw simple shapes --

  TEST_F(RendererDrawTest, vertexBaseDrawingTest) {
    auto window = pandora::video::Window::Builder{}
      .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
        ResizeMode::fixed)
      .setSize(__WIDTH,__HEIGHT)
      .create(L"_DRAW_TEST0", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor, Renderer::DeviceLevel::direct3D_11_0);

    SwapChainParams params{};
    params.setBackBufferFormat(ComponentFormat::rgba8_sRGB)
          .setFrameBufferNumber(2u).setHdrPreferred(false)
          .setRefreshRate(60u, 1u).setRenderTargetMode(SwapChainTargetMode::uniqueOutput);
    SwapChain chain1(renderer, params, window->handle(), __WIDTH,__HEIGHT);
    ASSERT_FALSE(chain1.isEmpty());

    Viewport viewport(0,0, __WIDTH,__HEIGHT, 0.,1.);

    // shaders/input
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto vertexShaderBuilder = Shader::Builder::compile(ShaderType::vertex, __vertexBaseShaderText(), strlen(__vertexBaseShaderText()), "VSMain");
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), (Shader::InputElementDescArray)inputLayoutDescr, (size_t)1u);
    auto vertexShader = vertexShaderBuilder.createShader(renderer->device());
    auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, __fragmentBaseShaderText(), strlen(__fragmentBaseShaderText()), "PSMain")
                                          .createShader(renderer->device());
    ASSERT_TRUE(inputLayout.handle() != nullptr);
    ASSERT_FALSE(vertexShader.isEmpty());
    ASSERT_FALSE(fragmentShader.isEmpty());

    // vertices
    float vertices1[] = { 
      0.0f,  0.5f,  0.0f,  // point at top-center
      0.5f, -0.5f,  0.0f,  // point at bottom-right
      -0.5f, -0.5f,  0.0f, // point at bottom-left
    };
    StaticBuffer vertexArray1(*renderer, DataBufferType::vertexArray, sizeof(vertices1), (const void*)vertices1, true);

    // drawing
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    renderer->setViewport(viewport);

    renderer->clearView(chain1.getRenderTargetView(), nullptr, { {{0.f,0.5f,0.6f}} });
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(renderer->createTopology(VertexTopology::triangles));

    renderer->bindVertexArrayBuffer(0, vertexArray1.handle(), (unsigned int)sizeof(float)*3u);
    renderer->draw(sizeof(vertices1) / (3*sizeof(float)));
    chain1.swapBuffersDiscard(true, nullptr);
  }

  TEST_F(RendererDrawTest, vertexIndexedDrawingWithStatesTest) {
    auto window = pandora::video::Window::Builder{}
                    .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
                                    ResizeMode::fixed)
                    .setSize(__WIDTH,__HEIGHT)
                    .create(L"_DRAW_TEST1", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor, Renderer::DeviceLevel::direct3D_11_0);

    SwapChainParams params{};
    params.setBackBufferFormat(ComponentFormat::rgba8_sRGB)
          .setFrameBufferNumber(2u).setHdrPreferred(false)
          .setRefreshRate(60u, 1u).setRenderTargetMode(SwapChainTargetMode::uniqueOutput);
    SwapChain chain1(renderer, params, window->handle(), __WIDTH,__HEIGHT);
    ASSERT_FALSE(chain1.isEmpty());
    
    Viewport viewport(0,0, __WIDTH,__HEIGHT, 0.,1.);

    // shaders/input
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",   0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto vertexShaderBuilder = Shader::Builder::compile(ShaderType::vertex, __vertexPosColorShaderText(), strlen(__vertexPosColorShaderText()), "VSMain");
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), (Shader::InputElementDescArray)inputLayoutDescr, sizeof(inputLayoutDescr)/sizeof(*inputLayoutDescr));
    auto vertexShader = vertexShaderBuilder.createShader(renderer->device());
    auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, __fragmentPosColorShaderText(), strlen(__fragmentPosColorShaderText()), "PSMain")
                                          .createShader(renderer->device());
    ASSERT_TRUE(inputLayout.handle() != nullptr);
    ASSERT_FALSE(vertexShader.isEmpty());
    ASSERT_FALSE(fragmentShader.isEmpty());
    
    // states
    DepthStencilBuffer depthBuffer(*renderer, ComponentFormat::d32_f, __WIDTH,__HEIGHT);
    DepthStencilState depthState = renderer->createDepthTestState(DepthOperationGroup{ DepthStencilOperation::incrementWrap, DepthStencilOperation::keep }, 
                                                                  DepthOperationGroup{ DepthStencilOperation::decrementWrap, DepthStencilOperation::keep }, 
                                                                  DepthComparison::less, true);
    ASSERT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthState);
    
    RasterizerState rasterState(renderer->createRasterizerState(CullMode::cullBack, true, DepthBias{}, false));
    EXPECT_TRUE(rasterState);
    
    FilterStates values;
    TextureAddressMode addrModes[3] { TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat };
    renderer->createFilter(values, MinificationFilter::linear, MagnificationFilter::linear, addrModes);
    ASSERT_EQ((size_t)1, values.size());

    // vertices
    VertexPosColorData vertices1[] = { 
      {{0.0f,0.5f,0.f,1.f},{1.f,0.f,0.f,1.f}}, {{0.5f,-0.5f,0.f,1.f},{0.f,1.f,0.f,1.f}}, {{-0.5f,-0.5f,0.f,1.f},{0.f,0.f,1.f,1.f}}
    };
    uint32_t indices1[] = { 0,1,2 };
    StaticBuffer vertexArray1(*renderer, DataBufferType::vertexArray, sizeof(vertices1), (const void*)vertices1, true);
    StaticBuffer vertexIndex1(*renderer, DataBufferType::vertexIndex, sizeof(indices1), (const void*)indices1, true);

    // drawing
    renderer->setRasterizerState(rasterState);
    renderer->setDepthStencilState(depthState);
    renderer->setFragmentFilterStates(0, values.get(), values.size());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView());
    renderer->setViewport(viewport);

    renderer->clearView(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView(), { {{0.f,0.f,0.f}} });
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(renderer->createTopology(VertexTopology::triangles));
   
    renderer->bindVertexArrayBuffer(0, vertexArray1.handle(), (unsigned int)sizeof(VertexPosColorData));
    renderer->bindVertexIndexBuffer(vertexIndex1.handle(), IndexBufferFormat::r32_ui);
    renderer->drawIndexed(sizeof(indices1)/sizeof(*indices1));
    chain1.swapBuffersDiscard(true, depthBuffer.getDepthStencilView());
  }

  TEST_F(RendererDrawTest, vertexInstanceIndexedDrawingWithStatesTest) {
    auto window = pandora::video::Window::Builder{}
      .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
        ResizeMode::fixed)
      .setSize(__WIDTH,__HEIGHT)
      .create(L"_DRAW_TEST2", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor, Renderer::DeviceLevel::direct3D_11_0);

    SwapChainParams params{};
    params.setBackBufferFormat(ComponentFormat::rgba8_sRGB)
      .setFrameBufferNumber(2u).setHdrPreferred(false)
      .setRefreshRate(60u, 1u).setRenderTargetMode(SwapChainTargetMode::uniqueOutput);
    SwapChain chain1(renderer, params, window->handle(), __WIDTH,__HEIGHT);
    ASSERT_FALSE(chain1.isEmpty());

    Viewport viewport(0,0, __WIDTH,__HEIGHT, 0.,1.);

    // shaders/input
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
      { "COLOR",    1, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    auto vertexShaderBuilder = Shader::Builder::compile(ShaderType::vertex, __vertexInstanceShaderText(), strlen(__vertexInstanceShaderText()), "VSMain");
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), (Shader::InputElementDescArray)inputLayoutDescr, sizeof(inputLayoutDescr)/sizeof(*inputLayoutDescr));
    auto vertexShader = vertexShaderBuilder.createShader(renderer->device());
    auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, __fragmentInstanceShaderText(), strlen(__fragmentInstanceShaderText()), "PSMain")
      .createShader(renderer->device());
    ASSERT_TRUE(inputLayout.handle() != nullptr);
    ASSERT_FALSE(vertexShader.isEmpty());
    ASSERT_FALSE(fragmentShader.isEmpty());

    // states
    DepthStencilBuffer depthBuffer(*renderer, ComponentFormat::d32_f, __WIDTH,__HEIGHT);
    DepthStencilState depthState = renderer->createDepthTestState(DepthOperationGroup{ DepthStencilOperation::incrementWrap, DepthStencilOperation::keep }, 
      DepthOperationGroup{ DepthStencilOperation::decrementWrap, DepthStencilOperation::keep }, 
      DepthComparison::less, true);
    ASSERT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthState);

    RasterizerState rasterState(renderer->createRasterizerState(CullMode::cullBack, true, DepthBias{}, false));
    EXPECT_TRUE(rasterState);

    FilterStates values;
    TextureAddressMode addrModes[3] { TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat };
    renderer->createFilter(values, MinificationFilter::linear, MagnificationFilter::linear, addrModes);
    ASSERT_EQ((size_t)1, values.size());

    // vertices
    VertexPosColorData vertices1[] = { 
      {{0.0f,0.25f,0.f,1.f},{0.5f,0.f,0.f,1.f}}, {{0.25f,-0.25f,0.f,1.f},{0.f,0.5f,0.f,1.f}}, {{-0.25f,-0.25f,0.f,1.f},{0.f,0.f,0.5f,1.f}}
    };
    uint32_t indices1[] = { 0,1,2 };
    StaticBuffer vertexArray1(*renderer, DataBufferType::vertexArray, sizeof(vertices1), (const void*)vertices1, true);
    StaticBuffer vertexIndex1(*renderer, DataBufferType::vertexIndex, sizeof(indices1), (const void*)indices1, true);
    InstanceData instances1[] = { {{-0.5f,-0.5f,0.f},{0.5f,0.f,0.f}}, {{-0.5f,0.5f,0.f},{0.f,0.5f,0.f}}, 
                                  {{0.5f,-0.5f,0.f}, {0.f,0.f,0.5f}}, {{0.5f,0.5f,0.f}, {0.25f,0.25f,0.25f}} };
    StaticBuffer instanceArray1(*renderer, DataBufferType::vertexArray, sizeof(instances1), (const void*)instances1, true);

    // drawing
    renderer->setRasterizerState(rasterState);
    renderer->setDepthStencilState(depthState);
    renderer->setFragmentFilterStates(0, values.get(), values.size());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView());
    renderer->setViewport(viewport);

    renderer->clearView(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView(), { {{0.f,0.f,0.f}} });
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(renderer->createTopology(VertexTopology::triangles));

    void* vertexBuffers[] = { vertexArray1.handle(), instanceArray1.handle() };
    unsigned int vertexStrides[] = { (unsigned int)sizeof(VertexPosColorData), (unsigned int)sizeof(InstanceData) };
    unsigned int offsets[] = { 0,0 };
    renderer->bindVertexArrayBuffers(0, size_t{ 2u }, vertexBuffers, vertexStrides, offsets);
    renderer->bindVertexIndexBuffer(vertexIndex1.handle(), IndexBufferFormat::r32_ui);
    renderer->drawInstancesIndexed(sizeof(instances1)/sizeof(*instances1), 0, sizeof(indices1)/sizeof(*indices1), 0, 0);
    chain1.swapBuffersDiscard(true, depthBuffer.getDepthStencilView());
  }

  TEST_F(RendererDrawTest, vertexInstanceDrawingWithStatesCamTest) {
    auto window = pandora::video::Window::Builder{}
      .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
        ResizeMode::fixed)
      .setSize(__WIDTH,__HEIGHT)
      .create(L"_DRAW_TEST2", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor, Renderer::DeviceLevel::direct3D_11_0);

    SwapChainParams params{};
    params.setBackBufferFormat(ComponentFormat::rgba8_sRGB)
      .setFrameBufferNumber(2u).setHdrPreferred(false)
      .setRefreshRate(60u, 1u).setRenderTargetMode(SwapChainTargetMode::uniqueOutput);
    SwapChain chain1(renderer, params, window->handle(), __WIDTH,__HEIGHT);
    ASSERT_FALSE(chain1.isEmpty());

    Viewport viewport(0,0, __WIDTH,__HEIGHT, 0.,1.);

    // shaders/input
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
      { "COLOR",    1, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    auto vertexShaderBuilder = Shader::Builder::compile(ShaderType::vertex, __vertexInstanceCamShaderText(), strlen(__vertexInstanceCamShaderText()), "VSMain");
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), (Shader::InputElementDescArray)inputLayoutDescr, sizeof(inputLayoutDescr)/sizeof(*inputLayoutDescr));
    auto vertexShader = vertexShaderBuilder.createShader(renderer->device());
    auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, __fragmentInstanceShaderText(), strlen(__fragmentInstanceShaderText()), "PSMain")
      .createShader(renderer->device());
    ASSERT_TRUE(inputLayout.handle() != nullptr);
    ASSERT_FALSE(vertexShader.isEmpty());
    ASSERT_FALSE(fragmentShader.isEmpty());

    // states
    DepthStencilBuffer depthBuffer(*renderer, ComponentFormat::d32_f, __WIDTH,__HEIGHT);
    DepthStencilState depthState = renderer->createDepthTestState(DepthOperationGroup{ DepthStencilOperation::incrementWrap, DepthStencilOperation::keep }, 
      DepthOperationGroup{ DepthStencilOperation::decrementWrap, DepthStencilOperation::keep }, 
      DepthComparison::less, true);
    ASSERT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthState);

    RasterizerState rasterState(renderer->createRasterizerState(CullMode::cullBack, true, DepthBias{}, false));
    EXPECT_TRUE(rasterState);

    FilterStates values;
    TextureAddressMode addrModes[3] { TextureAddressMode::repeat, TextureAddressMode::repeat, TextureAddressMode::repeat };
    renderer->createFilter(values, MinificationFilter::linear, MagnificationFilter::linear, addrModes);
    ASSERT_EQ((size_t)1, values.size());

    // vertices
    VertexPosColorData vertices1[] = { 
      {{-0.1875f,0.1875f,0.5f,1.f},{0.5f,0.f,0.f,1.f}}, {{0.1875f,0.1875f,0.5f,1.f},{0.f,0.5f,0.f,1.f}}, {{-0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.5f,1.f}},
      {{0.1875f,0.1875f,0.5f,1.f},{0.f,0.5f,0.f,1.f}}, {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.5f,1.f}}, {{-0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.5f,1.f}},

      {{-0.1875f,0.1875f,0.85f,1.f},{0.2f,0.f,0.2f,1.f}}, {{-0.1875f,0.1875f,0.5f,1.f},{0.4f,0.f,0.f,1.f}}, {{-0.1875f,-0.1875f,0.85f,1.f},{0.2f,0.f,0.2f,1.f}},
      {{-0.1875f,0.1875f,0.5f,1.f},{0.4f,0.f,0.f,1.f}}, {{-0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.4f,1.f}}, {{-0.1875f,-0.1875f,0.85f,1.f},{0.2f,0.f,0.2f,1.f}},

      {{0.1875f,0.1875f,0.5f,1.f},{0.f,0.4f,0.f,1.f}}, {{0.1875f,0.1875f,0.85f,1.f},{0.f,0.2f,0.2f,1.f}}, {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.4f,1.f}},
      {{0.1875f,0.1875f,0.85f,1.f},{0.f,0.2f,0.2f,1.f}}, {{0.1875f,-0.1875f,0.85f,1.f},{0.f,0.2f,0.2f,1.f}}, {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.4f,1.f}},

      {{-0.1875f,0.1875f,0.85f,1.f},{0.2f,0.2f,0.f,1.f}}, {{0.1875f,0.1875f,0.85f,1.f},{0.2f,0.2f,0.f,1.f}}, {{-0.1875f,0.1875f,0.5f,1.f},{0.4f,0.f,0.f,1.f}},
      {{0.1875f,0.1875f,0.85f,1.f},{0.2f,0.2f,0.f,1.f}}, {{0.1875f,0.1875f,0.5f,1.f},{0.f,0.4f,0.f,1.f}}, {{-0.1875f,0.1875f,0.5f,1.f},{0.4f,0.f,0.f,1.f}}, 

      {{-0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.25f,1.f}}, {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.25f,1.f}}, {{-0.1875f,-0.1875f,0.85f,1.f},{0.f,0.1f,0.1f,1.f}},
      {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.25f,1.f}}, {{0.1875f,-0.1875f,0.85f,1.f},{0.1f,0.f,0.1f,1.f}}, {{-0.1875f,-0.1875f,0.85f,1.f},{0.f,0.1f,0.1f,1.f}}
    };
    StaticBuffer vertexArray1(*renderer, DataBufferType::vertexArray, sizeof(vertices1), (const void*)vertices1, true);
    InstanceData instances1[] = { 
      {{-0.4f,-0.4f,0.f},{0.5f,0.f,0.f}}, {{-0.4f,0.4f,0.f},{0.f,0.5f,0.f}}, {{0.4f,-0.4f,0.f}, {0.f,0.f,0.5f}}, {{0.4f,0.4f,0.f}, {0.2f,0.f,0.2f}},
      {{-0.4f,-0.4f,0.7f},{0.f,0.4f,0.f}}, {{-0.4f,0.4f,0.7f},{0.f,0.f,0.4f}}, {{0.4f,-0.4f,0.7f}, {0.4f,0.f,0.f}}, {{0.4f,0.4f,0.7f}, {0.f,0.1f,0.3f}},
      {{-0.4f,-0.4f,1.4f},{0.f,0.f,0.3f}}, {{-0.4f,0.4f,1.4f},{0.3f,0.f,0.f}}, {{0.4f,-0.4f,1.4f}, {0.f,0.3f,0.f}}, {{0.4f,0.4f,1.4f}, {0.1f,0.f,0.3f}},
      {{-0.4f,-0.4f,2.1f},{0.2f,0.f,0.f}}, {{-0.4f,0.4f,2.1f},{0.f,0.2f,0.f}}, {{0.4f,-0.4f,2.1f}, {0.f,0.f,0.2f}}, {{0.4f,0.4f,2.1f}, {0.f,0.05f,0.1f}},
    };
    StaticBuffer instanceArray1(*renderer, DataBufferType::vertexArray, sizeof(instances1), (const void*)instances1, true);

    // camera
    CameraProjection proj(__WIDTH, __HEIGHT, 70.f);
    CamBuffer camData{ proj.projectionMatrix() };
    StaticBuffer camBuffer(*renderer, DataBufferType::constant, sizeof(CamBuffer), &camData, true);

    // drawing
    renderer->setRasterizerState(rasterState);
    renderer->setDepthStencilState(depthState);
    renderer->setFragmentFilterStates(0, values.get(), values.size());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView());
    renderer->setViewport(viewport);

    renderer->clearView(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView(), { {{0.f,0.f,0.f}} });
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(renderer->createTopology(VertexTopology::triangles));

    void* vertexBuffers[] = { vertexArray1.handle(), instanceArray1.handle() };
    unsigned int vertexStrides[] = { (unsigned int)sizeof(VertexPosColorData), (unsigned int)sizeof(InstanceData) };
    unsigned int offsets[] = { 0,0 };
    renderer->bindVertexConstantBuffers(0, camBuffer.handleArray(), size_t{ 1u });
    renderer->bindVertexArrayBuffers(0, size_t{ 2u }, vertexBuffers, vertexStrides, offsets);
    renderer->drawInstances(sizeof(instances1)/sizeof(*instances1), 0, sizeof(vertices1)/sizeof(*vertices1), 0);
    chain1.swapBuffersDiscard(true, depthBuffer.getDepthStencilView());
  }

#endif
