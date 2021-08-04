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
#include <io/file_system_locations.h>
#if defined(__APPLE__)
# include <TargetConditionals.h>
#endif

using namespace pandora::io;

class FileSystemLocationsTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
  
  // verifications to repeat for each standard location
  static inline void _verifyStandardLocation(FileSystemLocation location) {
#   if defined(_WINDOWS)
      std::vector<pandora::io::SystemPath> paths = FileSystemLocationFinder::standardLocation(location, L"FileSystemLocationsTest");
#   else
      std::vector<pandora::io::SystemPath> paths = FileSystemLocationFinder::standardLocation(location, "FileSystemLocationsTest");
#   endif
    EXPECT_FALSE(paths.empty());
    for (auto& it : paths) {
      EXPECT_FALSE(it.empty());
      if (!it.empty()) {
#       if defined(_WINDOWS)
          EXPECT_TRUE((it[0] >= 'A' && it[0] <= 'Z') || it[0] == '\\' || it[0] == '/');
#       elif !defined(__ANDROID__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE)
#         if defined(__APPLE__)
            EXPECT_TRUE(it[0] == '/' || it[0] == '~' || it[0] == '.');
#         else
            EXPECT_TRUE(it[0] == '/');
#         endif
#       endif
      }
    }
  }
};

// -- current location --

TEST_F(FileSystemLocationsTest, currentLocationPath) {
  auto path = FileSystemLocationFinder::currentLocation();
# ifdef _WINDOWS
    wprintf(L"%s\n", path.c_str());
    ASSERT_LE(size_t{ 4u }, path.size());
    EXPECT_TRUE(path[0] == L'%' || path[0] == L'\\' || path[0] == L'/' || (path[0] >= L'A' && path[0] <= L'Z'));
# elif defined(__ANDROID__) || (defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
    EXPECT_TRUE(path.empty() || path[0] == L'$' || path[0] == L'\\' || path[0] == L'/' || path[0] == 'h' || path[0] == '.' || path[0] == 's' || path[0] == '~');
# else
    printf("%s\n", path.c_str());
    ASSERT_LE(size_t{ 4u }, path.size());
    EXPECT_TRUE(path[0] == L'$' || path[0] == L'\\' || path[0] == L'/' || path[0] == 'h' || path[0] == '.' || path[0] == 's' || path[0] == '~');
# endif
}

// -- home location --

TEST_F(FileSystemLocationsTest, homeLocationPath) {
  auto path = FileSystemLocationFinder::homeLocation();
# ifdef _WINDOWS
    wprintf(L"%s\n", path.c_str());
    ASSERT_LE(size_t{ 4u }, path.size());
    EXPECT_TRUE(path[0] == L'%' || path[0] == L'\\' || path[0] == L'/' || (path[0] >= L'A' && path[0] <= L'Z'));
# elif defined(__ANDROID__)
    printf("%s\n", path.c_str());
    EXPECT_TRUE(path.empty() || path == std::string("files"));
# elif defined(__APPLE__) && defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    EXPECT_TRUE(path.empty() || path[0] == L'$' || path[0] == L'\\' || path[0] == L'/' || path[0] == 'h' || path[0] == '.' || path[0] == 's' || path[0] == '~');
# else
    printf("%s\n", path.c_str());
    ASSERT_LE(size_t{ 4u }, path.size());
    EXPECT_TRUE(path[0] == L'$' || path[0] == L'\\' || path[0] == L'/' || path[0] == 'h' || path[0] == '.' || path[0] == 's' || path[0] == '~');
# endif
}

// -- standard system locations --

TEST_F(FileSystemLocationsTest, standardLocation) {
  _verifyStandardLocation(FileSystemLocation::home);
  _verifyStandardLocation(FileSystemLocation::desktop);
  _verifyStandardLocation(FileSystemLocation::documents);
  _verifyStandardLocation(FileSystemLocation::pictures);
  _verifyStandardLocation(FileSystemLocation::music);
  _verifyStandardLocation(FileSystemLocation::movies);
  _verifyStandardLocation(FileSystemLocation::downloads);

  _verifyStandardLocation(FileSystemLocation::appData);
  _verifyStandardLocation(FileSystemLocation::localAppData);
  _verifyStandardLocation(FileSystemLocation::appConfig);
  _verifyStandardLocation(FileSystemLocation::commonAppConfig);
  _verifyStandardLocation(FileSystemLocation::cache);
  _verifyStandardLocation(FileSystemLocation::localCache);
  _verifyStandardLocation(FileSystemLocation::sharedCache);
  _verifyStandardLocation(FileSystemLocation::temp);
  _verifyStandardLocation(FileSystemLocation::fonts);

# if defined(_WINDOWS) || (!defined(__ANDROID__) && (!defined(TARGET_OS_IPHONE) || !TARGET_OS_IPHONE))
    _verifyStandardLocation(FileSystemLocation::applications);
    _verifyStandardLocation(FileSystemLocation::sharedApps);
    _verifyStandardLocation(FileSystemLocation::appMenu);

    _verifyStandardLocation(FileSystemLocation::system);
    _verifyStandardLocation(FileSystemLocation::localBinaries);
    _verifyStandardLocation(FileSystemLocation::sharedBinaries);
    _verifyStandardLocation(FileSystemLocation::sharedLibraries);
    _verifyStandardLocation(FileSystemLocation::commonBinaryFiles);
# endif
    EXPECT_NO_THROW(FileSystemLocationFinder::standardLocation(FileSystemLocation::printers));
    EXPECT_NO_THROW(FileSystemLocationFinder::standardLocation(FileSystemLocation::trash));
    EXPECT_TRUE(FileSystemLocationFinder::standardLocation((FileSystemLocation)12345678u).empty());
}
