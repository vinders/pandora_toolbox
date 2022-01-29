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
# include <fstream>
# include <video/vulkan/graphics_pipeline.h>

# if defined(_WIN64) || defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_AMD64_) || defined(_M_X64) || defined(_M_I64) || defined(_M_IX64) || defined(_M_AMD64) || defined(__IA64__) || defined(_IA64_)
#   define __P_COMPILED_SHADER "test_vertex64.spv"
# else
#   define __P_COMPILED_SHADER "test_vertex32.spv"
# endif

  using namespace pandora::video::vulkan;

  class VulkanGraphicsPipelineTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- Rasterizer/depth-stencil/blend state management --

  TEST_F(VulkanGraphicsPipelineTest, createRasterStatesTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
    GraphicsPipeline::Builder builder(renderer);

    EXPECT_ANY_THROW(GraphicsPipeline::Builder(nullptr));

    // rasterizer state
    RasterizerParams paramsR1(CullMode::none, FillMode::lines, false, true, false);
    builder.setRasterizerState(paramsR1);
    EXPECT_TRUE(builder.descriptor().pRasterizationState != nullptr);
    builder.descriptor().pRasterizationState = nullptr;
    RasterizerParams paramsR2(CullMode::cullBack, FillMode::fill, true, true, true);
    builder.setRasterizerState(paramsR2);
    EXPECT_TRUE(builder.descriptor().pRasterizationState != nullptr);
    builder.descriptor().pRasterizationState = nullptr;
    builder.setRasterizerState(RasterizerParams{});
    EXPECT_TRUE(builder.descriptor().pRasterizationState != nullptr);
  }

  TEST_F(VulkanGraphicsPipelineTest, createDepthStatesTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
    GraphicsPipeline::Builder builder(renderer);

    DepthStencilParams paramsD1(StencilCompare::less, StencilOp::incrementWrap, StencilOp::replace,
                                StencilOp::decrementWrap, StencilOp::invert);
    builder.setDepthStencilState(paramsD1);
    EXPECT_TRUE(builder.descriptor().pDepthStencilState != nullptr);
    builder.descriptor().pDepthStencilState = nullptr;
    DepthStencilParams paramsD2(StencilCompare::greaterEqual, StencilOp::zero, StencilOp::replace,
                                StencilOp::zero, StencilOp::invert);
    builder.setDepthStencilState(paramsD2);
    EXPECT_TRUE(builder.descriptor().pDepthStencilState != nullptr);
    builder.descriptor().pDepthStencilState = nullptr;
    DepthStencilParams paramsS1(StencilCompare::greater, StencilCompare::less, StencilOp::incrementWrap,
                                StencilOp::replace, StencilOp::decrementWrap, StencilOp::invert);
    builder.setDepthStencilState(paramsS1);
    EXPECT_TRUE(builder.descriptor().pDepthStencilState != nullptr);
    builder.descriptor().pDepthStencilState = nullptr;
    DepthStencilParams paramsS2(StencilCompare::always, StencilCompare::notEqual, StencilOp::zero,
                                StencilOp::replace, StencilOp::zero, StencilOp::invert);
    builder.setDepthStencilState(paramsS2);
    EXPECT_TRUE(builder.descriptor().pDepthStencilState != nullptr);
    builder.descriptor().pDepthStencilState = nullptr;
    DepthStencilParams paramsDS1(StencilCompare::less, StencilCompare::less, StencilCompare::greater, StencilOp::zero,
                         StencilOp::incrementWrap, StencilOp::keep, StencilOp::zero, StencilOp::decrementClamp, StencilOp::keep);
    builder.setDepthStencilState(paramsDS1);
    EXPECT_TRUE(builder.descriptor().pDepthStencilState != nullptr);
    builder.descriptor().pDepthStencilState = nullptr;
    DepthStencilParams paramsDS2(StencilCompare::greater, StencilCompare::always, StencilCompare::always, StencilOp::keep,
                         StencilOp::decrementClamp, StencilOp::replace, StencilOp::keep, StencilOp::incrementClamp, StencilOp::invert);
    builder.setDepthStencilState(paramsDS2);
    EXPECT_TRUE(builder.descriptor().pDepthStencilState != nullptr);
    builder.descriptor().pDepthStencilState = nullptr;
    builder.setDepthStencilState(DepthStencilParams{});
    EXPECT_TRUE(builder.descriptor().pDepthStencilState != nullptr);
    builder.clearDepthStencilState();
    EXPECT_TRUE(builder.descriptor().pDepthStencilState == nullptr);
  }

  TEST_F(VulkanGraphicsPipelineTest, createBlendingStatesTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
    GraphicsPipeline::Builder builder(renderer);
    const FLOAT color[4] = { 0.f,0.f,0.f,1.f };

    BlendParams paramsB1(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add,
                         BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add);
    builder.setBlendState(paramsB1);
    EXPECT_TRUE(builder.descriptor().pColorBlendState != nullptr);
    builder.descriptor().pColorBlendState = nullptr;
    BlendParams paramsB2(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::revSubtract,
                         BlendFactor::one, BlendFactor::zero, BlendOp::maximum);
    builder.setBlendState(paramsB2);
    EXPECT_TRUE(builder.descriptor().pColorBlendState != nullptr);
    builder.descriptor().pColorBlendState = nullptr;
    BlendParams paramsB3(BlendFactor::constantColor, BlendFactor::constantInvColor, BlendOp::subtract,
                         BlendFactor::constantAlpha, BlendFactor::constantInvAlpha, BlendOp::subtract);
    builder.setBlendState(paramsB3);
    EXPECT_TRUE(builder.descriptor().pColorBlendState != nullptr);
    builder.descriptor().pColorBlendState = nullptr;

    // blend state per target
    BlendPerTargetParams paramsBpt1;
    EXPECT_EQ(1.f, paramsBpt1.blendConstant()[0]);
    EXPECT_EQ(1.f, paramsBpt1.blendConstant()[1]);
    EXPECT_EQ(1.f, paramsBpt1.blendConstant()[2]);
    EXPECT_EQ(1.f, paramsBpt1.blendConstant()[3]);
    paramsBpt1.setTargetBlend(0, BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add,
                                 BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add);
    paramsBpt1.setTargetBlend(1, BlendFactor::sourceInvColor, BlendFactor::destColor, BlendOp::subtract,
                                 BlendFactor::sourceInvAlpha, BlendFactor::destAlpha, BlendOp::subtract);
    paramsBpt1.setTargetBlend(2, BlendFactor::one, BlendFactor::zero, BlendOp::add, BlendFactor::one, BlendFactor::zero, BlendOp::add);
    paramsBpt1.disableTargetBlend(2);
    paramsBpt1.blendConstant(color);
    EXPECT_EQ(color[0], paramsBpt1.blendConstant()[0]);
    EXPECT_EQ(color[1], paramsBpt1.blendConstant()[1]);
    EXPECT_EQ(color[2], paramsBpt1.blendConstant()[2]);
    EXPECT_EQ(color[3], paramsBpt1.blendConstant()[3]);
    builder.setBlendState(paramsBpt1);
    EXPECT_TRUE(builder.descriptor().pColorBlendState != nullptr);
    BlendPerTargetParams paramsBpt2;
    paramsBpt2.setTargetBlend(0, BlendFactor::sourceColor, BlendFactor::destColor, BlendOp::minimum,
                                 BlendFactor::one, BlendFactor::one, BlendOp::maximum);
    builder.setBlendState(paramsBpt2);
    EXPECT_TRUE(builder.descriptor().pColorBlendState != nullptr);
    builder.descriptor().pColorBlendState = nullptr;
    builder.setBlendState(BlendPerTargetParams{});
    EXPECT_TRUE(builder.descriptor().pColorBlendState != nullptr);
  }

  TEST_F(VulkanGraphicsPipelineTest, pipelineStateParamsTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);

    // rasterizer
    RasterizerParams raster;
    raster.vertexOrder(false).cullMode(CullMode::cullFront).fillMode(FillMode::lines)
          .depthClipping(false).scissorClipping(true).depthBias(1, 2.f, 3.f);
    EXPECT_EQ(VK_FRONT_FACE_COUNTER_CLOCKWISE, raster.descriptor().frontFace);
    EXPECT_EQ((VkCullModeFlags)VK_CULL_MODE_FRONT_BIT, raster.descriptor().cullMode);
    EXPECT_EQ(VK_POLYGON_MODE_LINE, raster.descriptor().polygonMode);
    EXPECT_FALSE(raster._isDepthClippingEnabled());
    EXPECT_TRUE(raster._isScissorClippingEnabled());
    EXPECT_EQ((VkBool32)VK_TRUE, raster.descriptor().depthBiasEnable);
    EXPECT_EQ(1.f, raster.descriptor().depthBiasConstantFactor);
    EXPECT_EQ(2.f, raster.descriptor().depthBiasClamp);
    EXPECT_EQ(3.f, raster.descriptor().depthBiasSlopeFactor);
    EXPECT_FALSE(raster._isDynamicCullingEnabled());

    RasterizerParams raster2(CullMode::none, FillMode::fill, false, true, true);
    EXPECT_EQ(VK_FRONT_FACE_COUNTER_CLOCKWISE, raster2.descriptor().frontFace);
    EXPECT_EQ((VkCullModeFlags)VK_CULL_MODE_NONE, raster2.descriptor().cullMode);
    EXPECT_EQ(VK_POLYGON_MODE_FILL, raster2.descriptor().polygonMode);
    EXPECT_TRUE(raster2._isDepthClippingEnabled());
    EXPECT_TRUE(raster2._isScissorClippingEnabled());
    EXPECT_EQ((VkBool32)VK_FALSE, raster2.descriptor().depthBiasEnable);
    EXPECT_EQ(0.f, raster2.descriptor().depthBiasConstantFactor);
    EXPECT_EQ(0.f, raster2.descriptor().depthBiasClamp);
    EXPECT_EQ(0.f, raster2.descriptor().depthBiasSlopeFactor);
    EXPECT_FALSE(raster2._isDynamicCullingEnabled());

    // depth/stencil
    DepthStencilParams depth;
    depth.enableDepthTest(false, false).enableStencilTest(true, true).depthTest(StencilCompare::always)
         .frontFaceStencilTest(StencilCompare::equal).backFaceStencilTest(StencilCompare::greater)
         .frontFaceOp(StencilOp::decrementClamp, StencilOp::incrementClamp, StencilOp::replace)
         .backFaceOp(StencilOp::invert, StencilOp::zero, StencilOp::incrementWrap)
         .depthMask(false).stencilMask((uint8_t)0, (uint8_t)1);
    EXPECT_EQ((VkBool32)VK_FALSE, depth.descriptor().depthTestEnable);
    EXPECT_EQ((VkBool32)VK_TRUE, depth.descriptor().stencilTestEnable);
    EXPECT_EQ(VK_COMPARE_OP_ALWAYS, depth.descriptor().depthCompareOp);
    EXPECT_EQ(VK_COMPARE_OP_EQUAL, depth.descriptor().front.compareOp);
    EXPECT_EQ(VK_COMPARE_OP_GREATER, depth.descriptor().back.compareOp);
    EXPECT_EQ(VK_STENCIL_OP_DECREMENT_AND_CLAMP, depth.descriptor().front.failOp);
    EXPECT_EQ(VK_STENCIL_OP_INCREMENT_AND_CLAMP, depth.descriptor().front.depthFailOp);
    EXPECT_EQ(VK_STENCIL_OP_REPLACE, depth.descriptor().front.passOp);
    EXPECT_EQ(VK_STENCIL_OP_INVERT, depth.descriptor().back.failOp);
    EXPECT_EQ(VK_STENCIL_OP_ZERO, depth.descriptor().back.depthFailOp);
    EXPECT_EQ(VK_STENCIL_OP_INCREMENT_AND_WRAP, depth.descriptor().back.passOp);
    EXPECT_EQ((VkBool32)VK_FALSE, depth.descriptor().depthWriteEnable);
    EXPECT_EQ((uint32_t)0, depth.descriptor().front.compareMask);
    EXPECT_EQ((uint32_t)0, depth.descriptor().back.compareMask);
    EXPECT_EQ((uint32_t)1, depth.descriptor().front.writeMask);
    EXPECT_EQ((uint32_t)1, depth.descriptor().back.writeMask);
    EXPECT_EQ((uint32_t)1, depth.descriptor().front.reference);
    EXPECT_EQ((uint32_t)1, depth.descriptor().back.reference);
    EXPECT_FALSE(depth._isDynamicDepthTestEnabled());
    EXPECT_TRUE(depth._isDynamicStencilTestEnabled());

    DepthStencilParams depth2(StencilCompare::greaterEqual, StencilOp::invert, StencilOp::replace,
                              StencilOp::incrementClamp, StencilOp::decrementClamp);
    EXPECT_EQ((VkBool32)VK_TRUE, depth2.descriptor().depthTestEnable);
    EXPECT_EQ((VkBool32)VK_FALSE, depth2.descriptor().stencilTestEnable);
    EXPECT_EQ(VK_COMPARE_OP_GREATER_OR_EQUAL, depth2.descriptor().depthCompareOp);
    EXPECT_EQ(VK_COMPARE_OP_ALWAYS, depth2.descriptor().front.compareOp);
    EXPECT_EQ(VK_COMPARE_OP_ALWAYS, depth2.descriptor().back.compareOp);
    EXPECT_EQ(VK_STENCIL_OP_INVERT, depth2.descriptor().front.depthFailOp);
    EXPECT_EQ(VK_STENCIL_OP_REPLACE, depth2.descriptor().front.passOp);
    EXPECT_EQ(VK_STENCIL_OP_INCREMENT_AND_CLAMP, depth2.descriptor().back.depthFailOp);
    EXPECT_EQ(VK_STENCIL_OP_DECREMENT_AND_CLAMP, depth2.descriptor().back.passOp);
    EXPECT_EQ((VkBool32)VK_TRUE, depth2.descriptor().depthWriteEnable);
    EXPECT_EQ((uint32_t)0xFF, depth2.descriptor().front.compareMask);
    EXPECT_EQ((uint32_t)0xFF, depth2.descriptor().back.compareMask);
    EXPECT_EQ((uint32_t)0xFF, depth2.descriptor().front.writeMask);
    EXPECT_EQ((uint32_t)0xFF, depth2.descriptor().back.writeMask);
    EXPECT_EQ((uint32_t)1, depth2.descriptor().front.reference);
    EXPECT_EQ((uint32_t)1, depth2.descriptor().back.reference);
    EXPECT_FALSE(depth2._isDynamicDepthTestEnabled());
    EXPECT_FALSE(depth2._isDynamicStencilTestEnabled());

    DepthStencilParams depth3(StencilCompare::notEqual, StencilCompare::lessEqual, StencilOp::replace,
                              StencilOp::invert, StencilOp::decrementClamp, StencilOp::incrementClamp);
    EXPECT_EQ((VkBool32)VK_FALSE, depth3.descriptor().depthTestEnable);
    EXPECT_EQ((VkBool32)VK_TRUE, depth3.descriptor().stencilTestEnable);
    EXPECT_EQ(VK_COMPARE_OP_ALWAYS, depth3.descriptor().depthCompareOp);
    EXPECT_EQ(VK_COMPARE_OP_NOT_EQUAL, depth3.descriptor().front.compareOp);
    EXPECT_EQ(VK_COMPARE_OP_LESS_OR_EQUAL, depth3.descriptor().back.compareOp);
    EXPECT_EQ(VK_STENCIL_OP_REPLACE, depth3.descriptor().front.failOp);
    EXPECT_EQ(VK_STENCIL_OP_INVERT, depth3.descriptor().front.passOp);
    EXPECT_EQ(VK_STENCIL_OP_DECREMENT_AND_CLAMP, depth3.descriptor().back.failOp);
    EXPECT_EQ(VK_STENCIL_OP_INCREMENT_AND_CLAMP, depth3.descriptor().back.passOp);
    EXPECT_EQ((VkBool32)VK_TRUE, depth3.descriptor().depthWriteEnable);
    EXPECT_EQ((uint32_t)0xFF, depth3.descriptor().front.compareMask);
    EXPECT_EQ((uint32_t)0xFF, depth3.descriptor().back.compareMask);
    EXPECT_EQ((uint32_t)0xFF, depth3.descriptor().front.writeMask);
    EXPECT_EQ((uint32_t)0xFF, depth3.descriptor().back.writeMask);
    EXPECT_EQ((uint32_t)1, depth3.descriptor().front.reference);
    EXPECT_EQ((uint32_t)1, depth3.descriptor().back.reference);
    EXPECT_FALSE(depth3._isDynamicDepthTestEnabled());
    EXPECT_FALSE(depth3._isDynamicStencilTestEnabled());

    DepthStencilParams depth4(StencilCompare::greaterEqual, StencilCompare::equal, StencilCompare::notEqual, 
                              StencilOp::replace, StencilOp::invert, StencilOp::zero, StencilOp::incrementClamp,
                              StencilOp::decrementClamp, StencilOp::invert);
    EXPECT_EQ((VkBool32)VK_TRUE, depth4.descriptor().depthTestEnable);
    EXPECT_EQ((VkBool32)VK_TRUE, depth4.descriptor().stencilTestEnable);
    EXPECT_EQ(VK_COMPARE_OP_GREATER_OR_EQUAL, depth4.descriptor().depthCompareOp);
    EXPECT_EQ(VK_COMPARE_OP_EQUAL, depth4.descriptor().front.compareOp);
    EXPECT_EQ(VK_COMPARE_OP_NOT_EQUAL, depth4.descriptor().back.compareOp);
    EXPECT_EQ(VK_STENCIL_OP_REPLACE, depth4.descriptor().front.failOp);
    EXPECT_EQ(VK_STENCIL_OP_INVERT, depth4.descriptor().front.depthFailOp);
    EXPECT_EQ(VK_STENCIL_OP_ZERO, depth4.descriptor().front.passOp);
    EXPECT_EQ(VK_STENCIL_OP_INCREMENT_AND_CLAMP, depth4.descriptor().back.failOp);
    EXPECT_EQ(VK_STENCIL_OP_DECREMENT_AND_CLAMP, depth4.descriptor().back.depthFailOp);
    EXPECT_EQ(VK_STENCIL_OP_INVERT, depth4.descriptor().back.passOp);
    EXPECT_EQ((VkBool32)VK_TRUE, depth4.descriptor().depthWriteEnable);
    EXPECT_EQ((uint32_t)0xFF, depth4.descriptor().front.compareMask);
    EXPECT_EQ((uint32_t)0xFF, depth4.descriptor().back.compareMask);
    EXPECT_EQ((uint32_t)0xFF, depth4.descriptor().front.writeMask);
    EXPECT_EQ((uint32_t)0xFF, depth4.descriptor().back.writeMask);
    EXPECT_EQ((uint32_t)1, depth4.descriptor().front.reference);
    EXPECT_EQ((uint32_t)1, depth4.descriptor().back.reference);
    EXPECT_EQ((uint32_t)1, depth4.stencilReference());
    EXPECT_FALSE(depth4._isDynamicDepthTestEnabled());
    EXPECT_FALSE(depth4._isDynamicStencilTestEnabled());
    depth4.enableDepthTest(true, true);
    depth4.enableStencilTest(true, true);
    EXPECT_TRUE(depth4._isDynamicDepthTestEnabled());
    EXPECT_TRUE(depth4._isDynamicStencilTestEnabled());

    // common blend
    const FLOAT color[4] = { 0.f,0.f,0.f,1.f };
    BlendParams blend;
    blend.colorBlend(BlendFactor::destInvColor, BlendFactor::destInvAlpha, BlendOp::subtract)
         .alphaBlend(BlendFactor::constantInvAlpha, BlendFactor::constantAlpha, BlendOp::revSubtract)
         .blendConstant(color, true).targetWriteMask(ColorComponentFlag::red).enable(false);
    EXPECT_EQ((VkBool32)VK_FALSE, blend._attachDesc().blendEnable);
    EXPECT_EQ((VkColorComponentFlags)VK_COLOR_COMPONENT_R_BIT, blend._attachDesc().colorWriteMask);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, blend._attachDesc().srcColorBlendFactor);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, blend._attachDesc().dstColorBlendFactor);
    EXPECT_EQ(VK_BLEND_OP_SUBTRACT, blend._attachDesc().colorBlendOp);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, blend._attachDesc().srcAlphaBlendFactor);
    EXPECT_EQ(VK_BLEND_FACTOR_CONSTANT_ALPHA, blend._attachDesc().dstAlphaBlendFactor);
    EXPECT_EQ(VK_BLEND_OP_REVERSE_SUBTRACT, blend._attachDesc().alphaBlendOp);
    EXPECT_EQ(color[0], blend.blendConstant()[0]);
    EXPECT_EQ(color[1], blend.blendConstant()[1]);
    EXPECT_EQ(color[2], blend.blendConstant()[2]);
    EXPECT_EQ(color[3], blend.blendConstant()[3]);
    EXPECT_TRUE(blend._isDynamicConstantEnabled());

    BlendParams blend2(BlendFactor::destInvColor, BlendFactor::destInvAlpha, BlendOp::subtract,
                       BlendFactor::constantInvAlpha, BlendFactor::constantAlpha, BlendOp::revSubtract);
    EXPECT_EQ((VkBool32)VK_TRUE, blend2._attachDesc().blendEnable);
    EXPECT_EQ((VkColorComponentFlags)(VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT),
              blend2._attachDesc().colorWriteMask);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, blend2._attachDesc().srcColorBlendFactor);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, blend2._attachDesc().dstColorBlendFactor);
    EXPECT_EQ(VK_BLEND_OP_SUBTRACT, blend2._attachDesc().colorBlendOp);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, blend2._attachDesc().srcAlphaBlendFactor);
    EXPECT_EQ(VK_BLEND_FACTOR_CONSTANT_ALPHA, blend2._attachDesc().dstAlphaBlendFactor);
    EXPECT_EQ(VK_BLEND_OP_REVERSE_SUBTRACT, blend2._attachDesc().alphaBlendOp);
    EXPECT_EQ(1.f, blend2.blendConstant()[0]);
    EXPECT_EQ(1.f, blend2.blendConstant()[1]);
    EXPECT_EQ(1.f, blend2.blendConstant()[2]);
    EXPECT_EQ(1.f, blend2.blendConstant()[3]);
    EXPECT_FALSE(blend2._isDynamicConstantEnabled());

    // blend per target
    BlendPerTargetParams blendPerTarget;
    EXPECT_EQ(size_t{ 0 }, blendPerTarget._attachDesc().size());
    blendPerTarget.setTargetBlend(0, BlendFactor::destInvColor, BlendFactor::destInvAlpha, BlendOp::subtract,
                     BlendFactor::constantInvAlpha, BlendFactor::constantAlpha, BlendOp::revSubtract, ColorComponentFlag::red)
                  .disableTargetBlend(1).blendConstant(color, false);
    ASSERT_EQ(size_t{ 2u }, blendPerTarget._attachDesc().size());
    EXPECT_EQ((VkBool32)VK_TRUE, blendPerTarget._attachDesc()[0].blendEnable);
    EXPECT_EQ((VkColorComponentFlags)VK_COLOR_COMPONENT_R_BIT, blendPerTarget._attachDesc()[0].colorWriteMask);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR, blendPerTarget._attachDesc()[0].srcColorBlendFactor);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA, blendPerTarget._attachDesc()[0].dstColorBlendFactor);
    EXPECT_EQ(VK_BLEND_OP_SUBTRACT, blendPerTarget._attachDesc()[0].colorBlendOp);
    EXPECT_EQ(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, blendPerTarget._attachDesc()[0].srcAlphaBlendFactor);
    EXPECT_EQ(VK_BLEND_FACTOR_CONSTANT_ALPHA, blendPerTarget._attachDesc()[0].dstAlphaBlendFactor);
    EXPECT_EQ(VK_BLEND_OP_REVERSE_SUBTRACT, blendPerTarget._attachDesc()[0].alphaBlendOp);
    EXPECT_EQ((VkBool32)VK_FALSE, blendPerTarget._attachDesc()[1].blendEnable);
    EXPECT_EQ(color[0], blendPerTarget.blendConstant()[0]);
    EXPECT_EQ(color[1], blendPerTarget.blendConstant()[1]);
    EXPECT_EQ(color[2], blendPerTarget.blendConstant()[2]);
    EXPECT_EQ(color[3], blendPerTarget.blendConstant()[3]);
    EXPECT_FALSE(blendPerTarget._isDynamicConstantEnabled());
  }

  // -- GraphicsPipeline --

  static std::unique_ptr<char[]> _readVertexShaderFile(size_t& outBufferSize) {
    std::ifstream binaryShaderFile(_P_TEST_RESOURCE_DIR "/vulkan/" __P_COMPILED_SHADER, std::fstream::in|std::fstream::binary);
    if (!binaryShaderFile.is_open())
      return nullptr;

    binaryShaderFile.seekg(0, binaryShaderFile.end);
    outBufferSize = (size_t)binaryShaderFile.tellg();
    binaryShaderFile.seekg(0, binaryShaderFile.beg);

    char* binaryShaderFileBuffer = new char[outBufferSize];
    binaryShaderFile.read(binaryShaderFileBuffer, outBufferSize);
    binaryShaderFile.close();
    return std::unique_ptr<char[]>(binaryShaderFileBuffer);
  }

  TEST_F(VulkanGraphicsPipelineTest, createGraphicsPipelineTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
    GraphicsPipeline::Builder builder(renderer);
    bool useDynamics = renderer->isExtensionEnabled("VK_EXT_extended_dynamic_state");

    GraphicsPipeline empty;
    EXPECT_TRUE(empty.handle() == nullptr);
    EXPECT_TRUE(empty.isEmpty());

    // create shader + input layout
    size_t shaderBufferSize = 0;
    auto shaderBuffer = _readVertexShaderFile(shaderBufferSize);
    ASSERT_TRUE(shaderBuffer != nullptr);

    Shader::Builder shaderBuilder(ShaderType::vertex, (const uint8_t*)shaderBuffer.get(), shaderBufferSize);
    Shader vertexShader = shaderBuilder.createShader(renderer->resourceManager());
    ASSERT_TRUE(vertexShader.handle() != nullptr);
    VkVertexInputBindingDescription inputBindings[] = {{ 0, sizeof(float)*8, VK_VERTEX_INPUT_RATE_VERTEX }};
    VkVertexInputAttributeDescription layoutAttributes[] = {
      { 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },
      { 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float)*4 }
    };
    auto inputLayout = shaderBuilder.createInputLayout(inputBindings, size_t{ 1u }, layoutAttributes, size_t{ 2u });
    ASSERT_TRUE(inputLayout.get() != nullptr);

    // create simple pipeline (no viewport)
    EXPECT_ANY_THROW(builder.build());
    builder.setRenderAttachments(nullptr, 0);//TODO
    EXPECT_ANY_THROW(builder.build());
    builder.setInputLayout(inputBindings, size_t{ 1u }, layoutAttributes, size_t{ 2u });
    builder.setInputLayout(inputLayout);
    EXPECT_ANY_THROW(builder.build());
#   ifndef __P_DISABLE_TESSELLATION_STAGE
      builder.setPatchTopology(4);
#   endif
    builder.setVertexTopology(VertexTopology::trianglesAdj);
    EXPECT_ANY_THROW(builder.build());
    builder.setShaderStages(&vertexShader, size_t{ 1u });
    EXPECT_ANY_THROW(builder.build());
    builder.setRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::fill, true, false, false));
    EXPECT_ANY_THROW(builder.build());
    builder.setDepthStencilState(DepthStencilParams(StencilCompare::less, StencilOp::incrementWrap, StencilOp::replace,
                                                    StencilOp::decrementWrap, StencilOp::invert, 2u));
    EXPECT_ANY_THROW(builder.build());
    builder.setBlendState(BlendParams(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add,
                                      BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add));

    /*GraphicsPipeline pipeline = builder.build();
    ASSERT_TRUE(pipeline.handle() != nullptr);
    EXPECT_FALSE(pipeline.isEmpty());
    EXPECT_TRUE(pipeline.handle() != VK_NULL_HANDLE);
    renderer->bindGraphicsPipeline(pipeline.handle());*/

    // create pipeline with viewport (+ set same state params -> read from cache)
    Viewport viewport(10.f, 20.f, 800.f, 600.f, 0.1f, 1.f);
    builder.setVertexTopology(VertexTopology::triangles);
    builder.setRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::fill, true, false, false));
    builder.setDepthStencilState(DepthStencilParams(StencilCompare::less, StencilOp::incrementWrap, StencilOp::replace,
                                                    StencilOp::decrementWrap, StencilOp::invert, 1u));
    builder.setBlendState(BlendParams(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add,
                                      BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add));
    builder.clearShaderStages();
    EXPECT_ANY_THROW(builder.build());
    builder.attachShaderStage(vertexShader);
    builder.setViewports(&viewport, size_t{ 1u }, nullptr, 0, useDynamics);

    /*GraphicsPipeline pipelineViewport = builder.build();
    ASSERT_TRUE(pipelineViewport.handle() != nullptr);
    EXPECT_FALSE(pipelineViewport.isEmpty());
    EXPECT_TRUE(pipelineViewport.handle() != VK_NULL_HANDLE);
    renderer->bindGraphicsPipeline(pipelineViewport.handle());*/

    // create pipeline with viewport + scissor test (+ different state params + split blending)
    Viewport viewports[]{ Viewport(0.f, 0.f, 800.f, 600.f, 0.1f, 1.f), Viewport(10.f, 20.f, 800.f, 600.f, 0.1f, 1.f) };
    ScissorRectangle scissors[]{ ScissorRectangle(0, 0, 800, 600), ScissorRectangle(10, 20, 400, 300) };
    BlendPerTargetParams blendPerTarget;
    blendPerTarget.setTargetBlend(0, BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add,
                                     BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add)
                  .setTargetBlend(1, BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::subtract,
                                     BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::subtract);
    builder.setRasterizerState(RasterizerParams(CullMode::none, FillMode::fill, true, false, false));
    builder.setDepthStencilState(DepthStencilParams(StencilCompare::greater, StencilOp::zero, StencilOp::keep,
                                                    StencilOp::zero, StencilOp::keep, 1u));
    builder.setBlendState(blendPerTarget);
    builder.setViewports(viewports, size_t{ 2u }, scissors, size_t{ 2u }, false);

    /*GraphicsPipeline pipelineViewportScissor = builder.build();
    ASSERT_TRUE(pipelineViewportScissor.handle() != nullptr);
    EXPECT_FALSE(pipelineViewportScissor.isEmpty());
    EXPECT_TRUE(pipelineViewportScissor.handle() != VK_NULL_HANDLE);
    renderer->bindGraphicsPipeline(pipelineViewportScissor.handle());*/

    Shader fakeFragmentShader = Shader(std::make_shared<ScopedResource<VkShaderModule> >(), ShaderType::fragment, "main");
    builder.attachShaderStage(fakeFragmentShader);
    builder.detachShaderStage(ShaderType::vertex);
    builder.detachShaderStage(ShaderType::fragment);
    builder.clearDepthStencilState();
    EXPECT_ANY_THROW(builder.build());

    // move
    /*auto pipelineHandle = pipeline.handle();
    GraphicsPipeline moved(std::move(pipeline));
    EXPECT_TRUE(pipeline.handle() == VK_NULL_HANDLE);
    EXPECT_TRUE(moved.handle() != VK_NULL_HANDLE);
    EXPECT_TRUE(moved.handle() == pipelineHandle);

    // release
    moved.release();
    EXPECT_TRUE(moved.handle() == VK_NULL_HANDLE);
    EXPECT_TRUE(moved.isEmpty());*/
  }

#endif
