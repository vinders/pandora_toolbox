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
#include <hardware/cpuid_property_location.h>

using namespace pandora::hardware;

class CpuidPropertyLocationTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- constructors / accessors --

TEST_F(CpuidPropertyLocationTest, defaultCtorAccessors) {
  CpuidPropertyLocation prop;
  EXPECT_EQ(0, prop.registerId());
  EXPECT_EQ(CpuRegisterPart::eax, prop.index());
  EXPECT_TRUE(prop.mask() == 0);
  EXPECT_EQ(0u, prop.offset());
  EXPECT_FALSE(prop.hasExclusivity());
  EXPECT_EQ(CpuVendor::unknown, prop.exclusivity());
}

TEST_F(CpuidPropertyLocationTest, initCtorAccessors) {
  int32_t regId = 24;
# if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
    CpuRegisterPart regIndex = CpuRegisterPart::eax;
    uint64_t mask = 0x123400FF00FFuLL;
# else
    CpuRegisterPart regIndex = CpuRegisterPart::ebx;
    uint32_t mask = 0x00FF00FFu;
# endif
  uint32_t offset = 3;
  CpuVendor exclusive = CpuVendor::intel;

  CpuidPropertyLocation prop(regId, regIndex, mask, offset);
  EXPECT_EQ(regId, prop.registerId());
  EXPECT_EQ(regIndex, prop.index());
  EXPECT_EQ(mask, prop.mask());
  EXPECT_EQ(offset, prop.offset());
  EXPECT_FALSE(prop.hasExclusivity());
  EXPECT_EQ(CpuVendor::unknown, prop.exclusivity());
  
  CpuidPropertyLocation propExclusive(regId, regIndex, mask, offset, exclusive);
  EXPECT_EQ(regId, propExclusive.registerId());
  EXPECT_EQ(regIndex, propExclusive.index());
  EXPECT_EQ(mask, propExclusive.mask());
  EXPECT_EQ(offset, propExclusive.offset());
  EXPECT_TRUE(propExclusive.hasExclusivity());
  EXPECT_EQ(exclusive, propExclusive.exclusivity());
}

TEST_F(CpuidPropertyLocationTest, copyMoveCtors) {
# if _SYSTEM_CPU_ARCH == _SYSTEM_CPU_ARCH_ARM
    CpuRegisterPart regIndex = CpuRegisterPart::eax;
    uint64_t mask = 0x123400FF00FFuLL;
# else
    CpuRegisterPart regIndex = CpuRegisterPart::ebx;
    uint32_t mask = 0x00FF00FFu;
# endif
  CpuidPropertyLocation prop(24, regIndex, mask, 3, CpuVendor::intel);

  CpuidPropertyLocation propCopy(prop);
  EXPECT_EQ(prop.registerId(), propCopy.registerId());
  EXPECT_EQ(prop.index(), propCopy.index());
  EXPECT_EQ(prop.mask(), propCopy.mask());
  EXPECT_EQ(prop.offset(), propCopy.offset());
  EXPECT_EQ(prop.hasExclusivity(), propCopy.hasExclusivity());
  EXPECT_EQ(prop.exclusivity(), propCopy.exclusivity());

  CpuidPropertyLocation propMove(std::move(propCopy));
  EXPECT_EQ(prop.registerId(), propMove.registerId());
  EXPECT_EQ(prop.index(), propMove.index());
  EXPECT_EQ(prop.mask(), propMove.mask());
  EXPECT_EQ(prop.offset(), propMove.offset());
  EXPECT_EQ(prop.hasExclusivity(), propMove.hasExclusivity());
  EXPECT_EQ(prop.exclusivity(), propMove.exclusivity());

  CpuidPropertyLocation propAssigned(1, regIndex, mask, 0);
  propAssigned = prop;
  EXPECT_EQ(prop.registerId(), propAssigned.registerId());
  EXPECT_EQ(prop.index(), propAssigned.index());
  EXPECT_EQ(prop.mask(), propAssigned.mask());
  EXPECT_EQ(prop.offset(), propAssigned.offset());
  EXPECT_EQ(prop.hasExclusivity(), propAssigned.hasExclusivity());
  EXPECT_EQ(prop.exclusivity(), propAssigned.exclusivity());

  CpuidPropertyLocation propMoveAssigned(1, regIndex, mask, 0);
  propMoveAssigned = std::move(propAssigned);
  EXPECT_EQ(prop.registerId(), propMoveAssigned.registerId());
  EXPECT_EQ(prop.index(), propMoveAssigned.index());
  EXPECT_EQ(prop.mask(), propMoveAssigned.mask());
  EXPECT_EQ(prop.offset(), propMoveAssigned.offset());
  EXPECT_EQ(prop.hasExclusivity(), propMoveAssigned.hasExclusivity());
  EXPECT_EQ(prop.exclusivity(), propMoveAssigned.exclusivity());
}
