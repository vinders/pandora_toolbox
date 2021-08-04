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
#define GTEST_HAS_TR1_TUPLE 0
#include <gtest/gtest.h>
#include <cstdio>
#include <cstring>
#include <io/_private/_user_home_path.h>

using namespace pandora::io;

class _UserHomePathTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};

// -- user standard home paths --

TEST_F(_UserHomePathTest, userHomePath) {
  auto path = _getUserHomePath();
# ifdef _WINDOWS
    wprintf(L"%s\n", path.c_str());
    ASSERT_LE(size_t{ 4u }, path.size());
    EXPECT_TRUE(path[0] == L'%' || path[0] == L'\\' || path[0] == L'/' || (path[0] >= L'A' && path[0] <= L'Z'));
# else
    printf("%s\n", path.c_str());
    ASSERT_LE(size_t{ 4u }, path.size());
    EXPECT_TRUE(path[0] == L'$' || path[0] == L'\\' || path[0] == L'/' || path[0] == 'h' || path[0] == '.' || path[0] == 's'); // 'h'/'s'->sda,hdb,...
  
#   if (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix)) && !defined(__APPLE__) && !defined(__ANDROID__)
      char* homeDir = __readUserDirectoryInPasswordFile();
      if (homeDir != nullptr) { // if supported
        size_t homeDirSize = strnlen(homeDir, 255);
        ASSERT_LE(size_t{ 4u }, homeDirSize);
        EXPECT_TRUE(homeDir[0] == L'$' || homeDir[0] == L'\\' || homeDir[0] == L'/' || homeDir[0] == 'h' || homeDir[0] == '.' || homeDir[0] == 's');
      }
#   endif
# endif
}

TEST_F(_UserHomePathTest, localConfigPath) {
  auto path = _getUserLocalConfigPath();
# ifdef _WINDOWS
    wprintf(L"%s\n", path.c_str());
    ASSERT_LE(size_t{ 4u }, path.size());
    EXPECT_TRUE(path[0] == L'%' || path[0] == L'\\' || path[0] == L'/' || (path[0] >= L'A' && path[0] <= L'Z'));
# else
    printf("%s\n", path.c_str());
    ASSERT_LE(size_t{ 4u }, path.size());
    EXPECT_TRUE(path[0] == L'$' || path[0] == L'\\' || path[0] == L'/' || path[0] == 'h' || path[0] == '.' || path[0] == 's');
# endif
}
