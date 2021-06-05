#if defined(_WINDOWS) && defined(_VIDEO_D3D11_SUPPORT)
# include <gtest/gtest.h>
# include <video/d3d11/renderer.h>
# include <video/d3d11/renderer_state.h>

  using namespace pandora::video::d3d11;

  class RendererStateTest : public testing::Test {
  public:
  protected:
    //static void SetUpTestCase() {}
    //static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
  };


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
    Renderer renderer(monitor, Renderer::DeviceLevel::direct3D_11_0);

    RasterizerState val1 = renderer.createRasterizerState(pandora::video::CullMode::wireFrame, false, 
                                    pandora::video::DepthBias{ 100, 0.5f, 0.5f, true }, false, false);
    EXPECT_TRUE(val1);
    EXPECT_TRUE(val1.isValid());
    EXPECT_TRUE(val1.get() != nullptr);
    RasterizerState val2(renderer.createRasterizerState(pandora::video::CullMode::cullBack, true, 
                                  pandora::video::DepthBias{ 0, -0.25f, 1.f, true }, true, true));
    EXPECT_TRUE(val2);
    EXPECT_TRUE(val2.isValid());
    EXPECT_TRUE(val2.get() != nullptr);
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

  // -- sampler/filter state management --

  TEST_F(RendererStateTest, emptyFilterContainer) {
    FilterStates empty;
    EXPECT_FALSE(empty);
    EXPECT_EQ((size_t)0, empty.size());
    FilterStates empty2{};
    EXPECT_FALSE(empty2);
    EXPECT_EQ((size_t)0, empty2.size());

    empty = std::move(empty2);
    EXPECT_FALSE(empty);
    EXPECT_EQ((size_t)0, empty.size());

    FilterStates empty3 = std::move(empty);
    EXPECT_FALSE(empty3);
    EXPECT_EQ((size_t)0, empty3.size());
  }

  TEST_F(RendererStateTest, emptyValuesFilterContainer) {
    FilterStates values;
    EXPECT_FALSE(values);
    EXPECT_TRUE(values.empty());
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_ANY_THROW(values.at(0));
    values.erase(0);
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_FALSE(values.insert(1, nullptr));
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.insert(0, nullptr));
    EXPECT_TRUE(values);
    EXPECT_FALSE(values.empty());
    EXPECT_EQ((size_t)1, values.size());
    EXPECT_ANY_THROW(values.at(1));
    EXPECT_TRUE(values.get() != nullptr);
    EXPECT_TRUE(values.at(0) == nullptr);
    EXPECT_TRUE(values.get()[0] == nullptr);
    values.erase(0);
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.append(nullptr));
    EXPECT_EQ((size_t)1, values.size());
    EXPECT_TRUE(values.get() != nullptr);
    EXPECT_TRUE(values.at(0) == nullptr);
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_TRUE(values.insert(0, nullptr));
    EXPECT_EQ((size_t)2, values.size());
    EXPECT_ANY_THROW(values.at(2));
    EXPECT_TRUE(values.get() != nullptr);
    EXPECT_TRUE(values.at(0) == nullptr);
    EXPECT_TRUE(values.get()[0] == nullptr);
    EXPECT_TRUE(values.at(1) == nullptr);
    EXPECT_TRUE(values.get()[1] == nullptr);

    size_t length = values.size();
    while (values.size() < values.maxSize()) {
      EXPECT_TRUE(values.append(nullptr));
      EXPECT_EQ((size_t)++length, values.size());
    }
    EXPECT_EQ(values.maxSize(), values.size());
    EXPECT_FALSE(values.append(nullptr));
    EXPECT_FALSE(values.insert(0, nullptr));
    EXPECT_FALSE(values.insert((uint32_t)values.size(), nullptr));
    EXPECT_EQ(values.maxSize(), values.size());

    values.clear();
    EXPECT_FALSE(values);
    EXPECT_ANY_THROW(values.at(0));
    EXPECT_EQ((size_t)0, values.size());
    EXPECT_TRUE(values.append(nullptr));
    EXPECT_EQ((size_t)1, values.size());
    length = values.size();
    while (values.size() < values.maxSize()) {
      EXPECT_TRUE(values.insert((uint32_t)values.size(), nullptr));
      EXPECT_EQ((size_t)++length, values.size());
    }
    EXPECT_EQ(values.maxSize(), values.size());
    EXPECT_FALSE(values.append(nullptr));
    EXPECT_FALSE(values.insert(0, nullptr));

    auto address = values.get();
    FilterStates moved(std::move(values));
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
    Renderer renderer(monitor, Renderer::DeviceLevel::direct3D_11_0);

    FilterStates values;
    renderer.createFilterState(values);
    ASSERT_EQ((size_t)1, values.size());
    auto val1 = values.at(0);
    renderer.createFilterState(values);
    ASSERT_EQ((size_t)2, values.size());
    EXPECT_EQ(val1, values.at(0));
    auto val2 = values.at(1);
    renderer.createFilterState(values, 0);
    ASSERT_EQ((size_t)3, values.size());
    EXPECT_EQ(val1, values.at(1));
    EXPECT_EQ(val2, values.at(2));
    auto val0 = values.at(0);
    renderer.createFilterState(values, 1);
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
