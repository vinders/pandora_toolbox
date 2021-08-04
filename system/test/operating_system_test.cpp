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
