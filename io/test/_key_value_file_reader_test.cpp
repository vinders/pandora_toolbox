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
#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#define GTEST_HAS_TR1_TUPLE 0
#include <gtest/gtest.h>
#include <cstdio>
#include <io/_private/_key_value_file_reader.h>

#define _TEST_FILE_PATH "_KeyValueFileReaderTest.txt"
#define _BUFFER_SIZE 128

using namespace pandora::io;

class _KeyValueFileReaderTest : public testing::Test {
public:
protected:
  static void SetUpTestCase() {
    FILE* testFile = fopen(_TEST_FILE_PATH, "w");
    _KeyValueFileReaderTest::_isSetUpSuccess = (testFile != nullptr);
    if (_isSetUpSuccess) {
      fprintf(testFile, "first_line = dummy text\n");
      fprintf(testFile, "second_line     =     line with spaces and = in value   \n");
      fprintf(testFile, "tabbed line\t\t=\tline with tabs\t\n");
      fprintf(testFile, "\n");
      fprintf(testFile, "empty_line=\n");
      fprintf(testFile, "empty_line2 =  \t  \n");
      fprintf(testFile, "last_line=end");
      fflush(testFile);
      fclose(testFile);
    }
  }
  static void TearDownTestCase() {
    remove(_TEST_FILE_PATH);
  }

  void SetUp() override { ASSERT_TRUE(_isSetUpSuccess); }
  void TearDown() override {}

  static bool _isSetUpSuccess;
};
bool _KeyValueFileReaderTest::_isSetUpSuccess = true;

// -- finding keys --

TEST_F(_KeyValueFileReaderTest, firstLine) {
  char buffer[_BUFFER_SIZE]{ 0 };
  EXPECT_EQ(size_t{ 10u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "first_line", buffer, _BUFFER_SIZE));
  EXPECT_EQ(std::string("dummy text"), std::string(buffer));
}

TEST_F(_KeyValueFileReaderTest, lastLine) {
  char buffer[_BUFFER_SIZE]{ 0 };
  EXPECT_EQ(size_t{ 3u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "last_line", buffer, _BUFFER_SIZE));
  EXPECT_EQ(std::string("end"), std::string(buffer));
}

TEST_F(_KeyValueFileReaderTest, emptyLine) {
  char buffer[_BUFFER_SIZE]{ 0 };
  EXPECT_EQ(size_t{ 0u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "empty_line", buffer, _BUFFER_SIZE));
  EXPECT_EQ(size_t{ 0u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "empty_line2", buffer, _BUFFER_SIZE));
}

TEST_F(_KeyValueFileReaderTest, notFoundLine) {
  char buffer[_BUFFER_SIZE]{ 0 };
  EXPECT_EQ(size_t{ 0u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "this_line_does_not_exist", buffer, _BUFFER_SIZE));
}

// -- invalid params --

TEST_F(_KeyValueFileReaderTest, invalidFile) {
  char buffer[_BUFFER_SIZE]{ 0 };
  EXPECT_EQ(size_t{ 0u }, KeyValueFileReader::searchFileForValue("dummy_file_that_does_not_exist.void", "first_line", buffer, _BUFFER_SIZE));
  EXPECT_EQ(size_t{ 0 }, KeyValueFileReader::searchForValue(nullptr, "key", buffer, _BUFFER_SIZE));
}

TEST_F(_KeyValueFileReaderTest, bufferTooSmall) {
  char buffer[15]{ 0 };
  EXPECT_EQ(size_t{ 1u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "first_line", buffer, 15));
  EXPECT_EQ(std::string("d"), std::string(buffer));
  EXPECT_EQ(size_t{ 0u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "first_line", buffer, 13));
  EXPECT_EQ(size_t{ 0u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "first_line", buffer, 10));
}

TEST_F(_KeyValueFileReaderTest, uninitializedBuffer) {
  char buffer[_BUFFER_SIZE];
  EXPECT_EQ(size_t{ 10u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "first_line", buffer, _BUFFER_SIZE));
  EXPECT_EQ(std::string("dummy text"), std::string(buffer));
}

// -- special values --

TEST_F(_KeyValueFileReaderTest, lineWithSpaces) {
  char buffer[_BUFFER_SIZE]{ 0 };
  EXPECT_EQ(size_t{ 31u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "second_line", buffer, _BUFFER_SIZE));
  EXPECT_EQ(std::string("line with spaces and = in value"), std::string(buffer));
}

TEST_F(_KeyValueFileReaderTest, lineWithTabs) {
  char buffer[_BUFFER_SIZE]{ 0 };
  EXPECT_EQ(size_t{ 14u }, KeyValueFileReader::searchFileForValue(_TEST_FILE_PATH, "tabbed line", buffer, _BUFFER_SIZE));
  EXPECT_EQ(std::string("line with tabs"), std::string(buffer));
}
