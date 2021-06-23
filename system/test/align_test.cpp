#include <cstdio>
#include <gtest/gtest.h>
#include <system/align.h>

class AlignTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- align macro --

TEST_F(AlignTest, alignedBaseTypes) {
  __align_type(16, int val1);
  val1 = 42;
  EXPECT_EQ((int)42, val1);
  val1 = 44;
  EXPECT_EQ((int)44, val1);
  __align_type(16, int val2) = 5;
  EXPECT_EQ((int)5, val2);
  __align_type(16, float val3) = 0.5f;
  EXPECT_EQ((float)0.5f, val3);
}

TEST_F(AlignTest, alignedArrays) {
  __align_type(16, int array[16]);
  EXPECT_EQ(size_t{16u}*sizeof(int), sizeof(array));
  __align_type(16, int array2[]) = { 1,2,3,4 };
  EXPECT_EQ(size_t{4u}*sizeof(int), sizeof(array2));
  EXPECT_EQ((int)1, array2[0]);
  EXPECT_EQ((int)2, array2[1]);
  EXPECT_EQ((int)3, array2[2]);
  EXPECT_EQ((int)4, array2[3]);
}

__align_type(16, struct ColorComponents {
  uint32_t r;
  uint32_t g;
  uint32_t b;
  uint32_t a;
});
__align_type(16, struct ColorComponents2 {
  float r;
  float g;
  float b;
  float a;
});
__align_type(16, 
struct ColorComponents3 {
  __align_type(16, float rgba[4]);
});
__align_prefix(16)
  struct ColorComponents3b {
  __align_type(16, float rgba[4]);
} __align_suffix(16);

TEST_F(AlignTest, alignedStructs) {
  ColorComponents  empty{};
  empty.r = 1; empty.g = 2; empty.b = 3; empty.a = 4;
  EXPECT_EQ((uint32_t)1, empty.r);
  EXPECT_EQ((uint32_t)2, empty.g);
  EXPECT_EQ((uint32_t)3, empty.b);
  EXPECT_EQ((uint32_t)4, empty.a);
  ColorComponents  val1 = { 1,2,3,4 };
  EXPECT_EQ((uint32_t)1, val1.r);
  EXPECT_EQ((uint32_t)2, val1.g);
  EXPECT_EQ((uint32_t)3, val1.b);
  EXPECT_EQ((uint32_t)4, val1.a);
  ColorComponents2 val2 { 0.1f,0.2f,0.3f,0.4f };
  EXPECT_EQ((float)0.1f, val2.r);
  EXPECT_EQ((float)0.2f, val2.g);
  EXPECT_EQ((float)0.3f, val2.b);
  EXPECT_EQ((float)0.4f, val2.a);
  ColorComponents3 val3 {{ 0.1f,0.2f,0.3f,0.4f }};
  EXPECT_EQ((float)0.1f, val3.rgba[0]);
  EXPECT_EQ((float)0.2f, val3.rgba[1]);
  EXPECT_EQ((float)0.3f, val3.rgba[2]);
  EXPECT_EQ((float)0.4f, val3.rgba[3]);
  ColorComponents3b val3b {{ 0.1f,0.2f,0.3f,0.4f }};
  EXPECT_EQ((float)0.1f, val3b.rgba[0]);
  EXPECT_EQ((float)0.2f, val3b.rgba[1]);
  EXPECT_EQ((float)0.3f, val3b.rgba[2]);
  EXPECT_EQ((float)0.4f, val3b.rgba[3]);
}
