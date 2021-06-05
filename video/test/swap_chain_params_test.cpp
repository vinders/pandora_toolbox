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
  EXPECT_EQ((uint32_t)2u, params.frameBufferCount());
  EXPECT_EQ(false, params.isHdrPreferred());
  EXPECT_EQ(SwapChainTargetMode::uniqueOutput, params.renderTargetMode());
  EXPECT_EQ(SwapChainOutputFlag::none, params.outputFlags());
  EXPECT_EQ((uint32_t)1u, params.multisampleCount());
  EXPECT_EQ((uint32_t)60u, params.rateNumerator());
  EXPECT_EQ((uint32_t)1u, params.rateDenominator());

  params.setBackBufferFormat(ComponentFormat::rgba32_f);
  EXPECT_EQ(ComponentFormat::rgba32_f, params.backBufferFormat());
  params.setFrameBufferNumber(1u);
  EXPECT_EQ((uint32_t)1u, params.frameBufferCount());
  params.setFrameBufferNumber(0);
  EXPECT_NE((uint32_t)0, params.frameBufferCount());
  params.setHdrPreferred(true);
  EXPECT_EQ(true, params.isHdrPreferred());
  params.setRenderTargetMode(SwapChainTargetMode::partialOutput);
  EXPECT_EQ(SwapChainTargetMode::partialOutput, params.renderTargetMode());
  params.setOutputFlags(SwapChainOutputFlag::localOutput | SwapChainOutputFlag::shaderInput);
  EXPECT_EQ((SwapChainOutputFlag::localOutput | SwapChainOutputFlag::shaderInput), params.outputFlags());
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
  
  params.setCustomBackBufferFormat(42);
  EXPECT_EQ(ComponentFormat::custom, params.backBufferFormat());
  EXPECT_EQ((int32_t)42, params.customBackBufferFormat());
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

