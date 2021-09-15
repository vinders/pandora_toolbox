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
#ifndef _P_CI_DISABLE_SLOW_TESTS
#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# ifndef __MINGW32__
#   pragma warning(push)
#   pragma warning(disable : 4324)
# endif
# include <gtest/gtest.h>
# include <video/window.h>
# include <video/d3d11/api/d3d_11.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/renderer_state_factory.h>
# include <video/d3d11/camera_utils.h>
# include <video/d3d11/texture.h>
# include <video/d3d11/static_buffer.h>
# include <video/d3d11/depth_stencil_buffer.h>

  using namespace pandora::video::d3d11;
  using namespace pandora::video;

  class D3d11RendererDrawTest : public testing::Test {
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

  TEST_F(D3d11RendererDrawTest, vertexBaseDrawingTest) {
    auto window = pandora::video::Window::Builder{}
      .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
        ResizeMode::fixed)
      .setSize(__WIDTH,__HEIGHT)
      .create(L"_DRAW_TEST0", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor);

    SwapChain::Descriptor params;
    params.width = __WIDTH;
    params.height = __HEIGHT;
    params.framebufferCount = 2u;
    params.refreshRate = RefreshRate(60u, 1u);
    SwapChain chain1(renderer, window->handle(), params, DataFormat::rgba8_sRGB);
    ASSERT_FALSE(chain1.isEmpty());

    Viewport viewport(0,0, __WIDTH,__HEIGHT, 0.,1.);

    // shaders/input
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto vertexShaderBuilder = Shader::Builder::compile(ShaderType::vertex, __vertexBaseShaderText(), strlen(__vertexBaseShaderText()), "VSMain");
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), inputLayoutDescr, (size_t)1u);
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
    StaticBuffer vertexArray1(*renderer, BaseBufferType::vertex, sizeof(vertices1), (const void*)vertices1, true);

    // drawing
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), nullptr);
    renderer->setViewport(viewport);

    float color[4] = { 0.f,0.5f,0.6f,1.f };
    FLOAT gammaCorrectColor[4]{ 0 };
    renderer->toGammaCorrectColor(color, gammaCorrectColor);

    renderer->clearView(chain1.getRenderTargetView(), nullptr, gammaCorrectColor);
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(VertexTopology::triangles);

    renderer->bindVertexArrayBuffer(0, vertexArray1.handle(), (unsigned int)sizeof(float)*3u);
    renderer->draw(sizeof(vertices1) / (3*sizeof(float)));
    chain1.swapBuffersDiscard(true, nullptr);
    renderer->flush();
  }

  TEST_F(D3d11RendererDrawTest, vertexIndexedDrawingWithStatesTest) {
    auto window = pandora::video::Window::Builder{}
                    .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
                                    ResizeMode::fixed)
                    .setSize(__WIDTH,__HEIGHT)
                    .create(L"_DRAW_TEST1", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor);
    RendererStateFactory factory(*renderer);

    SwapChain::Descriptor params;
    params.width = __WIDTH;
    params.height = __HEIGHT;
    params.framebufferCount = 2u;
    params.refreshRate = RefreshRate(60u, 1u);
    SwapChain chain1(renderer, window->handle(), params, DataFormat::rgba8_sRGB);
    ASSERT_FALSE(chain1.isEmpty());
    
    Viewport viewport(0,0, __WIDTH,__HEIGHT, 0.,1.);

    // shaders/input
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",   0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto vertexShaderBuilder = Shader::Builder::compile(ShaderType::vertex, __vertexPosColorShaderText(), strlen(__vertexPosColorShaderText()), "VSMain");
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), inputLayoutDescr, sizeof(inputLayoutDescr)/sizeof(*inputLayoutDescr));
    auto vertexShader = vertexShaderBuilder.createShader(renderer->device());
    auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, __fragmentPosColorShaderText(), strlen(__fragmentPosColorShaderText()), "PSMain")
                                          .createShader(renderer->device());
    ASSERT_TRUE(inputLayout.handle() != nullptr);
    ASSERT_FALSE(vertexShader.isEmpty());
    ASSERT_FALSE(fragmentShader.isEmpty());
    
    // states
    DepthStencilBuffer depthBuffer(*renderer, DepthStencilFormat::d32_f, __WIDTH,__HEIGHT);
    DepthStencilState depthState = factory.createDepthStencilTestState(DepthStencilParams{}); // default depth test (less)
    ASSERT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthState);
    
    RasterizerState rasterState(factory.createRasterizerState(RasterizerParams{})); // standard: back-cull, filled, clockwise, depth clipping
    EXPECT_TRUE(rasterState);
    
    FilterStateArray values;
    TextureWrap addrModes[3] { TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat };
    values.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear, addrModes)));
    ASSERT_EQ((size_t)1, values.size());

    // vertices
    VertexPosColorData vertices1[] = { 
      {{0.0f,0.5f,0.f,1.f},{1.f,0.f,0.f,1.f}}, {{0.5f,-0.5f,0.f,1.f},{0.f,1.f,0.f,1.f}}, {{-0.5f,-0.5f,0.f,1.f},{0.f,0.f,1.f,1.f}}
    };
    uint32_t indices1[] = { 0,1,2 };
    StaticBuffer vertexArray1(*renderer, BaseBufferType::vertex, sizeof(vertices1), (const void*)vertices1, true);
    StaticBuffer vertexIndex1(*renderer, BaseBufferType::vertexIndex, sizeof(indices1), (const void*)indices1, true);

    // drawing
    renderer->setRasterizerState(rasterState);
    renderer->setDepthStencilState(depthState);
    renderer->setFragmentFilterStates(0, values.get(), values.size());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView());
    renderer->setViewport(viewport);

    renderer->clearView(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView(), nullptr);
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(VertexTopology::triangles);
   
    renderer->bindVertexArrayBuffer(0, vertexArray1.handle(), (unsigned int)sizeof(VertexPosColorData));
    renderer->bindVertexIndexBuffer(vertexIndex1.handle(), VertexIndexFormat::r32_ui);
    renderer->drawIndexed(sizeof(indices1)/sizeof(*indices1));
    chain1.swapBuffersDiscard(true, depthBuffer.getDepthStencilView());
    renderer->flush();
  }

  TEST_F(D3d11RendererDrawTest, vertexInstanceIndexedDrawingWithStatesTest) {
    auto window = pandora::video::Window::Builder{}
      .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
        ResizeMode::fixed)
      .setSize(__WIDTH,__HEIGHT)
      .create(L"_DRAW_TEST2", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor);
    RendererStateFactory factory(*renderer);

    SwapChain::Descriptor params;
    params.width = __WIDTH;
    params.height = __HEIGHT;
    params.framebufferCount = 2u;
    params.refreshRate = RefreshRate(60u, 1u);
    SwapChain chain1(renderer, window->handle(), params, DataFormat::rgba8_sRGB);
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
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), inputLayoutDescr, sizeof(inputLayoutDescr)/sizeof(*inputLayoutDescr));
    auto vertexShader = vertexShaderBuilder.createShader(renderer->device());
    auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, __fragmentInstanceShaderText(), strlen(__fragmentInstanceShaderText()), "PSMain")
      .createShader(renderer->device());
    ASSERT_TRUE(inputLayout.handle() != nullptr);
    ASSERT_FALSE(vertexShader.isEmpty());
    ASSERT_FALSE(fragmentShader.isEmpty());

    // states
    DepthStencilBuffer depthBuffer(*renderer, DepthStencilFormat::d32_f, __WIDTH,__HEIGHT);
    DepthStencilState depthState = factory.createDepthStencilTestState(DepthStencilParams{}); // default depth test (less)
    ASSERT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthState);

    RasterizerState rasterState(factory.createRasterizerState(RasterizerParams{})); // standard: back-cull, filled, clockwise, depth clipping
    EXPECT_TRUE(rasterState);

    FilterStateArray values;
    TextureWrap addrModes[3] { TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat };
    values.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear, addrModes)));
    ASSERT_EQ((size_t)1, values.size());

    // vertices
    VertexPosColorData vertices1[] = { 
      {{0.0f,0.25f,0.f,1.f},{0.5f,0.f,0.f,1.f}}, {{0.25f,-0.25f,0.f,1.f},{0.f,0.5f,0.f,1.f}}, {{-0.25f,-0.25f,0.f,1.f},{0.f,0.f,0.5f,1.f}}
    };
    uint32_t indices1[] = { 0,1,2 };
    StaticBuffer vertexArray1(*renderer, BaseBufferType::vertex, sizeof(vertices1), (const void*)vertices1, true);
    StaticBuffer vertexIndex1(*renderer, BaseBufferType::vertexIndex, sizeof(indices1), (const void*)indices1, true);
    InstanceData instances1[] = { {{-0.5f,-0.5f,0.f},{0.5f,0.f,0.f}}, {{-0.5f,0.5f,0.f},{0.f,0.5f,0.f}}, 
                                  {{0.5f,-0.5f,0.f}, {0.f,0.f,0.5f}}, {{0.5f,0.5f,0.f}, {0.25f,0.25f,0.25f}} };
    StaticBuffer instanceArray1(*renderer, BaseBufferType::vertex, sizeof(instances1), (const void*)instances1, true);

    // drawing
    renderer->setRasterizerState(rasterState);
    renderer->setDepthStencilState(depthState);
    renderer->setFragmentFilterStates(0, values.get(), values.size());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView());
    renderer->setViewport(viewport);

    renderer->clearView(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView(), nullptr);
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(VertexTopology::triangles);

    BufferHandle vertexBuffers[] = { vertexArray1.handle(), instanceArray1.handle() };
    unsigned int vertexStrides[] = { (unsigned int)sizeof(VertexPosColorData), (unsigned int)sizeof(InstanceData) };
    unsigned int offsets[] = { 0,0 };
    renderer->bindVertexArrayBuffers(0, size_t{ 2u }, vertexBuffers, vertexStrides, offsets);
    renderer->bindVertexIndexBuffer(vertexIndex1.handle(), VertexIndexFormat::r32_ui);
    renderer->drawInstancesIndexed(sizeof(instances1)/sizeof(*instances1), 0, sizeof(indices1)/sizeof(*indices1), 0, 0);
    chain1.swapBuffersDiscard(true, depthBuffer.getDepthStencilView());
    renderer->flush();
  }

  TEST_F(D3d11RendererDrawTest, vertexInstanceDrawingWithStatesCamTest) {
    auto window = pandora::video::Window::Builder{}
      .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
        ResizeMode::fixed)
      .setSize(__WIDTH,__HEIGHT)
      .create(L"_DRAW_TEST3", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor);
    RendererStateFactory factory(*renderer);

    SwapChain::Descriptor params;
    params.width = __WIDTH;
    params.height = __HEIGHT;
    params.framebufferCount = 2u;
    params.refreshRate = RefreshRate(60u, 1u);
    SwapChain chain1(renderer, window->handle(), params, DataFormat::rgba8_sRGB);
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
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), inputLayoutDescr, sizeof(inputLayoutDescr)/sizeof(*inputLayoutDescr));
    auto vertexShader = vertexShaderBuilder.createShader(renderer->device());
    auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, __fragmentInstanceShaderText(), strlen(__fragmentInstanceShaderText()), "PSMain")
                                          .createShader(renderer->device());
    ASSERT_TRUE(inputLayout.handle() != nullptr);
    ASSERT_FALSE(vertexShader.isEmpty());
    ASSERT_FALSE(fragmentShader.isEmpty());

    // states
    DepthStencilBuffer depthBuffer(*renderer, DepthStencilFormat::d32_f, __WIDTH,__HEIGHT);
    DepthStencilState depthState = factory.createDepthStencilTestState(DepthStencilParams{}); // default depth test (less)
    ASSERT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthState);

    RasterizerState rasterState(factory.createRasterizerState(RasterizerParams{})); // standard: back-cull, filled, clockwise, depth clipping
    EXPECT_TRUE(rasterState);

    FilterStateArray values;
    TextureWrap addrModes[3] { TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat };
    values.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear, addrModes)));
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
    StaticBuffer vertexArray1(*renderer, BaseBufferType::vertex, sizeof(vertices1), (const void*)vertices1, true);
    InstanceData instances1[] = { 
      {{-0.4f,-0.4f,0.f},{0.5f,0.f,0.f}}, {{-0.4f,0.4f,0.f},{0.f,0.5f,0.f}}, {{0.4f,-0.4f,0.f}, {0.f,0.f,0.5f}}, {{0.4f,0.4f,0.f}, {0.2f,0.f,0.2f}},
      {{-0.4f,-0.4f,0.7f},{0.f,0.4f,0.f}}, {{-0.4f,0.4f,0.7f},{0.f,0.f,0.4f}}, {{0.4f,-0.4f,0.7f}, {0.4f,0.f,0.f}}, {{0.4f,0.4f,0.7f}, {0.f,0.1f,0.3f}},
      {{-0.4f,-0.4f,1.4f},{0.f,0.f,0.3f}}, {{-0.4f,0.4f,1.4f},{0.3f,0.f,0.f}}, {{0.4f,-0.4f,1.4f}, {0.f,0.3f,0.f}}, {{0.4f,0.4f,1.4f}, {0.1f,0.f,0.3f}},
      {{-0.4f,-0.4f,2.1f},{0.2f,0.f,0.f}}, {{-0.4f,0.4f,2.1f},{0.f,0.2f,0.f}}, {{0.4f,-0.4f,2.1f}, {0.f,0.f,0.2f}}, {{0.4f,0.4f,2.1f}, {0.f,0.05f,0.1f}},
    };
    StaticBuffer instanceArray1(*renderer, BaseBufferType::vertex, sizeof(instances1), (const void*)instances1, true);

    // camera
    CameraProjection proj(__WIDTH, __HEIGHT, 70.f);
    CamBuffer camData{ proj.projectionMatrix() };
    StaticBuffer camBuffer(*renderer, BaseBufferType::uniform, sizeof(CamBuffer), &camData, true);

    // drawing
    renderer->setRasterizerState(rasterState);
    renderer->setDepthStencilState(depthState);
    renderer->setFragmentFilterStates(0, values.get(), values.size());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView());
    renderer->setViewport(viewport);

    renderer->clearView(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView(), nullptr);
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(VertexTopology::triangles);

    BufferHandle vertexBuffers[] = { vertexArray1.handle(), instanceArray1.handle() };
    unsigned int vertexStrides[] = { (unsigned int)sizeof(VertexPosColorData), (unsigned int)sizeof(InstanceData) };
    unsigned int offsets[] = { 0,0 };
    renderer->bindVertexUniforms(0, camBuffer.handleArray(), size_t{ 1u });
    renderer->bindVertexArrayBuffers(0, size_t{ 2u }, vertexBuffers, vertexStrides, offsets);
    renderer->drawInstances(sizeof(instances1)/sizeof(*instances1), 0, sizeof(vertices1)/sizeof(*vertices1), 0);
    chain1.swapBuffersDiscard(true, depthBuffer.getDepthStencilView());
    renderer->flush();
  }


  // -- texture/light tests --

  static const uint8_t* textureLine128_A() noexcept {
    static int8_t line[] = {
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF' };
    return (const uint8_t*)line;
  }
  static const uint8_t* textureLine128_B() noexcept {
    static int8_t line[] = {
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF' };
    return (const uint8_t*)line;
  }
  static const uint8_t* textureLine64_A() noexcept {
    static int8_t line[] = {
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF',
      '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF', '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF' };
    return (const uint8_t*)line;
  }
  static const uint8_t* textureLine64_B() noexcept {
    static int8_t line[] = {
      '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF',
      '\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF','\xD0','\xB0','\xB0','\xFF', '\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF','\0','\0','\0','\xFF' };
    return (const uint8_t*)line;
  }

  static const char* __vertexInstanceTexLightShaderText() noexcept {
    return 
      "cbuffer CamLightData : register(b0)"
      "{"
      "  matrix projection;"
      "  float4 lightDiffuse;"
      "  float4 lightAmbient;"
      "  float4 lightSpecular;"
      "  float4 lightDirection;"
      "}"
      "struct VertexInputType {"
      "  float4 position : POSITION0;"
      "  float4 color : COLOR0;"
      "  float4 normal : NORMAL0;"
      "  float2 textureCoords : TEXCOORD0;"
      "  float3 instancePos : POSITION1;"
      "  float3 instanceColor : COLOR1;"
      "};"
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float3 color : COLOR0;"
      "  float4 normal : NORMAL0;"
      "  float2 textureCoords : TEXCOORD0;"
      "};"
      "PixelInputType VSMain(VertexInputType input) {"
      "  PixelInputType output;"
      "  output.position = float4(input.position.x + input.instancePos.x, input.position.y + input.instancePos.y, input.position.z + input.instancePos.z, input.position.w);"
      "  output.position = mul(projection, output.position);"
      "  if (input.position.z <= 0.5) { output.color = float4(input.color.x + input.instanceColor.x, input.color.y + input.instanceColor.y, input.color.z + input.instanceColor.z, input.color.w); }"
      "  else { output.color = float4((input.color.x + input.instanceColor.x)/4, (input.color.y + input.instanceColor.y)/4, (input.color.z + input.instanceColor.z)/4, input.color.w); }"
      "  output.normal = input.normal;"
      "  output.textureCoords = input.textureCoords;"
      "  return output;"
      "}";
  }
  static const char* __fragmentInstanceTexLightShaderText() noexcept {
    return 
      "cbuffer CamLightData : register(b0)"
      "{"
      "  matrix projection;"
      "  float4 lightDiffuse;"
      "  float4 lightAmbient;"
      "  float4 lightSpecular;"
      "  float4 lightDirection;"
      "}\n"
      "struct PixelInputType {"
      "  float4 position : SV_POSITION;"
      "  float3 color : COLOR0;"
      "  float4 normal : NORMAL0;"
      "  float2 textureCoords : TEXCOORD0;"
      "};\n"
      "SamplerState SamplerTrilinear : register(s0);\n"
      "Texture2D TextureDiffuse : register(t0);\n"
      "float4 ComputeLighting(float3 position, float3 normal)"
      "{"
      "  float3 unitVec = -lightDirection.xyz;\n"
      "  float4 diffuse = saturate(dot(unitVec, normal)) * lightDiffuse;\n"
      "  float3 viewDir = normalize(float4(0., 0., 0., 1.) - position); // cam pos - pos\n"
      "  float3 reflectDir = reflect(-unitVec, normal);\n"
      "  float4 specular = float4(pow(max(dot(viewDir, reflectDir), 0.0), lightSpecular.w) * lightSpecular.xyz, 1.0f); // lightSpec*materialSpec\n"
      "  return diffuse + lightAmbient + specular; // diffuse*materialDiffuse + lightAmbient*materialAmbient + specular\n"
      "}\n"
      "float4 PSMain(PixelInputType input) : SV_TARGET"
      "{"
      "  float4 diffuseColor = TextureDiffuse.Sample(SamplerTrilinear, input.textureCoords);"
      "  diffuseColor.x = input.color.x*diffuseColor.x*0.7 + input.color.x;"
      "  diffuseColor.y = input.color.y*diffuseColor.y*0.7 + input.color.y*0.9;"
      "  diffuseColor.z = input.color.z*diffuseColor.z*0.7 + input.color.z*0.9;"
      "  float4 directionalLight = ComputeLighting(input.position, input.normal);"
      "  return directionalLight*diffuseColor;"
      "}";
  }

  __align_type(16,
  struct VertexTexLightData final {
    float position[4];
    float color[4];
    float normal[4];
    float textureCoords[2];
  });
  __align_type(16,
  struct CamLightData final {
    DirectX::XMMATRIX projection;
    DirectX::XMFLOAT4 lightDiffuse;
    DirectX::XMFLOAT4 lightAmbient;
    DirectX::XMFLOAT4 lightSpecular;
    DirectX::XMFLOAT4 lightDirection;
  });

  TEST_F(D3d11RendererDrawTest, vertexInstanceTextureLightDrawingTest) {
    auto window = pandora::video::Window::Builder{}
      .setDisplayMode(pandora::video::WindowType::window, WindowBehavior::globalContext|WindowBehavior::topMost, 
        ResizeMode::fixed)
      .setSize(__WIDTH,__HEIGHT)
      .create(L"_DRAW_TEST4", L"Test");
    window->show();

    // renderer/swap-chain
    pandora::hardware::DisplayMonitor monitor;
    auto renderer = std::make_shared<Renderer>(monitor);
    RendererStateFactory factory(*renderer);

    SwapChain::Descriptor params;
    params.width = __WIDTH;
    params.height = __HEIGHT;
    params.framebufferCount = 2u;
    params.refreshRate = RefreshRate(60u, 1u);
    SwapChain chain1(renderer, window->handle(), params, DataFormat::rgba8_sRGB);
    ASSERT_FALSE(chain1.isEmpty());

    Viewport viewport(0,0, __WIDTH,__HEIGHT, 0.,1.);

    // shaders/input
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
      { "COLOR",    1, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    auto vertexShaderBuilder = Shader::Builder::compile(ShaderType::vertex, __vertexInstanceTexLightShaderText(), strlen(__vertexInstanceTexLightShaderText()), "VSMain");
    auto inputLayout = vertexShaderBuilder.createInputLayout(renderer->device(), inputLayoutDescr, sizeof(inputLayoutDescr)/sizeof(*inputLayoutDescr));
    auto vertexShader = vertexShaderBuilder.createShader(renderer->device());
    auto fragmentShader = Shader::Builder::compile(ShaderType::fragment, __fragmentInstanceTexLightShaderText(), strlen(__fragmentInstanceTexLightShaderText()), "PSMain")
      .createShader(renderer->device());
    ASSERT_TRUE(inputLayout.handle() != nullptr);
    ASSERT_FALSE(vertexShader.isEmpty());
    ASSERT_FALSE(fragmentShader.isEmpty());

    // states
    DepthStencilBuffer depthBuffer(*renderer, DepthStencilFormat::d32_f, __WIDTH,__HEIGHT);
    DepthStencilState depthState = factory.createDepthStencilTestState(DepthStencilParams{}); // default depth test (less)
    ASSERT_FALSE(depthBuffer.isEmpty());
    EXPECT_TRUE(depthState);

    RasterizerState rasterState(factory.createRasterizerState(RasterizerParams{})); // standard: back-cull, filled, clockwise, depth clipping
    EXPECT_TRUE(rasterState);

    FilterStateArray values;
    TextureWrap addrModes[3] { TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat };
    values.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear, addrModes)));
    ASSERT_EQ((size_t)1, values.size());

    // vertices
    VertexTexLightData vertices1[] = { 
      {{-0.1875f,0.1875f,0.5f,1.f},{0.5f,0.f,0.f,1.f},{0.f,0.f,-1.f,1.f},{0.f,0.f}}, {{0.1875f,0.1875f,0.5f,1.f},{0.f,0.5f,0.f,1.f},{0.f,0.f,-1.f,1.f},{1.f,0.f}}, {{-0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.5f,1.f},{0.f,0.f,-1.f,1.f},{0.f,1.f}},
      {{0.1875f,0.1875f,0.5f,1.f},{0.f,0.5f,0.f,1.f},{0.f,0.f,-1.f,1.f},{1.f,0.f}}, {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.5f,1.f},{0.f,0.f,-1.f,1.f},{1.f,1.f}}, {{-0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.5f,1.f},{0.f,0.f,-1.f,1.f},{0.f,1.f}},

      {{-0.1875f,0.1875f,0.85f,1.f},{0.2f,0.f,0.2f,1.f},{-1.f,0.f,0.f,1.f},{0.f,0.f}}, {{-0.1875f,0.1875f,0.5f,1.f},{0.4f,0.f,0.f,1.f},{-1.f,0.f,0.f,1.f},{1.f,0.f}}, {{-0.1875f,-0.1875f,0.85f,1.f},{0.2f,0.f,0.2f,1.f},{-1.f,0.f,0.f,1.f},{0.f,1.f}},
      {{-0.1875f,0.1875f,0.5f,1.f},{0.4f,0.f,0.f,1.f},{-1.f,0.f,0.f,1.f},{1.f,0.f}}, {{-0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.4f,1.f},{-1.f,0.f,0.f,1.f},{1.f,1.f}}, {{-0.1875f,-0.1875f,0.85f,1.f},{0.2f,0.f,0.2f,1.f},{-1.f,0.f,0.f,1.f},{0.f,1.f}},

      {{0.1875f,0.1875f,0.5f,1.f},{0.f,0.4f,0.f,1.f},{1.f,0.f,0.f,1.f},{0.f,0.f}}, {{0.1875f,0.1875f,0.85f,1.f},{0.f,0.2f,0.2f,1.f},{1.f,0.f,0.f,1.f},{1.f,0.f}}, {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.4f,1.f},{1.f,0.f,0.f,1.f},{0.f,1.f}},
      {{0.1875f,0.1875f,0.85f,1.f},{0.f,0.2f,0.2f,1.f},{1.f,0.f,0.f,1.f},{1.f,0.f}}, {{0.1875f,-0.1875f,0.85f,1.f},{0.f,0.2f,0.2f,1.f},{1.f,0.f,0.f,1.f},{1.f,1.f}}, {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.4f,1.f},{1.f,0.f,0.f,1.f},{0.f,1.f}},

      {{-0.1875f,0.1875f,0.85f,1.f},{0.2f,0.2f,0.f,1.f},{0.f,1.f,0.f,1.f},{0.f,0.f}}, {{0.1875f,0.1875f,0.85f,1.f},{0.2f,0.2f,0.f,1.f},{0.f,1.f,0.f,1.f},{1.f,0.f}}, {{-0.1875f,0.1875f,0.5f,1.f},{0.4f,0.f,0.f,1.f},{0.f,1.f,0.f,1.f},{0.f,1.f}},
      {{0.1875f,0.1875f,0.85f,1.f},{0.2f,0.2f,0.f,1.f},{0.f,1.f,0.f,1.f},{1.f,0.f}}, {{0.1875f,0.1875f,0.5f,1.f},{0.f,0.4f,0.f,1.f},{0.f,1.f,0.f,1.f},{1.f,1.f}}, {{-0.1875f,0.1875f,0.5f,1.f},{0.4f,0.f,0.f,1.f},{0.f,1.f,0.f,1.f},{0.f,1.f}}, 

      {{-0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.25f,1.f},{0.f,-1.f,0.f,1.f},{0.f,0.f}}, {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.25f,1.f},{0.f,-1.f,0.f,1.f},{1.f,0.f}}, {{-0.1875f,-0.1875f,0.85f,1.f},{0.f,0.1f,0.1f,1.f},{0.f,-1.f,0.f,1.f},{0.f,1.f}},
      {{0.1875f,-0.1875f,0.5f,1.f},{0.f,0.f,0.25f,1.f},{0.f,-1.f,0.f,1.f},{1.f,0.f}}, {{0.1875f,-0.1875f,0.85f,1.f},{0.1f,0.f,0.1f,1.f},{0.f,-1.f,0.f,1.f},{1.f,1.f}}, {{-0.1875f,-0.1875f,0.85f,1.f},{0.f,0.1f,0.1f,1.f},{0.f,-1.f,0.f,1.f},{0.f,1.f}}
    };
    StaticBuffer vertexArray1(*renderer, BaseBufferType::vertex, sizeof(vertices1), (const void*)vertices1, true);
    InstanceData instances1[] = { 
      {{-0.4f,-0.4f,0.f},{0.5f,0.f,0.f}}, {{-0.4f,0.4f,0.f},{0.f,0.5f,0.f}}, {{0.4f,-0.4f,0.f}, {0.f,0.f,0.5f}}, {{0.4f,0.4f,0.f}, {0.2f,0.f,0.2f}},
      {{-0.4f,-0.4f,0.7f},{0.f,0.4f,0.f}}, {{-0.4f,0.4f,0.7f},{0.f,0.f,0.4f}}, {{0.4f,-0.4f,0.7f}, {0.4f,0.f,0.f}}, {{0.4f,0.4f,0.7f}, {0.f,0.1f,0.3f}},
      {{-0.4f,-0.4f,1.4f},{0.f,0.f,0.3f}}, {{-0.4f,0.4f,1.4f},{0.3f,0.f,0.f}}, {{0.4f,-0.4f,1.4f}, {0.f,0.3f,0.f}}, {{0.4f,0.4f,1.4f}, {0.1f,0.f,0.3f}},
      {{-0.4f,-0.4f,2.1f},{0.2f,0.f,0.f}}, {{-0.4f,0.4f,2.1f},{0.f,0.2f,0.f}}, {{0.4f,-0.4f,2.1f}, {0.f,0.f,0.2f}}, {{0.4f,0.4f,2.1f}, {0.f,0.05f,0.1f}},
    };
    StaticBuffer instanceArray1(*renderer, BaseBufferType::vertex, sizeof(instances1), (const void*)instances1, true);

    // texture
    auto image2D = std::unique_ptr<uint8_t[]>(new uint8_t[128 * 128 * 4]());
    for (int i = 0; i < 128; ++i)
      memcpy(&image2D[128*4*i], ((i >> 3) & 0x1) ? textureLine128_A() : textureLine128_B(), 128*4);
    auto image2Dmip = std::unique_ptr<uint8_t[]>(new uint8_t[64 * 64 * 4]());
    for (int i = 0; i < 64; ++i)
      memcpy(&image2Dmip[64*4*i], ((i >> 2) & 0x1) ? textureLine64_A() : textureLine64_B(), 64*4);
    const uint8_t* image2Ddata[] = { &image2D[0], &image2Dmip[0] };

    Texture2DParams tex2Dparams(128u, 128u, DataFormat::rgba8_sRGB, 1u, 2u, 0, ResourceUsage::staticGpu);
    Texture2D tex2D(*renderer, tex2Dparams, image2Ddata);
    EXPECT_TRUE(tex2D.handle() != nullptr);
    EXPECT_TRUE(tex2D.resourceView() != nullptr);
    auto tex2Dview = tex2D.resourceView();

    // camera
    CameraProjection proj(__WIDTH, __HEIGHT, 70.f);
    CamLightData camLightData{ proj.projectionMatrix(),
                               DirectX::XMFLOAT4(1.1f, 1.1f, 0.8f, 1.0f),
                               DirectX::XMFLOAT4(0.1f, 0.25f, 0.25f, 0.0f),
                               DirectX::XMFLOAT4(0.25f, 0.2f, 0.1f, 32.0f),
                               DirectX::XMFLOAT4(-0.2f, -0.5f, 0.5f, 1.0f) };
    StaticBuffer camBuffer(*renderer, BaseBufferType::uniform, sizeof(CamLightData), &camLightData, true);

    // drawing
    renderer->setRasterizerState(rasterState);
    renderer->setDepthStencilState(depthState);
    renderer->setFragmentFilterStates(0, values.get(), values.size());
    renderer->setActiveRenderTarget(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView());
    renderer->setViewport(viewport);
    renderer->setScissorRectangles(nullptr, 0);

    renderer->clearView(chain1.getRenderTargetView(), depthBuffer.getDepthStencilView(), nullptr);
    renderer->bindInputLayout(inputLayout.handle());
    renderer->bindVertexShader(vertexShader.handle());
    renderer->bindFragmentShader(fragmentShader.handle());
    renderer->setVertexTopology(VertexTopology::triangles);

    BufferHandle vertexBuffers[] = { vertexArray1.handle(), instanceArray1.handle() };
    unsigned int vertexStrides[] = { (unsigned int)sizeof(VertexTexLightData), (unsigned int)sizeof(InstanceData) };
    unsigned int offsets[] = { 0,0 };
    renderer->bindVertexUniforms(0, camBuffer.handleArray(), size_t{ 1u });
    renderer->bindFragmentUniforms(0, camBuffer.handleArray(), size_t{ 1u });
    renderer->bindVertexArrayBuffers(0, size_t{ 2u }, vertexBuffers, vertexStrides, offsets);
    renderer->bindFragmentTextures(0, &tex2Dview, size_t{ 1u });
    renderer->drawInstances(sizeof(instances1)/sizeof(*instances1), 0, sizeof(vertices1)/sizeof(*vertices1), 0);
    chain1.swapBuffersDiscard(true, depthBuffer.getDepthStencilView());
    renderer->flush();
  }

# ifndef __MINGW32__
#   pragma warning(pop)
# endif
#endif
#endif
