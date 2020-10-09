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
