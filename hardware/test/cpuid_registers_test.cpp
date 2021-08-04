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
#include <hardware/cpuid_registers.h>

using namespace pandora::hardware;

class CpuidRegistersTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- CPUID registers --

inline void _validateRegister(const CpuidPropertyLocation& reg) {
  EXPECT_TRUE(static_cast<size_t>(reg.index()) < CpuRegisterPart_size());

  EXPECT_TRUE(reg.mask() != 0);
# if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
    EXPECT_TRUE(reg.offset() < 32);
# elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
    EXPECT_TRUE(reg.offset() < 64);
# endif
}

TEST_F(CpuidRegistersTest, cpuRegisterLocations) {
# if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_X86
    _validateRegister(cpuid_x86::baseSize());
    _validateRegister(cpuid_x86::extendedSize());
    _validateRegister(cpuid_x86::vendorPart1());
    _validateRegister(cpuid_x86::vendorPart2());
    _validateRegister(cpuid_x86::vendorPart3());
    _validateRegister(cpuid_x86::baseFamily());
    _validateRegister(cpuid_x86::extendedFamily());
    _validateRegister(cpuid_x86::brandIndex());
    _validateRegister(cpuid_x86::logicalProcessorsPerCpu());
    _validateRegister(cpuid_x86::localApicId());
    _validateRegister(cpuid_x86::sse3());
    _validateRegister(cpuid_x86::pclMulQdq());
    _validateRegister(cpuid_x86::monitor());
    _validateRegister(cpuid_x86::ssse3());
    _validateRegister(cpuid_x86::fma());
    _validateRegister(cpuid_x86::cmpXchg16b());
    _validateRegister(cpuid_x86::sse41());
    _validateRegister(cpuid_x86::sse42());
    _validateRegister(cpuid_x86::movBE());
    _validateRegister(cpuid_x86::popCnt());
    _validateRegister(cpuid_x86::aes());
    _validateRegister(cpuid_x86::extendedSave());
    _validateRegister(cpuid_x86::osExtendedSave());
    _validateRegister(cpuid_x86::avx());
    _validateRegister(cpuid_x86::f16c());
    _validateRegister(cpuid_x86::rdRand());
    _validateRegister(cpuid_x86::msr());
    _validateRegister(cpuid_x86::cx8());
    _validateRegister(cpuid_x86::sep());
    _validateRegister(cpuid_x86::cmov());
    _validateRegister(cpuid_x86::clFsh());
    _validateRegister(cpuid_x86::mmx());
    _validateRegister(cpuid_x86::fxSr());
    _validateRegister(cpuid_x86::sse());
    _validateRegister(cpuid_x86::sse2());
    _validateRegister(cpuid_x86::hyperThreading());
    _validateRegister(cpuid_x86::fsgsBase());
    _validateRegister(cpuid_x86::bmi1());
    _validateRegister(cpuid_x86::hle());
    _validateRegister(cpuid_x86::avx2());
    _validateRegister(cpuid_x86::bmi2());
    _validateRegister(cpuid_x86::erms());
    _validateRegister(cpuid_x86::invPcid());
    _validateRegister(cpuid_x86::rtm());
    _validateRegister(cpuid_x86::avx512f());
    _validateRegister(cpuid_x86::rdSeed());
    _validateRegister(cpuid_x86::adx());
    _validateRegister(cpuid_x86::avx512pf());
    _validateRegister(cpuid_x86::avx512er());
    _validateRegister(cpuid_x86::avx512cd());
    _validateRegister(cpuid_x86::sha());
    _validateRegister(cpuid_x86::prefetchWt1());
    _validateRegister(cpuid_x86::lahf());
    _validateRegister(cpuid_x86::amdCmpLegacy());
    _validateRegister(cpuid_x86::lzCnt());
    _validateRegister(cpuid_x86::abm());
    _validateRegister(cpuid_x86::sse4a());
    _validateRegister(cpuid_x86::xop());
    _validateRegister(cpuid_x86::tbm());
    _validateRegister(cpuid_x86::sysCall());
    _validateRegister(cpuid_x86::mmxExt());
    _validateRegister(cpuid_x86::rdTscp());
    _validateRegister(cpuid_x86::amd3dNowExt());
    _validateRegister(cpuid_x86::amd3dNow());
    _validateRegister(cpuid_x86::multiCoresInfo());
    _validateRegister(cpuid_x86::amdCoresInfo());
    _validateRegister(cpuid_x86::intelTopologyType());
    _validateRegister(cpuid_x86::intelTopologyValue());

# elif _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
    _validateRegister(cpuid_arm::id());
    _validateRegister(cpuid_arm::implementer());
    _validateRegister(cpuid_arm::variant());
    _validateRegister(cpuid_arm::architecture());
    _validateRegister(cpuid_arm::modelNumber());
    _validateRegister(cpuid_arm::partNumber());
    _validateRegister(cpuid_arm::revision());
# endif
}
