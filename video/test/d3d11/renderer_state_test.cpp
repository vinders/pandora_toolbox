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

    DepthStencilState valD1 = factory.createDepthTestState(DepthOperationGroup{ DepthStencilOperation::incrementWrap, DepthStencilOperation::keep }, 
                                                           DepthOperationGroup{ DepthStencilOperation::decrementWrap, DepthStencilOperation::keep }, 
                                                           DepthComparison::less, true);
    EXPECT_TRUE(valD1);
    EXPECT_TRUE(valD1.isValid());
    EXPECT_TRUE(valD1.get() != nullptr);
    renderer.setDepthStencilState(valD1);
    DepthStencilState valD2 = factory.createDepthTestState(DepthOperationGroup{ DepthStencilOperation::setZero, DepthStencilOperation::replace }, 
                                                           DepthOperationGroup{ DepthStencilOperation::setZero, DepthStencilOperation::invert }, 
                                                           DepthComparison::greaterEqual, false);
    EXPECT_TRUE(valD2);
    EXPECT_TRUE(valD2.isValid());
    EXPECT_TRUE(valD2.get() != nullptr);
    renderer.setDepthStencilState(valD2);
    
    DepthStencilState valS1 = factory.createStencilTestState(DepthStencilOperationGroup{ DepthStencilOperation::setZero,
                                                               DepthStencilOperation::incrementWrap, DepthStencilOperation::keep, 
                                                               DepthComparison::greater }, 
                                                             DepthStencilOperationGroup{ DepthStencilOperation::setZero,
                                                               DepthStencilOperation::decrementWrap, DepthStencilOperation::keep, 
                                                               DepthComparison::less },
                                                             (uint8_t)0xFF, (uint8_t)0xFF);
    EXPECT_TRUE(valS1);
    EXPECT_TRUE(valS1.isValid());
    EXPECT_TRUE(valS1.get() != nullptr);
    renderer.setDepthStencilState(valS1);
    DepthStencilState valS2 = factory.createStencilTestState(DepthStencilOperationGroup{ DepthStencilOperation::setZero,
                                                               DepthStencilOperation::setZero, DepthStencilOperation::replace, 
                                                               DepthComparison::always }, 
                                                             DepthStencilOperationGroup{ DepthStencilOperation::setZero,
                                                               DepthStencilOperation::setZero, DepthStencilOperation::invert, 
                                                               DepthComparison::notEqual },
                                                             (uint8_t)0x1, (uint8_t)0x1);
    EXPECT_TRUE(valS2);
    EXPECT_TRUE(valS2.isValid());
    EXPECT_TRUE(valS2.get() != nullptr);
    renderer.setDepthStencilState(valS2);
    
    DepthStencilState valDS1 = factory.createDepthStencilTestState(DepthStencilOperationGroup{ DepthStencilOperation::setZero,
                                                                     DepthStencilOperation::incrementWrap, DepthStencilOperation::keep, 
                                                                     DepthComparison::greater }, 
                                                                   DepthStencilOperationGroup{ DepthStencilOperation::setZero,
                                                                     DepthStencilOperation::decrementWrap, DepthStencilOperation::keep, 
                                                                     DepthComparison::less },
                                                                   DepthComparison::less, true, (uint8_t)0xFF, (uint8_t)0xFF);
    EXPECT_TRUE(valDS1);
    EXPECT_TRUE(valDS1.isValid());
    EXPECT_TRUE(valDS1.get() != nullptr);
    renderer.setDepthStencilState(valDS1);
    
    DepthStencilState valDS2 = factory.createDepthStencilTestState(DepthStencilOperationGroup{ DepthStencilOperation::keep,
                                                                     DepthStencilOperation::decrementWrap, DepthStencilOperation::replace, 
                                                                     DepthComparison::always }, 
                                                                   DepthStencilOperationGroup{ DepthStencilOperation::keep,
                                                                     DepthStencilOperation::incrementWrap, DepthStencilOperation::invert, 
                                                                     DepthComparison::always },
                                                                   DepthComparison::greater, false, (uint8_t)0x1, (uint8_t)0x1);
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

    RasterizerState val1 = factory.createRasterizerState(pandora::video::CullMode::wireFrame, false, 
                                   pandora::video::DepthBias{ 100, 0.5f, 0.5f, true }, false);
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.isValid());
    EXPECT_TRUE(val1.get() != nullptr);
    renderer.setRasterizerState(val1);
    RasterizerState val2(factory.createRasterizerState(pandora::video::CullMode::cullBack, true, 
                                 pandora::video::DepthBias{ 0, -0.25f, 1.f, true }, true));
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

    BlendState val1 = factory.createBlendState(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOperator::add);
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.isValid());
    EXPECT_TRUE(val1.get() != nullptr);
    renderer.setBlendState(val1);
    renderer.setBlendState(val1, color);
    BlendState val2(factory.createBlendState(BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOperator::revSubtract, 
                                             BlendFactor::one, BlendFactor::zero, BlendOperator::maximum));
    EXPECT_TRUE(val2);
    EXPECT_TRUE(val2.isValid());
    EXPECT_TRUE(val2.get() != nullptr);
    renderer.setBlendState(val2);
    renderer.setBlendState(val2, color);
    renderer.setBlendState(BlendState{});

    BlendState val3(factory.createBlendState(BlendFactor::constantColor, BlendFactor::constantInvColor, BlendOperator::subtract));
    EXPECT_TRUE(val3);
    EXPECT_TRUE(val3.isValid());
    EXPECT_TRUE(val3.get() != nullptr);
    renderer.setBlendState(val3);
    renderer.setBlendState(val3, color);
    renderer.setBlendState(BlendState{});

    TargetBlendingParams params1[]{ {BlendFactor::sourceColor, BlendFactor::destInvColor, BlendOperator::add}, 
                                    {BlendFactor::sourceInvColor, BlendFactor::destColor, BlendOperator::subtract},
                                    {BlendFactor::one, BlendFactor::zero, BlendOperator::none} };
    BlendState val4(factory.createBlendStatePerTarget(params1, (size_t)3u));
    EXPECT_TRUE(val4);
    EXPECT_TRUE(val4.isValid());
    EXPECT_TRUE(val4.get() != nullptr);
    renderer.setBlendState(val4);
    renderer.setBlendState(val4, color);
    renderer.setBlendState(BlendState{});

    TargetBlendingSplitParams params2 = { BlendFactor::sourceColor, BlendFactor::destColor, BlendOperator::minimum, 
                                          BlendFactor::one, BlendFactor::one, BlendOperator::maximum };
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
    EXPECT_TRUE(values.at(0) == nullptr);
    EXPECT_TRUE(values.get()[0] == nullptr);
    values.erase(0);
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.append(FilterState(nullptr)));
    EXPECT_EQ((size_t)1, values.size());
    EXPECT_TRUE(values.get() != nullptr);
    EXPECT_TRUE(values.at(0) == nullptr);
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_TRUE(values.insert(0, FilterState(nullptr)));
    EXPECT_EQ((size_t)2, values.size());
    EXPECT_ANY_THROW(values.at(2));
    EXPECT_TRUE(values.get() != nullptr);
    EXPECT_TRUE(values.at(0) == nullptr);
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_TRUE(values.at(1) == nullptr);
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
    
    float borderColor[4] { 0.,0.,0.,1. };
    pandora::video::TextureAddressMode addrModes[3] { 
      pandora::video::TextureAddressMode::repeat, pandora::video::TextureAddressMode::repeat, pandora::video::TextureAddressMode::repeat
    };

    FilterStateArray values;
    values.append(factory.createFilter(pandora::video::MinificationFilter::linear, 
                          pandora::video::MagnificationFilter::linear, addrModes, 0.,0., 0.0, borderColor));
    ASSERT_EQ((size_t)1, values.size());
    auto val1 = values.at(0);
    values.append(factory.createFilter(pandora::video::MinificationFilter::linear, 
                          pandora::video::MagnificationFilter::linear, addrModes, 0.,0., 0.0, borderColor));
    ASSERT_EQ((size_t)2, values.size());
    EXPECT_EQ(val1, values.at(0));
    auto val2 = values.at(1);
    values.insert(0, factory.createFilter(pandora::video::MinificationFilter::linear, 
                             pandora::video::MagnificationFilter::linear, addrModes, 0.,0., 0.0, borderColor));
    ASSERT_EQ((size_t)3, values.size());
    EXPECT_EQ(val1, values.at(1));
    EXPECT_EQ(val2, values.at(2));
    auto val0 = values.at(0);
    values.insert(1, factory.createFilter(pandora::video::MinificationFilter::linear, 
                             pandora::video::MagnificationFilter::linear, addrModes, 0.,0., 0.0, borderColor));
    ASSERT_EQ((size_t)4, values.size());
    EXPECT_EQ(val0, values.at(0));
    EXPECT_EQ(val1, values.at(2));
    EXPECT_EQ(val2, values.at(3));
    values.erase(1);
    ASSERT_EQ((size_t)3, values.size());
    EXPECT_EQ(val0, values.at(0));
    EXPECT_EQ(val1, values.at(1));
    EXPECT_EQ(val2, values.at(2));
    values.erase(0);
    ASSERT_EQ((size_t)2, values.size());
    EXPECT_EQ(val1, values.at(0));
    EXPECT_EQ(val2, values.at(1));
    values.erase(1);
    ASSERT_EQ((size_t)1, values.size());
    EXPECT_EQ(val1, values.at(0));
    values.clear();
    EXPECT_EQ((size_t)0, values.size());
  }

#endif
