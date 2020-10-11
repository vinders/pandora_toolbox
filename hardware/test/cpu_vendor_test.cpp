#include <gtest/gtest.h>
#include <string>
#include <unordered_set>
#include <hardware/cpu_vendor.h>

using namespace pandora::hardware;

class CpuVendorTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- enum serialization --

TEST_F(CpuVendorTest, enumToLabel) {
  std::unordered_set<std::string> foundLabels;

  EXPECT_TRUE(toString(CpuVendor::unknown).empty());
  for (auto vendor : {
    CpuVendor::intel,
    CpuVendor::amd,
    CpuVendor::armLtd,
    CpuVendor::broadcom,
    CpuVendor::dec,
    CpuVendor::ampere,
    CpuVendor::cavium,
    CpuVendor::fujitsu,
    CpuVendor::infineon,
    CpuVendor::motorola,
    CpuVendor::nvidia,
    CpuVendor::amcc,
    CpuVendor::qualcomm,
    CpuVendor::marvell
  }) {
    std::string result = toString(vendor);
    EXPECT_FALSE(result.empty());
    EXPECT_FALSE(foundLabels.find(result) != foundLabels.end());
    foundLabels.insert(result);
  }
}
