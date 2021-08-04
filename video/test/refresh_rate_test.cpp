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

