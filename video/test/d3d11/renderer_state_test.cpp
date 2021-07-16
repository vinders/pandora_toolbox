#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/renderer_state.h>
# include <video/d3d11/renderer_state_factory.h>

  using namespace pandora::video::d3d11;
  using namespace pandora::video;

  class RendererStateTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


  // -- depth/stencil state management --

  TEST_F(RendererStateTest, emptyDepthContainer) {
    DepthStencilState empty;
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.isValid());
    EXPECT_TRUE(empty.get() == nullptr);
    DepthStencilState empty2(nullptr);
    EXPECT_FALSE(empty2);
    EXPECT_FALSE(empty2.isValid());
    EXPECT_TRUE(empty2.get() == nullptr);

    empty = std::move(empty2);
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.isValid());
    EXPECT_TRUE(empty.get() == nullptr);

    DepthStencilState empty3 = std::move(empty);
    EXPECT_FALSE(empty3);
    EXPECT_FALSE(empty3.isValid());
    EXPECT_TRUE(empty3.get() == nullptr);
  }

  TEST_F(RendererStateTest, filledDepthContainer) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);

    RendererStateFactory factory(renderer);

    DepthStencilState valD1 = factory.createDepthStencilTestState(DepthStencilParams(StencilCompare::less, StencilOp::incrementWrap, StencilOp::replace, 
                                                                                     StencilOp::decrementWrap, StencilOp::invert));
    EXPECT_TRUE(valD1);
    EXPECT_TRUE(valD1.isValid());
    EXPECT_TRUE(valD1.get() != nullptr);
    renderer.setDepthStencilState(valD1);
    DepthStencilState valD2 = factory.createDepthStencilTestState(DepthStencilParams(StencilCompare::greaterEqual, StencilOp::zero, StencilOp::replace, 
                                                                                     StencilOp::zero, StencilOp::invert));
    EXPECT_TRUE(valD2);
    EXPECT_TRUE(valD2.isValid());
    EXPECT_TRUE(valD2.get() != nullptr);
    renderer.setDepthStencilState(valD2);
    
    DepthStencilState valS1 = factory.createDepthStencilTestState(DepthStencilParams(StencilCompare::greater, StencilCompare::less, 
                                                                                     StencilOp::incrementWrap, StencilOp::replace, 
                                                                                     StencilOp::decrementWrap, StencilOp::invert));
    EXPECT_TRUE(valS1);
    EXPECT_TRUE(valS1.isValid());
    EXPECT_TRUE(valS1.get() != nullptr);
    renderer.setDepthStencilState(valS1);
    DepthStencilState valS2 = factory.createDepthStencilTestState(DepthStencilParams(StencilCompare::always, StencilCompare::notEqual, 
                                                                                     StencilOp::zero, StencilOp::replace, 
                                                                                     StencilOp::zero, StencilOp::invert));
    EXPECT_TRUE(valS2);
    EXPECT_TRUE(valS2.isValid());
    EXPECT_TRUE(valS2.get() != nullptr);
    renderer.setDepthStencilState(valS2);
    
    DepthStencilState valDS1 = factory.createDepthStencilTestState(DepthStencilParams(StencilCompare::less, StencilCompare::less, StencilCompare::greater, 
                                                                                      StencilOp::zero, StencilOp::incrementWrap, StencilOp::keep, 
                                                                                      StencilOp::zero, StencilOp::decrementClamp, StencilOp::keep));
    EXPECT_TRUE(valDS1);
    EXPECT_TRUE(valDS1.isValid());
    EXPECT_TRUE(valDS1.get() != nullptr);
    renderer.setDepthStencilState(valDS1);
    
    DepthStencilState valDS2 = factory.createDepthStencilTestState(DepthStencilParams(StencilCompare::greater, StencilCompare::always, StencilCompare::always, 
                                                                                      StencilOp::keep, StencilOp::decrementClamp, StencilOp::replace, 
                                                                                      StencilOp::keep, StencilOp::incrementClamp, StencilOp::invert));
    EXPECT_TRUE(valDS2);
    EXPECT_TRUE(valDS2.isValid());
    EXPECT_TRUE(valDS2.get() != nullptr);
    renderer.setDepthStencilState(valDS2);
    renderer.setDepthStencilState(DepthStencilState{});

    auto valGet = valS1.get();
    valD1 = std::move(valS1);
    EXPECT_TRUE(valD1);
    EXPECT_TRUE(valD1.isValid());
    EXPECT_TRUE(valD1.get() != nullptr);
    EXPECT_TRUE(valD1.get() == valGet);
    EXPECT_FALSE(valS1);
    EXPECT_FALSE(valS1.isValid());
    EXPECT_TRUE(valS1.get() == nullptr);

    DepthStencilState moved = std::move(valD1);
    EXPECT_TRUE(moved);
    EXPECT_TRUE(moved.isValid());
    EXPECT_TRUE(moved.get() != nullptr);
    EXPECT_TRUE(moved.get() == valGet);
    EXPECT_FALSE(valD1);
    EXPECT_FALSE(valD1.isValid());
    EXPECT_TRUE(valD1.get() == nullptr);
    EXPECT_FALSE(valS1);
    EXPECT_FALSE(valS1.isValid());
    EXPECT_TRUE(valS1.get() == nullptr);
  }
  
  // -- rasterizer state management --

  TEST_F(RendererStateTest, emptyRasterContainer) {
    RasterizerState empty;
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.isValid());
    EXPECT_TRUE(empty.get() == nullptr);
    RasterizerState empty2(nullptr);
    EXPECT_FALSE(empty2);
    EXPECT_FALSE(empty2.isValid());
    EXPECT_TRUE(empty2.get() == nullptr);

    empty = std::move(empty2);
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.isValid());
    EXPECT_TRUE(empty.get() == nullptr);

    RasterizerState empty3 = std::move(empty);
    EXPECT_FALSE(empty3);
    EXPECT_FALSE(empty3.isValid());
    EXPECT_TRUE(empty3.get() == nullptr);
  }

  TEST_F(RendererStateTest, filledRasterContainer) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    RendererStateFactory factory(renderer);

    RasterizerState val1 = factory.createRasterizerState(RasterizerParams(CullMode::none, FillMode::lines, false, true, false));
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.isValid());
    EXPECT_TRUE(val1.get() != nullptr);
    renderer.setRasterizerState(val1);
    RasterizerState val2(factory.createRasterizerState(RasterizerParams(CullMode::cullBack, FillMode::fill, true, true, true)));
    EXPECT_TRUE(val2);
    EXPECT_TRUE(val2.isValid());
    EXPECT_TRUE(val2.get() != nullptr);
    renderer.setRasterizerState(val2);
    renderer.setRasterizerState(RasterizerState{});

    auto val2Get = val2.get();
    val1 = std::move(val2);
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.isValid());
    EXPECT_TRUE(val1.get() != nullptr);
    EXPECT_TRUE(val1.get() == val2Get);
    EXPECT_FALSE(val2);
    EXPECT_FALSE(val2.isValid());
    EXPECT_TRUE(val2.get() == nullptr);

    RasterizerState val3 = std::move(val1);
    EXPECT_TRUE(val3);
    EXPECT_TRUE(val3.isValid());
    EXPECT_TRUE(val3.get() != nullptr);
    EXPECT_TRUE(val3.get() == val2Get);
    EXPECT_FALSE(val1);
    EXPECT_FALSE(val1.isValid());
    EXPECT_TRUE(val1.get() == nullptr);
    EXPECT_FALSE(val2);
    EXPECT_FALSE(val2.isValid());
    EXPECT_TRUE(val2.get() == nullptr);
  }

  // -- blend state management --

  TEST_F(RendererStateTest, emptyBlendContainer) {
    BlendState empty;
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.isValid());
    EXPECT_TRUE(empty.get() == nullptr);
    BlendState empty2(nullptr);
    EXPECT_FALSE(empty2);
    EXPECT_FALSE(empty2.isValid());
    EXPECT_TRUE(empty2.get() == nullptr);

    empty = std::move(empty2);
    EXPECT_FALSE(empty);
    EXPECT_FALSE(empty.isValid());
    EXPECT_TRUE(empty.get() == nullptr);

    BlendState empty3 = std::move(empty);
    EXPECT_FALSE(empty3);
    EXPECT_FALSE(empty3.isValid());
    EXPECT_TRUE(empty3.get() == nullptr);
  }

  TEST_F(RendererStateTest, filledBlendContainer) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    RendererStateFactory factory(renderer);
    const FLOAT color[4] = { 0.f,0.f,0.f,1.f };

    BlendState val1 = factory.createBlendState(BlendParams(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add, 
                                                           BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add));
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.isValid());
    EXPECT_TRUE(val1.get() != nullptr);
    renderer.setBlendState(val1);
    renderer.setBlendState(val1, color);
    BlendState val2(factory.createBlendState(BlendParams(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::revSubtract, 
                                                         BlendFactor::one, BlendFactor::zero, BlendOp::maximum)));
    EXPECT_TRUE(val2);
    EXPECT_TRUE(val2.isValid());
    EXPECT_TRUE(val2.get() != nullptr);
    renderer.setBlendState(val2);
    renderer.setBlendState(val2, color);
    renderer.setBlendState(BlendState{});

    BlendState val3(factory.createBlendState(BlendParams(BlendFactor::constantColor, BlendFactor::constantInvColor, BlendOp::subtract,
                                                         BlendFactor::constantAlpha, BlendFactor::constantInvAlpha, BlendOp::subtract)));
    EXPECT_TRUE(val3);
    EXPECT_TRUE(val3.isValid());
    EXPECT_TRUE(val3.get() != nullptr);
    renderer.setBlendState(val3);
    renderer.setBlendState(val3, color);
    renderer.setBlendState(BlendState{});

    BlendTargetParams params1[]{ BlendTargetParams(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOp::add, BlendFactor::sourceAlpha, BlendFactor::destInvAlpha, BlendOp::add), 
                                 BlendTargetParams(BlendFactor::sourceInvColor, BlendFactor::destColor, BlendOp::subtract, BlendFactor::sourceInvAlpha, BlendFactor::destAlpha, BlendOp::subtract),
                                 BlendTargetParams(BlendFactor::one, BlendFactor::zero, BlendOp::add, BlendFactor::one, BlendFactor::zero, BlendOp::add, false) };
    BlendState val4(factory.createBlendStatePerTarget(params1, (size_t)3u));
    EXPECT_TRUE(val4);
    EXPECT_TRUE(val4.isValid());
    EXPECT_TRUE(val4.get() != nullptr);
    renderer.setBlendState(val4);
    renderer.setBlendState(val4, color);
    renderer.setBlendState(BlendState{});

    BlendTargetParams params2(BlendFactor::sourceColor, BlendFactor::destColor, BlendOp::minimum, 
                              BlendFactor::one, BlendFactor::one, BlendOp::maximum);
    BlendState val5(factory.createBlendStatePerTarget(&params2, (size_t)1u));
    EXPECT_TRUE(val5);
    EXPECT_TRUE(val5.isValid());
    EXPECT_TRUE(val5.get() != nullptr);
    renderer.setBlendState(val5);
    renderer.setBlendState(val5, color);
    renderer.setBlendState(BlendState{});

    auto val2Get = val2.get();
    val1 = std::move(val2);
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.isValid());
    EXPECT_TRUE(val1.get() != nullptr);
    EXPECT_TRUE(val1.get() == val2Get);
    EXPECT_FALSE(val2);
    EXPECT_FALSE(val2.isValid());
    EXPECT_TRUE(val2.get() == nullptr);

    BlendState valM = std::move(val1);
    EXPECT_TRUE(valM);
    EXPECT_TRUE(valM.isValid());
    EXPECT_TRUE(valM.get() != nullptr);
    EXPECT_TRUE(valM.get() == val2Get);
    EXPECT_FALSE(val1);
    EXPECT_FALSE(val1.isValid());
    EXPECT_TRUE(val1.get() == nullptr);
    EXPECT_FALSE(val2);
    EXPECT_FALSE(val2.isValid());
    EXPECT_TRUE(val2.get() == nullptr);
  }

  // -- sampler/filter state management --

  TEST_F(RendererStateTest, emptyFilterContainer) {
    FilterStateArray empty;
    EXPECT_FALSE(empty);
    EXPECT_EQ((size_t)0, empty.size());
    FilterStateArray empty2{};
    EXPECT_FALSE(empty2);
    EXPECT_EQ((size_t)0, empty2.size());

    empty = std::move(empty2);
    EXPECT_FALSE(empty);
    EXPECT_EQ((size_t)0, empty.size());

    FilterStateArray empty3 = std::move(empty);
    EXPECT_FALSE(empty3);
    EXPECT_EQ((size_t)0, empty3.size());
  }

  TEST_F(RendererStateTest, emptyValuesFilterContainer) {
    FilterStateArray values;
    EXPECT_FALSE(values);
    EXPECT_TRUE(values.empty());
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_ANY_THROW(values.at(0));
    values.erase(0);
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_FALSE(values.insert(1, FilterState(nullptr)));
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.insert(0, FilterState(nullptr)));
    EXPECT_TRUE(values);
    EXPECT_FALSE(values.empty());
    EXPECT_EQ((size_t)1, values.size());
    EXPECT_ANY_THROW(values.at(1));
    EXPECT_TRUE(values.get() != nullptr);
    EXPECT_FALSE(values.at(0).isValid());
    EXPECT_TRUE(values.get()[0] == nullptr);
    values.erase(0);
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.append(FilterState(nullptr)));
    EXPECT_EQ((size_t)1, values.size());
    EXPECT_TRUE(values.get() != nullptr);
    EXPECT_FALSE(values.at(0).isValid());
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_TRUE(values.insert(0, FilterState(nullptr)));
    EXPECT_EQ((size_t)2, values.size());
    EXPECT_ANY_THROW(values.at(2));
    EXPECT_TRUE(values.get() != nullptr);
    EXPECT_FALSE(values.at(0).isValid());
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_FALSE(values.at(1).isValid());
    EXPECT_TRUE(values.get()[1] == nullptr);

    size_t length = values.size();
    while (values.size() < values.maxSize()) {
      EXPECT_TRUE(values.append(FilterState(nullptr)));
      EXPECT_EQ((size_t)++length, values.size());
    }
    EXPECT_EQ(values.maxSize(), values.size());
    EXPECT_FALSE(values.append(FilterState(nullptr)));
    EXPECT_FALSE(values.insert(0, FilterState(nullptr)));
    EXPECT_FALSE(values.insert((uint32_t)values.size(), FilterState(nullptr)));
    EXPECT_EQ(values.maxSize(), values.size());

    values.clear();
    EXPECT_FALSE(values);
    EXPECT_ANY_THROW(values.at(0));
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.append(FilterState(nullptr)));
    EXPECT_EQ((size_t)1, values.size());
    length = values.size();
    while (values.size() < values.maxSize()) {
      EXPECT_TRUE(values.insert((uint32_t)values.size(), FilterState(nullptr)));
      EXPECT_EQ((size_t)++length, values.size());
    }
    EXPECT_EQ(values.maxSize(), values.size());
    EXPECT_FALSE(values.append(FilterState(nullptr)));
    EXPECT_FALSE(values.insert(0, FilterState(nullptr)));

    auto address = values.get();
    FilterStateArray moved(std::move(values));
    ASSERT_TRUE(moved.get() != nullptr);
    ASSERT_TRUE(values.get() == nullptr);
    EXPECT_EQ(moved.maxSize(), moved.size());
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(moved.get() == address);
    values = std::move(moved);
    ASSERT_TRUE(values.get() != nullptr);
    ASSERT_TRUE(moved.get() == nullptr);
    EXPECT_EQ(values.maxSize(), values.size());
    EXPECT_EQ((size_t)0, moved.size());
    EXPECT_TRUE(values.get() == address);

    values.erase((uint32_t)values.size()); // out of range
    EXPECT_EQ(values.maxSize(), values.size());
    values.erase((uint32_t)values.size() - 1u);
    EXPECT_EQ(values.maxSize() - 1u, values.size());
    values.erase(0);
    EXPECT_EQ(values.maxSize() - 2u, values.size());
    values.erase((uint32_t)values.size()); // out of range
    EXPECT_EQ(values.maxSize() - 2u, values.size());
  }

  TEST_F(RendererStateTest, filledFilterContainer) {
    pandora::hardware::DisplayMonitor monitor;
    Renderer renderer(monitor);
    RendererStateFactory factory(renderer);
    
    TextureWrap addrModes[3] { 
      TextureWrap::repeat, TextureWrap::repeat, TextureWrap::repeat
    };

    FilterStateArray values;
    values.append(factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::linear,
                                                         addrModes, 0.f,0.f)));
    ASSERT_EQ((size_t)1, values.size());
    auto val1 = values.at(0).extract();
    values.append(factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::linear, TextureFilter::linear,
                                                         addrModes, 0.f,0.f)));
    ASSERT_EQ((size_t)2, values.size());
    EXPECT_EQ(val1, values.at(0).extract());
    auto val2 = values.at(1).extract();
    values.insert(0, factory.createFilterState(FilterParams(TextureFilter::linear, TextureFilter::linear, TextureFilter::nearest,
                                                            addrModes, 0.f,0.f)));
    ASSERT_EQ((size_t)3, values.size());
    EXPECT_EQ(val1, values.at(1).extract());
    EXPECT_EQ(val2, values.at(2).extract());
    auto val0 = values.at(0).extract();
    values.insert(1, factory.createFilterState(FilterParams(TextureFilter::nearest, TextureFilter::nearest, TextureFilter::nearest,
                                                            addrModes, 0.f,0.f)));
    ASSERT_EQ((size_t)4, values.size());
    EXPECT_EQ(val0, values.at(0).extract());
    EXPECT_EQ(val1, values.at(2).extract());
    EXPECT_EQ(val2, values.at(3).extract());
    values.erase(1);
    ASSERT_EQ((size_t)3, values.size());
    EXPECT_EQ(val0, values.at(0).extract());
    EXPECT_EQ(val1, values.at(1).extract());
    EXPECT_EQ(val2, values.at(2).extract());
    values.erase(0);
    ASSERT_EQ((size_t)2, values.size());
    EXPECT_EQ(val1, values.at(0).extract());
    EXPECT_EQ(val2, values.at(1).extract());
    values.erase(1);
    ASSERT_EQ((size_t)1, values.size());
    EXPECT_EQ(val1, values.at(0).extract());
    values.clear();
    EXPECT_EQ((size_t)0, values.size());
  }

  TEST_F(RendererStateTest, stateParamsAccessors) {
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

    DepthStencilParams depth;
    depth.enableDepthTest(false).enableStencilTest(true).depthTest(StencilCompare::always)
      .frontFaceStencilTest(StencilCompare::equal).backFaceStencilTest(StencilCompare::greater)
      .frontFaceOp(StencilOp::decrementClamp, StencilOp::incrementClamp, StencilOp::replace).backFaceOp(StencilOp::invert, StencilOp::zero, StencilOp::incrementWrap)
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

    DepthStencilParams depth2(StencilCompare::greaterEqual, StencilOp::invert, StencilOp::replace, StencilOp::incrementClamp, StencilOp::decrementClamp);
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

    DepthStencilParams depth3(StencilCompare::notEqual, StencilCompare::lessEqual, StencilOp::replace, StencilOp::invert, StencilOp::decrementClamp, StencilOp::incrementClamp);
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

    DepthStencilParams depth4(StencilCompare::greaterEqual, StencilCompare::equal, StencilCompare::notEqual, 
                              StencilOp::replace, StencilOp::invert, StencilOp::zero, StencilOp::incrementClamp, StencilOp::decrementClamp, StencilOp::invert);
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

    BlendTargetParams blendTarget(BlendFactor::destInvColor, BlendFactor::destInvAlpha, BlendOp::subtract,
                                  BlendFactor::constantInvAlpha, BlendFactor::constantAlpha, BlendOp::revSubtract, false, ColorComponentFlag::red);
    EXPECT_EQ(BlendFactor::destInvColor, blendTarget.srcColorFactor);
    EXPECT_EQ(BlendFactor::destInvAlpha, blendTarget.destColorFactor);
    EXPECT_EQ(BlendOp::subtract, blendTarget.colorBlendOp);
    EXPECT_EQ(BlendFactor::constantInvAlpha, blendTarget.srcAlphaFactor);
    EXPECT_EQ(BlendFactor::constantAlpha, blendTarget.destAlphaFactor);
    EXPECT_EQ(BlendOp::revSubtract, blendTarget.alphaBlendOp);
    EXPECT_EQ(false, blendTarget.isEnabled);
    EXPECT_EQ(ColorComponentFlag::red, blendTarget.targetWriteMask);

    BlendParams blend;
    blend.srcColorFactor(BlendFactor::destInvColor).destColorFactor(BlendFactor::destInvAlpha).colorBlendOp(BlendOp::subtract)
      .srcAlphaFactor(BlendFactor::constantInvAlpha).destAlphaFactor(BlendFactor::constantAlpha).alphaBlendOp(BlendOp::revSubtract)
      .targetWriteMask(ColorComponentFlag::red).enable(false);
    EXPECT_EQ((BOOL)FALSE, blend.descriptor().IndependentBlendEnable);
    EXPECT_EQ((BOOL)FALSE, blend.descriptor().RenderTarget->BlendEnable);
    EXPECT_EQ((UINT8)D3D10_COLOR_WRITE_ENABLE_RED, blend.descriptor().RenderTarget->RenderTargetWriteMask);
    EXPECT_EQ(D3D11_BLEND_INV_DEST_COLOR, blend.descriptor().RenderTarget->SrcBlend);
    EXPECT_EQ(D3D11_BLEND_INV_DEST_ALPHA, blend.descriptor().RenderTarget->DestBlend);
    EXPECT_EQ(D3D11_BLEND_OP_SUBTRACT, blend.descriptor().RenderTarget->BlendOp);
    EXPECT_EQ(D3D11_BLEND_INV_BLEND_FACTOR, blend.descriptor().RenderTarget->SrcBlendAlpha);
    EXPECT_EQ(D3D11_BLEND_BLEND_FACTOR, blend.descriptor().RenderTarget->DestBlendAlpha);
    EXPECT_EQ(D3D11_BLEND_OP_REV_SUBTRACT, blend.descriptor().RenderTarget->BlendOpAlpha);

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

    ColorChannel border[4]{ 1.f, 2.f, 3.f, 4.f };
    TextureWrap wrap[3]{ TextureWrap::repeatMirror, TextureWrap::mirrorOnce, TextureWrap::clampToEdge };
    FilterParams filter;
    filter.setFilter(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest).textureWrap(wrap).borderColor(border).lod(1.f, 2.f).lodBias(3.f);
    EXPECT_EQ(D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, filter.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter.descriptor().AddressW);
    EXPECT_EQ((FLOAT)1.f, filter.descriptor().BorderColor[0]);
    EXPECT_EQ((FLOAT)2.f, filter.descriptor().BorderColor[1]);
    EXPECT_EQ((FLOAT)3.f, filter.descriptor().BorderColor[2]);
    EXPECT_EQ((FLOAT)4.f, filter.descriptor().BorderColor[3]);
    EXPECT_EQ((UINT)1u, filter.descriptor().MaxAnisotropy);
    EXPECT_EQ((FLOAT)1.f, filter.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)2.f, filter.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)3.f, filter.descriptor().MipLODBias);
    FilterParams filter2;
    filter2.setFilter(TextureFilter::linear, TextureFilter::nearest, TextureFilter::linear, StencilCompare::greaterEqual).textureWrap(wrap).lod(0.f, 1.f);
    EXPECT_EQ(D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR, filter2.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter2.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter2.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter2.descriptor().AddressW);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().BorderColor[0]);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().BorderColor[1]);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().BorderColor[2]);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().BorderColor[3]);
    EXPECT_EQ((UINT)1u, filter2.descriptor().MaxAnisotropy);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, filter2.descriptor().ComparisonFunc);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)1.f, filter2.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter2.descriptor().MipLODBias);

    FilterParams filter3;
    filter3.setAnisotropicFilter(1u).textureWrap(wrap).borderColor(border).lod(1.f, 2.f).lodBias(3.f);
    EXPECT_EQ(D3D11_FILTER_ANISOTROPIC, filter3.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter3.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter3.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter3.descriptor().AddressW);
    EXPECT_EQ((FLOAT)1.f, filter3.descriptor().BorderColor[0]);
    EXPECT_EQ((FLOAT)2.f, filter3.descriptor().BorderColor[1]);
    EXPECT_EQ((FLOAT)3.f, filter3.descriptor().BorderColor[2]);
    EXPECT_EQ((FLOAT)4.f, filter3.descriptor().BorderColor[3]);
    EXPECT_EQ((UINT)1u, filter3.descriptor().MaxAnisotropy);
    EXPECT_EQ((FLOAT)1.f, filter3.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)2.f, filter3.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)3.f, filter3.descriptor().MipLODBias);
    FilterParams filter4;
    filter4.setAnisotropicFilter(8u, StencilCompare::greaterEqual).textureWrap(wrap).lod(0.f, 1.f);
    EXPECT_EQ(D3D11_FILTER_COMPARISON_ANISOTROPIC, filter4.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter4.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter4.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter4.descriptor().AddressW);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().BorderColor[0]);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().BorderColor[1]);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().BorderColor[2]);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().BorderColor[3]);
    EXPECT_EQ((UINT)8u, filter4.descriptor().MaxAnisotropy);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, filter4.descriptor().ComparisonFunc);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)1.f, filter4.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter4.descriptor().MipLODBias);

    FilterParams filter5(TextureFilter::nearest, TextureFilter::linear, TextureFilter::nearest, wrap, 1.f, 2.f);
    EXPECT_EQ(D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, filter5.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter5.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter5.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter5.descriptor().AddressW);
    EXPECT_EQ((UINT)1u, filter5.descriptor().MaxAnisotropy);
    EXPECT_EQ((FLOAT)1.f, filter5.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)2.f, filter5.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter5.descriptor().MipLODBias);
    FilterParams filter6(TextureFilter::linear, TextureFilter::nearest, TextureFilter::linear, wrap, 0.f, 1.f, StencilCompare::greaterEqual);
    EXPECT_EQ(D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR, filter6.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter6.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter6.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter6.descriptor().AddressW);
    EXPECT_EQ((UINT)1u, filter6.descriptor().MaxAnisotropy);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, filter6.descriptor().ComparisonFunc);
    EXPECT_EQ((FLOAT)0.f, filter6.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)1.f, filter6.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter6.descriptor().MipLODBias);

    FilterParams filter7(1u, wrap, 1.f, 2.f);
    EXPECT_EQ(D3D11_FILTER_ANISOTROPIC, filter7.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter7.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter7.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter7.descriptor().AddressW);
    EXPECT_EQ((UINT)1u, filter7.descriptor().MaxAnisotropy);
    EXPECT_EQ((FLOAT)1.f, filter7.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)2.f, filter7.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter7.descriptor().MipLODBias);
    FilterParams filter8(8u, wrap, 0.f, 1.f, StencilCompare::greaterEqual);
    EXPECT_EQ(D3D11_FILTER_COMPARISON_ANISOTROPIC, filter8.descriptor().Filter);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR, filter8.descriptor().AddressU);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_MIRROR_ONCE, filter8.descriptor().AddressV);
    EXPECT_EQ(D3D11_TEXTURE_ADDRESS_CLAMP, filter8.descriptor().AddressW);
    EXPECT_EQ((UINT)8u, filter8.descriptor().MaxAnisotropy);
    EXPECT_EQ(D3D11_COMPARISON_GREATER_EQUAL, filter8.descriptor().ComparisonFunc);
    EXPECT_EQ((FLOAT)0.f, filter8.descriptor().MinLOD);
    EXPECT_EQ((FLOAT)1.f, filter8.descriptor().MaxLOD);
    EXPECT_EQ((FLOAT)0.f, filter8.descriptor().MipLODBias);
  }

#endif
