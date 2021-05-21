#include <gtest/gtest.h>
#include <io/_private/_key_value_serializer_common.h>

using namespace pandora::io;

class _KeyValueSerializerCommonTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};

// -- identation --

TEST_F(_KeyValueSerializerCommonTest, addIndentTest) {
  std::string empty = "";
  __addIndent(size_t{0u}, empty);
  EXPECT_EQ(std::string(""), empty);
  std::string withVal = "abc";
  __addIndent(size_t{0u}, withVal);
  EXPECT_EQ(std::string("abc"), withVal);
  
  empty = "";
  __addIndent(size_t{1u}, empty);
  EXPECT_EQ(std::string(" "), empty);
  withVal = "abc";
  __addIndent(size_t{1u}, withVal);
  EXPECT_EQ(std::string("abc "), withVal);
  
  empty = "";
  __addIndent(size_t{2u}, empty);
  EXPECT_EQ(std::string("  "), empty);
  withVal = "abc";
  __addIndent(size_t{2u}, withVal);
  EXPECT_EQ(std::string("abc  "), withVal);
  
  empty = "";
  __addIndent(size_t{8u}, empty);
  EXPECT_EQ(std::string("        "), empty);
  withVal = "abc";
  __addIndent(size_t{8u}, withVal);
  EXPECT_EQ(std::string("abc        "), withVal);
}

TEST_F(_KeyValueSerializerCommonTest, removeIndentTest) {
  std::string empty = "";
  __removeIndent(size_t{0u}, empty);
  EXPECT_EQ(std::string(""), empty);
  std::string withVal = "     ";
  __removeIndent(size_t{0u}, withVal);
  EXPECT_EQ(std::string("     "), withVal);
  std::string withVal2 = "abc";
  __removeIndent(size_t{0u}, withVal2);
  EXPECT_EQ(std::string("abc"), withVal2);
  
  empty = "";
  __removeIndent(size_t{2u}, empty);
  EXPECT_EQ(std::string(""), empty);
  withVal = "     ";
  __removeIndent(size_t{2u}, withVal);
  EXPECT_EQ(std::string("   "), withVal);
  withVal2 = "abc";
  __removeIndent(size_t{2u}, withVal2);
  EXPECT_EQ(std::string("a"), withVal2);
  
  withVal = "     ";
  __removeIndent(size_t{5u}, withVal);
  EXPECT_EQ(std::string(""), withVal);
  withVal2 = "abc";
  __removeIndent(size_t{5u}, withVal2);
  EXPECT_EQ(std::string(""), withVal2);
  
  withVal = "     ";
  __removeIndent(size_t{8u}, withVal);
  EXPECT_EQ(std::string(""), withVal);
  withVal2 = "abc";
  __removeIndent(size_t{8u}, withVal2);
  EXPECT_EQ(std::string(""), withVal2);
}


// -- serialization helpers --

TEST_F(_KeyValueSerializerCommonTest, copyEscapedTextInQuotesTest) {
  std::string result = "";
  _copyEscapedTextInQuotes("", result);
  EXPECT_EQ(std::string("\"\""), result);
  _copyEscapedTextInQuotes(nullptr, result);
  EXPECT_EQ(std::string("\"\"\"\""), result);
  
  result = "a:";
  _copyEscapedTextInQuotes("b", result);
  EXPECT_EQ(std::string("a:\"b\""), result);
  
  result = "";
  _copyEscapedTextInQuotes("@_string\\ \"with\" $péc°aL' %Chars! /*--*/\xF0", result);
  EXPECT_EQ(std::string("\"@_string\\\\ \\\"with\\\" $péc°aL' %Chars! /*--*/\xF0\""), result);
  
  result = "";
  _copyEscapedTextInQuotes("\tthis is a multi-line string:\r\nLine 2...\r\nLine3 !\nEnd\n\t", result);
  EXPECT_EQ(std::string("\"this is a multi-line string:\\\nLine 2...\\\nLine3 !\\\nEnd\\\n\""), result);
}

TEST_F(_KeyValueSerializerCommonTest, copyCommentTest) {
  // empty comment -> no prefix
  std::string result = "";
  _copyComment("", false, "#", "", result);
  EXPECT_EQ(std::string(""), result);
  result = "";
  _copyComment("", true, "// ", "", result);
  EXPECT_EQ(std::string(""), result);
  
  // comments with values
  result = "";
  _copyComment(" ", false, "#", "", result);
  EXPECT_EQ(std::string("# "), result);
  result = "";
  _copyComment(" ", true, "// ", "", result);
  EXPECT_EQ(std::string(" //  "), result);

  result = "";
  _copyComment("@dummy comment !!!", true, "#", "    ", result);
  EXPECT_EQ(std::string(" #@dummy comment !!!"), result);
  result = "";
  _copyComment("@dummy comment !!!", false, "// ", "    ", result);
  EXPECT_EQ(std::string("// @dummy comment !!!"), result);
  
  result = "abc";
  _copyComment("@dummy comment !!!", false, "#", "    ", result);
  EXPECT_EQ(std::string("abc#@dummy comment !!!"), result);
  result = "abc";
  _copyComment("@dummy comment !!!", true, "// ", "    ", result);
  EXPECT_EQ(std::string("abc // @dummy comment !!!"), result);
  
  result = "";
  _copyComment("@dummy comment !!!\r\nAnd another line\nAnd last but not least...", false, "#", "   ", result);
  EXPECT_EQ(std::string("#@dummy comment !!!\n   #And another line\n   #And last but not least..."), result);
  result = "a,";
  _copyComment("@dummy comment !!!\r\nAnd another line\nAnd last but not least...", true, "// ", "    ", result);
  EXPECT_EQ(std::string("a, // @dummy comment !!!\n    // And another line\n    // And last but not least..."), result);
}


// -- deserialization helpers --

TEST_F(_KeyValueSerializerCommonTest, readBooleanTest) {
  bool result = false;
  EXPECT_TRUE(_readBoolean("", result) == nullptr);
  EXPECT_FALSE(result);
  EXPECT_TRUE(_readBoolean("blabla", result) == nullptr);
  EXPECT_FALSE(result);
  EXPECT_TRUE(_readBoolean(" true ", result) == nullptr);
  EXPECT_FALSE(result);
  EXPECT_TRUE(_readBoolean("t r u e", result) == nullptr);
  EXPECT_FALSE(result);
  EXPECT_TRUE(_readBoolean("True", result) == nullptr);
  EXPECT_FALSE(result);
  EXPECT_TRUE(_readBoolean("TRUE", result) == nullptr);
  EXPECT_FALSE(result);
  EXPECT_TRUE(_readBoolean(" false ", result) == nullptr);
  EXPECT_TRUE(_readBoolean("f a l s e", result) == nullptr);
  EXPECT_TRUE(_readBoolean("False", result) == nullptr);
  EXPECT_TRUE(_readBoolean("FALSE", result) == nullptr);
  
  result = false;
  EXPECT_TRUE(_readBoolean("true", result) != nullptr);
  EXPECT_TRUE(result);
  result = false;
  EXPECT_TRUE(_readBoolean("true ", result) != nullptr);
  EXPECT_TRUE(result);
  result = false;
  EXPECT_TRUE(_readBoolean("true); other stuff after it...\nnew line...", result) != nullptr);
  EXPECT_TRUE(result);
  
  result = true;
  EXPECT_TRUE(_readBoolean("false", result) != nullptr);
  EXPECT_FALSE(result);
  result = true;
  EXPECT_TRUE(_readBoolean("false ", result) != nullptr);
  EXPECT_FALSE(result);
  result = true;
  EXPECT_TRUE(_readBoolean("false--other stuff...", result) != nullptr);
  EXPECT_FALSE(result);
  
  const char* end = _readBoolean("false--other stuff...", result);
  EXPECT_TRUE(end != nullptr && *end == '-');
  end = _readBoolean("false ", result);
  EXPECT_TRUE(end != nullptr && *end == ' ');
  end = _readBoolean("true); other stuff...", result);
  EXPECT_TRUE(end != nullptr && *end == ')');
}

TEST_F(_KeyValueSerializerCommonTest, readNumberTest) {
  __Number value;
  bool isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)0, value.integer);
  isInt = false;
  EXPECT_EQ('a', *(_readNumber("abc", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)0, value.integer);
  isInt = false;
  EXPECT_EQ(' ', *(_readNumber("  22", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)0, value.integer);
  isInt = false;
  EXPECT_EQ(' ', *(_readNumber("2 2", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)2, value.integer);
  isInt = false;
  EXPECT_EQ('a', *(_readNumber("3a", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)3, value.integer);
  isInt = false;
  EXPECT_EQ('-', *(_readNumber("2-2", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)2, value.integer);
  isInt = true;
  EXPECT_EQ('.', *(_readNumber("2.2.2", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(2.2, value.number);
  isInt = false;
  EXPECT_EQ('-', *(_readNumber("+-2", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)0, value.integer);
  
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("0", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)0, value.integer);
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("0000000", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)0, value.integer);
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("000005", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)5, value.integer);
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("1000000", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)1000000, value.integer);
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("123456789", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)123456789, value.integer);
  EXPECT_EQ((char)0, *(_readNumber("3000000000", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_TRUE(value.integer < 0); // overflow
  
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("+0", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)0, value.integer);
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("-0", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)0, value.integer);
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("+57", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)57, value.integer);
  isInt = false;
  EXPECT_EQ((char)0, *(_readNumber("-62", value, isInt)));
  EXPECT_TRUE(isInt);
  EXPECT_EQ((int32_t)-62, value.integer);
  
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber(".", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(0.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber(".5", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(0.5, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("0.5", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(0.5, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("000000.5", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(0.5, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("1.", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(1.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("000001.", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(1.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("1.0000", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(1.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("150000000.", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(150000000.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("0.00000005", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(0.00000005, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("85.57", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(85.57, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("1456.087", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(1456.087, value.number);
  isInt = true;
  
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("+1.0000", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(1.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("+150000000.", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(150000000.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("+0.00000005", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(0.00000005, value.number);
  EXPECT_EQ((char)0, *(_readNumber("+85.57", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(85.57, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("-1.0000", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(-1.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("-150000000.", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(-150000000.0, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("-0.00000005", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(-0.00000005, value.number);
  EXPECT_EQ((char)0, *(_readNumber("-85.57", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(-85.57, value.number);
  
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("-.5", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_EQ(-0.5, value.number);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("0.333333333333333333", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_TRUE(value.number >= 0.3333333 && value.number <= 0.3333334);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("-1480252.78421005", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_TRUE(value.number >= -1480252.785 && value.number <= -1480252.784);
  isInt = true;
  EXPECT_EQ((char)0, *(_readNumber("59.94005994005994", value, isInt)));
  EXPECT_FALSE(isInt);
  EXPECT_TRUE(value.number >= 59.94005994 && value.number <= 59.94005995);
}

TEST_F(_KeyValueSerializerCommonTest, readTextInQuotesTest) {
  size_t length = 0;
  char* text = nullptr;
  EXPECT_EQ(nullptr, _readText("", &text, length));
  EXPECT_EQ(size_t{0u}, length);
  EXPECT_TRUE(text == nullptr);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_EQ(nullptr, _readText("abc-missing-quotes", &text, length));
  EXPECT_EQ(size_t{0u}, length);
  EXPECT_TRUE(text == nullptr);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_EQ(nullptr, _readText("\"abc-missing-last-quote", &text, length));
  EXPECT_EQ(size_t{0u}, length);
  EXPECT_TRUE(text == nullptr);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_EQ(nullptr, _readText(" \"abc\"", &text, length));
  EXPECT_EQ(size_t{0u}, length);
  EXPECT_TRUE(text == nullptr);
  if (text) { free(text); text = nullptr; }
  
  const char* it = nullptr;
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("\"abc\"", &text, length)));
  if (it) { EXPECT_EQ((char)0, *it); EXPECT_STREQ("abc", text); }
  EXPECT_EQ(size_t{3u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("\"abc\" other stuff", &text, length)));
  if (it) { EXPECT_EQ(' ', *it); EXPECT_STREQ("abc", text); }
  EXPECT_EQ(size_t{3u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("\"abc - def\\\nghi\\na\\\"a\"---", &text, length)));
  if (it) { EXPECT_EQ('-', *it); EXPECT_STREQ("abc - def\nghi\na\"a", text); }
  EXPECT_EQ(size_t{17u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("' simple quotes\\r\\t \"double allowed\"' ", &text, length)));
  if (it) { EXPECT_EQ(' ', *it); EXPECT_STREQ(" simple quotes\r\t \"double allowed\"", text); }
  EXPECT_EQ(size_t{33u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("\"\\\"''\\\"\"", &text, length)));
  if (it) { EXPECT_EQ((char)0, *it); EXPECT_STREQ("\"''\"", text); }
  EXPECT_EQ(size_t{4u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("'very-long-012345678901234567890123456789012345678901234567890123456789' ", &text, length)));
  if (it) { EXPECT_EQ(' ', *it); EXPECT_STREQ("very-long-012345678901234567890123456789012345678901234567890123456789", text); }
  EXPECT_EQ(size_t{70u}, length);
  if (text) { free(text); text = nullptr; }
}

TEST_F(_KeyValueSerializerCommonTest, readTextWithEndSymbolTest) {
  size_t length = 0;
  char* text = nullptr;
  EXPECT_EQ(nullptr, _readText("", &text, length, false, '"'));
  EXPECT_EQ(size_t{0u}, length);
  EXPECT_TRUE(text == nullptr);
  if (text) { free(text); text = nullptr; }
  length = 0;
  const char* it = nullptr;
  EXPECT_NE(nullptr, (it = _readText("", &text, length, true, '"')));
  if (it) { EXPECT_EQ((char)0, *it); }
  EXPECT_EQ(size_t{0u}, length);
  EXPECT_TRUE(text == nullptr);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("", &text, length, true, '"', '\'')));
  if (it) { EXPECT_EQ((char)0, *it); }
  EXPECT_EQ(size_t{0u}, length);
  EXPECT_TRUE(text == nullptr);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_EQ(nullptr, _readText("abc", &text, length, false, '"', ' '));
  EXPECT_EQ(size_t{0u}, length);
  EXPECT_TRUE(text == nullptr);
  if (text) { free(text); text = nullptr; }
  
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("abc$ ", &text, length, false, '$')));
  if (it) { EXPECT_EQ(' ', *it); EXPECT_STREQ("abc", text); }
  EXPECT_EQ(size_t{3u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("$abc$ ", &text, length, true, '$')));
  if (it) { EXPECT_EQ('a', *it); }
  EXPECT_EQ(size_t{0u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("abc$ ", &text, length, false, '$', ' ')));
  if (it) { EXPECT_EQ(' ', *it); EXPECT_STREQ("abc", text); }
  EXPECT_EQ(size_t{3u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("abc$ ", &text, length, true, ' ', '$')));
  if (it) { EXPECT_EQ(' ', *it); EXPECT_STREQ("abc", text); }
  EXPECT_EQ(size_t{3u}, length);
  if (text) { free(text); text = nullptr; }
  
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("\"abc\"", &text, length, true, '\'')));
  if (it) { EXPECT_EQ((char)0, *it); EXPECT_STREQ("\"abc\"", text); }
  EXPECT_EQ(size_t{5u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("\\$abc$", &text, length, false, ' ', ' ', '$')));
  if (it) { EXPECT_EQ((char)0, *it); EXPECT_STREQ("$abc", text); }
  EXPECT_EQ(size_t{4u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("\"abc\" other stuff$", &text, length, false, '$')));
  if (it) { EXPECT_EQ((char)0, *it); EXPECT_STREQ("\"abc\" other stuff", text); }
  EXPECT_EQ(size_t{17u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("abc { def\\\nghi\\na\\\"a\"- -", &text, length, true, '$', '-')));
  if (it) { EXPECT_EQ(' ', *it); EXPECT_STREQ("abc { def\nghi\na\"a\"", text); }
  EXPECT_EQ(size_t{18u}, length);
  if (text) { free(text); text = nullptr; }
  length = 0;
  EXPECT_NE(nullptr, (it = _readText("very-long-012345678901234567890123456789012345678901234567890123456' ", &text, length, true, '\'', '"')));
  if (it) { EXPECT_EQ(' ', *it); EXPECT_STREQ("very-long-012345678901234567890123456789012345678901234567890123456", text); }
  EXPECT_EQ(size_t{67u}, length);
  if (text) { free(text); text = nullptr; }
}
