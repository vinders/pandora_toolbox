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
#include <hardware/cpu_instruction_set.h>

using namespace pandora::hardware;

class CpuInstructionSetTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- enumerations --

TEST_F(CpuInstructionSetTest, instructionFamilySerializer) {
  EXPECT_TRUE(*toString(CpuInstructionFamily::assembly));
  EXPECT_TRUE(*toString(CpuInstructionFamily::mmx));
  EXPECT_TRUE(*toString(CpuInstructionFamily::sse));
  EXPECT_TRUE(*toString(CpuInstructionFamily::avx));
  EXPECT_TRUE(*toString(CpuInstructionFamily::neon));
  EXPECT_TRUE(CpuInstructionFamily_size() > 0);

  CpuInstructionFamily converted = CpuInstructionFamily::assembly;
  EXPECT_TRUE(fromString(toString(CpuInstructionFamily::assembly), converted));
  EXPECT_EQ(CpuInstructionFamily::assembly, converted);
  EXPECT_TRUE(fromString(toString(CpuInstructionFamily::mmx), converted));
  EXPECT_EQ(CpuInstructionFamily::mmx, converted);
  EXPECT_TRUE(fromString(toString(CpuInstructionFamily::sse), converted));
  EXPECT_EQ(CpuInstructionFamily::sse, converted);
  EXPECT_TRUE(fromString(toString(CpuInstructionFamily::avx), converted));
  EXPECT_EQ(CpuInstructionFamily::avx, converted);
  EXPECT_TRUE(fromString(toString(CpuInstructionFamily::neon), converted));
  EXPECT_EQ(CpuInstructionFamily::neon, converted);
}

TEST_F(CpuInstructionSetTest, instructionFamilyFlagOps) {
  CpuInstructionFamily flag1 = CpuInstructionFamily::sse;
  CpuInstructionFamily flag2 = CpuInstructionFamily::sse;
  EXPECT_TRUE(flag1 == flag2);
  EXPECT_FALSE(flag1 != flag2);
  EXPECT_FALSE(flag1 < flag2);
  EXPECT_TRUE(flag1 <= flag2);
  EXPECT_FALSE(flag1 > flag2);
  EXPECT_TRUE(flag1 >= flag2);
  flag2 = CpuInstructionFamily::avx;
  EXPECT_FALSE(flag1 == flag2);
  EXPECT_TRUE(flag1 != flag2);
  EXPECT_EQ((static_cast<uint32_t>(flag1) < static_cast<uint32_t>(flag2)), (flag1 < flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) <= static_cast<uint32_t>(flag2)), (flag1 <= flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) > static_cast<uint32_t>(flag2)), (flag1 > flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) >= static_cast<uint32_t>(flag2)), (flag1 >= flag2));

  EXPECT_EQ(static_cast<CpuInstructionFamily>(static_cast<uint32_t>(flag1) & static_cast<uint32_t>(flag2)), (flag1 & flag2));
  EXPECT_EQ(static_cast<CpuInstructionFamily>(static_cast<uint32_t>(flag1) | static_cast<uint32_t>(flag2)), (flag1 | flag2));
  EXPECT_EQ(static_cast<CpuInstructionFamily>(static_cast<uint32_t>(flag1) ^ static_cast<uint32_t>(flag2)), (flag1 ^ flag2));
  EXPECT_EQ(static_cast<CpuInstructionFamily>(~static_cast<uint32_t>(flag1)), (~flag1));

  EXPECT_EQ((CpuInstructionFamily::sse | CpuInstructionFamily::avx), addFlag(flag1, flag2));
  EXPECT_EQ((CpuInstructionFamily::sse | CpuInstructionFamily::avx), flag1);
  EXPECT_EQ((CpuInstructionFamily::avx), removeFlag(flag1, CpuInstructionFamily::sse));
  EXPECT_EQ((CpuInstructionFamily::avx), flag1);
}

TEST_F(CpuInstructionSetTest, instructionSetSerializer) {
  CpuInstructionSet converted = CpuInstructionSet::cpp;

  EXPECT_TRUE(*toString(CpuInstructionSet::cpp));
  EXPECT_TRUE(fromString(toString(CpuInstructionSet::cpp), converted));
  EXPECT_EQ(CpuInstructionSet::cpp, converted);
  EXPECT_TRUE(CpuInstructionSet_size() > 0);

  for (auto instSet : CpuInstructionSet_x86_values()) {
    EXPECT_TRUE(*toString(instSet));
    EXPECT_TRUE(fromString(toString(instSet), converted));
    EXPECT_EQ(instSet, converted);
  }
  for (auto instSet : CpuInstructionSet_arm_values()) {
    EXPECT_TRUE(*toString(instSet));
    EXPECT_TRUE(fromString(toString(instSet), converted));
    EXPECT_EQ(instSet, converted);
  }
}

TEST_F(CpuInstructionSetTest, instructionSetFlagOps) {
  CpuInstructionSet flag1 = CpuInstructionSet::sse;
  CpuInstructionSet flag2 = CpuInstructionSet::sse;
  EXPECT_TRUE(flag1 == flag2);
  EXPECT_FALSE(flag1 != flag2);
  EXPECT_FALSE(flag1 < flag2);
  EXPECT_TRUE(flag1 <= flag2);
  EXPECT_FALSE(flag1 > flag2);
  EXPECT_TRUE(flag1 >= flag2);
  flag2 = CpuInstructionSet::avx;
  EXPECT_FALSE(flag1 == flag2);
  EXPECT_TRUE(flag1 != flag2);
  EXPECT_EQ((static_cast<uint32_t>(flag1) < static_cast<uint32_t>(flag2)), (flag1 < flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) <= static_cast<uint32_t>(flag2)), (flag1 <= flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) > static_cast<uint32_t>(flag2)), (flag1 > flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) >= static_cast<uint32_t>(flag2)), (flag1 >= flag2));

  EXPECT_EQ(static_cast<CpuInstructionSet>(static_cast<uint32_t>(flag1) & static_cast<uint32_t>(flag2)), (flag1 & flag2));
  EXPECT_EQ(static_cast<CpuInstructionSet>(static_cast<uint32_t>(flag1) | static_cast<uint32_t>(flag2)), (flag1 | flag2));
  EXPECT_EQ(static_cast<CpuInstructionSet>(static_cast<uint32_t>(flag1) ^ static_cast<uint32_t>(flag2)), (flag1 ^ flag2));
  EXPECT_EQ(static_cast<CpuInstructionSet>(~static_cast<uint32_t>(flag1)), (~flag1));

  EXPECT_EQ((CpuInstructionSet::sse | CpuInstructionSet::avx), addFlag(flag1, flag2));
  EXPECT_EQ((CpuInstructionSet::sse | CpuInstructionSet::avx), flag1);
}

TEST_F(CpuInstructionSetTest, instructionSetSubEnumValues) {
  EXPECT_TRUE(CpuInstructionSet_size() > 0);
  EXPECT_FALSE(CpuInstructionSet_x86_values().empty());
  EXPECT_FALSE(CpuInstructionSet_x86_rvalues().empty());
  EXPECT_FALSE(CpuInstructionSet_arm_values().empty());
  EXPECT_FALSE(CpuInstructionSet_arm_rvalues().empty());
  EXPECT_EQ(CpuInstructionSet_x86_values().size(), CpuInstructionSet_x86_rvalues().size());
  EXPECT_EQ(CpuInstructionSet_arm_values().size(), CpuInstructionSet_arm_rvalues().size());
  EXPECT_TRUE(CpuInstructionSet_x86_values().size() + CpuInstructionSet_arm_values().size() <= CpuInstructionSet_size());
}

// -- builder / extractors --

TEST_F(CpuInstructionSetTest, buildInstructionSet) {
  EXPECT_EQ(CpuInstructionSet::sse, toCpuInstructionSet(pandora::system::CpuArchitecture::x86, CpuInstructionFamily::sse, 0x2));
}

TEST_F(CpuInstructionSetTest, extractInstructionFamily) {
  EXPECT_EQ(CpuInstructionFamily::assembly, toCpuInstructionFamily(CpuInstructionSet::cpp));
  for (auto instSet : CpuInstructionSet_x86_values()) {
    EXPECT_TRUE(toCpuInstructionFamily(instSet) == CpuInstructionFamily::mmx
             || toCpuInstructionFamily(instSet) == CpuInstructionFamily::sse
             || toCpuInstructionFamily(instSet) == CpuInstructionFamily::avx);
  }
  for (auto instSet : CpuInstructionSet_arm_values()) {
    EXPECT_EQ(CpuInstructionFamily::neon, toCpuInstructionFamily(instSet));
  }
}

TEST_F(CpuInstructionSetTest, extractCpuArch) {
  EXPECT_EQ(pandora::system::CpuArchitecture::all, toCpuArchitecture(CpuInstructionSet::cpp));
  for (auto instSet : CpuInstructionSet_x86_values()) {
    EXPECT_EQ(pandora::system::CpuArchitecture::x86, toCpuArchitecture(instSet));
  }
  for (auto instSet : CpuInstructionSet_arm_values()) {
    EXPECT_EQ(pandora::system::CpuArchitecture::arm, toCpuArchitecture(instSet));
  }
}
