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
#include <logic/strings.h>

using namespace pandora::logic;

class StringsTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- string analysis --

TEST_F(StringsTest, strLength) {
  EXPECT_EQ(size_t{ 0 }, stringLength(""));
  EXPECT_EQ(size_t{ 0 }, stringLength(L""));
  EXPECT_EQ(size_t{ 1 }, stringLength("a"));
  EXPECT_EQ(size_t{ 1 }, stringLength(L"a"));
  EXPECT_EQ(size_t{ 3 }, stringLength("abc"));
  EXPECT_EQ(size_t{ 3 }, stringLength(L"abc"));
  EXPECT_EQ(size_t{ 20 }, stringLength("12345678901234567890"));
  EXPECT_EQ(size_t{ 20 }, stringLength(L"12345678901234567890"));
}
TEST_F(StringsTest, nonTrimmableIndex) {
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex(""));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex(L""));
  EXPECT_EQ(size_t{ 0 }, rfindAfterNoTrimIndex("", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, rfindAfterNoTrimIndex(L"", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex("a"));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex(L"a"));
  EXPECT_EQ(size_t{ 1 }, rfindAfterNoTrimIndex("a", size_t{ 1 }));
  EXPECT_EQ(size_t{ 1 }, rfindAfterNoTrimIndex(L"a", size_t{ 1 }));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex("12345678901234567890"));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex(L"12345678901234567890"));
  EXPECT_EQ(size_t{ 20 }, rfindAfterNoTrimIndex("12345678901234567890", size_t{ 20 }));
  EXPECT_EQ(size_t{ 20 }, rfindAfterNoTrimIndex(L"12345678901234567890", size_t{ 20 }));
  EXPECT_EQ(size_t{ 1 }, findNoTrimIndex(" "));
  EXPECT_EQ(size_t{ 1 }, findNoTrimIndex(L" "));
  EXPECT_EQ(size_t{ 0 }, rfindAfterNoTrimIndex(" ", size_t{ 1 }));
  EXPECT_EQ(size_t{ 0 }, rfindAfterNoTrimIndex(L" ", size_t{ 1 }));
  EXPECT_EQ(size_t{ 1 }, findNoTrimIndex(" a"));
  EXPECT_EQ(size_t{ 1 }, findNoTrimIndex(L" a"));
  EXPECT_EQ(size_t{ 2 }, rfindAfterNoTrimIndex(" a", size_t{ 2 }));
  EXPECT_EQ(size_t{ 2 }, rfindAfterNoTrimIndex(L" a", size_t{ 2 }));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex("a "));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex(L"a "));
  EXPECT_EQ(size_t{ 1 }, rfindAfterNoTrimIndex("a ", size_t{ 2 }));
  EXPECT_EQ(size_t{ 1 }, rfindAfterNoTrimIndex(L"a ", size_t{ 2 }));
  EXPECT_EQ(size_t{ 1 }, findNoTrimIndex(" 12345678901234567890"));
  EXPECT_EQ(size_t{ 1 }, findNoTrimIndex(L" 12345678901234567890"));
  EXPECT_EQ(size_t{ 21 }, rfindAfterNoTrimIndex(" 12345678901234567890", size_t{ 21 }));
  EXPECT_EQ(size_t{ 21 }, rfindAfterNoTrimIndex(L" 12345678901234567890", size_t{ 21 }));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex("12345678901234567890 "));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex(L"12345678901234567890 "));
  EXPECT_EQ(size_t{ 20 }, rfindAfterNoTrimIndex("12345678901234567890 ", size_t{ 21 }));
  EXPECT_EQ(size_t{ 20 }, rfindAfterNoTrimIndex(L"12345678901234567890 ", size_t{ 21 }));
  EXPECT_EQ(size_t{ 3 }, findNoTrimIndex("   "));
  EXPECT_EQ(size_t{ 3 }, findNoTrimIndex(L"   "));
  EXPECT_EQ(size_t{ 0 }, rfindAfterNoTrimIndex("   ", size_t{ 3 }));
  EXPECT_EQ(size_t{ 0 }, rfindAfterNoTrimIndex(L"   ", size_t{ 3 }));
  EXPECT_EQ(size_t{ 3 }, findNoTrimIndex("   a"));
  EXPECT_EQ(size_t{ 3 }, findNoTrimIndex(L"   a"));
  EXPECT_EQ(size_t{ 4 }, rfindAfterNoTrimIndex("   a", size_t{ 4 }));
  EXPECT_EQ(size_t{ 4 }, rfindAfterNoTrimIndex(L"   a", size_t{ 4 }));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex("a   "));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex(L"a   "));
  EXPECT_EQ(size_t{ 1 }, rfindAfterNoTrimIndex("a   ", size_t{ 4 }));
  EXPECT_EQ(size_t{ 1 }, rfindAfterNoTrimIndex(L"a   ", size_t{ 4 }));
  EXPECT_EQ(size_t{ 3 }, findNoTrimIndex("   12345678901234567890"));
  EXPECT_EQ(size_t{ 3 }, findNoTrimIndex(L"   12345678901234567890"));
  EXPECT_EQ(size_t{ 23 }, rfindAfterNoTrimIndex("   12345678901234567890", size_t{ 23 }));
  EXPECT_EQ(size_t{ 23 }, rfindAfterNoTrimIndex(L"   12345678901234567890", size_t{ 23 }));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex("12345678901234567890   "));
  EXPECT_EQ(size_t{ 0 }, findNoTrimIndex(L"12345678901234567890   "));
  EXPECT_EQ(size_t{ 20 }, rfindAfterNoTrimIndex("12345678901234567890   ", size_t{ 23 }));
  EXPECT_EQ(size_t{ 20 }, rfindAfterNoTrimIndex(L"12345678901234567890   ", size_t{ 23 }));
}


// -- string initialization --

TEST_F(StringsTest, strAssign) {
  char buffer[21]{ 0 };
  wchar_t wbuffer[21]{ 0 };
  stringAssign(buffer, size_t{ 0 }, 'a');
  stringAssign(wbuffer, size_t{ 0 }, L'a');
  EXPECT_STREQ(buffer, "");
  EXPECT_STREQ(wbuffer, L"");
  stringAssign(buffer, size_t{ 1 }, 'b');
  stringAssign(wbuffer, size_t{ 1 }, L'b');
  EXPECT_STREQ(buffer, "b");
  EXPECT_STREQ(wbuffer, L"b");
  stringAssign(buffer, size_t{ 2 }, 'c');
  stringAssign(wbuffer, size_t{ 2 }, L'c');
  EXPECT_STREQ(buffer, "cc");
  EXPECT_STREQ(wbuffer, L"cc");
  stringAssign(buffer, size_t{ 20 }, 'd');
  stringAssign(wbuffer, size_t{ 20 }, L'd');
  EXPECT_STREQ(buffer, "dddddddddddddddddddd");
  EXPECT_STREQ(wbuffer, L"dddddddddddddddddddd");

  char buffer2[3]{ 'z', 'z', 'z' };
  wchar_t wbuffer2[3]{ L'z', L'z', L'z' };
  stringAssign<char,false>(buffer2, size_t{ 2 }, 'c');
  stringAssign<wchar_t,false>(wbuffer2, size_t{ 2 }, L'c');
  EXPECT_EQ('c', buffer2[0]);
  EXPECT_EQ('c', buffer2[1]);
  EXPECT_EQ('z', buffer2[2]);
  EXPECT_EQ(L'c', wbuffer2[0]);
  EXPECT_EQ(L'c', wbuffer2[1]);
  EXPECT_EQ(L'z', wbuffer2[2]);
}


// -- padding --

TEST_F(StringsTest, strPad) {
  EXPECT_EQ(std::string(""), pad("", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("          "), pad("", size_t{ 10 }, ' '));
  EXPECT_EQ(std::string("a"), pad("a", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("a"), pad("a", size_t{ 1 }, ' '));
  EXPECT_EQ(std::string("a "), pad("a", size_t{ 2 }, ' '));
  EXPECT_EQ(std::string(" a "), pad("a", size_t{ 3 }, ' '));
  EXPECT_EQ(std::string("    a     "), pad("a", size_t{ 10 }, ' '));
  EXPECT_EQ(std::string("     a     "), pad("a", size_t{ 11 }, ' '));
  EXPECT_EQ(std::string("azerty"), pad("azerty", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("azerty"), pad("azerty", size_t{ 6 }, ' '));
  EXPECT_EQ(std::string("azerty "), pad("azerty", size_t{ 7 }, ' '));
  EXPECT_EQ(std::string(" azerty "), pad("azerty", size_t{ 8 }, ' '));
  EXPECT_EQ(std::string("    azerty     "), pad("azerty", size_t{ 15 }, ' '));
  EXPECT_EQ(std::string("     azerty     "), pad("azerty", size_t{ 16 }, ' '));
  EXPECT_EQ(std::string("     azerty     "), pad(" azerty", size_t{ 16 }, ' '));
  EXPECT_EQ(std::string("      azerty     "), pad(" azerty", size_t{ 17 }, ' '));
  EXPECT_EQ(std::wstring(L""), pad(L"", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"          "), pad(L"", size_t{ 10 }, L' '));
  EXPECT_EQ(std::wstring(L"a"), pad(L"a", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"a"), pad(L"a", size_t{ 1 }, L' '));
  EXPECT_EQ(std::wstring(L"a "), pad(L"a", size_t{ 2 }, L' '));
  EXPECT_EQ(std::wstring(L" a "), pad(L"a", size_t{ 3 }, L' '));
  EXPECT_EQ(std::wstring(L"    a     "), pad(L"a", size_t{ 10 }, L' '));
  EXPECT_EQ(std::wstring(L"     a     "), pad(L"a", size_t{ 11 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty"), pad(L"azerty", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty"), pad(L"azerty", size_t{ 6 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty "), pad(L"azerty", size_t{ 7 }, L' '));
  EXPECT_EQ(std::wstring(L" azerty "), pad(L"azerty", size_t{ 8 }, L' '));
  EXPECT_EQ(std::wstring(L"    azerty     "), pad(L"azerty", size_t{ 15 }, L' '));
  EXPECT_EQ(std::wstring(L"     azerty     "), pad(L"azerty", size_t{ 16 }, L' '));
  EXPECT_EQ(std::wstring(L"     azerty     "), pad(L" azerty", size_t{ 16 }, L' '));
  EXPECT_EQ(std::wstring(L"      azerty     "), pad(L" azerty", size_t{ 17 }, L' '));
}

TEST_F(StringsTest, strPadBuffer) {
  char buffer[21]{ 0 };
  wchar_t wbuffer[21]{ 0 };
  pad(buffer, "", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "");
  pad(buffer, "", size_t{ 10 }, ' ');
  EXPECT_STREQ(buffer, "          ");
  pad(buffer, "a", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "a");
  pad(buffer, "a", size_t{ 1 }, ' ');
  EXPECT_STREQ(buffer, "a");
  pad(buffer, "a", size_t{ 2 }, ' ');
  EXPECT_STREQ(buffer, "a ");
  pad(buffer, "a", size_t{ 3 }, ' ');
  EXPECT_STREQ(buffer, " a ");
  pad(buffer, "a", size_t{ 10 }, ' ');
  EXPECT_STREQ(buffer, "    a     ");
  pad(buffer, "a", size_t{ 11 }, ' ');
  EXPECT_STREQ(buffer, "     a     ");
  pad(buffer, "azerty", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "azerty");
  pad(buffer, "azerty", size_t{ 6 }, ' ');
  EXPECT_STREQ(buffer, "azerty");
  pad(buffer, "azerty", size_t{ 7 }, ' ');
  EXPECT_STREQ(buffer, "azerty ");
  pad(buffer, "azerty", size_t{ 8 }, ' ');
  EXPECT_STREQ(buffer, " azerty ");
  pad(buffer, "azerty", size_t{ 15 }, ' ');
  EXPECT_STREQ(buffer, "    azerty     ");
  pad(buffer, "azerty", size_t{ 16 }, ' ');
  EXPECT_STREQ(buffer, "     azerty     ");
  pad(buffer, " azerty", size_t{ 16 }, ' ');
  EXPECT_STREQ(buffer, "     azerty     ");
  pad(buffer, " azerty", size_t{ 17 }, ' ');
  EXPECT_STREQ(buffer, "      azerty     ");
  pad(wbuffer, L"", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"");
  pad(wbuffer, L"", size_t{ 10 }, L' ');
  EXPECT_STREQ(wbuffer, L"          ");
  pad(wbuffer, L"a", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"a");
  pad(wbuffer, L"a", size_t{ 1 }, L' ');
  EXPECT_STREQ(wbuffer, L"a");
  pad(wbuffer, L"a", size_t{ 2 }, L' ');
  EXPECT_STREQ(wbuffer, L"a ");
  pad(wbuffer, L"a", size_t{ 3 }, L' ');
  EXPECT_STREQ(wbuffer, L" a ");
  pad(wbuffer, L"a", size_t{ 10 }, L' ');
  EXPECT_STREQ(wbuffer, L"    a     ");
  pad(wbuffer, L"a", size_t{ 11 }, L' ');
  EXPECT_STREQ(wbuffer, L"     a     ");
  pad(wbuffer, L"azerty", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty");
  pad(wbuffer, L"azerty", size_t{ 6 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty");
  pad(wbuffer, L"azerty", size_t{ 7 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty ");
  pad(wbuffer, L"azerty", size_t{ 8 }, L' ');
  EXPECT_STREQ(wbuffer, L" azerty ");
  pad(wbuffer, L"azerty", size_t{ 15 }, L' ');
  EXPECT_STREQ(wbuffer, L"    azerty     ");
  pad(wbuffer, L"azerty", size_t{ 16 }, L' ');
  EXPECT_STREQ(wbuffer, L"     azerty     ");
  pad(wbuffer, L" azerty", size_t{ 16 }, L' ');
  EXPECT_STREQ(wbuffer, L"     azerty     ");
  pad(wbuffer, L" azerty", size_t{ 17 }, L' ');
  EXPECT_STREQ(wbuffer, L"      azerty     ");
}

template <typename _CharType>
std::basic_string<_CharType> _strSelfPad(const _CharType* value, size_t valueLength, size_t padLength) {
  _CharType buffer[20]{ 0 };
  memcpy((void*)buffer, (void*)value, (valueLength + 1u)*sizeof(_CharType));
  padSelf(buffer, padLength, static_cast<_CharType>(' '));
  return std::basic_string<_CharType>(buffer);
}
TEST_F(StringsTest, strPadSelf) {
  EXPECT_EQ(std::string(""), _strSelfPad("", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(std::string("          "), _strSelfPad("", size_t{ 0 }, size_t{ 10 }));
  EXPECT_EQ(std::string("a"), _strSelfPad("a", size_t{ 1 }, size_t{ 0 }));
  EXPECT_EQ(std::string("a"), _strSelfPad("a", size_t{ 1 }, size_t{ 1 }));
  EXPECT_EQ(std::string("a "), _strSelfPad("a", size_t{ 1 }, size_t{ 2 }));
  EXPECT_EQ(std::string(" a "), _strSelfPad("a", size_t{ 1 }, size_t{ 3 }));
  EXPECT_EQ(std::string("    a     "), _strSelfPad("a", size_t{ 1 }, size_t{ 10 }));
  EXPECT_EQ(std::string("     a     "), _strSelfPad("a", size_t{ 1 }, size_t{ 11 }));
  EXPECT_EQ(std::string("azerty"), _strSelfPad("azerty", size_t{ 6 }, size_t{ 0 }));
  EXPECT_EQ(std::string("azerty"), _strSelfPad("azerty", size_t{ 6 }, size_t{ 6 }));
  EXPECT_EQ(std::string("azerty "), _strSelfPad("azerty", size_t{ 6 }, size_t{ 7 }));
  EXPECT_EQ(std::string(" azerty "), _strSelfPad("azerty", size_t{ 6 }, size_t{ 8 }));
  EXPECT_EQ(std::string("    azerty     "), _strSelfPad("azerty", size_t{ 6 }, size_t{ 15 }));
  EXPECT_EQ(std::string("     azerty     "), _strSelfPad("azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::string("     azerty     "), _strSelfPad(" azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::string("      azerty     "), _strSelfPad(" azerty", size_t{ 6 }, size_t{ 17 }));
  EXPECT_EQ(std::wstring(L""), _strSelfPad(L"", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"          "), _strSelfPad(L"", size_t{ 0 }, size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfPad(L"a", size_t{ 1 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfPad(L"a", size_t{ 1 }, size_t{ 1 }));
  EXPECT_EQ(std::wstring(L"a "), _strSelfPad(L"a", size_t{ 1 }, size_t{ 2 }));
  EXPECT_EQ(std::wstring(L" a "), _strSelfPad(L"a", size_t{ 1 }, size_t{ 3 }));
  EXPECT_EQ(std::wstring(L"    a     "), _strSelfPad(L"a", size_t{ 1 }, size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"     a     "), _strSelfPad(L"a", size_t{ 1 }, size_t{ 11 }));
  EXPECT_EQ(std::wstring(L"azerty"), _strSelfPad(L"azerty", size_t{ 6 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"azerty"), _strSelfPad(L"azerty", size_t{ 6 }, size_t{ 6 }));
  EXPECT_EQ(std::wstring(L"azerty "), _strSelfPad(L"azerty", size_t{ 6 }, size_t{ 7 }));
  EXPECT_EQ(std::wstring(L" azerty "), _strSelfPad(L"azerty", size_t{ 6 }, size_t{ 8 }));
  EXPECT_EQ(std::wstring(L"    azerty     "), _strSelfPad(L"azerty", size_t{ 6 }, size_t{ 15 }));
  EXPECT_EQ(std::wstring(L"     azerty     "), _strSelfPad(L"azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::wstring(L"     azerty     "), _strSelfPad(L" azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::wstring(L"      azerty     "), _strSelfPad(L" azerty", size_t{ 6 }, size_t{ 17 }));
}

TEST_F(StringsTest, strLPad) {
  EXPECT_EQ(std::string(""), lpad("", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("          "), lpad("", size_t{ 10 }, ' '));
  EXPECT_EQ(std::string("a"), lpad("a", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("a"), lpad("a", size_t{ 1 }, ' '));
  EXPECT_EQ(std::string(" a"), lpad("a", size_t{ 2 }, ' '));
  EXPECT_EQ(std::string("  a"), lpad("a", size_t{ 3 }, ' '));
  EXPECT_EQ(std::string("         a"), lpad("a", size_t{ 10 }, ' '));
  EXPECT_EQ(std::string("          a"), lpad("a", size_t{ 11 }, ' '));
  EXPECT_EQ(std::string("azerty"), lpad("azerty", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("azerty"), lpad("azerty", size_t{ 6 }, ' '));
  EXPECT_EQ(std::string(" azerty"), lpad("azerty", size_t{ 7 }, ' '));
  EXPECT_EQ(std::string("  azerty"), lpad("azerty", size_t{ 8 }, ' '));
  EXPECT_EQ(std::string("         azerty"), lpad("azerty", size_t{ 15 }, ' '));
  EXPECT_EQ(std::string("          azerty"), lpad("azerty", size_t{ 16 }, ' '));
  EXPECT_EQ(std::string("          azerty"), lpad(" azerty", size_t{ 16 }, ' '));
  EXPECT_EQ(std::string("           azerty"), lpad(" azerty", size_t{ 17 }, ' '));
  EXPECT_EQ(std::wstring(L""), lpad(L"", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"          "), lpad(L"", size_t{ 10 }, L' '));
  EXPECT_EQ(std::wstring(L"a"), lpad(L"a", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"a"), lpad(L"a", size_t{ 1 }, L' '));
  EXPECT_EQ(std::wstring(L" a"), lpad(L"a", size_t{ 2 }, L' '));
  EXPECT_EQ(std::wstring(L"  a"), lpad(L"a", size_t{ 3 }, L' '));
  EXPECT_EQ(std::wstring(L"         a"), lpad(L"a", size_t{ 10 }, L' '));
  EXPECT_EQ(std::wstring(L"          a"), lpad(L"a", size_t{ 11 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty"), lpad(L"azerty", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty"), lpad(L"azerty", size_t{ 6 }, L' '));
  EXPECT_EQ(std::wstring(L" azerty"), lpad(L"azerty", size_t{ 7 }, L' '));
  EXPECT_EQ(std::wstring(L"  azerty"), lpad(L"azerty", size_t{ 8 }, L' '));
  EXPECT_EQ(std::wstring(L"         azerty"), lpad(L"azerty", size_t{ 15 }, L' '));
  EXPECT_EQ(std::wstring(L"          azerty"), lpad(L"azerty", size_t{ 16 }, L' '));
  EXPECT_EQ(std::wstring(L"          azerty"), lpad(L" azerty", size_t{ 16 }, L' '));
  EXPECT_EQ(std::wstring(L"           azerty"), lpad(L" azerty", size_t{ 17 }, L' '));
}

TEST_F(StringsTest, strLPadBuffer) {
  char buffer[21]{ 0 };
  wchar_t wbuffer[21]{ 0 };
  lpad(buffer, "", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "");
  lpad(buffer, "", size_t{ 10 }, ' ');
  EXPECT_STREQ(buffer, "          ");
  lpad(buffer, "a", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "a");
  lpad(buffer, "a", size_t{ 1 }, ' ');
  EXPECT_STREQ(buffer, "a");
  lpad(buffer, "a", size_t{ 2 }, ' ');
  EXPECT_STREQ(buffer, " a");
  lpad(buffer, "a", size_t{ 3 }, ' ');
  EXPECT_STREQ(buffer, "  a");
  lpad(buffer, "a", size_t{ 10 }, ' ');
  EXPECT_STREQ(buffer, "         a");
  lpad(buffer, "a", size_t{ 11 }, ' ');
  EXPECT_STREQ(buffer, "          a");
  lpad(buffer, "azerty", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "azerty");
  lpad(buffer, "azerty", size_t{ 6 }, ' ');
  EXPECT_STREQ(buffer, "azerty");
  lpad(buffer, "azerty", size_t{ 7 }, ' ');
  EXPECT_STREQ(buffer, " azerty");
  lpad(buffer, "azerty", size_t{ 8 }, ' ');
  EXPECT_STREQ(buffer, "  azerty");
  lpad(buffer, "azerty", size_t{ 15 }, ' ');
  EXPECT_STREQ(buffer, "         azerty");
  lpad(buffer, "azerty", size_t{ 16 }, ' ');
  EXPECT_STREQ(buffer, "          azerty");
  lpad(buffer, " azerty", size_t{ 16 }, ' ');
  EXPECT_STREQ(buffer, "          azerty");
  lpad(buffer, " azerty", size_t{ 17 }, ' ');
  EXPECT_STREQ(buffer, "           azerty");
  lpad(wbuffer, L"", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"");
  lpad(wbuffer, L"", size_t{ 10 }, L' ');
  EXPECT_STREQ(wbuffer, L"          ");
  lpad(wbuffer, L"a", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"a");
  lpad(wbuffer, L"a", size_t{ 1 }, L' ');
  EXPECT_STREQ(wbuffer, L"a");
  lpad(wbuffer, L"a", size_t{ 2 }, L' ');
  EXPECT_STREQ(wbuffer, L" a");
  lpad(wbuffer, L"a", size_t{ 3 }, L' ');
  EXPECT_STREQ(wbuffer, L"  a");
  lpad(wbuffer, L"a", size_t{ 10 }, L' ');
  EXPECT_STREQ(wbuffer, L"         a");
  lpad(wbuffer, L"a", size_t{ 11 }, L' ');
  EXPECT_STREQ(wbuffer, L"          a");
  lpad(wbuffer, L"azerty", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty");
  lpad(wbuffer, L"azerty", size_t{ 6 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty");
  lpad(wbuffer, L"azerty", size_t{ 7 }, L' ');
  EXPECT_STREQ(wbuffer, L" azerty");
  lpad(wbuffer, L"azerty", size_t{ 8 }, L' ');
  EXPECT_STREQ(wbuffer, L"  azerty");
  lpad(wbuffer, L"azerty", size_t{ 15 }, L' ');
  EXPECT_STREQ(wbuffer, L"         azerty");
  lpad(wbuffer, L"azerty", size_t{ 16 }, L' ');
  EXPECT_STREQ(wbuffer, L"          azerty");
  lpad(wbuffer, L" azerty", size_t{ 16 }, L' ');
  EXPECT_STREQ(wbuffer, L"          azerty");
  lpad(wbuffer, L" azerty", size_t{ 17 }, L' ');
  EXPECT_STREQ(wbuffer, L"           azerty");
}

template <typename _CharType>
std::basic_string<_CharType> _strSelfLPad(const _CharType* value, size_t valueLength, size_t padLength) {
  _CharType buffer[20]{ 0 };
  memcpy((void*)buffer, (void*)value, (valueLength + 1u) * sizeof(_CharType));
  lpadSelf(buffer, padLength, static_cast<_CharType>(' '));
  return std::basic_string<_CharType>(buffer);
}
TEST_F(StringsTest, strLPadSelf) {
  EXPECT_EQ(std::string(""), _strSelfLPad("", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(std::string("          "), _strSelfLPad("", size_t{ 0 }, size_t{ 10 }));
  EXPECT_EQ(std::string("a"), _strSelfLPad("a", size_t{ 1 }, size_t{ 0 }));
  EXPECT_EQ(std::string("a"), _strSelfLPad("a", size_t{ 1 }, size_t{ 1 }));
  EXPECT_EQ(std::string(" a"), _strSelfLPad("a", size_t{ 1 }, size_t{ 2 }));
  EXPECT_EQ(std::string("  a"), _strSelfLPad("a", size_t{ 1 }, size_t{ 3 }));
  EXPECT_EQ(std::string("         a"), _strSelfLPad("a", size_t{ 1 }, size_t{ 10 }));
  EXPECT_EQ(std::string("          a"), _strSelfLPad("a", size_t{ 1 }, size_t{ 11 }));
  EXPECT_EQ(std::string("azerty"), _strSelfLPad("azerty", size_t{ 6 }, size_t{ 0 }));
  EXPECT_EQ(std::string("azerty"), _strSelfLPad("azerty", size_t{ 6 }, size_t{ 6 }));
  EXPECT_EQ(std::string(" azerty"), _strSelfLPad("azerty", size_t{ 6 }, size_t{ 7 }));
  EXPECT_EQ(std::string("  azerty"), _strSelfLPad("azerty", size_t{ 6 }, size_t{ 8 }));
  EXPECT_EQ(std::string("         azerty"), _strSelfLPad("azerty", size_t{ 6 }, size_t{ 15 }));
  EXPECT_EQ(std::string("          azerty"), _strSelfLPad("azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::string("          azerty"), _strSelfLPad(" azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::string("           azerty"), _strSelfLPad(" azerty", size_t{ 6 }, size_t{ 17 }));
  EXPECT_EQ(std::wstring(L""), _strSelfLPad(L"", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"          "), _strSelfLPad(L"", size_t{ 0 }, size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfLPad(L"a", size_t{ 1 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfLPad(L"a", size_t{ 1 }, size_t{ 1 }));
  EXPECT_EQ(std::wstring(L" a"), _strSelfLPad(L"a", size_t{ 1 }, size_t{ 2 }));
  EXPECT_EQ(std::wstring(L"  a"), _strSelfLPad(L"a", size_t{ 1 }, size_t{ 3 }));
  EXPECT_EQ(std::wstring(L"         a"), _strSelfLPad(L"a", size_t{ 1 }, size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"          a"), _strSelfLPad(L"a", size_t{ 1 }, size_t{ 11 }));
  EXPECT_EQ(std::wstring(L"azerty"), _strSelfLPad(L"azerty", size_t{ 6 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"azerty"), _strSelfLPad(L"azerty", size_t{ 6 }, size_t{ 6 }));
  EXPECT_EQ(std::wstring(L" azerty"), _strSelfLPad(L"azerty", size_t{ 6 }, size_t{ 7 }));
  EXPECT_EQ(std::wstring(L"  azerty"), _strSelfLPad(L"azerty", size_t{ 6 }, size_t{ 8 }));
  EXPECT_EQ(std::wstring(L"         azerty"), _strSelfLPad(L"azerty", size_t{ 6 }, size_t{ 15 }));
  EXPECT_EQ(std::wstring(L"          azerty"), _strSelfLPad(L"azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::wstring(L"          azerty"), _strSelfLPad(L" azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::wstring(L"           azerty"), _strSelfLPad(L" azerty", size_t{ 6 }, size_t{ 17 }));
}

TEST_F(StringsTest, strRPad) {
  EXPECT_EQ(std::string(""), rpad("", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("          "), rpad("", size_t{ 10 }, ' '));
  EXPECT_EQ(std::string("a"), rpad("a", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("a"), rpad("a", size_t{ 1 }, ' '));
  EXPECT_EQ(std::string("a "), rpad("a", size_t{ 2 }, ' '));
  EXPECT_EQ(std::string("a  "), rpad("a", size_t{ 3 }, ' '));
  EXPECT_EQ(std::string("a         "), rpad("a", size_t{ 10 }, ' '));
  EXPECT_EQ(std::string("a          "), rpad("a", size_t{ 11 }, ' '));
  EXPECT_EQ(std::string("azerty"), rpad("azerty", size_t{ 0 }, ' '));
  EXPECT_EQ(std::string("azerty"), rpad("azerty", size_t{ 6 }, ' '));
  EXPECT_EQ(std::string("azerty "), rpad("azerty", size_t{ 7 }, ' '));
  EXPECT_EQ(std::string("azerty  "), rpad("azerty", size_t{ 8 }, ' '));
  EXPECT_EQ(std::string("azerty         "), rpad("azerty", size_t{ 15 }, ' '));
  EXPECT_EQ(std::string("azerty          "), rpad("azerty", size_t{ 16 }, ' '));
  EXPECT_EQ(std::string(" azerty         "), rpad(" azerty", size_t{ 16 }, ' '));
  EXPECT_EQ(std::string(" azerty          "), rpad(" azerty", size_t{ 17 }, ' '));
  EXPECT_EQ(std::wstring(L""), rpad(L"", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"          "), rpad(L"", size_t{ 10 }, L' '));
  EXPECT_EQ(std::wstring(L"a"), rpad(L"a", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"a"), rpad(L"a", size_t{ 1 }, L' '));
  EXPECT_EQ(std::wstring(L"a "), rpad(L"a", size_t{ 2 }, L' '));
  EXPECT_EQ(std::wstring(L"a  "), rpad(L"a", size_t{ 3 }, L' '));
  EXPECT_EQ(std::wstring(L"a         "), rpad(L"a", size_t{ 10 }, L' '));
  EXPECT_EQ(std::wstring(L"a          "), rpad(L"a", size_t{ 11 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty"), rpad(L"azerty", size_t{ 0 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty"), rpad(L"azerty", size_t{ 6 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty "), rpad(L"azerty", size_t{ 7 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty  "), rpad(L"azerty", size_t{ 8 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty         "), rpad(L"azerty", size_t{ 15 }, L' '));
  EXPECT_EQ(std::wstring(L"azerty          "), rpad(L"azerty", size_t{ 16 }, L' '));
  EXPECT_EQ(std::wstring(L" azerty         "), rpad(L" azerty", size_t{ 16 }, L' '));
  EXPECT_EQ(std::wstring(L" azerty          "), rpad(L" azerty", size_t{ 17 }, L' '));
}

TEST_F(StringsTest, strRPadBuffer) {
  char buffer[21]{ 0 };
  wchar_t wbuffer[21]{ 0 };
  rpad(buffer, "", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "");
  rpad(buffer, "", size_t{ 10 }, ' ');
  EXPECT_STREQ(buffer, "          ");
  rpad(buffer, "a", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "a");
  rpad(buffer, "a", size_t{ 1 }, ' ');
  EXPECT_STREQ(buffer, "a");
  rpad(buffer, "a", size_t{ 2 }, ' ');
  EXPECT_STREQ(buffer, "a ");
  rpad(buffer, "a", size_t{ 3 }, ' ');
  EXPECT_STREQ(buffer, "a  ");
  rpad(buffer, "a", size_t{ 10 }, ' ');
  EXPECT_STREQ(buffer, "a         ");
  rpad(buffer, "a", size_t{ 11 }, ' ');
  EXPECT_STREQ(buffer, "a          ");
  rpad(buffer, "azerty", size_t{ 0 }, ' ');
  EXPECT_STREQ(buffer, "azerty");
  rpad(buffer, "azerty", size_t{ 6 }, ' ');
  EXPECT_STREQ(buffer, "azerty");
  rpad(buffer, "azerty", size_t{ 7 }, ' ');
  EXPECT_STREQ(buffer, "azerty ");
  rpad(buffer, "azerty", size_t{ 8 }, ' ');
  EXPECT_STREQ(buffer, "azerty  ");
  rpad(buffer, "azerty", size_t{ 15 }, ' ');
  EXPECT_STREQ(buffer, "azerty         ");
  rpad(buffer, "azerty", size_t{ 16 }, ' ');
  EXPECT_STREQ(buffer, "azerty          ");
  rpad(buffer, " azerty", size_t{ 16 }, ' ');
  EXPECT_STREQ(buffer, " azerty         ");
  rpad(buffer, " azerty", size_t{ 17 }, ' ');
  EXPECT_STREQ(buffer, " azerty          ");
  rpad(wbuffer, L"", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"");
  rpad(wbuffer, L"", size_t{ 10 }, L' ');
  EXPECT_STREQ(wbuffer, L"          ");
  rpad(wbuffer, L"a", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"a");
  rpad(wbuffer, L"a", size_t{ 1 }, L' ');
  EXPECT_STREQ(wbuffer, L"a");
  rpad(wbuffer, L"a", size_t{ 2 }, L' ');
  EXPECT_STREQ(wbuffer, L"a ");
  rpad(wbuffer, L"a", size_t{ 3 }, L' ');
  EXPECT_STREQ(wbuffer, L"a  ");
  rpad(wbuffer, L"a", size_t{ 10 }, L' ');
  EXPECT_STREQ(wbuffer, L"a         ");
  rpad(wbuffer, L"a", size_t{ 11 }, L' ');
  EXPECT_STREQ(wbuffer, L"a          ");
  rpad(wbuffer, L"azerty", size_t{ 0 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty");
  rpad(wbuffer, L"azerty", size_t{ 6 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty");
  rpad(wbuffer, L"azerty", size_t{ 7 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty ");
  rpad(wbuffer, L"azerty", size_t{ 8 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty  ");
  rpad(wbuffer, L"azerty", size_t{ 15 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty         ");
  rpad(wbuffer, L"azerty", size_t{ 16 }, L' ');
  EXPECT_STREQ(wbuffer, L"azerty          ");
  rpad(wbuffer, L" azerty", size_t{ 16 }, L' ');
  EXPECT_STREQ(wbuffer, L" azerty         ");
  rpad(wbuffer, L" azerty", size_t{ 17 }, L' ');
  EXPECT_STREQ(wbuffer, L" azerty          ");
}

template <typename _CharType>
std::basic_string<_CharType> _strSelfRPad(const _CharType* value, size_t valueLength, size_t padLength) {
  _CharType buffer[20]{ 0 };
  memcpy((void*)buffer, (void*)value, (valueLength + 1u) * sizeof(_CharType));
  rpadSelf(buffer, padLength, static_cast<_CharType>(' '));
  return std::basic_string<_CharType>(buffer);
}
TEST_F(StringsTest, strRPadSelf) {
  EXPECT_EQ(std::string(""), _strSelfRPad("", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(std::string("          "), _strSelfRPad("", size_t{ 0 }, size_t{ 10 }));
  EXPECT_EQ(std::string("a"), _strSelfRPad("a", size_t{ 1 }, size_t{ 0 }));
  EXPECT_EQ(std::string("a"), _strSelfRPad("a", size_t{ 1 }, size_t{ 1 }));
  EXPECT_EQ(std::string("a "), _strSelfRPad("a", size_t{ 1 }, size_t{ 2 }));
  EXPECT_EQ(std::string("a  "), _strSelfRPad("a", size_t{ 1 }, size_t{ 3 }));
  EXPECT_EQ(std::string("a         "), _strSelfRPad("a", size_t{ 1 }, size_t{ 10 }));
  EXPECT_EQ(std::string("a          "), _strSelfRPad("a", size_t{ 1 }, size_t{ 11 }));
  EXPECT_EQ(std::string("azerty"), _strSelfRPad("azerty", size_t{ 6 }, size_t{ 0 }));
  EXPECT_EQ(std::string("azerty"), _strSelfRPad("azerty", size_t{ 6 }, size_t{ 6 }));
  EXPECT_EQ(std::string("azerty "), _strSelfRPad("azerty", size_t{ 6 }, size_t{ 7 }));
  EXPECT_EQ(std::string("azerty  "), _strSelfRPad("azerty", size_t{ 6 }, size_t{ 8 }));
  EXPECT_EQ(std::string("azerty         "), _strSelfRPad("azerty", size_t{ 6 }, size_t{ 15 }));
  EXPECT_EQ(std::string("azerty          "), _strSelfRPad("azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::string(" azerty         "), _strSelfRPad(" azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::string(" azerty          "), _strSelfRPad(" azerty", size_t{ 6 }, size_t{ 17 }));
  EXPECT_EQ(std::wstring(L""), _strSelfRPad(L"", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"          "), _strSelfRPad(L"", size_t{ 0 }, size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfRPad(L"a", size_t{ 1 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfRPad(L"a", size_t{ 1 }, size_t{ 1 }));
  EXPECT_EQ(std::wstring(L"a "), _strSelfRPad(L"a", size_t{ 1 }, size_t{ 2 }));
  EXPECT_EQ(std::wstring(L"a  "), _strSelfRPad(L"a", size_t{ 1 }, size_t{ 3 }));
  EXPECT_EQ(std::wstring(L"a         "), _strSelfRPad(L"a", size_t{ 1 }, size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"a          "), _strSelfRPad(L"a", size_t{ 1 }, size_t{ 11 }));
  EXPECT_EQ(std::wstring(L"azerty"), _strSelfRPad(L"azerty", size_t{ 6 }, size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"azerty"), _strSelfRPad(L"azerty", size_t{ 6 }, size_t{ 6 }));
  EXPECT_EQ(std::wstring(L"azerty "), _strSelfRPad(L"azerty", size_t{ 6 }, size_t{ 7 }));
  EXPECT_EQ(std::wstring(L"azerty  "), _strSelfRPad(L"azerty", size_t{ 6 }, size_t{ 8 }));
  EXPECT_EQ(std::wstring(L"azerty         "), _strSelfRPad(L"azerty", size_t{ 6 }, size_t{ 15 }));
  EXPECT_EQ(std::wstring(L"azerty          "), _strSelfRPad(L"azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::wstring(L" azerty         "), _strSelfRPad(L" azerty", size_t{ 6 }, size_t{ 16 }));
  EXPECT_EQ(std::wstring(L" azerty          "), _strSelfRPad(L" azerty", size_t{ 6 }, size_t{ 17 }));
}


// -- trim --

TEST_F(StringsTest, strTrim) {
  EXPECT_EQ(std::string(""), trim(""));
  EXPECT_EQ(std::string(""), trim(" "));
  EXPECT_EQ(std::string(""), trim("          "));
  EXPECT_EQ(std::string("a"), trim("a"));
  EXPECT_EQ(std::string("a"), trim("a "));
  EXPECT_EQ(std::string("a"), trim(" a"));
  EXPECT_EQ(std::string("a"), trim(" a "));
  EXPECT_EQ(std::string("a"), trim("    a     "));
  EXPECT_EQ(std::string("a"), trim("     a     "));
  EXPECT_EQ(std::string("aze ty"), trim("aze ty"));
  EXPECT_EQ(std::string("aze ty"), trim("aze ty "));
  EXPECT_EQ(std::string("azerty"), trim(" azerty"));
  EXPECT_EQ(std::string("aze ty"), trim(" aze ty "));
  EXPECT_EQ(std::string("aze ty"), trim("     aze ty     "));
  EXPECT_EQ(std::string("aze ty"), trim("\t aze ty\t "));
  EXPECT_EQ(std::wstring(L""), trim(L""));
  EXPECT_EQ(std::wstring(L""), trim(L" "));
  EXPECT_EQ(std::wstring(L""), trim(L"          "));
  EXPECT_EQ(std::wstring(L"a"), trim(L"a"));
  EXPECT_EQ(std::wstring(L"a"), trim(L"a "));
  EXPECT_EQ(std::wstring(L"a"), trim(L" a"));
  EXPECT_EQ(std::wstring(L"a"), trim(L" a "));
  EXPECT_EQ(std::wstring(L"a"), trim(L"    a     "));
  EXPECT_EQ(std::wstring(L"a"), trim(L"     a     "));
  EXPECT_EQ(std::wstring(L"aze ty"), trim(L"aze ty"));
  EXPECT_EQ(std::wstring(L"aze ty"), trim(L"aze ty "));
  EXPECT_EQ(std::wstring(L"azerty"), trim(L" azerty"));
  EXPECT_EQ(std::wstring(L"aze ty"), trim(L" aze ty "));
  EXPECT_EQ(std::wstring(L"aze ty"), trim(L"     aze ty     "));
  EXPECT_EQ(std::wstring(L"aze ty"), trim(L"\t aze ty\t "));
}

TEST_F(StringsTest, strTrimBuffer) {
  char buffer[21]{ 0 };
  trim(buffer, "");
  EXPECT_STREQ(buffer, "");
  trim(buffer, " ");
  EXPECT_STREQ(buffer, "");
  trim(buffer, "          ");
  EXPECT_STREQ(buffer, "");
  trim(buffer, "a");
  EXPECT_STREQ(buffer, "a");
  trim(buffer, "a ");
  EXPECT_STREQ(buffer, "a");
  trim(buffer, " a");
  EXPECT_STREQ(buffer, "a");
  trim(buffer, " a ");
  EXPECT_STREQ(buffer, "a");
  trim(buffer, "    a     ");
  EXPECT_STREQ(buffer, "a");
  trim(buffer, "     a     ");
  EXPECT_STREQ(buffer, "a");
  trim(buffer, "aze ty");
  EXPECT_STREQ(buffer, "aze ty");
  trim(buffer, "aze ty ");
  EXPECT_STREQ(buffer, "aze ty");
  trim(buffer, " aze ty");
  EXPECT_STREQ(buffer, "aze ty");
  trim(buffer, " aze ty ");
  EXPECT_STREQ(buffer, "aze ty");
  trim(buffer, "     aze ty     ");
  EXPECT_STREQ(buffer, "aze ty");
  trim(buffer, "\t aze ty\t ");
  EXPECT_STREQ(buffer, "aze ty");
  wchar_t wbuffer[21]{ 0 };
  trim(wbuffer, L"");
  EXPECT_STREQ(wbuffer, L"");
  trim(wbuffer, L" ");
  EXPECT_STREQ(wbuffer, L"");
  trim(wbuffer, L"          ");
  EXPECT_STREQ(wbuffer, L"");
  trim(wbuffer, L"a");
  EXPECT_STREQ(wbuffer, L"a");
  trim(wbuffer, L"a ");
  EXPECT_STREQ(wbuffer, L"a");
  trim(wbuffer, L" a");
  EXPECT_STREQ(wbuffer, L"a");
  trim(wbuffer, L" a ");
  EXPECT_STREQ(wbuffer, L"a");
  trim(wbuffer, L"    a     ");
  EXPECT_STREQ(wbuffer, L"a");
  trim(wbuffer, L"     a     ");
  EXPECT_STREQ(wbuffer, L"a");
  trim(wbuffer, L"aze ty");
  EXPECT_STREQ(wbuffer, L"aze ty");
  trim(wbuffer, L"aze ty ");
  EXPECT_STREQ(wbuffer, L"aze ty");
  trim(wbuffer, L" aze ty");
  EXPECT_STREQ(wbuffer, L"aze ty");
  trim(wbuffer, L" aze ty ");
  EXPECT_STREQ(wbuffer, L"aze ty");
  trim(wbuffer, L"     aze ty     ");
  EXPECT_STREQ(wbuffer, L"aze ty");
  trim(wbuffer, L"\t aze ty\t ");
  EXPECT_STREQ(wbuffer, L"aze ty");
}

template <typename _CharType>
std::basic_string<_CharType> _strSelfTrim(const _CharType* value, size_t valueLength) {
  _CharType buffer[20]{ 0 };
  memcpy((void*)buffer, (void*)value, (valueLength + 1u)*sizeof(_CharType));
  trimSelf(buffer);
  return std::basic_string<_CharType>(buffer);
}
TEST_F(StringsTest, strTrimSelf) {
  EXPECT_EQ(std::string(""), _strSelfTrim("", size_t{ 0 }));
  EXPECT_EQ(std::string(""), _strSelfTrim(" ", size_t{ 1 }));
  EXPECT_EQ(std::string(""), _strSelfTrim("          ", size_t{ 10 }));
  EXPECT_EQ(std::string("a"), _strSelfTrim("a", size_t{ 1 }));
  EXPECT_EQ(std::string("a"), _strSelfTrim("a ", size_t{ 2 }));
  EXPECT_EQ(std::string("a"), _strSelfTrim(" a", size_t{ 2 }));
  EXPECT_EQ(std::string("a"), _strSelfTrim(" a ", size_t{ 3 }));
  EXPECT_EQ(std::string("a"), _strSelfTrim("    a     ", size_t{ 10 }));
  EXPECT_EQ(std::string("a"), _strSelfTrim("     a     ", size_t{ 11 }));
  EXPECT_EQ(std::string("aze ty"), _strSelfTrim("aze ty", size_t{ 6 }));
  EXPECT_EQ(std::string("aze ty"), _strSelfTrim("aze ty ", size_t{ 7 }));
  EXPECT_EQ(std::string("azerty"), _strSelfTrim(" azerty", size_t{ 7 }));
  EXPECT_EQ(std::string("aze ty"), _strSelfTrim(" aze ty ", size_t{ 8 }));
  EXPECT_EQ(std::string("aze ty"), _strSelfTrim("     aze ty     ", size_t{ 16 }));
  EXPECT_EQ(std::string("aze ty"), _strSelfTrim("\t aze ty\t ", size_t{ 10 }));
  EXPECT_EQ(std::wstring(L""), _strSelfTrim(L"", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L""), _strSelfTrim(L" ", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L""), _strSelfTrim(L"          ", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfTrim(L"a", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfTrim(L"a ", size_t{ 2 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfTrim(L" a", size_t{ 2 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfTrim(L" a ", size_t{ 3 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfTrim(L"    a     ", size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfTrim(L"     a     ", size_t{ 11 }));
  EXPECT_EQ(std::wstring(L"aze ty"), _strSelfTrim(L"aze ty", size_t{ 6 }));
  EXPECT_EQ(std::wstring(L"aze ty"), _strSelfTrim(L"aze ty ", size_t{ 7 }));
  EXPECT_EQ(std::wstring(L"azerty"), _strSelfTrim(L" azerty", size_t{ 7 }));
  EXPECT_EQ(std::wstring(L"aze ty"), _strSelfTrim(L" aze ty ", size_t{ 8 }));
  EXPECT_EQ(std::wstring(L"aze ty"), _strSelfTrim(L"     aze ty     ", size_t{ 16 }));
  EXPECT_EQ(std::wstring(L"aze ty"), _strSelfTrim(L"\t aze ty\t ", size_t{ 10 }));
}

TEST_F(StringsTest, strLTrim) {
  EXPECT_EQ(std::string(""), ltrim(""));
  EXPECT_EQ(std::string(""), ltrim(" "));
  EXPECT_EQ(std::string(""), ltrim("          "));
  EXPECT_EQ(std::string("a"), ltrim("a"));
  EXPECT_EQ(std::string("a "), ltrim("a "));
  EXPECT_EQ(std::string("a"), ltrim(" a"));
  EXPECT_EQ(std::string("a "), ltrim(" a "));
  EXPECT_EQ(std::string("a     "), ltrim("    a     "));
  EXPECT_EQ(std::string("a     "), ltrim("     a     "));
  EXPECT_EQ(std::string("aze ty"), ltrim("aze ty"));
  EXPECT_EQ(std::string("aze ty "), ltrim("aze ty "));
  EXPECT_EQ(std::string("azerty"), ltrim(" azerty"));
  EXPECT_EQ(std::string("aze ty "), ltrim(" aze ty "));
  EXPECT_EQ(std::string("aze ty     "), ltrim("     aze ty     "));
  EXPECT_EQ(std::string("aze ty\t "), ltrim("\t aze ty\t "));
  EXPECT_EQ(std::wstring(L""), ltrim(L""));
  EXPECT_EQ(std::wstring(L""), ltrim(L" "));
  EXPECT_EQ(std::wstring(L""), ltrim(L"          "));
  EXPECT_EQ(std::wstring(L"a"), ltrim(L"a"));
  EXPECT_EQ(std::wstring(L"a "), ltrim(L"a "));
  EXPECT_EQ(std::wstring(L"a"), ltrim(L" a"));
  EXPECT_EQ(std::wstring(L"a "), ltrim(L" a "));
  EXPECT_EQ(std::wstring(L"a     "), ltrim(L"    a     "));
  EXPECT_EQ(std::wstring(L"a     "), ltrim(L"     a     "));
  EXPECT_EQ(std::wstring(L"aze ty"), ltrim(L"aze ty"));
  EXPECT_EQ(std::wstring(L"aze ty "), ltrim(L"aze ty "));
  EXPECT_EQ(std::wstring(L"azerty"), ltrim(L" azerty"));
  EXPECT_EQ(std::wstring(L"aze ty "), ltrim(L" aze ty "));
  EXPECT_EQ(std::wstring(L"aze ty     "), ltrim(L"     aze ty     "));
  EXPECT_EQ(std::wstring(L"aze ty\t "), ltrim(L"\t aze ty\t "));
}

TEST_F(StringsTest, strLTrimBuffer) {
  char buffer[21]{ 0 };
  ltrim(buffer, "");
  EXPECT_STREQ(buffer, "");
  ltrim(buffer, " ");
  EXPECT_STREQ(buffer, "");
  ltrim(buffer, "          ");
  EXPECT_STREQ(buffer, "");
  ltrim(buffer, "a");
  EXPECT_STREQ(buffer, "a");
  ltrim(buffer, "a ");
  EXPECT_STREQ(buffer, "a ");
  ltrim(buffer, " a");
  EXPECT_STREQ(buffer, "a");
  ltrim(buffer, " a ");
  EXPECT_STREQ(buffer, "a ");
  ltrim(buffer, "    a     ");
  EXPECT_STREQ(buffer, "a     ");
  ltrim(buffer, "     a     ");
  EXPECT_STREQ(buffer, "a     ");
  ltrim(buffer, "aze ty");
  EXPECT_STREQ(buffer, "aze ty");
  ltrim(buffer, "aze ty ");
  EXPECT_STREQ(buffer, "aze ty ");
  ltrim(buffer, " aze ty");
  EXPECT_STREQ(buffer, "aze ty");
  ltrim(buffer, " aze ty ");
  EXPECT_STREQ(buffer, "aze ty ");
  ltrim(buffer, "     aze ty     ");
  EXPECT_STREQ(buffer, "aze ty     ");
  ltrim(buffer, "\t aze ty\t ");
  EXPECT_STREQ(buffer, "aze ty\t ");
  wchar_t wbuffer[21]{ 0 };
  ltrim(wbuffer, L"");
  EXPECT_STREQ(wbuffer, L"");
  ltrim(wbuffer, L" ");
  EXPECT_STREQ(wbuffer, L"");
  ltrim(wbuffer, L"          ");
  EXPECT_STREQ(wbuffer, L"");
  ltrim(wbuffer, L"a");
  EXPECT_STREQ(wbuffer, L"a");
  ltrim(wbuffer, L"a ");
  EXPECT_STREQ(wbuffer, L"a ");
  ltrim(wbuffer, L" a");
  EXPECT_STREQ(wbuffer, L"a");
  ltrim(wbuffer, L" a ");
  EXPECT_STREQ(wbuffer, L"a ");
  ltrim(wbuffer, L"    a     ");
  EXPECT_STREQ(wbuffer, L"a     ");
  ltrim(wbuffer, L"     a     ");
  EXPECT_STREQ(wbuffer, L"a     ");
  ltrim(wbuffer, L"aze ty");
  EXPECT_STREQ(wbuffer, L"aze ty");
  ltrim(wbuffer, L"aze ty ");
  EXPECT_STREQ(wbuffer, L"aze ty ");
  ltrim(wbuffer, L" aze ty");
  EXPECT_STREQ(wbuffer, L"aze ty");
  ltrim(wbuffer, L" aze ty ");
  EXPECT_STREQ(wbuffer, L"aze ty ");
  ltrim(wbuffer, L"     aze ty     ");
  EXPECT_STREQ(wbuffer, L"aze ty     ");
  ltrim(wbuffer, L"\t aze ty\t ");
  EXPECT_STREQ(wbuffer, L"aze ty\t ");
}

template <typename _CharType>
std::basic_string<_CharType> _strSelfLTrim(const _CharType* value, size_t valueLength) {
  _CharType buffer[20]{ 0 };
  memcpy((void*)buffer, (void*)value, (valueLength + 1u) * sizeof(_CharType));
  ltrimSelf(buffer);
  return std::basic_string<_CharType>(buffer);
}
TEST_F(StringsTest, strLTrimSelf) {
  EXPECT_EQ(std::string(""), _strSelfLTrim("", size_t{ 0 }));
  EXPECT_EQ(std::string(""), _strSelfLTrim(" ", size_t{ 1 }));
  EXPECT_EQ(std::string(""), _strSelfLTrim("          ", size_t{ 10 }));
  EXPECT_EQ(std::string("a"), _strSelfLTrim("a", size_t{ 1 }));
  EXPECT_EQ(std::string("a "), _strSelfLTrim("a ", size_t{ 2 }));
  EXPECT_EQ(std::string("a"), _strSelfLTrim(" a", size_t{ 2 }));
  EXPECT_EQ(std::string("a "), _strSelfLTrim(" a ", size_t{ 3 }));
  EXPECT_EQ(std::string("a     "), _strSelfLTrim("    a     ", size_t{ 10 }));
  EXPECT_EQ(std::string("a     "), _strSelfLTrim("     a     ", size_t{ 11 }));
  EXPECT_EQ(std::string("aze ty"), _strSelfLTrim("aze ty", size_t{ 6 }));
  EXPECT_EQ(std::string("aze ty "), _strSelfLTrim("aze ty ", size_t{ 7 }));
  EXPECT_EQ(std::string("azerty"), _strSelfLTrim(" azerty", size_t{ 7 }));
  EXPECT_EQ(std::string("aze ty "), _strSelfLTrim(" aze ty ", size_t{ 8 }));
  EXPECT_EQ(std::string("aze ty     "), _strSelfLTrim("     aze ty     ", size_t{ 16 }));
  EXPECT_EQ(std::string("aze ty\t "), _strSelfLTrim("\t aze ty\t ", size_t{ 10 }));
  EXPECT_EQ(std::wstring(L""), _strSelfLTrim(L"", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L""), _strSelfLTrim(L" ", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L""), _strSelfLTrim(L"          ", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfLTrim(L"a", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a "), _strSelfLTrim(L"a ", size_t{ 2 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfLTrim(L" a", size_t{ 2 }));
  EXPECT_EQ(std::wstring(L"a "), _strSelfLTrim(L" a ", size_t{ 3 }));
  EXPECT_EQ(std::wstring(L"a     "), _strSelfLTrim(L"    a     ", size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"a     "), _strSelfLTrim(L"     a     ", size_t{ 11 }));
  EXPECT_EQ(std::wstring(L"aze ty"), _strSelfLTrim(L"aze ty", size_t{ 6 }));
  EXPECT_EQ(std::wstring(L"aze ty "), _strSelfLTrim(L"aze ty ", size_t{ 7 }));
  EXPECT_EQ(std::wstring(L"azerty"), _strSelfLTrim(L" azerty", size_t{ 7 }));
  EXPECT_EQ(std::wstring(L"aze ty "), _strSelfLTrim(L" aze ty ", size_t{ 8 }));
  EXPECT_EQ(std::wstring(L"aze ty     "), _strSelfLTrim(L"     aze ty     ", size_t{ 16 }));
  EXPECT_EQ(std::wstring(L"aze ty\t "), _strSelfLTrim(L"\t aze ty\t ", size_t{ 10 }));
}

TEST_F(StringsTest, strRTrim) {
  EXPECT_EQ(std::string(""), rtrim(""));
  EXPECT_EQ(std::string(""), rtrim(" "));
  EXPECT_EQ(std::string(""), rtrim("          "));
  EXPECT_EQ(std::string("a"), rtrim("a"));
  EXPECT_EQ(std::string("a"), rtrim("a "));
  EXPECT_EQ(std::string(" a"), rtrim(" a"));
  EXPECT_EQ(std::string(" a"), rtrim(" a "));
  EXPECT_EQ(std::string("    a"), rtrim("    a     "));
  EXPECT_EQ(std::string("     a"), rtrim("     a     "));
  EXPECT_EQ(std::string("aze ty"), rtrim("aze ty"));
  EXPECT_EQ(std::string("aze ty"), rtrim("aze ty "));
  EXPECT_EQ(std::string(" azerty"), rtrim(" azerty"));
  EXPECT_EQ(std::string(" aze ty"), rtrim(" aze ty "));
  EXPECT_EQ(std::string("     aze ty"), rtrim("     aze ty     "));
  EXPECT_EQ(std::string("\t aze ty"), rtrim("\t aze ty\t "));
  EXPECT_EQ(std::wstring(L""), rtrim(L""));
  EXPECT_EQ(std::wstring(L""), rtrim(L" "));
  EXPECT_EQ(std::wstring(L""), rtrim(L"          "));
  EXPECT_EQ(std::wstring(L"a"), rtrim(L"a"));
  EXPECT_EQ(std::wstring(L"a"), rtrim(L"a "));
  EXPECT_EQ(std::wstring(L" a"), rtrim(L" a"));
  EXPECT_EQ(std::wstring(L" a"), rtrim(L" a "));
  EXPECT_EQ(std::wstring(L"    a"), rtrim(L"    a     "));
  EXPECT_EQ(std::wstring(L"     a"), rtrim(L"     a     "));
  EXPECT_EQ(std::wstring(L"aze ty"), rtrim(L"aze ty"));
  EXPECT_EQ(std::wstring(L"aze ty"), rtrim(L"aze ty "));
  EXPECT_EQ(std::wstring(L" azerty"), rtrim(L" azerty"));
  EXPECT_EQ(std::wstring(L" aze ty"), rtrim(L" aze ty "));
  EXPECT_EQ(std::wstring(L"     aze ty"), rtrim(L"     aze ty     "));
  EXPECT_EQ(std::wstring(L"\t aze ty"), rtrim(L"\t aze ty\t "));
}

TEST_F(StringsTest, strRTrimBuffer) {
  char buffer[21]{ 0 };
  rtrim(buffer, "");
  EXPECT_STREQ(buffer, "");
  rtrim(buffer, " ");
  EXPECT_STREQ(buffer, "");
  rtrim(buffer, "          ");
  EXPECT_STREQ(buffer, "");
  rtrim(buffer, "a");
  EXPECT_STREQ(buffer, "a");
  rtrim(buffer, "a ");
  EXPECT_STREQ(buffer, "a");
  rtrim(buffer, " a");
  EXPECT_STREQ(buffer, " a");
  rtrim(buffer, " a ");
  EXPECT_STREQ(buffer, " a");
  rtrim(buffer, "    a     ");
  EXPECT_STREQ(buffer, "    a");
  rtrim(buffer, "     a     ");
  EXPECT_STREQ(buffer, "     a");
  rtrim(buffer, "aze ty");
  EXPECT_STREQ(buffer, "aze ty");
  rtrim(buffer, "aze ty ");
  EXPECT_STREQ(buffer, "aze ty");
  rtrim(buffer, " aze ty");
  EXPECT_STREQ(buffer, " aze ty");
  rtrim(buffer, " aze ty ");
  EXPECT_STREQ(buffer, " aze ty");
  rtrim(buffer, "     aze ty     ");
  EXPECT_STREQ(buffer, "     aze ty");
  rtrim(buffer, "\t aze ty\t ");
  EXPECT_STREQ(buffer, "\t aze ty");
  wchar_t wbuffer[21]{ 0 };
  rtrim(wbuffer, L"");
  EXPECT_STREQ(wbuffer, L"");
  rtrim(wbuffer, L" ");
  EXPECT_STREQ(wbuffer, L"");
  rtrim(wbuffer, L"          ");
  EXPECT_STREQ(wbuffer, L"");
  rtrim(wbuffer, L"a");
  EXPECT_STREQ(wbuffer, L"a");
  rtrim(wbuffer, L"a ");
  EXPECT_STREQ(wbuffer, L"a");
  rtrim(wbuffer, L" a");
  EXPECT_STREQ(wbuffer, L" a");
  rtrim(wbuffer, L" a ");
  EXPECT_STREQ(wbuffer, L" a");
  rtrim(wbuffer, L"    a     ");
  EXPECT_STREQ(wbuffer, L"    a");
  rtrim(wbuffer, L"     a     ");
  EXPECT_STREQ(wbuffer, L"     a");
  rtrim(wbuffer, L"aze ty");
  EXPECT_STREQ(wbuffer, L"aze ty");
  rtrim(wbuffer, L"aze ty ");
  EXPECT_STREQ(wbuffer, L"aze ty");
  rtrim(wbuffer, L" aze ty");
  EXPECT_STREQ(wbuffer, L" aze ty");
  rtrim(wbuffer, L" aze ty ");
  EXPECT_STREQ(wbuffer, L" aze ty");
  rtrim(wbuffer, L"     aze ty     ");
  EXPECT_STREQ(wbuffer, L"     aze ty");
  rtrim(wbuffer, L"\t aze ty\t ");
  EXPECT_STREQ(wbuffer, L"\t aze ty");
}

template <typename _CharType>
std::basic_string<_CharType> _strSelfRTrim(const _CharType* value, size_t valueLength) {
  _CharType buffer[20]{ 0 };
  memcpy((void*)buffer, (void*)value, (valueLength + 1u) * sizeof(_CharType));
  rtrimSelf(buffer);
  return std::basic_string<_CharType>(buffer);
}
TEST_F(StringsTest, strRTrimSelf) {
  EXPECT_EQ(std::string(""), _strSelfRTrim("", size_t{ 0 }));
  EXPECT_EQ(std::string(""), _strSelfRTrim(" ", size_t{ 1 }));
  EXPECT_EQ(std::string(""), _strSelfRTrim("          ", size_t{ 10 }));
  EXPECT_EQ(std::string("a"), _strSelfRTrim("a", size_t{ 1 }));
  EXPECT_EQ(std::string("a"), _strSelfRTrim("a ", size_t{ 2 }));
  EXPECT_EQ(std::string(" a"), _strSelfRTrim(" a", size_t{ 2 }));
  EXPECT_EQ(std::string(" a"), _strSelfRTrim(" a ", size_t{ 3 }));
  EXPECT_EQ(std::string("    a"), _strSelfRTrim("    a     ", size_t{ 10 }));
  EXPECT_EQ(std::string("     a"), _strSelfRTrim("     a     ", size_t{ 11 }));
  EXPECT_EQ(std::string("aze ty"), _strSelfRTrim("aze ty", size_t{ 6 }));
  EXPECT_EQ(std::string("aze ty"), _strSelfRTrim("aze ty ", size_t{ 7 }));
  EXPECT_EQ(std::string(" azerty"), _strSelfRTrim(" azerty", size_t{ 7 }));
  EXPECT_EQ(std::string(" aze ty"), _strSelfRTrim(" aze ty ", size_t{ 8 }));
  EXPECT_EQ(std::string("     aze ty"), _strSelfRTrim("     aze ty     ", size_t{ 16 }));
  EXPECT_EQ(std::string("\t aze ty"), _strSelfRTrim("\t aze ty\t ", size_t{ 10 }));
  EXPECT_EQ(std::wstring(L""), _strSelfRTrim(L"", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L""), _strSelfRTrim(L" ", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L""), _strSelfRTrim(L"          ", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfRTrim(L"a", size_t{ 0 }));
  EXPECT_EQ(std::wstring(L"a"), _strSelfRTrim(L"a ", size_t{ 2 }));
  EXPECT_EQ(std::wstring(L" a"), _strSelfRTrim(L" a", size_t{ 2 }));
  EXPECT_EQ(std::wstring(L" a"), _strSelfRTrim(L" a ", size_t{ 3 }));
  EXPECT_EQ(std::wstring(L"    a"), _strSelfRTrim(L"    a     ", size_t{ 10 }));
  EXPECT_EQ(std::wstring(L"     a"), _strSelfRTrim(L"     a     ", size_t{ 11 }));
  EXPECT_EQ(std::wstring(L"aze ty"), _strSelfRTrim(L"aze ty", size_t{ 6 }));
  EXPECT_EQ(std::wstring(L"aze ty"), _strSelfRTrim(L"aze ty ", size_t{ 7 }));
  EXPECT_EQ(std::wstring(L" azerty"), _strSelfRTrim(L" azerty", size_t{ 7 }));
  EXPECT_EQ(std::wstring(L" aze ty"), _strSelfRTrim(L" aze ty ", size_t{ 8 }));
  EXPECT_EQ(std::wstring(L"     aze ty"), _strSelfRTrim(L"     aze ty     ", size_t{ 16 }));
  EXPECT_EQ(std::wstring(L"\t aze ty"), _strSelfRTrim(L"\t aze ty\t ", size_t{ 10 }));
}
