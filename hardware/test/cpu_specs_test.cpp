#include <gtest/gtest.h>
#include <cstdio>
#include <hardware/cpu_specs.h>
#include <hardware/_private/_cpu_cores_specs.h>
#include <hardware/_private/_cpuid_register_reader.h>

using namespace pandora::hardware;

class CpuSpecsTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- CPU specifications --

TEST_F(CpuSpecsTest, readBaseCpuSpecs) {
  CpuSpecs specs(CpuSpecs::SpecMode::vendor | CpuSpecs::SpecMode::brandName | CpuSpecs::SpecMode::cores);
  EXPECT_FALSE(specs.vendor().empty());
  EXPECT_NE(std::string("unknown"), specs.vendor());
  EXPECT_EQ(specs.vendor(), toString(specs.vendorId()));
  EXPECT_FALSE(specs.brand().empty());

  EXPECT_TRUE(specs.physicalCores() > 0u);
  EXPECT_TRUE(specs.physicalCores() <= specs.logicalCores());
  EXPECT_EQ((specs.physicalCores() < specs.logicalCores()), specs.isHyperThreadingCapable());

  printf("CPU vendor : %s\nCPU brand  : %s\ncores : %d (%d logical)\n", 
          specs.vendor().c_str(), specs.brand().c_str(), 
          specs.physicalCores(), specs.logicalCores());
}

TEST_F(CpuSpecsTest, readExtendedCpuSpecs) {
  CpuSpecs specs(CpuSpecs::SpecMode::all);
  EXPECT_FALSE(specs.vendor().empty());
  EXPECT_EQ(specs.vendor(), toString(specs.vendorId()));
  EXPECT_FALSE(specs.brand().empty());
  EXPECT_TRUE(specs.physicalCores() > 0u);
  EXPECT_TRUE(specs.physicalCores() <= specs.logicalCores());

  auto instSets = specs.getAvailableCpuInstructionSets();
  EXPECT_TRUE(instSets.size() >= 1u);

# if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
    EXPECT_EQ(specs.hasBaseProperty(cpuid_x86::brandIndex()), (specs.getProperty(cpuid_x86::brandIndex()) != 0u));
    EXPECT_EQ(specs.hasAdvancedProperty(cpuid_x86::fsgsBase()), (specs.getProperty(cpuid_x86::fsgsBase()) != 0u));
    EXPECT_EQ(specs.hasExtendedProperty(cpuid_x86::mmxExt()), (specs.getProperty(cpuid_x86::mmxExt()) != 0u));

    EXPECT_TRUE(specs.isInstructionSetSupported(CpuInstructionSet::cpp));
    EXPECT_EQ(specs.hasBaseProperty(cpuid_x86::mmx()), specs.isInstructionSetSupported(CpuInstructionSet::mmx));
    EXPECT_EQ(specs.hasBaseProperty(cpuid_x86::sse()), specs.isInstructionSetSupported(CpuInstructionSet::sse));
    EXPECT_EQ(specs.hasBaseProperty(cpuid_x86::sse2()), specs.isInstructionSetSupported(CpuInstructionSet::sse2));
    EXPECT_EQ(specs.hasBaseProperty(cpuid_x86::sse3()), specs.isInstructionSetSupported(CpuInstructionSet::sse3));
    EXPECT_EQ(specs.hasBaseProperty(cpuid_x86::ssse3()), specs.isInstructionSetSupported(CpuInstructionSet::ssse3));
    EXPECT_EQ(specs.hasBaseProperty(cpuid_x86::sse41()), specs.isInstructionSetSupported(CpuInstructionSet::sse4_1));
    EXPECT_EQ(specs.hasBaseProperty(cpuid_x86::sse42()), specs.isInstructionSetSupported(CpuInstructionSet::sse4_2));
    bool hasAvx = (specs.hasBaseProperty(cpuid_x86::avx()) && specs.hasBaseProperty(cpuid_x86::osExtendedSave()) 
                && (CpuidRegisterReader::readExtendedControlRegister(0) & 0x6uLL) == 0x6uLL);
    EXPECT_EQ(hasAvx, specs.isInstructionSetSupported(CpuInstructionSet::avx));
    EXPECT_EQ(specs.hasAdvancedProperty(cpuid_x86::avx2()), specs.isInstructionSetSupported(CpuInstructionSet::avx2));
    EXPECT_EQ(specs.hasAdvancedProperty(cpuid_x86::avx512f()), specs.isInstructionSetSupported(CpuInstructionSet::avx512f));
    
    EXPECT_FALSE(specs.isInstructionSetSupported((CpuInstructionSet)1234567u));

# elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
    EXPECT_TRUE(specs.hasBaseProperty(cpuid_arm::id()));
    EXPECT_EQ(specs.hasBaseProperty(cpuid_arm::id()), (specs.getProperty(cpuid_arm::id()) != 0));

    EXPECT_TRUE(specs.isInstructionSetSupported(CpuInstructionSet::cpp));
    if (specs.isInstructionSetSupported(CpuInstructionSet::neon64)) {
      EXPECT_TRUE(specs.isInstructionSetSupported(CpuInstructionSet::neon));
    }
# endif
}

TEST_F(CpuSpecsTest, cpuCoreInfo) {
  auto coreInfo = _readCpuCoresFromOS(CpuVendor::intel);
  EXPECT_TRUE(coreInfo.physicalCores > 0u);
  EXPECT_TRUE(coreInfo.physicalCores <= coreInfo.logicalCores);
}

// -- enumeration --

TEST_F(CpuSpecsTest, cpuSpecModeFlagOps) {
  CpuSpecs::SpecMode flag1 = CpuSpecs::SpecMode::vendor;
  CpuSpecs::SpecMode flag2 = CpuSpecs::SpecMode::vendor;
  EXPECT_TRUE(flag1 == flag2);
  EXPECT_FALSE(flag1 != flag2);
  EXPECT_FALSE(flag1 < flag2);
  EXPECT_TRUE(flag1 <= flag2);
  EXPECT_FALSE(flag1 > flag2);
  EXPECT_TRUE(flag1 >= flag2);
  flag2 = CpuSpecs::SpecMode::cores;
  EXPECT_FALSE(flag1 == flag2);
  EXPECT_TRUE(flag1 != flag2);
  EXPECT_EQ((static_cast<uint32_t>(flag1) < static_cast<uint32_t>(flag2)), (flag1 < flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) <= static_cast<uint32_t>(flag2)), (flag1 <= flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) > static_cast<uint32_t>(flag2)), (flag1 > flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) >= static_cast<uint32_t>(flag2)), (flag1 >= flag2));

  EXPECT_EQ(static_cast<CpuSpecs::SpecMode>(static_cast<uint32_t>(flag1) & static_cast<uint32_t>(flag2)), (flag1 & flag2));
  EXPECT_EQ(static_cast<CpuSpecs::SpecMode>(static_cast<uint32_t>(flag1) | static_cast<uint32_t>(flag2)), (flag1 | flag2));
  EXPECT_EQ(static_cast<CpuSpecs::SpecMode>(static_cast<uint32_t>(flag1) ^ static_cast<uint32_t>(flag2)), (flag1 ^ flag2));
  EXPECT_EQ(static_cast<CpuSpecs::SpecMode>(~static_cast<uint32_t>(flag1)), (~flag1));

  EXPECT_EQ((CpuSpecs::SpecMode::vendor | CpuSpecs::SpecMode::cores), addFlag(flag1, flag2));
  EXPECT_EQ((CpuSpecs::SpecMode::vendor | CpuSpecs::SpecMode::cores), flag1);
  EXPECT_EQ((CpuSpecs::SpecMode::cores), removeFlag(flag1, CpuSpecs::SpecMode::vendor));
  EXPECT_EQ((CpuSpecs::SpecMode::cores), flag1);
}
