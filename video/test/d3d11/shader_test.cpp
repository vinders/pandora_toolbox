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
# include <cstddef>
# include <iostream>
# include <fstream>
# include <string>
# include <video/d3d11/api/d3d_11.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/shader.h>

# if defined(_WIN64) || defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_AMD64_) || defined(_M_X64) || defined(_M_I64) || defined(_M_IX64) || defined(_M_AMD64) || defined(__IA64__) || defined(_IA64_)
#   define __P_COMPILED_SHADER "test_vertex64.cso"
# else
#   define __P_COMPILED_SHADER "test_vertex32.cso"
# endif

  using namespace pandora::video::d3d11;

  class D3d11ShaderTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };

  static const wchar_t* __createPath(const wchar_t* suffix) {
     static const char* prefix = _P_TEST_RESOURCE_DIR;
     static std::wstring path;
     path.clear();

     for (const char* it = prefix; *it; ++it)
       path += (wchar_t)*it;
     path += suffix;
     return path.c_str();
  }


  // -- create/bind shaders --

  TEST_F(D3d11ShaderTest, compileBindFromBinaryFile) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    ASSERT_TRUE(renderer.device() != nullptr);
    
    Shader emptyShader;
    EXPECT_TRUE(emptyShader.handle() == nullptr);
    EXPECT_TRUE(emptyShader.isEmpty());
    
    std::ifstream binaryFile(_P_TEST_RESOURCE_DIR "/d3d11/" __P_COMPILED_SHADER, std::fstream::in|std::fstream::binary);
    ASSERT_TRUE(binaryFile.is_open());
    binaryFile.seekg(0, binaryFile.end);
    int binaryFileSize = (int)binaryFile.tellg();
    binaryFile.seekg(0, binaryFile.beg);
    
    char* binaryFileBuffer = new char[binaryFileSize];
    binaryFile.read(binaryFileBuffer, binaryFileSize);
    binaryFile.close();
    Shader::Builder binaryBuilder(ShaderType::vertex, (const uint8_t*)binaryFileBuffer, (size_t)binaryFileSize);
    auto binaryShader = binaryBuilder.createShader(renderer.resourceManager());
    EXPECT_TRUE(binaryShader.handle() != nullptr);
    EXPECT_TRUE(binaryShader.type() == ShaderType::vertex);
    EXPECT_FALSE(binaryShader.isEmpty());
    
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",   0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto inputLayout = binaryBuilder.createInputLayout(renderer.resourceManager(), inputLayoutDescr, (size_t)2u);
    EXPECT_TRUE(inputLayout.hasValue());
  }

  // ---
  
# ifdef _P_VIDEO_SHADER_COMPILERS
    TEST_F(D3d11ShaderTest, compileBindFromTextFiles) {
      pandora::hardware::DisplayMonitor monitor;
      std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
      ASSERT_TRUE(renderer->device() != nullptr);
      GraphicsPipeline::Builder pipelineBuilder(renderer);

      D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",   0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
      };
      auto vertexBuilder = Shader::Builder::compileFromFile(ShaderType::vertex, __createPath(L"/d3d11/test_vertex.vs.hlsl"), "VSMain");
      auto vertexShader = vertexBuilder.createShader(renderer->resourceManager());
      auto inputLayout = vertexBuilder.createInputLayout(renderer->resourceManager(), inputLayoutDescr, (size_t)2u);
      EXPECT_TRUE(vertexShader.handle() != nullptr);
      EXPECT_TRUE(vertexShader.type() == ShaderType::vertex);
      EXPECT_FALSE(vertexShader.isEmpty());
      EXPECT_TRUE(inputLayout.hasValue());
      pipelineBuilder.setInputLayout(inputLayout);
      pipelineBuilder.attachShaderStage(vertexShader);
    
      auto fragmentShader = Shader::Builder::compileFromFile(ShaderType::fragment, __createPath(L"/d3d11/test_fragment.ps.hlsl"), "PSMain")
                            .createShader(renderer->resourceManager());
      EXPECT_TRUE(fragmentShader.handle() != nullptr);
      EXPECT_TRUE(fragmentShader.type() == ShaderType::fragment);
      EXPECT_FALSE(fragmentShader.isEmpty());
      pipelineBuilder.attachShaderStage(fragmentShader);
    
#     ifndef __P_DISABLE_GEOMETRY_STAGE
        auto geometryShader = Shader::Builder::compileFromFile(ShaderType::geometry, __createPath(L"/d3d11/test_geometry.gs.hlsl"), "GSMain")
                              .createShader(renderer->resourceManager());
        EXPECT_TRUE(geometryShader.handle() != nullptr);
        EXPECT_TRUE(geometryShader.type() == ShaderType::geometry);
        EXPECT_FALSE(geometryShader.isEmpty());
        pipelineBuilder.attachShaderStage(geometryShader);
#     endif
      auto computeShader = Shader::Builder::compileFromFile(ShaderType::compute, __createPath(L"/d3d11/test_compute.cs.hlsl"), "CSMain")
                           .createShader(renderer->resourceManager());
      EXPECT_TRUE(computeShader.handle() != nullptr);
      EXPECT_TRUE(computeShader.type() == ShaderType::compute);
      EXPECT_FALSE(computeShader.isEmpty());
      renderer->bindComputeShader(computeShader.handle());
      renderer->bindComputeShader(nullptr);
    
#     ifndef __P_DISABLE_TESSELLATION_STAGE
        auto controlShader = Shader::Builder::compileFromFile(ShaderType::tessCtrl, __createPath(L"/d3d11/test_control.hs.hlsl"), "HSMain")
                             .createShader(renderer->resourceManager());
        EXPECT_TRUE(controlShader.handle() != nullptr);
        EXPECT_TRUE(controlShader.type() == ShaderType::tessCtrl);
        EXPECT_FALSE(controlShader.isEmpty());
        pipelineBuilder.attachShaderStage(controlShader);
    
        auto evalShader = Shader::Builder::compileFromFile(ShaderType::tessEval, __createPath(L"/d3d11/test_eval.ds.hlsl"), "DSMain")
                          .createShader(renderer->resourceManager());
        EXPECT_TRUE(evalShader.handle() != nullptr);
        EXPECT_TRUE(evalShader.type() == ShaderType::tessEval);
        EXPECT_FALSE(evalShader.isEmpty());
        pipelineBuilder.attachShaderStage(evalShader);
#     endif

      pipelineBuilder.setRenderTargetFormat(nullptr, 0);//TODO
      pipelineBuilder.setInputLayout(inputLayout);
      pipelineBuilder.setVertexTopology(VertexTopology::triangles);
      pipelineBuilder.setRasterizerState(RasterizerParams(*renderer, CullMode::cullBack, FillMode::fill, true, false, false));
      pipelineBuilder.setDepthStencilState(DepthStencilParams(StencilCompare::less, StencilOp::incrementWrap, StencilOp::replace,
                                                              StencilOp::decrementWrap, StencilOp::invert), 1u);
      pipelineBuilder.setBlendState(BlendParams(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add,
                                                BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add));
      auto pipeline = pipelineBuilder.build();
      renderer->bindGraphicsPipeline(pipeline.handle());
      renderer->bindGraphicsPipeline(nullptr);
    
      const char* vertexShaderText = "cbuffer MatrixBuffer {"
        "  matrix worldMatrix;"
        "  matrix viewMatrix;"
        "  matrix projectionMatrix;"
        "};"
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
        "  input.position.w = 1.0f;"
        "  output.position = mul(input.position, worldMatrix);"
        "  output.position = mul(output.position, viewMatrix);"
        "  output.position = mul(output.position, projectionMatrix);"
        "  output.color = input.color;"
        "  return output;"
        "}";
      auto vertexTextBuilder = Shader::Builder::compile(ShaderType::vertex, vertexShaderText, strlen(vertexShaderText), "VSMain");
      auto vertexShader2 = vertexTextBuilder.createShader(renderer->resourceManager());
      auto inputLayout2 = vertexTextBuilder.createInputLayout(renderer->resourceManager(), inputLayoutDescr, (size_t)2u);
      EXPECT_TRUE(vertexShader2.handle() != nullptr);
      EXPECT_TRUE(vertexShader2.type() == ShaderType::vertex);
      EXPECT_FALSE(vertexShader2.isEmpty());
      EXPECT_TRUE(inputLayout2.hasValue());
      pipelineBuilder.setInputLayout(inputLayout2);
      pipelineBuilder.attachShaderStage(vertexShader2);

      pipeline = pipelineBuilder.build();
      renderer->bindGraphicsPipeline(pipeline.handle());
      renderer->bindGraphicsPipeline(nullptr);
    }
# endif
#endif
