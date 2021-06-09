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

  class ShaderTest : public testing::Test {
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

  TEST_F(ShaderTest, compileBindFromBinaryFile) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor, Renderer::DeviceLevel::direct3D_11_0);
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
    Shader::Builder binaryBuilder(pandora::video::ShaderType::vertex, (const uint8_t*)binaryFileBuffer, (size_t)binaryFileSize);
    auto binaryShader = binaryBuilder.createShader(renderer.device());
    EXPECT_TRUE(binaryShader.handle() != nullptr);
    EXPECT_TRUE(binaryShader.type() == pandora::video::ShaderType::vertex);
    EXPECT_FALSE(binaryShader.isEmpty());
    
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",   0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto inputLayout = binaryBuilder.createInputLayout(renderer.device(), (Shader::InputElementDescArray)inputLayoutDescr, (size_t)2u);
    EXPECT_TRUE(inputLayout.handle() != nullptr);
    
    renderer.bindInputLayout(inputLayout.handle());
    renderer.bindVertexShader(binaryShader.handle());
  }

  // ---
  
  TEST_F(ShaderTest, compileBindFromTextFiles) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor, Renderer::DeviceLevel::direct3D_11_0);
    ASSERT_TRUE(renderer.device() != nullptr);

    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",   0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto vertexBuilder = Shader::Builder::compileFromFile(pandora::video::ShaderType::vertex, __createPath(L"/d3d11/test_vertex.vs.hlsl"), "VSMain");
    auto vertexShader = vertexBuilder.createShader(renderer.device());
    auto inputLayout = vertexBuilder.createInputLayout(renderer.device(), (Shader::InputElementDescArray)inputLayoutDescr, (size_t)2u);
    EXPECT_TRUE(vertexShader.handle() != nullptr);
    EXPECT_TRUE(vertexShader.type() == pandora::video::ShaderType::vertex);
    EXPECT_FALSE(vertexShader.isEmpty());
    EXPECT_TRUE(inputLayout.handle() != nullptr);
    renderer.bindInputLayout(inputLayout.handle());
    renderer.bindVertexShader(vertexShader.handle());
    renderer.bindVertexShader(nullptr);
    
    auto fragmentShader = Shader::Builder::compileFromFile(pandora::video::ShaderType::fragment, __createPath(L"/d3d11/test_fragment.ps.hlsl"), "PSMain")
                          .createShader(renderer.device());
    EXPECT_TRUE(fragmentShader.handle() != nullptr);
    EXPECT_TRUE(fragmentShader.type() == pandora::video::ShaderType::fragment);
    EXPECT_FALSE(fragmentShader.isEmpty());
    renderer.bindFragmentShader(fragmentShader.handle());
    renderer.bindFragmentShader(nullptr);
    
    auto geometryShader = Shader::Builder::compileFromFile(pandora::video::ShaderType::geometry, __createPath(L"/d3d11/test_geometry.gs.hlsl"), "GSMain")
                          .createShader(renderer.device());
    EXPECT_TRUE(geometryShader.handle() != nullptr);
    EXPECT_TRUE(geometryShader.type() == pandora::video::ShaderType::geometry);
    EXPECT_FALSE(geometryShader.isEmpty());
    renderer.bindGeometryShader(geometryShader.handle());
    renderer.bindGeometryShader(nullptr);

    auto computeShader = Shader::Builder::compileFromFile(pandora::video::ShaderType::compute, __createPath(L"/d3d11/test_compute.cs.hlsl"), "CSMain")
                         .createShader(renderer.device());
    EXPECT_TRUE(computeShader.handle() != nullptr);
    EXPECT_TRUE(computeShader.type() == pandora::video::ShaderType::compute);
    EXPECT_FALSE(computeShader.isEmpty());
    renderer.bindComputeShader(computeShader.handle());
    renderer.bindComputeShader(nullptr);
    
    auto controlShader = Shader::Builder::compileFromFile(pandora::video::ShaderType::tesselControl, __createPath(L"/d3d11/test_control.hs.hlsl"), "HSMain")
                         .createShader(renderer.device());
    EXPECT_TRUE(controlShader.handle() != nullptr);
    EXPECT_TRUE(controlShader.type() == pandora::video::ShaderType::tesselControl);
    EXPECT_FALSE(controlShader.isEmpty());
    renderer.bindTesselControlShader(controlShader.handle());
    renderer.bindTesselControlShader(nullptr);
    
    auto evalShader = Shader::Builder::compileFromFile(pandora::video::ShaderType::tesselEval, __createPath(L"/d3d11/test_eval.ds.hlsl"), "DSMain")
                      .createShader(renderer.device());
    EXPECT_TRUE(evalShader.handle() != nullptr);
    EXPECT_TRUE(evalShader.type() == pandora::video::ShaderType::tesselEval);
    EXPECT_FALSE(evalShader.isEmpty());
    renderer.bindTesselEvalShader(evalShader.handle());
    renderer.bindTesselEvalShader(nullptr);
    
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
    auto vertexTextBuilder = Shader::Builder::compile(pandora::video::ShaderType::vertex, vertexShaderText, strlen(vertexShaderText), "VSMain");
    auto vertexShader2 = vertexTextBuilder.createShader(renderer.device());
    auto inputLayout2 = vertexTextBuilder.createInputLayout(renderer.device(), (Shader::InputElementDescArray)inputLayoutDescr, (size_t)2u);
    EXPECT_TRUE(vertexShader2.handle() != nullptr);
    EXPECT_TRUE(vertexShader2.type() == pandora::video::ShaderType::vertex);
    EXPECT_FALSE(vertexShader2.isEmpty());
    EXPECT_TRUE(inputLayout2.handle() != nullptr);
    renderer.bindInputLayout(inputLayout2.handle());
    renderer.bindVertexShader(vertexShader2.handle());
    renderer.bindVertexShader(nullptr);
  }

#endif
