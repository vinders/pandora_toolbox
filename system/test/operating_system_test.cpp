#include <gtest/gtest.h>
#include <system/operating_system.h>

using namespace pandora::system;

class OperatingSystemTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};

// -- OS detection --

TEST_F(OperatingSystemTest, macroVersion) {
# if defined(_WINDOWS)
    EXPECT_EQ(_SYSTEM_OS_WINDOWS, _SYSTEM_OPERATING_SYSTEM);
# elif !defined(__APPLE__) && !defined(__ANDROID__) && defined(__linux__)
    EXPECT_EQ(_SYSTEM_OS_LINUX, _SYSTEM_OPERATING_SYSTEM);
# else
    EXPECT_NE(_SYSTEM_OS_UNKNOWN, _SYSTEM_OPERATING_SYSTEM);
# endif
}

TEST_F(OperatingSystemTest, functionVersion) {
# if defined(_WINDOWS)
    EXPECT_EQ(OperatingSystem::windows, getOperatingSystem());
# elif !defined(__APPLE__) && !defined(__ANDROID__) && defined(__linux__)
    EXPECT_EQ(OperatingSystem::gnu_linux, getOperatingSystem());
# else
    EXPECT_NE(OperatingSystem::unknown, getOperatingSystem());
# endif
}

// -- enumeration --

TEST_F(OperatingSystemTest, operatingSystemFlagOps) {
  OperatingSystem flag1 = OperatingSystem::gnu_linux;
  OperatingSystem flag2 = OperatingSystem::gnu_linux;
  EXPECT_TRUE(flag1 == flag2);
  EXPECT_FALSE(flag1 != flag2);
  EXPECT_FALSE(flag1 < flag2);
  EXPECT_TRUE(flag1 <= flag2);
  EXPECT_FALSE(flag1 > flag2);
  EXPECT_TRUE(flag1 >= flag2);
  flag2 = OperatingSystem::windows;
  EXPECT_FALSE(flag1 == flag2);
  EXPECT_TRUE(flag1 != flag2);
  EXPECT_EQ((static_cast<uint32_t>(flag1) < static_cast<uint32_t>(flag2)), (flag1 < flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) <= static_cast<uint32_t>(flag2)), (flag1 <= flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) > static_cast<uint32_t>(flag2)), (flag1 > flag2));
  EXPECT_EQ((static_cast<uint32_t>(flag1) >= static_cast<uint32_t>(flag2)), (flag1 >= flag2));

  EXPECT_EQ(static_cast<OperatingSystem>(static_cast<uint32_t>(flag1) & static_cast<uint32_t>(flag2)), (flag1 & flag2));
  EXPECT_EQ(static_cast<OperatingSystem>(static_cast<uint32_t>(flag1) | static_cast<uint32_t>(flag2)), (flag1 | flag2));
  EXPECT_EQ(static_cast<OperatingSystem>(static_cast<uint32_t>(flag1) ^ static_cast<uint32_t>(flag2)), (flag1 ^ flag2));
  EXPECT_EQ(static_cast<OperatingSystem>(~static_cast<uint32_t>(flag1)), (~flag1));

  EXPECT_EQ((OperatingSystem::gnu_linux | OperatingSystem::windows), addFlag(flag1, flag2));
  EXPECT_EQ((OperatingSystem::gnu_linux | OperatingSystem::windows), flag1);
  EXPECT_EQ((OperatingSystem::windows), removeFlag(flag1, OperatingSystem::gnu_linux));
  EXPECT_EQ((OperatingSystem::windows), flag1);
}
