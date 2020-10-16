#include <gtest/gtest.h>
#include <memory/endian.h>

using namespace pandora::memory;

class EndianTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};

// -- endianness swap --

TEST_F(EndianTest, swapInt16) {
  EXPECT_EQ(0xB1A0u, swapEndianInt16(0xA0B1u));
  EXPECT_EQ(65432u, swapEndianInt16(swapEndianInt16(65432u)));
}

TEST_F(EndianTest, swapInt32) {
  EXPECT_EQ(0xD3C2B1A0u, swapEndianInt32(0xA0B1C2D3u));
  EXPECT_EQ(6543210u, swapEndianInt32(swapEndianInt32(6543210u)));
}

TEST_F(EndianTest, swapInt64) {
  EXPECT_EQ(0xB7A6F5E4D3C2B1A0uLL, swapEndianInt64(0xA0B1C2D3E4F5A6B7uLL));
  EXPECT_EQ(6543210987uLL, swapEndianInt64(swapEndianInt64(6543210987uLL)));
}

TEST_F(EndianTest, swapInt80) {
  Int80 result;
  swapEndianInt80(Int80{ 0xA0B1C2D3E4F5A6B7uLL, 0xC8D9u }, result);
  EXPECT_EQ(0xD9C8B7A6F5E4D3C2uLL, result.high);
  EXPECT_EQ(0xB1A0u, result.low);

  Int80 result2 = swapEndianInt80(swapEndianInt80(result));
  EXPECT_EQ(result.high, result2.high);
  EXPECT_EQ(result.low, result2.low);
}

TEST_F(EndianTest, swapInt128) {
  Int128 result;
  swapEndianInt128(Int128{ 0xA0B1C2D3E4F5A6B7uLL, 0xC8D9E0F1A2B3C4D5uLL }, result);
  EXPECT_EQ(0xD5C4B3A2F1E0D9C8uLL, result.high);
  EXPECT_EQ(0xB7A6F5E4D3C2B1A0uLL, result.low);

  Int128 result2 = swapEndianInt128(swapEndianInt128(result));
  EXPECT_EQ(result.high, result2.high);
  EXPECT_EQ(result.low, result2.low);
}

TEST_F(EndianTest, swapFloat) {
  uint32_t sourceData = 0xA0B1C2D3u;
  uint32_t expectedData = 0xD3C2B1A0u;
  float source = 0.0f;
  float expected = 0.0f;
  memcpy((void*)&source, (void*)&sourceData, sizeof(uint32_t));
  memcpy((void*)&expected, (void*)&expectedData, sizeof(uint32_t));
  EXPECT_EQ(expected, swapEndianFloat(source));

  EXPECT_EQ(123456.7f, swapEndianFloat(swapEndianFloat(123456.7f)));
}

TEST_F(EndianTest, swapDouble) {
  EXPECT_EQ(123456.78901, swapEndianDouble(swapEndianDouble(123456.78901)));
}
