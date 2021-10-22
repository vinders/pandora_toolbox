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
# if defined(_WINDOWS)
#   ifndef __MINGW32__
#     pragma warning(push)
#     pragma warning(disable: 26812) // disable warnings about vulkan enums
#   endif
# endif
# include <gtest/gtest.h>
# include <cstddef>
# include <iostream>
# include <fstream>
# include <string>
# include <video/vulkan/renderer.h>
# include <video/vulkan/shader.h>

# if defined(_WIN64) || defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_AMD64_) || defined(_M_X64) || defined(_M_I64) || defined(_M_IX64) || defined(_M_AMD64) || defined(__IA64__) || defined(_IA64_)
#   define __P_COMPILED_SHADER "test_vertex64.spv"
# else
#   define __P_COMPILED_SHADER "test_vertex32.spv"
# endif

  using namespace pandora::video::vulkan;

  class VulkanShaderTest : public testing::Test {
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

  TEST_F(VulkanShaderTest, compileFromBinaryFile) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    ASSERT_TRUE(renderer.device() != nullptr);
    
    Shader emptyShader;
    EXPECT_TRUE(emptyShader.handle() == nullptr);
    EXPECT_TRUE(emptyShader.isEmpty());
    
    std::ifstream binaryFile(_P_TEST_RESOURCE_DIR "/vulkan/" __P_COMPILED_SHADER, std::fstream::in|std::fstream::binary);
    ASSERT_TRUE(binaryFile.is_open());
    binaryFile.seekg(0, binaryFile.end);
    int binaryFileSize = (int)binaryFile.tellg();
    binaryFile.seekg(0, binaryFile.beg);
    
    char* binaryFileBuffer = new char[binaryFileSize];
    binaryFile.read(binaryFileBuffer, binaryFileSize);
    binaryFile.close();
    Shader::Builder binaryBuilder(ShaderType::vertex, (const uint8_t*)binaryFileBuffer, (size_t)binaryFileSize, "main");
    auto binaryShader = binaryBuilder.createShader(renderer.resourceManager());
    EXPECT_TRUE(binaryShader.handle() != nullptr);
    EXPECT_TRUE(binaryShader.type() == ShaderType::vertex);
    EXPECT_FALSE(binaryShader.isEmpty());
    
    VkVertexInputBindingDescription inputBindings{ 0, 5*sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX };
    VkVertexInputAttributeDescription layoutAttributes[] = {
      { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
      { 1, 0, VK_FORMAT_R32G32_SFLOAT,    3*sizeof(float) }};
    auto inputLayout = binaryBuilder.createInputLayout(&inputBindings, 1, layoutAttributes, sizeof(layoutAttributes)/sizeof(*layoutAttributes));
    EXPECT_TRUE(inputLayout.handle() != nullptr);
  }

  // ---
  
# ifdef _P_VIDEO_SHADER_COMPILERS
    TEST_F(VulkanShaderTest, compileFromTextFiles) {
      pandora::hardware::DisplayMonitor monitor;
      Renderer renderer(monitor);
      ASSERT_TRUE(renderer.device() != nullptr);

      VkVertexInputBindingDescription inputBindings{ 0, 8*sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX };
      VkVertexInputAttributeDescription layoutAttributes[] = {
        { 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },
        { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 4*sizeof(float) }};
      auto vertexBuilder = Shader::Builder::compileFromFile(ShaderType::vertex, __createPath(L"/vulkan/test_vertex.vert"), "VSMain");
      auto vertexShader = vertexBuilder.createShader(renderer.resourceManager());
      auto inputLayout = vertexBuilder.createInputLayout(&inputBindings, 1, layoutAttributes, sizeof(layoutAttributes)/sizeof(*layoutAttributes));
      EXPECT_TRUE(vertexShader.handle() != nullptr);
      EXPECT_TRUE(vertexShader.type() == ShaderType::vertex);
      EXPECT_FALSE(vertexShader.isEmpty());
      EXPECT_TRUE(inputLayout.handle() != nullptr);
    
      auto fragmentShader = Shader::Builder::compileFromFile(ShaderType::fragment, __createPath(L"/vulkan/test_fragment.frag"), "PSMain")
                            .createShader(renderer.resourceManager());
      EXPECT_TRUE(fragmentShader.handle() != nullptr);
      EXPECT_TRUE(fragmentShader.type() == ShaderType::fragment);
      EXPECT_FALSE(fragmentShader.isEmpty());

      auto geometryShader = Shader::Builder::compileFromFile(ShaderType::geometry, __createPath(L"/vulkan/test_geometry.geom"), "GSMain")
                            .createShader(renderer.resourceManager());
      EXPECT_TRUE(geometryShader.handle() != nullptr);
      EXPECT_TRUE(geometryShader.type() == ShaderType::geometry);
      EXPECT_FALSE(geometryShader.isEmpty());
      renderer.bindGeometryShader(geometryShader.handle());
      renderer.bindGeometryShader(nullptr);

      auto computeShader = Shader::Builder::compileFromFile(ShaderType::compute, __createPath(L"/vulkan/test_compute.comp"), "CSMain")
                           .createShader(renderer.resourceManager());
      EXPECT_TRUE(computeShader.handle() != nullptr);
      EXPECT_TRUE(computeShader.type() == ShaderType::compute);
      EXPECT_FALSE(computeShader.isEmpty());

      auto controlShader = Shader::Builder::compileFromFile(ShaderType::tessCtrl, __createPath(L"/vulkan/test_control.tesc"), "HSMain")
                           .createShader(renderer.resourceManager());
      EXPECT_TRUE(controlShader.handle() != nullptr);
      EXPECT_TRUE(controlShader.type() == ShaderType::tessCtrl);
      EXPECT_FALSE(controlShader.isEmpty());

      auto evalShader = Shader::Builder::compileFromFile(ShaderType::tessEval, __createPath(L"/vulkan/test_eval.tese"), "DSMain")
                        .createShader(renderer.resourceManager());
      EXPECT_TRUE(evalShader.handle() != nullptr);
      EXPECT_TRUE(evalShader.type() == ShaderType::tessEval);
      EXPECT_FALSE(evalShader.isEmpty());

      EXPECT_ANY_THROW(Shader::Builder::compileFromFile(ShaderType::fragment, __createPath(L"/nodirectory/blabla_--_.ext"), "PSMain"));

      const char* vertexShaderText =
        "#version 450\n\n"
        "void VSMain() {\n"
        "  gl_Position = vec4(1.0, 1.0, 0.0, 1.0);\n"
        "}";
      auto vertexTextBuilder = Shader::Builder::compile(ShaderType::vertex, vertexShaderText, strlen(vertexShaderText), "VSMain");
      auto vertexShader2 = vertexTextBuilder.createShader(renderer.resourceManager());
      auto inputLayout2 = vertexTextBuilder.createInputLayout(&inputBindings, 1, layoutAttributes, sizeof(layoutAttributes)/sizeof(*layoutAttributes));
      EXPECT_TRUE(vertexShader2.handle() != nullptr);
      EXPECT_TRUE(vertexShader2.type() == ShaderType::vertex);
      EXPECT_FALSE(vertexShader2.isEmpty());
      EXPECT_TRUE(inputLayout2.handle() != nullptr);
    }
# endif

# if defined(_WINDOWS) && !defined(__MINGW32__)
#   pragma warning(pop)
# endif
#endif
