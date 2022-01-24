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
# include <fstream>
# include <video/d3d11/graphics_pipeline.h>

# if defined(_WIN64) || defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_AMD64_) || defined(_M_X64) || defined(_M_I64) || defined(_M_IX64) || defined(_M_AMD64) || defined(__IA64__) || defined(_IA64_)
#   define __P_COMPILED_SHADER "test_vertex64.cso"
# else
#   define __P_COMPILED_SHADER "test_vertex32.cso"
# endif

  using namespace pandora::video::d3d11;

  class D3d11GraphicsPipelineTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- Rasterizer/depth-stencil/blend state management --

  TEST_F(D3d11GraphicsPipelineTest, createRasterStatesTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
    GraphicsPipeline::Builder builder(renderer);

    // rasterizer state
    RasterizerParams paramsR1(CullMode::none, FillMode::lines, false, true, false);
    RasterizerState valR1 = builder.createRasterizerState(paramsR1);
    EXPECT_TRUE(valR1);
    EXPECT_TRUE(valR1.hasValue());
    EXPECT_TRUE(valR1.get() != nullptr);
    renderer->setRasterizerState(valR1);
    RasterizerParams paramsR2(CullMode::cullBack, FillMode::fill, true, true, true);
    RasterizerState valR2(builder.createRasterizerState(paramsR2));
    EXPECT_TRUE(valR2);
    EXPECT_TRUE(valR2.hasValue());
    EXPECT_TRUE(valR2.get() != nullptr);
    renderer->setRasterizerState(valR2);
    renderer->setRasterizerState(RasterizerState{});

    EXPECT_TRUE(paramsR1.computeId() != paramsR2.computeId());

    auto valR2Get = valR2.get();
    valR1 = std::move(valR2);
    EXPECT_TRUE(valR1);
    EXPECT_TRUE(valR1.hasValue());
    EXPECT_TRUE(valR1.get() != nullptr);
    EXPECT_TRUE(valR1.get() == valR2Get);
    EXPECT_FALSE(valR2);
    EXPECT_FALSE(valR2.hasValue());
    EXPECT_TRUE(valR2.get() == nullptr);
    RasterizerState valR3 = std::move(valR1);
    EXPECT_TRUE(valR3);
    EXPECT_TRUE(valR3.hasValue());
    EXPECT_TRUE(valR3.get() != nullptr);
    EXPECT_TRUE(valR3.get() == valR2Get);
    EXPECT_FALSE(valR1);
    EXPECT_FALSE(valR1.hasValue());
    EXPECT_TRUE(valR1.get() == nullptr);
    EXPECT_FALSE(valR2);
    EXPECT_FALSE(valR2.hasValue());
    EXPECT_TRUE(valR2.get() == nullptr);
  }

  TEST_F(D3d11GraphicsPipelineTest, createDepthStatesTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
    GraphicsPipeline::Builder builder(renderer);

    DepthStencilParams paramsD1(StencilCompare::less, StencilOp::incrementWrap, StencilOp::replace,
                                StencilOp::decrementWrap, StencilOp::invert);
    DepthStencilState valD1 = builder.createDepthStencilState(paramsD1);
    auto idD1 = paramsD1.computeId();
    EXPECT_TRUE(valD1);
    EXPECT_TRUE(valD1.hasValue());
    EXPECT_TRUE(valD1.get() != nullptr);
    renderer->setDepthStencilState(valD1);
    DepthStencilParams paramsD2(StencilCompare::greaterEqual, StencilOp::zero, StencilOp::replace,
                                StencilOp::zero, StencilOp::invert);
    DepthStencilState valD2 = builder.createDepthStencilState(paramsD2);
    auto idD2 = paramsD2.computeId();
    EXPECT_TRUE(valD2);
    EXPECT_TRUE(valD2.hasValue());
    EXPECT_TRUE(valD2.get() != nullptr);
    renderer->setDepthStencilState(valD2);
    DepthStencilParams paramsS1(StencilCompare::greater, StencilCompare::less, StencilOp::incrementWrap,
                                StencilOp::replace, StencilOp::decrementWrap, StencilOp::invert);
    DepthStencilState valS1 = builder.createDepthStencilState(paramsS1);
    auto idS1 = paramsS1.computeId();
    EXPECT_TRUE(valS1);
    EXPECT_TRUE(valS1.hasValue());
    EXPECT_TRUE(valS1.get() != nullptr);
    renderer->setDepthStencilState(valS1);
    DepthStencilParams paramsS2(StencilCompare::always, StencilCompare::notEqual, StencilOp::zero,
                                StencilOp::replace, StencilOp::zero, StencilOp::invert);
    DepthStencilState valS2 = builder.createDepthStencilState(paramsS2);
    auto idS2 = paramsS2.computeId();
    EXPECT_TRUE(valS2);
    EXPECT_TRUE(valS2.hasValue());
    EXPECT_TRUE(valS2.get() != nullptr);
    renderer->setDepthStencilState(valS2);
    DepthStencilParams paramsDS1(StencilCompare::less, StencilCompare::less, StencilCompare::greater, StencilOp::zero,
                         StencilOp::incrementWrap, StencilOp::keep, StencilOp::zero, StencilOp::decrementClamp, StencilOp::keep);
    DepthStencilState valDS1 = builder.createDepthStencilState(paramsDS1);
    auto idDS1 = paramsDS1.computeId();
    EXPECT_TRUE(valDS1);
    EXPECT_TRUE(valDS1.hasValue());
    EXPECT_TRUE(valDS1.get() != nullptr);
    renderer->setDepthStencilState(valDS1);
    DepthStencilParams paramsDS2(StencilCompare::greater, StencilCompare::always, StencilCompare::always, StencilOp::keep,
                         StencilOp::decrementClamp, StencilOp::replace, StencilOp::keep, StencilOp::incrementClamp, StencilOp::invert);
    DepthStencilState valDS2 = builder.createDepthStencilState(paramsDS2);
    auto idDS2 = paramsDS2.computeId();
    EXPECT_TRUE(valDS2);
    EXPECT_TRUE(valDS2.hasValue());
    EXPECT_TRUE(valDS2.get() != nullptr);
    renderer->setDepthStencilState(valDS2);
    renderer->setDepthStencilState(DepthStencilState{});

    EXPECT_TRUE(idD1 != idD2);
    EXPECT_TRUE(idD1 != idS1);
    EXPECT_TRUE(idD1 != idS2);
    EXPECT_TRUE(idD1 != idDS1);
    EXPECT_TRUE(idD1 != idDS2);
    EXPECT_TRUE(idD2 != idS1);
    EXPECT_TRUE(idD2 != idS2);
    EXPECT_TRUE(idD2 != idDS1);
    EXPECT_TRUE(idD2 != idDS2);
    EXPECT_TRUE(idS1 != idS2);
    EXPECT_TRUE(idS1 != idDS1);
    EXPECT_TRUE(idS1 != idDS2);
    EXPECT_TRUE(idS2 != idDS1);
    EXPECT_TRUE(idS2 != idDS2);
    EXPECT_TRUE(idDS1 != idDS2);

    auto valSGet = valS1.get();
    valD1 = std::move(valS1);
    EXPECT_TRUE(valD1);
    EXPECT_TRUE(valD1.hasValue());
    EXPECT_TRUE(valD1.get() != nullptr);
    EXPECT_TRUE(valD1.get() == valSGet);
    EXPECT_FALSE(valS1);
    EXPECT_FALSE(valS1.hasValue());
    EXPECT_TRUE(valS1.get() == nullptr);
    DepthStencilState movedDS = std::move(valD1);
    EXPECT_TRUE(movedDS);
    EXPECT_TRUE(movedDS.hasValue());
    EXPECT_TRUE(movedDS.get() != nullptr);
    EXPECT_TRUE(movedDS.get() == valSGet);
    EXPECT_FALSE(valD1);
    EXPECT_FALSE(valD1.hasValue());
    EXPECT_TRUE(valD1.get() == nullptr);
    EXPECT_FALSE(valS1);
    EXPECT_FALSE(valS1.hasValue());
    EXPECT_TRUE(valS1.get() == nullptr);
  }

  TEST_F(D3d11GraphicsPipelineTest, createBlendingStatesTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
    GraphicsPipeline::Builder builder(renderer);
    const FLOAT color[4] = { 0.f,0.f,0.f,1.f };

    BlendParams paramsB1(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add,
                         BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add);
    BlendState valB1 = builder.createBlendState(paramsB1);
    auto idB1 = paramsB1.computeId();
    EXPECT_TRUE(valB1);
    EXPECT_TRUE(valB1.hasValue());
    EXPECT_TRUE(valB1.get() != nullptr);
    renderer->setBlendState(valB1);
    renderer->setBlendState(valB1, color);
    BlendParams paramsB2(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::revSubtract,
                         BlendFactor::one, BlendFactor::zero, BlendOp::maximum);
    BlendState valB2(builder.createBlendState(paramsB2));
    auto idB2 = paramsB2.computeId();
    EXPECT_TRUE(valB2);
    EXPECT_TRUE(valB2.hasValue());
    EXPECT_TRUE(valB2.get() != nullptr);
    renderer->setBlendState(valB2);
    renderer->setBlendState(valB2, color);
    BlendParams paramsB3(BlendFactor::constantColor, BlendFactor::constantInvColor, BlendOp::subtract,
                         BlendFactor::constantAlpha, BlendFactor::constantInvAlpha, BlendOp::subtract);
    BlendState valB3(builder.createBlendState(paramsB3));
    auto idB3 = paramsB3.computeId();
    EXPECT_TRUE(valB3);
    EXPECT_TRUE(valB3.hasValue());
    EXPECT_TRUE(valB3.get() != nullptr);
    renderer->setBlendState(valB3);
    renderer->setBlendState(valB3, color);

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
    auto idBpt1A = paramsBpt1.computeId();
    paramsBpt1.disableTargetBlend(2);
    auto idBpt1B = paramsBpt1.computeId();
    paramsBpt1.blendConstant(color);
    auto idBpt1C = paramsBpt1.computeId();
    EXPECT_EQ(color[0], paramsBpt1.blendConstant()[0]);
    EXPECT_EQ(color[1], paramsBpt1.blendConstant()[1]);
    EXPECT_EQ(color[2], paramsBpt1.blendConstant()[2]);
    EXPECT_EQ(color[3], paramsBpt1.blendConstant()[3]);
    BlendState valB4(builder.createBlendState(paramsBpt1));
    EXPECT_TRUE(valB4);
    EXPECT_TRUE(valB4.hasValue());
    EXPECT_TRUE(valB4.get() != nullptr);
    renderer->setBlendState(valB4);
    renderer->setBlendState(valB4, color);
    BlendPerTargetParams paramsBpt2;
    paramsBpt2.setTargetBlend(0, BlendFactor::sourceColor, BlendFactor::destColor, BlendOp::minimum,
                                 BlendFactor::one, BlendFactor::one, BlendOp::maximum);
    BlendState valB5(builder.createBlendState(paramsBpt2));
    auto idBpt2 = paramsBpt2.computeId();
    EXPECT_TRUE(valB5);
    EXPECT_TRUE(valB5.hasValue());
    EXPECT_TRUE(valB5.get() != nullptr);
    renderer->setBlendState(valB5);
    renderer->setBlendState(valB5, color);
    renderer->setBlendState(BlendState{});

    EXPECT_TRUE(idB1 != idB2);
    EXPECT_TRUE(idB1 != idB3);
    EXPECT_TRUE(idB2 != idB3);
    EXPECT_TRUE(idBpt1A != idBpt1B);
    EXPECT_TRUE(idBpt1A != idBpt1C);
    EXPECT_TRUE(idBpt1A != idBpt2);
    EXPECT_TRUE(idBpt1B != idBpt1C);
    EXPECT_TRUE(idBpt1B != idBpt2);
    EXPECT_TRUE(idBpt1C != idBpt2);

    auto valB2Get = valB2.get();
    valB1 = std::move(valB2);
    EXPECT_TRUE(valB1);
    EXPECT_TRUE(valB1.hasValue());
    EXPECT_TRUE(valB1.get() != nullptr);
    EXPECT_TRUE(valB1.get() == valB2Get);
    EXPECT_FALSE(valB2);
    EXPECT_FALSE(valB2.hasValue());
    EXPECT_TRUE(valB2.get() == nullptr);
    BlendState valBM = std::move(valB1);
    EXPECT_TRUE(valBM);
    EXPECT_TRUE(valBM.hasValue());
    EXPECT_TRUE(valBM.get() != nullptr);
    EXPECT_TRUE(valBM.get() == valB2Get);
    EXPECT_FALSE(valB1);
    EXPECT_FALSE(valB1.hasValue());
    EXPECT_TRUE(valB1.get() == nullptr);
    EXPECT_FALSE(valB2);
    EXPECT_FALSE(valB2.hasValue());
    EXPECT_TRUE(valB2.get() == nullptr);
  }

  TEST_F(D3d11GraphicsPipelineTest, pipelineStateParamsTest) {
    // rasterizer
    RasterizerParams raster;
    raster.vertexOrder(false).cullMode(CullMode::cullFront).fillMode(FillMode::lines).depthClipping(false).scissorClipping(true).depthBias(1, 2.f, 3.f);
    EXPECT_EQ((BOOL)TRUE, raster.descriptor().FrontCounterClockwise);
    EXPECT_EQ(D3D11_CULL_FRONT, raster.descriptor().CullMode);
    EXPECT_EQ(D3D11_FILL_WIREFRAME, raster.descriptor().FillMode);
    EXPECT_EQ((BOOL)FALSE, raster.descriptor().AntialiasedLineEnable);
    EXPECT_EQ((BOOL)FALSE, raster.descriptor().DepthClipEnable);
    EXPECT_EQ((BOOL)TRUE, raster.descriptor().ScissorEnable);
    EXPECT_EQ((INT)1, raster.descriptor().DepthBias);
    EXPECT_EQ((FLOAT)2.f, raster.descriptor().DepthBiasClamp);
    EXPECT_EQ((FLOAT)3.f, raster.descriptor().SlopeScaledDepthBias);

    RasterizerParams raster2(CullMode::none, FillMode::fill, false, true, true);
    EXPECT_EQ((BOOL)TRUE, raster2.descriptor().FrontCounterClockwise);
    EXPECT_EQ(D3D11_CULL_NONE, raster2.descriptor().CullMode);
    EXPECT_EQ(D3D11_FILL_SOLID, raster2.descriptor().FillMode);
    EXPECT_EQ((BOOL)FALSE, raster2.descriptor().AntialiasedLineEnable);
    EXPECT_EQ((BOOL)TRUE, raster2.descriptor().DepthClipEnable);
    EXPECT_EQ((BOOL)TRUE, raster2.descriptor().ScissorEnable);
    EXPECT_EQ((INT)0, raster2.descriptor().DepthBias);
    EXPECT_EQ((FLOAT)0.f, raster2.descriptor().DepthBiasClamp);
    EXPECT_EQ((FLOAT)0.f, raster2.descriptor().SlopeScaledDepthBias);

    EXPECT_TRUE(raster.computeId() != raster2.computeId());

    // depth/stencil
    DepthStencilParams depth;
    depth.enableDepthTest(false).enableStencilTest(true).depthTest(StencilCompare::always)
         .frontFaceStencilTest(StencilCompare::equal).backFaceStencilTest(StencilCompare::greater)
         .frontFaceOp(StencilOp::decrementClamp, StencilOp::incrementClamp, StencilOp::replace)
         .backFaceOp(StencilOp::invert, StencilOp::zero, StencilOp::incrementWrap)
         .depthMask(false).stencilMask((uint8_t)0, (uint8_t)1);
    EXPECT_EQ((BOOL)FALSE, depth.descriptor().DepthEnable);
    EXPECT_EQ((BOOL)TRUE, depth.descriptor().StencilEnable);
    EXPECT_EQ(D3D11_COMPARISON_ALWAYS, depth.descriptor().DepthFunc);
    EXPECT_EQ(D3D11_COMPARISON_EQUAL, depth.descriptor().FrontFace.StencilFunc);
    EXPECT_EQ(D3D11_COMPARISON_GREATER, depth.descriptor().BackFace.StencilFunc);
    EXPECT_EQ(D3D11_STENCIL_OP_DECR_SAT, depth.descriptor().FrontFace.StencilFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INCR_SAT, depth.descriptor().FrontFace.StencilDepthFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_REPLACE, depth.descriptor().FrontFace.StencilPassOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INVERT, depth.descriptor().BackFace.StencilFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_ZERO, depth.descriptor().BackFace.StencilDepthFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INCR, depth.descriptor().BackFace.StencilPassOp);
    EXPECT_EQ(D3D11_DEPTH_WRITE_MASK_ZERO, depth.descriptor().DepthWriteMask);
    EXPECT_EQ((UINT8)0, depth.descriptor().StencilReadMask);
    EXPECT_EQ((UINT8)1, depth.descriptor().StencilWriteMask);
    auto depthId = depth.computeId();

    DepthStencilParams depth2(StencilCompare::greaterEqual, StencilOp::invert, StencilOp::replace,
                              StencilOp::incrementClamp, StencilOp::decrementClamp);
    EXPECT_EQ((BOOL)TRUE, depth2.descriptor().DepthEnable);
    EXPECT_EQ((BOOL)FALSE, depth2.descriptor().StencilEnable);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, depth2.descriptor().DepthFunc);
    EXPECT_EQ(D3D11_COMPARISON_ALWAYS, depth2.descriptor().FrontFace.StencilFunc);
    EXPECT_EQ(D3D11_COMPARISON_ALWAYS, depth2.descriptor().BackFace.StencilFunc);
    EXPECT_EQ(D3D11_STENCIL_OP_INVERT, depth2.descriptor().FrontFace.StencilDepthFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_REPLACE, depth2.descriptor().FrontFace.StencilPassOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INCR_SAT, depth2.descriptor().BackFace.StencilDepthFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_DECR_SAT, depth2.descriptor().BackFace.StencilPassOp);
    EXPECT_EQ(D3D11_DEPTH_WRITE_MASK_ALL, depth2.descriptor().DepthWriteMask);
    EXPECT_EQ((UINT8)0xFF, depth2.descriptor().StencilReadMask);
    EXPECT_EQ((UINT8)0xFF, depth2.descriptor().StencilWriteMask);
    auto depthId2 = depth2.computeId();

    DepthStencilParams depth3(StencilCompare::notEqual, StencilCompare::lessEqual, StencilOp::replace,
                              StencilOp::invert, StencilOp::decrementClamp, StencilOp::incrementClamp);
    EXPECT_EQ((BOOL)FALSE, depth3.descriptor().DepthEnable);
    EXPECT_EQ((BOOL)TRUE, depth3.descriptor().StencilEnable);
    EXPECT_EQ(D3D11_COMPARISON_ALWAYS, depth3.descriptor().DepthFunc);
    EXPECT_EQ(D3D11_COMPARISON_NOT_EQUAL, depth3.descriptor().FrontFace.StencilFunc);
    EXPECT_EQ(D3D11_COMPARISON_LESS_EQUAL, depth3.descriptor().BackFace.StencilFunc);
    EXPECT_EQ(D3D11_STENCIL_OP_REPLACE, depth3.descriptor().FrontFace.StencilFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INVERT, depth3.descriptor().FrontFace.StencilPassOp);
    EXPECT_EQ(D3D11_STENCIL_OP_DECR_SAT, depth3.descriptor().BackFace.StencilFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INCR_SAT, depth3.descriptor().BackFace.StencilPassOp);
    EXPECT_EQ(D3D11_DEPTH_WRITE_MASK_ALL, depth3.descriptor().DepthWriteMask);
    EXPECT_EQ((UINT8)0xFF, depth3.descriptor().StencilReadMask);
    EXPECT_EQ((UINT8)0xFF, depth3.descriptor().StencilWriteMask);
    auto depthId3 = depth3.computeId();

    DepthStencilParams depth4(StencilCompare::greaterEqual, StencilCompare::equal, StencilCompare::notEqual, 
                              StencilOp::replace, StencilOp::invert, StencilOp::zero, StencilOp::incrementClamp,
                              StencilOp::decrementClamp, StencilOp::invert);
    EXPECT_EQ((BOOL)TRUE, depth4.descriptor().DepthEnable);
    EXPECT_EQ((BOOL)TRUE, depth4.descriptor().StencilEnable);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, depth4.descriptor().DepthFunc);
    EXPECT_EQ(D3D11_COMPARISON_EQUAL, depth4.descriptor().FrontFace.StencilFunc);
    EXPECT_EQ(D3D11_COMPARISON_NOT_EQUAL, depth4.descriptor().BackFace.StencilFunc);
    EXPECT_EQ(D3D11_STENCIL_OP_REPLACE, depth4.descriptor().FrontFace.StencilFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INVERT, depth4.descriptor().FrontFace.StencilDepthFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_ZERO, depth4.descriptor().FrontFace.StencilPassOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INCR_SAT, depth4.descriptor().BackFace.StencilFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_DECR_SAT, depth4.descriptor().BackFace.StencilDepthFailOp);
    EXPECT_EQ(D3D11_STENCIL_OP_INVERT, depth4.descriptor().BackFace.StencilPassOp);
    EXPECT_EQ(D3D11_DEPTH_WRITE_MASK_ALL, depth4.descriptor().DepthWriteMask);
    EXPECT_EQ((UINT8)0xFF, depth4.descriptor().StencilReadMask);
    EXPECT_EQ((UINT8)0xFF, depth4.descriptor().StencilWriteMask);
    auto depthId4 = depth4.computeId();

    EXPECT_TRUE(depthId != depthId2);
    EXPECT_TRUE(depthId != depthId3);
    EXPECT_TRUE(depthId != depthId4);
    EXPECT_TRUE(depthId2 != depthId3);
    EXPECT_TRUE(depthId2 != depthId4);
    EXPECT_TRUE(depthId3 != depthId4);

    // common blend
    const FLOAT color[4] = { 0.f,0.f,0.f,1.f };
    BlendParams blend;
    blend.colorBlend(BlendFactor::destInvColor, BlendFactor::destInvAlpha, BlendOp::subtract)
         .alphaBlend(BlendFactor::constantInvAlpha, BlendFactor::constantAlpha, BlendOp::revSubtract)
         .blendConstant(color).targetWriteMask(ColorComponentFlag::red).enable(false);
    EXPECT_EQ((BOOL)FALSE, blend.descriptor().IndependentBlendEnable);
    EXPECT_EQ((BOOL)FALSE, blend.descriptor().RenderTarget->BlendEnable);
    EXPECT_EQ((UINT8)D3D10_COLOR_WRITE_ENABLE_RED, blend.descriptor().RenderTarget->RenderTargetWriteMask);
    EXPECT_EQ(D3D11_BLEND_INV_DEST_COLOR, blend.descriptor().RenderTarget->SrcBlend);
    EXPECT_EQ(D3D11_BLEND_INV_DEST_ALPHA, blend.descriptor().RenderTarget->DestBlend);
    EXPECT_EQ(D3D11_BLEND_OP_SUBTRACT, blend.descriptor().RenderTarget->BlendOp);
    EXPECT_EQ(D3D11_BLEND_INV_BLEND_FACTOR, blend.descriptor().RenderTarget->SrcBlendAlpha);
    EXPECT_EQ(D3D11_BLEND_BLEND_FACTOR, blend.descriptor().RenderTarget->DestBlendAlpha);
    EXPECT_EQ(D3D11_BLEND_OP_REV_SUBTRACT, blend.descriptor().RenderTarget->BlendOpAlpha);
    EXPECT_EQ(color[0], blend.blendConstant()[0]);
    EXPECT_EQ(color[1], blend.blendConstant()[1]);
    EXPECT_EQ(color[2], blend.blendConstant()[2]);
    EXPECT_EQ(color[3], blend.blendConstant()[3]);

    BlendParams blend2(BlendFactor::destInvColor, BlendFactor::destInvAlpha, BlendOp::subtract,
                       BlendFactor::constantInvAlpha, BlendFactor::constantAlpha, BlendOp::revSubtract);
    EXPECT_EQ((BOOL)FALSE, blend2.descriptor().IndependentBlendEnable);
    EXPECT_EQ((BOOL)TRUE, blend2.descriptor().RenderTarget->BlendEnable);
    EXPECT_EQ((UINT8)D3D11_COLOR_WRITE_ENABLE_ALL, blend2.descriptor().RenderTarget->RenderTargetWriteMask);
    EXPECT_EQ(D3D11_BLEND_INV_DEST_COLOR, blend2.descriptor().RenderTarget->SrcBlend);
    EXPECT_EQ(D3D11_BLEND_INV_DEST_ALPHA, blend2.descriptor().RenderTarget->DestBlend);
    EXPECT_EQ(D3D11_BLEND_OP_SUBTRACT, blend2.descriptor().RenderTarget->BlendOp);
    EXPECT_EQ(D3D11_BLEND_INV_BLEND_FACTOR, blend2.descriptor().RenderTarget->SrcBlendAlpha);
    EXPECT_EQ(D3D11_BLEND_BLEND_FACTOR, blend2.descriptor().RenderTarget->DestBlendAlpha);
    EXPECT_EQ(D3D11_BLEND_OP_REV_SUBTRACT, blend2.descriptor().RenderTarget->BlendOpAlpha);
    EXPECT_EQ(1.f, blend2.blendConstant()[0]);
    EXPECT_EQ(1.f, blend2.blendConstant()[1]);
    EXPECT_EQ(1.f, blend2.blendConstant()[2]);
    EXPECT_EQ(1.f, blend2.blendConstant()[3]);

    // blend per target
    BlendPerTargetParams blendPerTarget;
    blendPerTarget.setTargetBlend(0, BlendFactor::destInvColor, BlendFactor::destInvAlpha, BlendOp::subtract,
                     BlendFactor::constantInvAlpha, BlendFactor::constantAlpha, BlendOp::revSubtract, ColorComponentFlag::red)
                  .disableTargetBlend(1) .blendConstant(color);
    EXPECT_EQ((BOOL)TRUE, blendPerTarget.descriptor().IndependentBlendEnable);
    EXPECT_EQ((BOOL)TRUE, blendPerTarget.descriptor().RenderTarget[0].BlendEnable);
    EXPECT_EQ((UINT8)D3D11_COLOR_WRITE_ENABLE_RED, blendPerTarget.descriptor().RenderTarget[0].RenderTargetWriteMask);
    EXPECT_EQ(D3D11_BLEND_INV_DEST_COLOR, blendPerTarget.descriptor().RenderTarget[0].SrcBlend);
    EXPECT_EQ(D3D11_BLEND_INV_DEST_ALPHA, blendPerTarget.descriptor().RenderTarget[0].DestBlend);
    EXPECT_EQ(D3D11_BLEND_OP_SUBTRACT, blendPerTarget.descriptor().RenderTarget[0].BlendOp);
    EXPECT_EQ(D3D11_BLEND_INV_BLEND_FACTOR, blendPerTarget.descriptor().RenderTarget[0].SrcBlendAlpha);
    EXPECT_EQ(D3D11_BLEND_BLEND_FACTOR, blendPerTarget.descriptor().RenderTarget[0].DestBlendAlpha);
    EXPECT_EQ(D3D11_BLEND_OP_REV_SUBTRACT, blendPerTarget.descriptor().RenderTarget[0].BlendOpAlpha);
    EXPECT_EQ(color[0], blendPerTarget.blendConstant()[0]);
    EXPECT_EQ(color[1], blendPerTarget.blendConstant()[1]);
    EXPECT_EQ(color[2], blendPerTarget.blendConstant()[2]);
    EXPECT_EQ(color[3], blendPerTarget.blendConstant()[3]);
    EXPECT_EQ((BOOL)FALSE, blendPerTarget.descriptor().RenderTarget[1].BlendEnable);
  }

  // -- GraphicsPipeline --

  static std::unique_ptr<char[]> _readVertexShaderFile(size_t& outBufferSize) {
    std::ifstream binaryShaderFile(_P_TEST_RESOURCE_DIR "/d3d11/" __P_COMPILED_SHADER, std::fstream::in|std::fstream::binary);
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

  TEST_F(D3d11GraphicsPipelineTest, createGraphicsPipelineTest) {
    pandora::hardware::DisplayMonitor monitor;
    std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(monitor);
    GraphicsPipeline::Builder builder(renderer);

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
    D3D11_INPUT_ELEMENT_DESC inputLayoutDescr[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",   0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    auto inputLayout = shaderBuilder.createInputLayout(renderer->resourceManager(), inputLayoutDescr, (size_t)2u);
    ASSERT_TRUE(inputLayout.hasValue());

    // create simple pipeline (no viewport)
    EXPECT_ANY_THROW(builder.build());
    builder.setRenderTargetFormat(nullptr, 0);//TODO
    EXPECT_ANY_THROW(builder.build());
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
                                                    StencilOp::decrementWrap, StencilOp::invert), 2u);
    EXPECT_ANY_THROW(builder.build());
    builder.setBlendState(BlendParams(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add,
                                      BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add));

    GraphicsPipeline pipeline = builder.build();
    ASSERT_TRUE(pipeline.handle() != nullptr);
    EXPECT_FALSE(pipeline.isEmpty());
    EXPECT_TRUE(pipeline.handle()->shaderStages[0].hasValue());
    for (int i = 1; i < __P_D3D11_MAX_DISPLAY_SHADER_STAGE_INDEX + 1; ++i) {
      EXPECT_FALSE(pipeline.handle()->shaderStages[i].hasValue());
    }
    EXPECT_TRUE(pipeline.handle()->blendPerTargetCacheId == nullptr);
    for (int i = 1; i < 4; ++i) {
      EXPECT_EQ(1.f, pipeline.handle()->blendConstant[i]);
    }
    EXPECT_TRUE(pipeline.handle()->rasterizerCacheId != RasterizerStateId{});
    EXPECT_TRUE(pipeline.handle()->depthStencilCacheId != DepthStencilStateId{});
    EXPECT_TRUE(pipeline.handle()->blendCacheId != BlendStateId{});
    EXPECT_TRUE(pipeline.handle()->rasterizerState.hasValue());
    EXPECT_TRUE(pipeline.handle()->depthStencilState.hasValue());
    EXPECT_TRUE(pipeline.handle()->blendState.hasValue());
    EXPECT_TRUE(pipeline.handle()->inputLayout.hasValue());
    EXPECT_EQ(VertexTopology::trianglesAdj, pipeline.handle()->topology);
    EXPECT_EQ((uint32_t)2u, pipeline.handle()->stencilRef);
    EXPECT_EQ(size_t{ 0 }, pipeline.handle()->viewports.size());
    EXPECT_EQ(size_t{ 0 }, pipeline.handle()->scissorTests.size());
    EXPECT_EQ((uint64_t)0, pipeline.handle()->viewportScissorId);
    renderer->bindGraphicsPipeline(pipeline.handle());

    // create pipeline with viewport (+ set same state params -> read from cache)
    Viewport viewport(10.f, 20.f, 800.f, 600.f, 0.1f, 100.f);
    builder.setVertexTopology(VertexTopology::triangles);
    builder.setRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::fill, true, false, false));
    builder.setDepthStencilState(DepthStencilParams(StencilCompare::less, StencilOp::incrementWrap, StencilOp::replace,
                                                    StencilOp::decrementWrap, StencilOp::invert), 1u);
    builder.setBlendState(BlendParams(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add,
                                      BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add));
    builder.clearShaderStages();
    EXPECT_ANY_THROW(builder.build());
    builder.attachShaderStage(vertexShader);
    builder.setViewports(&viewport, size_t{ 1u }, nullptr, 0, true);

    GraphicsPipeline pipelineViewport = builder.build();
    ASSERT_TRUE(pipelineViewport.handle() != nullptr);
    EXPECT_FALSE(pipelineViewport.isEmpty());
    EXPECT_TRUE(pipelineViewport.handle()->shaderStages[0].hasValue());
    EXPECT_TRUE(pipelineViewport.handle()->blendPerTargetCacheId == nullptr);
    EXPECT_TRUE(pipelineViewport.handle()->rasterizerCacheId == pipeline.handle()->rasterizerCacheId);
    EXPECT_TRUE(pipelineViewport.handle()->depthStencilCacheId == pipeline.handle()->depthStencilCacheId);
    EXPECT_TRUE(pipelineViewport.handle()->blendCacheId == pipeline.handle()->blendCacheId);
    EXPECT_TRUE(pipelineViewport.handle()->rasterizerState.get() == pipeline.handle()->rasterizerState.get());
    EXPECT_TRUE(pipelineViewport.handle()->depthStencilState.get() == pipeline.handle()->depthStencilState.get());
    EXPECT_TRUE(pipelineViewport.handle()->blendState.get() == pipeline.handle()->blendState.get());
    EXPECT_TRUE(pipelineViewport.handle()->inputLayout.hasValue());
    EXPECT_EQ(VertexTopology::triangles, pipelineViewport.handle()->topology);
    EXPECT_EQ((uint32_t)1u, pipelineViewport.handle()->stencilRef);
    EXPECT_EQ(size_t{ 1 }, pipelineViewport.handle()->viewports.size());
    EXPECT_EQ(800.f, pipelineViewport.handle()->viewports[0].Width);
    EXPECT_EQ(600.f, pipelineViewport.handle()->viewports[0].Height);
    EXPECT_EQ(10.f, pipelineViewport.handle()->viewports[0].TopLeftX);
    EXPECT_EQ(20.f, pipelineViewport.handle()->viewports[0].TopLeftY);
    EXPECT_EQ(0.1f, pipelineViewport.handle()->viewports[0].MinDepth);
    EXPECT_EQ(100.f, pipelineViewport.handle()->viewports[0].MaxDepth);
    EXPECT_EQ(size_t{ 0 }, pipelineViewport.handle()->scissorTests.size());
    EXPECT_EQ((uint64_t)1LL, pipelineViewport.handle()->viewportScissorId);
    renderer->bindGraphicsPipeline(pipeline.handle());

    // create pipeline with viewport + scissor test (+ different state params + split blending)
    Viewport viewports[]{ Viewport(0.f, 0.f, 800.f, 600.f, 0.1f, 1.f), Viewport(10.f, 20.f, 800.f, 600.f, 0.1f, 100.f) };
    ScissorRectangle scissors[]{ ScissorRectangle(0, 0, 800, 600), ScissorRectangle(10, 20, 400, 300) };
    BlendPerTargetParams blendPerTarget;
    blendPerTarget.setTargetBlend(0, BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add,
                                     BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add)
                  .setTargetBlend(1, BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::subtract,
                                     BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::subtract);
    builder.setRasterizerState(RasterizerParams(CullMode::none, FillMode::fill, true, false, false));
    builder.setDepthStencilState(DepthStencilParams(StencilCompare::greater, StencilOp::zero, StencilOp::keep,
                                                    StencilOp::zero, StencilOp::keep), 1u);
    builder.setBlendState(blendPerTarget);
    builder.setViewports(viewports, size_t{ 2u }, scissors, size_t{ 2u }, false);

    GraphicsPipeline pipelineViewportScissor = builder.build();
    ASSERT_TRUE(pipelineViewportScissor.handle() != nullptr);
    EXPECT_FALSE(pipelineViewportScissor.isEmpty());
    EXPECT_TRUE(pipelineViewportScissor.handle()->shaderStages[0].hasValue());
    EXPECT_TRUE(pipelineViewportScissor.handle()->blendPerTargetCacheId != nullptr);
    EXPECT_TRUE(pipelineViewportScissor.handle()->rasterizerCacheId != RasterizerStateId{});
    EXPECT_TRUE(pipelineViewportScissor.handle()->depthStencilCacheId != DepthStencilStateId{});
    EXPECT_TRUE(pipelineViewportScissor.handle()->blendCacheId == BlendStateId{});
    EXPECT_TRUE(pipelineViewportScissor.handle()->rasterizerCacheId != pipeline.handle()->rasterizerCacheId);
    EXPECT_TRUE(pipelineViewportScissor.handle()->depthStencilCacheId != pipeline.handle()->depthStencilCacheId);
    EXPECT_TRUE(pipelineViewportScissor.handle()->blendCacheId != pipeline.handle()->blendCacheId);
    EXPECT_TRUE(pipelineViewportScissor.handle()->rasterizerState.hasValue());
    EXPECT_TRUE(pipelineViewportScissor.handle()->depthStencilState.hasValue());
    EXPECT_TRUE(pipelineViewportScissor.handle()->blendState.hasValue());
    EXPECT_TRUE(pipelineViewportScissor.handle()->rasterizerState.get() != pipeline.handle()->rasterizerState.get());
    EXPECT_TRUE(pipelineViewportScissor.handle()->depthStencilState.get() != pipeline.handle()->depthStencilState.get());
    EXPECT_TRUE(pipelineViewportScissor.handle()->blendState.get() != pipeline.handle()->blendState.get());
    EXPECT_TRUE(pipelineViewportScissor.handle()->inputLayout.hasValue());
    EXPECT_EQ(VertexTopology::triangles, pipelineViewportScissor.handle()->topology);
    EXPECT_EQ((uint32_t)1u, pipelineViewportScissor.handle()->stencilRef);
    EXPECT_EQ(size_t{ 2u }, pipelineViewportScissor.handle()->viewports.size());
    EXPECT_EQ(800.f, pipelineViewportScissor.handle()->viewports[0].Width);
    EXPECT_EQ(600.f, pipelineViewportScissor.handle()->viewports[0].Height);
    EXPECT_EQ(0.f, pipelineViewportScissor.handle()->viewports[0].TopLeftX);
    EXPECT_EQ(0.f, pipelineViewportScissor.handle()->viewports[0].TopLeftY);
    EXPECT_EQ(0.1f, pipelineViewportScissor.handle()->viewports[0].MinDepth);
    EXPECT_EQ(1.f, pipelineViewportScissor.handle()->viewports[0].MaxDepth);
    EXPECT_EQ(800.f, pipelineViewportScissor.handle()->viewports[1].Width);
    EXPECT_EQ(600.f, pipelineViewportScissor.handle()->viewports[1].Height);
    EXPECT_EQ(10.f, pipelineViewportScissor.handle()->viewports[1].TopLeftX);
    EXPECT_EQ(20.f, pipelineViewportScissor.handle()->viewports[1].TopLeftY);
    EXPECT_EQ(0.1f, pipelineViewportScissor.handle()->viewports[1].MinDepth);
    EXPECT_EQ(100.f, pipelineViewportScissor.handle()->viewports[1].MaxDepth);
    EXPECT_EQ(size_t{ 2u }, pipelineViewportScissor.handle()->scissorTests.size());
    EXPECT_EQ((LONG)0, pipelineViewportScissor.handle()->scissorTests[0].left);
    EXPECT_EQ((LONG)0, pipelineViewportScissor.handle()->scissorTests[0].top);
    EXPECT_EQ((LONG)800, pipelineViewportScissor.handle()->scissorTests[0].right);
    EXPECT_EQ((LONG)600, pipelineViewportScissor.handle()->scissorTests[0].bottom);
    EXPECT_EQ((LONG)10, pipelineViewportScissor.handle()->scissorTests[1].left);
    EXPECT_EQ((LONG)20, pipelineViewportScissor.handle()->scissorTests[1].top);
    EXPECT_EQ((LONG)410, pipelineViewportScissor.handle()->scissorTests[1].right);
    EXPECT_EQ((LONG)320, pipelineViewportScissor.handle()->scissorTests[1].bottom);
    EXPECT_EQ((uint64_t)2LL, pipelineViewportScissor.handle()->viewportScissorId);
    renderer->bindGraphicsPipeline(pipeline.handle());

    builder.detachShaderStage(ShaderType::vertex);
    EXPECT_ANY_THROW(builder.build());

    // move
    auto pipelineHandle = pipeline.handle();
    GraphicsPipeline moved(std::move(pipeline));
    EXPECT_TRUE(pipeline.handle() == nullptr);
    EXPECT_TRUE(moved.handle() != nullptr);
    EXPECT_TRUE(moved.handle().get() == pipelineHandle.get());

    // release
    moved.release();
    EXPECT_TRUE(moved.handle() == nullptr);
    EXPECT_TRUE(moved.isEmpty());
  }

#endif
