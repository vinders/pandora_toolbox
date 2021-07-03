#include <gtest/gtest.h>
#include <video/common_types.h>

using namespace pandora::video;

class RefreshRateTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- RefreshRate params get/set --

TEST_F(RefreshRateTest, gettersSettersTest) {
  RefreshRate params;
  EXPECT_EQ((uint32_t)60u, params.numerator());
  EXPECT_EQ((uint32_t)1u, params.denominator());
  params = RefreshRate(60000u, 1001u);
  EXPECT_EQ((uint32_t)60000u, params.numerator());
  EXPECT_EQ((uint32_t)1001u, params.denominator());
  params = RefreshRate(60u, 0);
  EXPECT_EQ((uint32_t)60u, params.numerator());
  EXPECT_NE((uint32_t)0, params.denominator());
}

TEST_F(RefreshRateTest, rateMilliHzTest) {
  RefreshRate params;
  EXPECT_EQ((uint32_t)60u, params.numerator());
  EXPECT_EQ((uint32_t)1u, params.denominator());

  params = RefreshRate(60000000u/1001u);
  EXPECT_EQ((uint32_t)60000u, params.numerator());
  EXPECT_EQ((uint32_t)1001u, params.denominator());
  params = RefreshRate(30000000u/1001u);
  EXPECT_EQ((uint32_t)30000u, params.numerator());
  EXPECT_EQ((uint32_t)1001u, params.denominator());
  params = RefreshRate(24000000u/1001u);
  EXPECT_EQ((uint32_t)24000u, params.numerator());
  EXPECT_EQ((uint32_t)1001u, params.denominator());
  params = RefreshRate(12345u);
  EXPECT_EQ((uint32_t)12345u, params.numerator());
  EXPECT_EQ((uint32_t)1000u, params.denominator());
  params = RefreshRate(12000u);
  EXPECT_EQ((uint32_t)12u, params.numerator());
  EXPECT_EQ((uint32_t)1u, params.denominator());
}

