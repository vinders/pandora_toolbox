#include <gtest/gtest.h>
#include <memory/memory_register.h>
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 4309)
#endif

using namespace pandora::memory;

class MemoryRegisterTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};

// -- constructors/accessors --

TEST_F(MemoryRegisterTest, defaultCtorAccessors) {
  const MemoryRegister<> regDef;
  const MemoryRegister<uint8_t> reg8;
  const MemoryRegister<uint16_t> reg16;
  const MemoryRegister<uint32_t> reg32;
  const MemoryRegister<uint64_t> reg64;

  EXPECT_EQ(0u, regDef.value());
  EXPECT_EQ(0u, reg8.value());
  EXPECT_EQ(0u, reg16.value());
  EXPECT_EQ(0u, reg32.value());
  EXPECT_EQ(0uLL, reg64.value());

  EXPECT_EQ(size_t{8u}, reg8.size());
  EXPECT_EQ(size_t{16u}, reg16.size());
  EXPECT_EQ(size_t{32u}, reg32.size());
  EXPECT_EQ(size_t{64u}, reg64.size());
  EXPECT_EQ(size_t{0u}, reg8.count());
  EXPECT_EQ(size_t{0u}, reg16.count());
  EXPECT_EQ(size_t{0u}, reg32.count());
  EXPECT_EQ(size_t{0u}, reg64.count());

  EXPECT_TRUE(regDef.empty());
  EXPECT_TRUE(reg8.empty());
  EXPECT_TRUE(reg16.empty());
  EXPECT_TRUE(reg32.empty());
  EXPECT_TRUE(reg64.empty());
  EXPECT_FALSE(regDef.full());
  EXPECT_FALSE(reg8.full());
  EXPECT_FALSE(reg16.full());
  EXPECT_FALSE(reg32.full());
  EXPECT_FALSE(reg64.full());
}

TEST_F(MemoryRegisterTest, initCtorAccessors) {
  MemoryRegister<uint8_t> reg8(250u);
  MemoryRegister<uint16_t> reg16(65001u);
  MemoryRegister<uint32_t> reg32(4001010101u);
  MemoryRegister<uint64_t> reg64(9001001001001001uLL);

  EXPECT_EQ(250u, reg8.value());
  EXPECT_EQ(65001u, reg16.value());
  EXPECT_EQ(4001010101u, reg32.value());
  EXPECT_EQ(9001001001001001uLL, reg64.value());

  EXPECT_EQ(size_t{8u}, reg8.size());
  EXPECT_EQ(size_t{16u}, reg16.size());
  EXPECT_EQ(size_t{32u}, reg32.size());
  EXPECT_EQ(size_t{64u}, reg64.size());
  EXPECT_NE(size_t{0u}, reg8.count());
  EXPECT_NE(size_t{0u}, reg16.count());
  EXPECT_NE(size_t{0u}, reg32.count());
  EXPECT_NE(size_t{0u}, reg64.count());

  EXPECT_FALSE(reg8.empty());
  EXPECT_FALSE(reg16.empty());
  EXPECT_FALSE(reg32.empty());
  EXPECT_FALSE(reg64.empty());
  EXPECT_FALSE(reg8.full());
  EXPECT_FALSE(reg16.full());
  EXPECT_FALSE(reg32.full());
  EXPECT_FALSE(reg64.full());
}

TEST_F(MemoryRegisterTest, copyMoveCtorAccessors) {
  MemoryRegister<uint8_t> reg8(250u);
  MemoryRegister<uint16_t> reg16(65001u);
  MemoryRegister<uint32_t> reg32(4001010101u);
  MemoryRegister<uint64_t> reg64(9001001001001001uLL);

  MemoryRegister<uint8_t> copy8(reg8);
  MemoryRegister<uint16_t> copy16(reg16);
  MemoryRegister<uint32_t> copy32(reg32);
  MemoryRegister<uint64_t> copy64(reg64);
  MemoryRegister<uint8_t> move8(std::move(reg8));
  MemoryRegister<uint16_t> move16(std::move(reg16));
  MemoryRegister<uint32_t> move32(std::move(reg32));
  MemoryRegister<uint64_t> move64(std::move(reg64));

  EXPECT_EQ(250u, copy8.value());
  EXPECT_EQ(250u, move8.value());
  EXPECT_EQ(65001u, copy16.value());
  EXPECT_EQ(65001u, move16.value());
  EXPECT_EQ(4001010101u, copy32.value());
  EXPECT_EQ(4001010101u, move32.value());
  EXPECT_EQ(9001001001001001uLL, copy64.value());
  EXPECT_EQ(9001001001001001uLL, move64.value());

  EXPECT_EQ(size_t{8u}, copy8.size());
  EXPECT_EQ(size_t{16u}, copy16.size());
  EXPECT_EQ(size_t{32u}, copy32.size());
  EXPECT_EQ(size_t{64u}, copy64.size());
  EXPECT_NE(size_t{0u}, move8.count());
  EXPECT_NE(size_t{0u}, move16.count());
  EXPECT_NE(size_t{0u}, move32.count());
  EXPECT_NE(size_t{0u}, move64.count());
}

TEST_F(MemoryRegisterTest, swapObjects) {
  MemoryRegister<uint32_t> regA(128u);
  MemoryRegister<uint32_t> regB(4001010101u);
  EXPECT_EQ(128u, regA.value());
  EXPECT_EQ(4001010101u, regB.value());

  std::swap(regA, regB);
  EXPECT_EQ(4001010101u, regA.value());
  EXPECT_EQ(128u, regB.value());

  regA.swap(regB);
  EXPECT_EQ(128u, regA.value());
  EXPECT_EQ(4001010101u, regB.value());
}

TEST_F(MemoryRegisterTest, clearFill) {
  MemoryRegister<uint32_t> regA(7u);
  EXPECT_EQ(7u, regA.value());
  EXPECT_EQ(size_t{32u}, regA.size());
  EXPECT_EQ(size_t{3u}, regA.count());
  EXPECT_FALSE(regA.empty());
  EXPECT_FALSE(regA.full());

  regA.clear();
  EXPECT_EQ(0u, regA.value());
  EXPECT_EQ(size_t{32u}, regA.size());
  EXPECT_EQ(size_t{0u}, regA.count());
  EXPECT_TRUE(regA.empty());
  EXPECT_FALSE(regA.full());

  regA.fill();
  EXPECT_EQ(0xFFFFFFFFu, regA.value());
  EXPECT_EQ(size_t{32u}, regA.size());
  EXPECT_EQ(size_t{32u}, regA.count());
  EXPECT_FALSE(regA.empty());
  EXPECT_TRUE(regA.full());

  regA.value(7u);
  EXPECT_EQ(7u, regA.value());
  EXPECT_EQ(size_t{32u}, regA.size());
  EXPECT_EQ(size_t{3u}, regA.count());
  EXPECT_FALSE(regA.empty());
  EXPECT_FALSE(regA.full());
}

// -- bit accessors --

TEST_F(MemoryRegisterTest, bitmasks) {
  MemoryRegister<uint8_t> reg8(0xF0u);
  MemoryRegister<uint16_t> reg16(0xF0F0u);
  MemoryRegister<uint32_t> reg32(0xF0F0F0F0u);
  MemoryRegister<uint64_t> reg64(uint64_t{0xF0F0F0F0F0F0F0F0uLL});
  EXPECT_EQ(size_t{8u}, reg8.size());
  EXPECT_EQ(size_t{16u}, reg16.size());
  EXPECT_EQ(size_t{32u}, reg32.size());
  EXPECT_EQ(size_t{64u}, reg64.size());
  EXPECT_EQ(size_t{4u}, reg8.count());
  EXPECT_EQ(size_t{8u}, reg16.count());
  EXPECT_EQ(size_t{16u}, reg32.count());
  EXPECT_EQ(size_t{32u}, reg64.count());
  EXPECT_EQ(0xF0u, reg8.getBits(0xFFu));
  EXPECT_EQ(0xF0u, reg8.getBits(static_cast<uint8_t>(0xFFFFFFFFu)));
  EXPECT_EQ(0xFu, reg8.getBits(0xFFu, 4u));
  EXPECT_EQ(0xF0u, reg8.getBits(0xF0u));
  EXPECT_EQ(0xFu, reg8.getBits(0xF0u, 4u));
  EXPECT_EQ(0u, reg8.getBits(0x0Fu));
  EXPECT_EQ(0u, reg8.getBits(0x0Fu, 4u));
  EXPECT_EQ(0x70u, reg8.getBits(0x77u));
  EXPECT_EQ(0x7u, reg8.getBits(0x70u, 4u));
  EXPECT_EQ(0xF0u, reg8.getBits(0xFFu, 0u));
  EXPECT_EQ(0x38u, reg8.getBits(0x7Fu, 1u));
  EXPECT_EQ(0xF0F0u, reg16.getBits(0xFFFFu));
  EXPECT_EQ(0xF000u, reg16.getBits(0xFF00u));
  EXPECT_EQ(0xF0Fu, reg16.getBits(0xF0F0u, 4u));
  EXPECT_EQ(0xFu, reg16.getBits(0xF0F0u, 12u));
  EXPECT_EQ(0u, reg16.getBits(0x0F0Fu));
  EXPECT_EQ(0x7070u, reg16.getBits(0x7777u));
  EXPECT_EQ(0x700u, reg16.getBits(0x7000u, 4u));
  EXPECT_EQ(0xF0F0u, reg16.getBits(0xFFFFu, 0u));
  EXPECT_EQ(0x3838u, reg16.getBits(0x7F70u, 1u));
  EXPECT_EQ(0xF0F0F0F0u, reg32.getBits(0xFFFFFFFFu));
  EXPECT_EQ(0xF000F000u, reg32.getBits(0xFF00FF00u));
  EXPECT_EQ(0xF0u, reg32.getBits(0xF0F0F0F0u, 24u));
  EXPECT_EQ(0u, reg32.getBits(0x0F0F0F0Fu));
  EXPECT_EQ(0x70703030u, reg32.getBits(0x77773333u));
  EXPECT_EQ(0x1u, reg32.getBits(0x1000u, 12u));
  EXPECT_EQ(0x1u, reg32.getBits(0xFFFFFFFFu, 31u));
  EXPECT_EQ(0xF0F0u, reg32.getBits(0xFFFFu, 0u));
  EXPECT_EQ(0x38381818u, reg32.getBits(0x7F703F30u, 1u));
  EXPECT_EQ(0xF0F0F0F0F0F0F0F0uLL, reg64.getBits(0xFFFFFFFFFFFFFFFFuLL));
  EXPECT_EQ(0xF000F000F000F000uLL, reg64.getBits(0xFF00FF00FF00FF00uLL));
  EXPECT_EQ(0xF000F000uLL, reg64.getBits(0xFF00FF00uLL));
  EXPECT_EQ(0xF000F000uLL, reg64.getBits(0xFF00FF0000000000uLL, 32u));
  EXPECT_EQ(0x7070303010100000uLL, reg64.getBits(0x7777333311110000uLL));
  EXPECT_EQ(0x7uLL, reg64.getBits(0x7777333311110000uLL, 60u));

  reg8.value(0x07u);
  reg16.value(0x0707u);
  reg32.value(0x07070707u);
  reg64.value(uint64_t{0x0707070707070707uLL});
  EXPECT_EQ(size_t{3u}, reg8.count());
  EXPECT_EQ(size_t{6u}, reg16.count());
  EXPECT_EQ(size_t{12u}, reg32.count());
  EXPECT_EQ(size_t{24u}, reg64.count());
  EXPECT_EQ(0x07u, reg8.getBits(0xFFu));
  EXPECT_EQ(0x1u, reg8.getBits(0xFFu, 2u));
  EXPECT_EQ(0u, reg8.getBits(0xF0u));
  EXPECT_EQ(0x07u, reg8.getBits(0x77u));
  EXPECT_EQ(0u, reg8.getBits(0x77u, 4u));
  EXPECT_EQ(0x01u, reg8.getBits(0x13u, 1u));
  EXPECT_EQ(0x0707u, reg16.getBits(0xFFFFu));
  EXPECT_EQ(0x0700u, reg16.getBits(0xFF00u));
  EXPECT_EQ(0x70, reg16.getBits(0xFFF0u, 4u));
  EXPECT_EQ(0u, reg16.getBits(0xF0F0u));
  EXPECT_EQ(0x40u, reg16.getBits(0x400u, 4u));
  EXPECT_EQ(0x07070707u, reg32.getBits(0xFFFFFFFFu));
  EXPECT_EQ(0x07000700u, reg32.getBits(0xFF00FF00u));
  EXPECT_EQ(0u, reg32.getBits(0xF0F0F0F0u));
  EXPECT_EQ(0x7u, reg32.getBits(0x0F0F0F0Fu, 24u));
  EXPECT_EQ(0x1u, reg32.getBits(0x100u, 8u));
  EXPECT_EQ(0x0707070707070707uLL, reg64.getBits(0xFFFFFFFFFFFFFFFFuLL));
  EXPECT_EQ(0x0700070007000700uLL, reg64.getBits(0xFF00FF00FF00FF00uLL));
  EXPECT_EQ(0x07000700uLL, reg64.getBits(0xFF00FF00uLL));
  EXPECT_EQ(0x07000700uLL, reg64.getBits(0xFF00FF0000000000uLL, 32u));

  reg8.clear();
  reg16.clear();
  reg32.clear();
  reg64.clear();
  EXPECT_EQ(size_t{0u}, reg8.count());
  EXPECT_EQ(size_t{0u}, reg16.count());
  EXPECT_EQ(size_t{0u}, reg32.count());
  EXPECT_EQ(size_t{0u}, reg64.count());
  EXPECT_EQ(0u, reg8.getBits(0xFFu));
  EXPECT_EQ(0u, reg8.getBits(0xFFu, 4u));
  EXPECT_EQ(0u, reg8.getBits(0xF7u));
  EXPECT_EQ(0u, reg16.getBits(0xFFFFu));
  EXPECT_EQ(0u, reg16.getBits(0x7F7Fu, 4u));
  EXPECT_EQ(0u, reg32.getBits(0xFFFFFFFFu));
  EXPECT_EQ(0u, reg32.getBits(0x7F7F7Fu, 4u));
  EXPECT_EQ(0uLL, reg64.getBits(0xFFFFFFFFFFFFFFFFuLL));
  EXPECT_EQ(0uLL, reg64.getBits(0x7F00F7007F7FuLL, 4u));

  reg8.fill();
  reg16.fill();
  reg32.fill();
  reg64.fill();
  EXPECT_EQ(size_t{8u}, reg8.count());
  EXPECT_EQ(size_t{16u}, reg16.count());
  EXPECT_EQ(size_t{32u}, reg32.count());
  EXPECT_EQ(size_t{64u}, reg64.count());
  EXPECT_EQ(0xFFu, reg8.getBits(0xFFu));
  EXPECT_EQ(0xFu, reg8.getBits(0xFFu, 4u));
  EXPECT_EQ(0xF7u, reg8.getBits(0xF7u));
  EXPECT_EQ(0xFFFFu, reg16.getBits(0xFFFFu));
  EXPECT_EQ(0x7F3u, reg16.getBits(0x7F3Fu, 4u));
  EXPECT_EQ(0xFFFFFFFFu, reg32.getBits(0xFFFFFFFFu));
  EXPECT_EQ(0x7F307F7u, reg32.getBits(0x7F307F7Fu, 4u));
  EXPECT_EQ(0xFFFFFFFFFFFFFFFFuLL, reg64.getBits(0xFFFFFFFFFFFFFFFFuLL));
  EXPECT_EQ(0x7F00F7007FuLL, reg64.getBits(0x7F00F7007F7FuLL, 8u));
}

TEST_F(MemoryRegisterTest, getSetBit) {
  MemoryRegister<uint32_t> regA(0xF0807031u);
  EXPECT_EQ(0xF0807031u, regA.value());
  EXPECT_TRUE(regA.getBit(0));
  EXPECT_FALSE(regA.getBit(1));
  EXPECT_FALSE(regA.getBit(2));
  EXPECT_FALSE(regA.getBit(3));
  EXPECT_TRUE(regA.getBit(4));
  EXPECT_TRUE(regA.getBit(5));
  EXPECT_FALSE(regA.getBit(6));
  EXPECT_FALSE(regA.getBit(7));
  EXPECT_FALSE(regA.getBit(8));
  EXPECT_FALSE(regA.getBit(9));
  EXPECT_FALSE(regA.getBit(10));
  EXPECT_FALSE(regA.getBit(11));
  EXPECT_TRUE(regA.getBit(12));
  EXPECT_TRUE(regA.getBit(13));
  EXPECT_TRUE(regA.getBit(14));
  EXPECT_FALSE(regA.getBit(15));
  EXPECT_FALSE(regA.getBit(16));
  EXPECT_FALSE(regA.getBit(17));
  EXPECT_FALSE(regA.getBit(18));
  EXPECT_FALSE(regA.getBit(19));
  EXPECT_FALSE(regA.getBit(20));
  EXPECT_FALSE(regA.getBit(21));
  EXPECT_FALSE(regA.getBit(22));
  EXPECT_TRUE(regA.getBit(23));
  EXPECT_FALSE(regA.getBit(24));
  EXPECT_FALSE(regA.getBit(25));
  EXPECT_FALSE(regA.getBit(26));
  EXPECT_FALSE(regA.getBit(27));
  EXPECT_TRUE(regA.getBit(28));
  EXPECT_TRUE(regA.getBit(29));
  EXPECT_TRUE(regA.getBit(30));
  EXPECT_TRUE(regA.getBit(31));

  regA.setBit(0, false);
  EXPECT_EQ(0xF0807030u, regA.value());
  EXPECT_FALSE(regA.getBit(0));
  EXPECT_FALSE(regA.getBit(1));
  regA.setBit(3, true);
  EXPECT_EQ(0xF0807038u, regA.value());
  EXPECT_FALSE(regA.getBit(0));
  EXPECT_FALSE(regA.getBit(2));
  EXPECT_TRUE(regA.getBit(3));
  EXPECT_TRUE(regA.getBit(4));

  regA.clear();
  EXPECT_FALSE(regA.getBit(3));
  regA.fill();
  EXPECT_TRUE(regA.getBit(3));
}

TEST_F(MemoryRegisterTest, getSetByte) {
  MemoryRegister<uint32_t> regA(0xF0807031u);
  EXPECT_EQ(0xF0807031u, regA.value());
  EXPECT_EQ(0x31u, regA.getByte8(0));
  EXPECT_EQ(0x70u, regA.getByte8(1));
  EXPECT_EQ(0x80u, regA.getByte8(2));
  EXPECT_EQ(0xF0u, regA.getByte8(3));

  regA.setByte8(1, 0x55u);
  EXPECT_EQ(0xF0805531u, regA.value());
  EXPECT_EQ(0x31u, regA.getByte8(0));
  EXPECT_EQ(0x55u, regA.getByte8(1));
  EXPECT_EQ(0x80u, regA.getByte8(2));
  EXPECT_EQ(0xF0u, regA.getByte8(3));
  regA.setByte8(0, 0u);
  EXPECT_EQ(0xF0805500u, regA.value());
  EXPECT_EQ(0u, regA.getByte8(0));
  EXPECT_EQ(0x55u, regA.getByte8(1));
  EXPECT_EQ(0x80u, regA.getByte8(2));
  EXPECT_EQ(0xF0u, regA.getByte8(3));
  regA.setByte8(0, static_cast<uint8_t>(0xFFFFFFFFu));
  EXPECT_EQ(0xF08055FFu, regA.value());
  EXPECT_EQ(0xFFu, regA.getByte8(0));
  EXPECT_EQ(0x55u, regA.getByte8(1));
  EXPECT_EQ(0x80u, regA.getByte8(2));
  EXPECT_EQ(0xF0u, regA.getByte8(3));

  regA.clear();
  EXPECT_EQ(0u, regA.getByte8(1));
  regA.fill();
  EXPECT_EQ(0xFFu, regA.getByte8(1));

  MemoryRegister<uint8_t> regB(0x31u);
  EXPECT_EQ(0x31u, regB.getByte8(0));
}

TEST_F(MemoryRegisterTest, getSetWord) {
  MemoryRegister<uint64_t> regA(0xF0807031A0605011uLL);
  EXPECT_EQ(0xF0807031A0605011uLL, regA.value());
  EXPECT_EQ(0x5011u, regA.getWord16(0));
  EXPECT_EQ(0xA060u, regA.getWord16(1));
  EXPECT_EQ(0x7031u, regA.getWord16(2));
  EXPECT_EQ(0xF080u, regA.getWord16(3));

  regA.setWord16(1, 0x5544uLL);
  EXPECT_EQ(0xF080703155445011uLL, regA.value());
  EXPECT_EQ(0x5011u, regA.getWord16(0));
  EXPECT_EQ(0x5544u, regA.getWord16(1));
  EXPECT_EQ(0x7031u, regA.getWord16(2));
  EXPECT_EQ(0xF080u, regA.getWord16(3));
  regA.setWord16(0, 0uLL);
  EXPECT_EQ(0xF080703155440000uLL, regA.value());
  EXPECT_EQ(0uLL, regA.getWord16(0));
  EXPECT_EQ(0x5544u, regA.getWord16(1));
  EXPECT_EQ(0x7031u, regA.getWord16(2));
  EXPECT_EQ(0xF080u, regA.getWord16(3));

  regA.clear();
  EXPECT_EQ(0u, regA.getWord16(1));
  regA.fill();
  EXPECT_EQ(0xFFFFu, regA.getWord16(1));

  MemoryRegister<uint8_t> regB(0x31u);
  EXPECT_EQ(0x31u, regB.getWord16(0));
  regB.setWord16(0, 0x22u);
  EXPECT_EQ(0x22u, regB.getWord16(0));
  regB.setWord16(0, static_cast<uint16_t>(0xFFFFFFFFu));
  EXPECT_EQ(0xFFu, regB.getWord16(0));
  EXPECT_EQ(0xFFu, regB.value());

  MemoryRegister<uint32_t> regC(0xF0807031u);
  EXPECT_EQ(0x7031u, regC.getWord16(0));
  regC.setWord16(0, 0x22u);
  EXPECT_EQ(0x22u, regC.getWord16(0));
  regC.setWord16(0, static_cast<uint16_t>(0xFFFFFFFFu));
  EXPECT_EQ(0xFFFFu, regC.getWord16(0));
  EXPECT_EQ(0xF080FFFFu, regC.value());
}

TEST_F(MemoryRegisterTest, getSetDword) {
  MemoryRegister<uint64_t> regA(0xF0807031A0605011uLL);
  EXPECT_EQ(0xF0807031A0605011uLL, regA.value());
  EXPECT_EQ(0xA0605011u, regA.getDword32(0));
  EXPECT_EQ(0xF0807031u, regA.getDword32(1));

  regA.setDword32(1, 0x55443322uLL);
  EXPECT_EQ(0x55443322A0605011uLL, regA.value());
  EXPECT_EQ(0xA0605011u, regA.getDword32(0));
  EXPECT_EQ(0x55443322u, regA.getDword32(1));
  regA.setDword32(0, 0uLL);
  EXPECT_EQ(0x5544332200000000uLL, regA.value());
  EXPECT_EQ(0u, regA.getDword32(0));
  EXPECT_EQ(0x55443322u, regA.getDword32(1));

  regA.clear();
  EXPECT_EQ(0u, regA.getDword32(0));
  EXPECT_EQ(0u, regA.getDword32(1));
  regA.fill();
  EXPECT_EQ(0xFFFFFFFFu, regA.getDword32(0));
  EXPECT_EQ(0xFFFFFFFFu, regA.getDword32(1));

  MemoryRegister<uint8_t> regB(0x31u);
  EXPECT_EQ(0x31u, regB.getDword32(0));
  regB.setDword32(0, 0x22u);
  EXPECT_EQ(0x22u, regB.getDword32(0));
  regB.setDword32(0, 0xFFFFFFFFu);
  EXPECT_EQ(0xFFu, regB.getDword32(0));
  EXPECT_EQ(0xFFu, regB.value());

  MemoryRegister<uint32_t> regC(0xF0807031u);
  EXPECT_EQ(0xF0807031u, regC.getDword32(0));
  regC.setDword32(0, 0xAA997722u);
  EXPECT_EQ(0xAA997722u, regC.getDword32(0));
  EXPECT_EQ(0xAA997722u, regC.value());
}

TEST_F(MemoryRegisterTest, getSetQword) {
  MemoryRegister<uint64_t> regA(0xF0807031A0605011uLL);
  EXPECT_EQ(0xF0807031A0605011uLL, regA.value());
  EXPECT_EQ(0xF0807031A0605011uLL, regA.getQword64(0));

  regA.setQword64(0, 0x99775544AABBCCDDuLL);
  EXPECT_EQ(0x99775544AABBCCDDuLL, regA.value());
  EXPECT_EQ(0x99775544AABBCCDDuLL, regA.getQword64(0));
  regA.setQword64(0, 0uLL);
  EXPECT_EQ(0uLL, regA.value());
  EXPECT_EQ(0uLL, regA.getQword64(0));

  regA.clear();
  EXPECT_EQ(0u, regA.getQword64(0));
  regA.fill();
  EXPECT_EQ(0xFFFFFFFFFFFFFFFFu, regA.getQword64(0));

  MemoryRegister<uint8_t> regB(0x31u);
  EXPECT_EQ(0x31uLL, regB.getQword64(0));
  regB.setQword64(0, 0x22uLL);
  EXPECT_EQ(0x22uLL, regB.getQword64(0));
  regB.setQword64(0, 0xFFFFFFFFFFFFFFFFuLL);
  EXPECT_EQ(0xFFuLL, regB.getQword64(0));
  EXPECT_EQ(0xFFu, regB.value());
}

// -- bitwise operators --

TEST_F(MemoryRegisterTest, bitwiseOperators) {
  MemoryRegister<uint32_t> regA(0xF0807031u);
  EXPECT_EQ(0xF0800000u, (regA & 0xFFFF0000u).value());
  EXPECT_EQ(0xF0800000u, (regA & MemoryRegister<uint32_t>(0xFFFF0000u)).value());
  EXPECT_EQ(0xFFFF7031u, (regA | 0xFFFF0000u).value());
  EXPECT_EQ(0xFFFF7031u, (regA | MemoryRegister<uint32_t>(0xFFFF0000u)).value());
  EXPECT_EQ(0x0F7F7031u, (regA ^ 0xFFFF0000u).value());
  EXPECT_EQ(0x0F7F7031u, (regA ^ MemoryRegister<uint32_t>(0xFFFF0000u)).value());

  MemoryRegister<uint32_t> regB(regA);
  regB &= 0xFFFF0000u;
  EXPECT_EQ(0xF0800000u, regB.value());
  regB = regA;
  regB &= MemoryRegister<uint32_t>(0xFFFF0000u);
  EXPECT_EQ(0xF0800000u, regB.value());
  regB = regA;
  regB |= 0xFFFF0000u;
  EXPECT_EQ(0xFFFF7031u, regB.value());
  regB = regA;
  regB |= MemoryRegister<uint32_t>(0xFFFF0000u);
  EXPECT_EQ(0xFFFF7031u, regB.value());
  regB = regA;
  regB ^= 0xFFFF0000u;
  EXPECT_EQ(0x0F7F7031u, regB.value());
  regB = std::move(regA);
  regB ^= MemoryRegister<uint32_t>(0xFFFF0000u);
  EXPECT_EQ(0x0F7F7031u, regB.value());

  regA.value(0xF0F0F0F0u);
  EXPECT_EQ(0x0F0F0F0Fu, (~regA).value());
}

TEST_F(MemoryRegisterTest, shiftOperators) {
  MemoryRegister<uint32_t> regA(0xF0807031u);
  MemoryRegister<uint64_t> regB(0xF0807031F0807031uLL);
  MemoryRegister<uint8_t> regC(0x31u);

  EXPECT_EQ(0xF0807031u, (regA >> 0u).value());
  EXPECT_EQ(0x78403818u, (regA >> 1u).value());
  EXPECT_EQ(0xF080703u, (regA >> 4u).value());
  EXPECT_EQ(0xF080u, (regA >> 16u).value());
  EXPECT_EQ(0x1u, (regA >> 31u).value());

  EXPECT_EQ(0xF0807031F0807031uLL, (regB >> 0u).value());
  EXPECT_EQ(0x78403818F8403818uLL, (regB >> 1u).value());
  EXPECT_EQ(0xF0807031F080703uLL, (regB >> 4u).value());
  EXPECT_EQ(0xF0807031uLL, (regB >> 32u).value());
  EXPECT_EQ(0x1uLL, (regB >> 63u).value());

  EXPECT_EQ(0x31u, (regC >> 0u).value());
  EXPECT_EQ(0x18u, (regC >> 1u).value());
  EXPECT_EQ(0x3u, (regC >> 4u).value());
  EXPECT_EQ(0u, (regC >> 7u).value());

  EXPECT_EQ(0xF0807031u, (regA << 0u).value());
  EXPECT_EQ(0x8070310u, (regA << 4u).value());
  EXPECT_EQ(0x70310000u, (regA << 16u).value());
  EXPECT_EQ(0x80000000u, (regA << 31u).value());

  EXPECT_EQ(0xF0807031F0807031uLL, (regB << 0u).value());
  EXPECT_EQ(0x807031F08070310uLL, (regB << 4u).value());
  EXPECT_EQ(0xF080703100000000uLL, (regB << 32u).value());
  EXPECT_EQ(0x8000000000000000uLL, (regB << 63u).value());

  EXPECT_EQ(0x31u, (regC << 0u).value());
  EXPECT_EQ(0x10u, (regC << 4u).value());
  EXPECT_EQ(0x80u, (regC << 7u).value());

  regA <<= 1u;
  EXPECT_EQ(0xE100E062u, regA.value());
  regB >>= 1u;
  EXPECT_EQ(0x78403818F8403818uLL, regB.value());
}

// -- comparisons --

TEST_F(MemoryRegisterTest, toBool) {
  MemoryRegister<uint32_t> regDef;
  MemoryRegister<uint32_t> regA(0xF0807031u);
  MemoryRegister<uint32_t> regB(0u);
  EXPECT_FALSE(static_cast<bool>(regDef));
  EXPECT_TRUE(static_cast<bool>(regA));
  EXPECT_FALSE(static_cast<bool>(regB));

  regA.clear();
  EXPECT_FALSE(static_cast<bool>(regA));
  regB.fill();
  EXPECT_TRUE(static_cast<bool>(regB));
}

TEST_F(MemoryRegisterTest, compare) {
  MemoryRegister<uint32_t> regA(0xF0807031u);
  MemoryRegister<uint32_t> regB(0xF0807031u);
  MemoryRegister<uint32_t> regC(0xF0800000u);
  MemoryRegister<uint32_t> regD(0xFFFFFFFFu);

  EXPECT_TRUE(regA == regB);
  EXPECT_FALSE(regA == regC);
  EXPECT_FALSE(regA == regD);

  EXPECT_FALSE(regA != regB);
  EXPECT_TRUE(regA != regC);
  EXPECT_TRUE(regA != regD);

  EXPECT_TRUE(regA <= regB);
  EXPECT_FALSE(regA <= regC);
  EXPECT_TRUE(regA <= regD);

  EXPECT_TRUE(regA >= regB);
  EXPECT_TRUE(regA >= regC);
  EXPECT_FALSE(regA >= regD);

  EXPECT_FALSE(regA < regB);
  EXPECT_FALSE(regA < regC);
  EXPECT_TRUE(regA < regD);

  EXPECT_FALSE(regA > regB);
  EXPECT_TRUE(regA > regC);
  EXPECT_FALSE(regA > regD);
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif
