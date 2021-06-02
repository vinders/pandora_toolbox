#include <gtest/gtest.h>
#include <video/swap_chain_params.h>

using namespace pandora::video;

class SwapChainParamsTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- Swap-chain params get/set --

TEST_F(SwapChainParamsTest, gettersSettersTest) {
  SwapChainParams params;
  EXPECT_EQ(ComponentFormat::rgba8_sRGB, params.backBufferFormat());
  EXPECT_EQ((int32_t)0, params.customBackBufferFormat());
  EXPECT_EQ(ComponentFormat::d32_f, params.depthBufferFormat());
  EXPECT_EQ((int32_t)0, params.customDepthBufferFormat());
  EXPECT_EQ((uint32_t)2u, params.drawingBufferCount());
  EXPECT_EQ(RenderTargetMode::uniqueOutput, params.renderTargetMode());
  EXPECT_EQ(SwapChainOutputFlag::none, params.outputFlags());
  EXPECT_EQ(AlphaBlending::standard, params.alphaBlending());
  EXPECT_EQ((uint32_t)1u, params.multisampleCount());
  EXPECT_EQ((uint32_t)60u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1u, params.rateDenominator());

  params.setBackBufferFormat(ComponentFormat::rgba32_f);
  EXPECT_EQ(ComponentFormat::rgba32_f, params.backBufferFormat());
  params.setDepthBufferFormat(ComponentFormat::d16_unorm);
  EXPECT_EQ(ComponentFormat::d16_unorm, params.depthBufferFormat());
  params.setDrawingBufferNumber(1u);
  EXPECT_EQ((uint32_t)1u, params.drawingBufferCount());
  params.setDrawingBufferNumber(0);
  EXPECT_NE((uint32_t)0, params.drawingBufferCount());
  params.setRenderTargetMode(RenderTargetMode::none);
  EXPECT_EQ(RenderTargetMode::none, params.renderTargetMode());
  params.setOutputFlags(SwapChainOutputFlag::localOutput | SwapChainOutputFlag::shaderInput);
  EXPECT_EQ((SwapChainOutputFlag::localOutput | SwapChainOutputFlag::shaderInput), params.outputFlags());
  params.setAlphaBlending(AlphaBlending::preMultiplied);
  EXPECT_EQ(AlphaBlending::preMultiplied, params.alphaBlending());
  params.setMultisample(4u);
  EXPECT_EQ((uint32_t)4u, params.multisampleCount());
  params.setRefreshRate(60000u, 1001u);
  EXPECT_EQ((uint32_t)60000u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1001u, params.rateDenominator());
  params.setRefreshRate(60u, 0);
  EXPECT_EQ((uint32_t)60u, params.rateNumerator());
  EXPECT_NE((uint32_t)0, params.rateDenominator());
}

TEST_F(SwapChainParamsTest, gettersSettersCustomFormatTest) {
  SwapChainParams params;
  EXPECT_EQ(ComponentFormat::rgba8_sRGB, params.backBufferFormat());
  EXPECT_EQ((int32_t)0, params.customBackBufferFormat());
  EXPECT_EQ(ComponentFormat::d32_f, params.depthBufferFormat());
  EXPECT_EQ((int32_t)0, params.customDepthBufferFormat());
  
  params.setCustomBackBufferFormat(42);
  EXPECT_EQ(ComponentFormat::custom, params.backBufferFormat());
  EXPECT_EQ((int32_t)42, params.customBackBufferFormat());
  params.setCustomDepthBufferFormat(-42);
  EXPECT_EQ(ComponentFormat::custom, params.depthBufferFormat());
  EXPECT_EQ((int32_t)-42, params.customDepthBufferFormat());
}

TEST_F(SwapChainParamsTest, rateMilliHzTest) {
  SwapChainParams params;
  EXPECT_EQ((uint32_t)60u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1u, params.rateDenominator());

  params.setRefreshRate(60000000u/1001u);
  EXPECT_EQ((uint32_t)60000u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1001u, params.rateDenominator());
  params.setRefreshRate(30000000u/1001u);
  EXPECT_EQ((uint32_t)30000u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1001u, params.rateDenominator());
  params.setRefreshRate(24000000u/1001u);
  EXPECT_EQ((uint32_t)24000u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1001u, params.rateDenominator());
  params.setRefreshRate(12345u);
  EXPECT_EQ((uint32_t)12345u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1000u, params.rateDenominator());
  params.setRefreshRate(12000u);
  EXPECT_EQ((uint32_t)12u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1u, params.rateDenominator());
}

