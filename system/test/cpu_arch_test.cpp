#include <gtest/gtest.h>
#include <system/cpu_arch.h>

using namespace pandora::system;

class CpuArchTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};

// -- OS detection --

TEST_F(CpuArchTest, macroVersion) {
# if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
    EXPECT_EQ(_SYSTEM_CPU_ARCH_X86, _SYSTEM_CPU_ARCH);
    EXPECT_EQ(64, _SYSTEM_CPU_REGSIZE);
# else
    EXPECT_NE(_SYSTEM_CPU_ARCH_UNK, _SYSTEM_CPU_ARCH);
    EXPECT_TRUE(_SYSTEM_CPU_REGSIZE == 32 || _SYSTEM_CPU_REGSIZE == 64 || _SYSTEM_CPU_REGSIZE == 128);
# endif
}

TEST_F(CpuArchTest, functionVersion) {
# if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64)
    EXPECT_EQ(CpuArchitecture::x86, getCpuArchitecture());
    EXPECT_EQ(64u, getCpuRegisterSize());
# else
    EXPECT_NE(CpuArchitecture::unknown, getCpuArchitecture());
    EXPECT_TRUE(getCpuRegisterSize() == 32u || getCpuRegisterSize() == 64u || getCpuRegisterSize() == 128u);
# endif
}

// -- enumeration --

TEST_F(CpuArchTest, cpuArchSerializer) {
  EXPECT_TRUE(*toString(CpuArchitecture::unknown));
  EXPECT_TRUE(*toString(CpuArchitecture::x86));
  EXPECT_TRUE(*toString(CpuArchitecture::powerpc));
  EXPECT_TRUE(*toString(CpuArchitecture::arm));
  EXPECT_TRUE(CpuArchitecture_size() > 0);

  CpuArchitecture converted = CpuArchitecture::unknown;
  EXPECT_TRUE(fromString(toString(CpuArchitecture::unknown), converted));
  EXPECT_EQ(CpuArchitecture::unknown, converted);
  EXPECT_TRUE(fromString(toString(CpuArchitecture::x86), converted));
  EXPECT_EQ(CpuArchitecture::x86, converted);
  EXPECT_TRUE(fromString(toString(CpuArchitecture::powerpc), converted));
  EXPECT_EQ(CpuArchitecture::powerpc, converted);
  EXPECT_TRUE(fromString(toString(CpuArchitecture::arm), converted));
  EXPECT_EQ(CpuArchitecture::arm, converted);
}

TEST_F(CpuArchTest, cpuArchFlagOps) {
  CpuArchitecture flag1 = CpuArchitecture::x86;
  CpuArchitecture flag2 = CpuArchitecture::x86;
  EXPECT_TRUE(flag1 == flag2);
  EXPECT_FALSE(flag1 != flag2);
  EXPECT_FALSE(flag1 < flag2);
  EXPECT_TRUE(flag1 <= flag2);
  EXPECT_FALSE(flag1 > flag2);
  EXPECT_TRUE(flag1 >= flag2);
  flag2 = CpuArchitecture::arm;
  EXPECT_FALSE(flag1 == flag2);
  EXPECT_TRUE(flag1 != flag2);
  EXPECT_EQ((static_cast<uint32_t>(flag1) < static_cast<uint32_t>(flag2)), (flag1 < flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) <= static_cast<uint32_t>(flag2)), (flag1 <= flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) > static_cast<uint32_t>(flag2)), (flag1 > flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) >= static_cast<uint32_t>(flag2)), (flag1 >= flag2));

  EXPECT_EQ(static_cast<CpuArchitecture>(static_cast<uint32_t>(flag1) & static_cast<uint32_t>(flag2)), (flag1 & flag2));
  EXPECT_EQ(static_cast<CpuArchitecture>(static_cast<uint32_t>(flag1) | static_cast<uint32_t>(flag2)), (flag1 | flag2));
  EXPECT_EQ(static_cast<CpuArchitecture>(static_cast<uint32_t>(flag1) ^ static_cast<uint32_t>(flag2)), (flag1 ^ flag2));
  EXPECT_EQ(static_cast<CpuArchitecture>(~static_cast<uint32_t>(flag1)), (~flag1));

  EXPECT_EQ((CpuArchitecture::x86 | CpuArchitecture::arm), addFlag(flag1, flag2));
  EXPECT_EQ((CpuArchitecture::x86 | CpuArchitecture::arm), flag1);
  EXPECT_EQ((CpuArchitecture::arm), removeFlag(flag1, CpuArchitecture::x86));
  EXPECT_EQ((CpuArchitecture::arm), flag1);
}
