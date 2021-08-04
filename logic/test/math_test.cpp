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
#include <logic/math.h>

using namespace pandora::logic;

class MathTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- number analysis --

TEST_F(MathTest, nearEqual) {
  EXPECT_TRUE(nearEqual(1234000, 1234001, 1));
  EXPECT_TRUE(nearEqual(1234001, 1234000, 1));
  EXPECT_FALSE(nearEqual(1233999, 1234001, 1));
  EXPECT_FALSE(nearEqual(1234001, 1233999, 1));
  EXPECT_FALSE(nearEqual(1234000, 1234001, 0));
  EXPECT_FALSE(nearEqual(1234001, 1234000, 0));

  EXPECT_TRUE(nearEqual(1234000, 1235000, 1000));
  EXPECT_TRUE(nearEqual(1235000, 1234000, 1000));
  EXPECT_FALSE(nearEqual(1233999, 1235000, 1000));
  EXPECT_FALSE(nearEqual(1235000, 1233999, 1000));
  EXPECT_FALSE(nearEqual(1234000, 1235000, 999));
  EXPECT_FALSE(nearEqual(1235000, 1234000, 999));

  EXPECT_TRUE(nearEqual(1234.0, 1234.001, 0.001));
  EXPECT_TRUE(nearEqual(1234.001, 1234.0, 0.001));
  EXPECT_FALSE(nearEqual(1233.999, 1234.001, 0.001));
  EXPECT_FALSE(nearEqual(1234.001, 1233.999, 0.001));
  EXPECT_FALSE(nearEqual(1234.0, 1234.001, 0.0));
  EXPECT_FALSE(nearEqual(1234.001, 1234.0, 0.0));
  EXPECT_FALSE(nearEqual(1234.0, 1234.001, 0.0009));
  EXPECT_FALSE(nearEqual(1234.001, 1234.0, 0.0009));
}
TEST_F(MathTest, floatEqual) {
  EXPECT_TRUE(floatEqual(12345.6f, 12345.6f));
  EXPECT_TRUE(floatEqual(12345.6000001f, 12345.6000002f));
  EXPECT_TRUE(floatEqual(12345.6000002f, 12345.6000001f));
  EXPECT_FALSE(floatEqual(12345.7f, 12345.6f));
  EXPECT_FALSE(floatEqual(12345.6f, 12345.7f));
}
TEST_F(MathTest, doubleEqual) {
  EXPECT_TRUE(doubleEqual(123456789.1, 123456789.1));
  EXPECT_TRUE(doubleEqual(123456789.12345000001, 123456789.12345000002));
  EXPECT_TRUE(doubleEqual(123456789.12345000002, 123456789.12345000001));
  EXPECT_FALSE(doubleEqual(123456789.1, 123456789.2));
  EXPECT_FALSE(doubleEqual(123456789.2, 123456789.1));
}

TEST_F(MathTest, isPow2) {
  EXPECT_TRUE(isPow2(1));
  EXPECT_TRUE(isPow2(2));
  for (int i = 4; i <= 65536; i *= 2) {
    EXPECT_TRUE(isPow2(i));
    EXPECT_FALSE(isPow2(-i));
    EXPECT_FALSE(isPow2(i - 1));
    EXPECT_FALSE(isPow2(i + 1));
  }
  EXPECT_FALSE(isPow2(6));
  EXPECT_FALSE(isPow2(12));
  EXPECT_FALSE(isPow2(18));
  EXPECT_FALSE(isPow2(24));
  EXPECT_FALSE(isPow2(-1));
  EXPECT_FALSE(isPow2(-2));
  EXPECT_FALSE(isPow2(-3));
}

// -- calculations --

TEST_F(MathTest, gcd) {
  EXPECT_EQ(2, gcd(0, 2));
  EXPECT_EQ(2, gcd(2, 0));
  EXPECT_EQ(1, gcd(1, 1));
  EXPECT_EQ(1, gcd(-1, 1));
  EXPECT_EQ(1, gcd(1, -1));
  EXPECT_EQ(1, gcd(-1, -1));
  EXPECT_EQ(1, gcd(2, 1));
  EXPECT_EQ(1, gcd(-2, 1));
  EXPECT_EQ(1, gcd(2, -1));
  EXPECT_EQ(2, gcd(2, 2));
  EXPECT_EQ(2, gcd(-2, 2));
  EXPECT_EQ(1, gcd(3, 2));
  EXPECT_EQ(2, gcd(4, 2));
  EXPECT_EQ(1, gcd(87651, 35168));
  EXPECT_EQ(2, gcd(87652, 35166));
  EXPECT_EQ(3, gcd(87651, 35166));
  EXPECT_EQ(4, gcd(87652, 35168));
  EXPECT_EQ(1024, gcd(65536, 1024));
  EXPECT_EQ(512, gcd(65536, 1536));
  EXPECT_EQ(1, gcd(6765, 2));
  EXPECT_EQ(3, gcd(6765, 3));
  EXPECT_EQ(1, gcd(6765, 4181));
}
