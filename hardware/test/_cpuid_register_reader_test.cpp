#include <gtest/gtest.h>
#include <hardware/_private/_cpuid_register_reader.h>

using namespace pandora::hardware;

class _CpuidRegisterReaderTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- read CPUID register --

TEST_F(_CpuidRegisterReaderTest, readCpuidRegister) {
# if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
    CpuRegister128 reg1 = CpuidRegisterReader::readCpuid(cpuid_x86::RegisterId::sizeAndVendor);
    CpuRegister128 reg2 = CpuidRegisterReader::readCpuid(cpuid_x86::RegisterId::baseFeatures);
    EXPECT_TRUE(reg1[0] != 0 || reg1[1] != 0 || reg1[2] != 0 || reg1[3] != 0);
    EXPECT_TRUE(reg2[0] != 0 || reg2[1] != 0 || reg2[2] != 0 || reg2[3] != 0);
    EXPECT_NE(reg1, reg2);

    CpuRegister128 buffer1{ 0 };
    CpuidRegisterReader::fillCpuid(buffer1, cpuid_x86::RegisterId::sizeAndVendor);
    EXPECT_EQ(reg1, buffer1);

# elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
    CpuRegister64 reg1 = CpuidRegisterReader::readCpuid(cpuid_arm::RegisterId::id);
    CpuRegister64 reg2 = CpuidRegisterReader::readCpuid(cpuid_arm::RegisterId::cache);
    EXPECT_TRUE(reg1 != 0uLL);
    EXPECT_TRUE(reg2 != 0uLL);
    EXPECT_NE(reg1, reg2);

    CpuRegister64 buffer1{ 0 };
    CpuidRegisterReader::fillCpuid(buffer1, cpuid_arm::RegisterId::id);
    EXPECT_EQ(reg1, buffer1);
# endif
}

#if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
  TEST_F(_CpuidRegisterReaderTest, readExtendedControlRegister) {
    EXPECT_TRUE(CpuidRegisterReader::readExtendedControlRegister(0) != 0uLL);
  }
#endif

TEST_F(_CpuidRegisterReaderTest, registerBits) {
# if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
    CpuRegister128 fakeReg{ 0, 0x3u, 0x4u, 0xFFFFu };
    EXPECT_EQ(0u, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::eax, 0xFFFFFFFu, 0u)));
    EXPECT_EQ(0x3u, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::ebx, 0xFFFFFFFu, 0u)));
    EXPECT_EQ(0x1u, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::ebx, 0xFFFFFFFu, 1u)));
    EXPECT_EQ(0x4u, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::ecx, 0xFFFFFFFu, 0u)));
    EXPECT_EQ(0x1u, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::ecx, 0xFFFFFFFu, 2u)));
    EXPECT_EQ(0xFFFFu, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::edx, 0xFFFFFFFu, 0u)));
    EXPECT_EQ(0xFFu, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::edx, 0xFFFFFFFu, 8u)));
    EXPECT_EQ(0x12u, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::edx, 0x1212u, 8u)));

    CpuRegister128 buffer;
    CpuidPropertyLocation baseRegLocation(static_cast<int32_t>(cpuid_x86::RegisterId::baseFeatures), CpuRegisterPart::eax, 0xFFFFFFFu, 0u);
    auto baseRegBits = CpuidRegisterReader::readRegisterBits(baseRegLocation);
    EXPECT_EQ(baseRegBits, CpuidRegisterReader::readRegisterBits(buffer, baseRegLocation));
    EXPECT_EQ(CpuidRegisterReader::hasRegisterBits(baseRegLocation), CpuidRegisterReader::hasRegisterBits(buffer, baseRegLocation));

# elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
    CpuRegister64 fakeReg{ 0x0304FFFFuLL };
    EXPECT_EQ(0x0304FFFFuLL, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::eax, 0xFFFFFFFFuLL, 0u)));
    EXPECT_EQ(0x0304FFuLL, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::eax, 0xFFFFFFFFuLL, 8u)));
    EXPECT_EQ(0x010012uLL, CpuidRegisterReader::extractRegisterBits(fakeReg, CpuidPropertyLocation(0, CpuRegisterPart::edx, 0x01011212uLL, 8u)));

    CpuRegister64 buffer;
    CpuidPropertyLocation idRegLocation(static_cast<int32_t>(cpuid_arm::RegisterId::id), CpuRegisterPart::eax, 0xFFFFFFFFuLL, 0u);
    auto idRegBits = CpuidRegisterReader::readRegisterBits(idRegLocation);
    EXPECT_EQ(idRegBits, CpuidRegisterReader::readRegisterBits(buffer, idRegLocation));
    EXPECT_EQ(CpuidRegisterReader::hasRegisterBits(idRegLocation), CpuidRegisterReader::hasRegisterBits(buffer, idRegLocation));
# endif
}

// -- CPU info files (Linux) --

#if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86 && !defined(_WINDOWS) && !defined(__APPLE__) && (defined(__clang__) || defined(__GNUC__) || defined(__GNUG__) || defined(__linux__))
  TEST_F(_CpuidRegisterReaderTest, readCpuInfoFile) {
    EXPECT_FALSE(CpuidRegisterReader::readCpuInfoFile("vendor_id").empty()); // vendor ID found
  }

  TEST_F(_CpuidRegisterReaderTest, readAuxVectorsFile) {
    EXPECT_FALSE(CpuidRegisterReader::readAuxVectorsFile(AT_HWCAP, 4096)); // no NEON in x86 -> always false
  }
#endif