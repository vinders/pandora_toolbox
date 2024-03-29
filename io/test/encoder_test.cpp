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
#include <memory>
#include <map>
#include <io/encoder.h>

using namespace pandora::io;

class EncoderTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- resources --

#define __P_UNKNOWN_CHAR 0xFFFDu

static const char* g_asciiU8 = u8"this is a simple string\nwith two lines.\n\nand a third one too!";
static const char16_t* g_asciiU16 = u"this is a simple string\nwith two lines.\n\nand a third one too!";
static const char* g_specialU8 = u8"Cha\u00EEne sp\u00E9ciale en Fran\u00E7ais,\n avec @ccents (& symboles: ~%\u00B5$\u20AC\u00A3#\u00A7\u00B0).";
static const char16_t* g_specialU16 = u"Cha\u00EEne sp\u00E9ciale en Fran\u00E7ais,\n avec @ccents (& symboles: ~%\u00B5$\u20AC\u00A3#\u00A7\u00B0).";
static const char* g_extendedU8 = u8"\u0429\u043D\u0460\u052A\u0532\U00020001\U0002000A\U0002000B\u306A\u306B\u305D\u3037\u306E\u3004\u25A9\u25C4\u20A9\u10A1\U00010338\U00010334\U00010349\u0601\u0641\uFB52\uFB67\uFB78\uFBF2\U00013000\U00013040\U0001F600\U0001F428\n";
static const char16_t* g_extendedU16 = u"\u0429\u043D\u0460\u052A\u0532\U00020001\U0002000A\U0002000B\u306A\u306B\u305D\u3037\u306E\u3004\u25A9\u25C4\u20A9\u10A1\U00010338\U00010334\U00010349\u0601\u0641\uFB52\uFB67\uFB78\uFBF2\U00013000\U00013040\U0001F600\U0001F428\n";

static const std::map<uint32_t, std::string> utf8Codes{
  {0x0025, u8"\u0025"}, {0x005D, u8"\u005D"}, {0x007E, u8"\u007E"}, {0x00A2, u8"\u00A2"},
  {0x00BB, u8"\u00BB"}, {0x00BF, u8"\u00BF"}, {0x00C0, u8"\u00C0"}, {0x00CB, u8"\u00CB"},
  {0x00FF, u8"\u00FF"}, {0x0100, u8"\u0100"}, {0x0134, u8"\u0134"}, {0x013F, u8"\u013F"},
  {0x0140, u8"\u0140"}, {0x017F, u8"\u017F"}, {0x0180, u8"\u0180"}, {0x01BF, u8"\u01BF"},
  {0x01C0, u8"\u01C0"}, {0x01FF, u8"\u01FF"}, {0x0200, u8"\u0200"}, {0x023F, u8"\u023F"},
  {0x0240, u8"\u0240"}, {0x027F, u8"\u027F"}, {0x0280, u8"\u0280"}, {0x0372, u8"\u0372"},
  {0x03FF, u8"\u03FF"}, {0x040A, u8"\u040A"}, {0x0494, u8"\u0494"}, {0x0516, u8"\u0516"},
  {0x0663, u8"\u0663"}, {0x06A9, u8"\u06A9"}, {0x071F, u8"\u071F"}, {0x079D, u8"\u079D"},
  {0x07FF, u8"\u07FF"}, {0x08A3, u8"\u08A3"}, {0x08FF, u8"\u08FF"}, {0x0C69, u8"\u0C69"},
  {0x0E11, u8"\u0E11"}, {0x1000, u8"\u1000"}, {0xA497, u8"\uA497"}, {0xFFFD, u8"\uFFFD"},
  {0x10400, u8"\U00010400"}, {0x104FB, u8"\U000104FB"}, {0x211D8, u8"\U000211D8"}
};
static const std::map<uint32_t, std::u16string> utf16Codes{
  {0x0025, u"\u0025"}, {0x005D, u"\u005D"}, {0x007E, u"\u007E"}, {0x00A2, u"\u00A2"},
  {0x00BB, u"\u00BB"}, {0x00BF, u"\u00BF"}, {0x00C0, u"\u00C0"}, {0x00CB, u"\u00CB"},
  {0x00FF, u"\u00FF"}, {0x0100, u"\u0100"}, {0x0134, u"\u0134"}, {0x013F, u"\u013F"},
  {0x0140, u"\u0140"}, {0x017F, u"\u017F"}, {0x0180, u"\u0180"}, {0x01BF, u"\u01BF"},
  {0x01C0, u"\u01C0"}, {0x01FF, u"\u01FF"}, {0x0200, u"\u0200"}, {0x023F, u"\u023F"},
  {0x0240, u"\u0240"}, {0x027F, u"\u027F"}, {0x0280, u"\u0280"}, {0x0372, u"\u0372"},
  {0x03FF, u"\u03FF"}, {0x040A, u"\u040A"}, {0x0494, u"\u0494"}, {0x0516, u"\u0516"},
  {0x0663, u"\u0663"}, {0x06A9, u"\u06A9"}, {0x071F, u"\u071F"}, {0x079D, u"\u079D"},
  {0x07FF, u"\u07FF"}, {0x08A3, u"\u08A3"}, {0x08FF, u"\u08FF"}, {0x0C69, u"\u0C69"},
  {0x0E11, u"\u0E11"}, {0x1000, u"\u1000"}, {0xA497, u"\uA497"}, {0xFFFD, u"\uFFFD"},
  {0x10400, u"\U00010400"}, {0x104FB, u"\U000104FB"}, {0x211D8, u"\U000211D8"}
};


// -- helpers --

static bool __isBigEndian() noexcept {
  char16_t endianTest = 0x0A0B;
  return (*((char*)&endianTest) == 0x0A);
}

static void __addBomU8(const char* source, char* outDest) {
  size_t length = strlen(source);
  memcpy((void*)outDest, (void*)Encoder::Utf8::byteOrderMark(), 3 * sizeof(char));
  memcpy((void*)&outDest[3], (void*)source, length * sizeof(char));
  outDest[length + 3] = (char)0;
}
static const char* asciiU8Bom() {
  static char buffer[256]; static bool isInit = false;
  if (!isInit) { __addBomU8(g_asciiU8, buffer); isInit = true; }
  return (const char*)buffer;
}
static const char* specialU8Bom() {
  static char buffer[256]; static bool isInit = false;
  if (!isInit) { __addBomU8(g_specialU8, buffer); isInit = true; }
  return (const char*)buffer;
}
static const char* extendedU8Bom() {
  static char buffer[256]; static bool isInit = false;
  if (!isInit) { __addBomU8(g_extendedU8, buffer); isInit = true; }
  return (const char*)buffer;
}

static std::unique_ptr<char[]> __toUtf16BE(const char16_t* value, size_t& outLength, bool addBom = false) {
  outLength = 0;
  for (const char16_t* it = value; *it ; ++it)
    outLength += 2u;
  auto bigEndianValue = std::unique_ptr<char[]>(new char[outLength + 4u]);
  memset((void*)&(bigEndianValue.get()[outLength]), 0, 4*sizeof(char));
  if (addBom) { bigEndianValue.get()[0] = '\xFE'; bigEndianValue.get()[1] = '\xFF'; }
  
  if (__isBigEndian()) {
    memcpy(addBom ? (void*)&(bigEndianValue.get()[2]) : (void*)bigEndianValue.get(), (void*)value, outLength*sizeof(char));
  }
  else {
    char* bufferIt = addBom ? &(bigEndianValue.get()[2]) : bigEndianValue.get();
    size_t remaining = outLength;
    for (const char* it = (const char*)value; remaining >= size_t{ 2u }; it += 2, bufferIt += 2, remaining -= 2u) {
      *bufferIt = it[1];
      bufferIt[1] = *it;
    }
  }
  if (addBom) { outLength += 2u; }
  return bigEndianValue;
}
static std::unique_ptr<char[]> __toUtf16LE(const char16_t* value, size_t& outLength, bool addBom = false) {
  outLength = 0;
  for (const char16_t* it = value; *it ; ++it)
    outLength += 2u;
  auto ltEndianValue = std::unique_ptr<char[]>(new char[outLength + 4u]);
  memset((void*)&(ltEndianValue.get()[outLength]), 0, 4*sizeof(char));
  if (addBom) { ltEndianValue.get()[0] = '\xFF'; ltEndianValue.get()[1] = '\xFE'; }

  if (!__isBigEndian()) {
    memcpy(addBom ? (void*)&(ltEndianValue.get()[2]) : (void*)ltEndianValue.get(), (void*)value, outLength*sizeof(char));
  }
  else {
    char* bufferIt = addBom ? &(ltEndianValue.get()[2]) : ltEndianValue.get();
    size_t remaining = outLength;
    for (const char* it = (const char*)value; remaining >= size_t{ 2u }; it += 2, bufferIt += 2, remaining -= 2u) {
      *bufferIt = it[1];
      bufferIt[1] = *it;
    }
  }
  if (addBom) { outLength += 2u; }
  return ltEndianValue;
}


// -- encoding detection --

TEST_F(EncoderTest, detectEmpty) {
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding(nullptr, 0));
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding("", 0));
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding("", size_t{1u}));
}

TEST_F(EncoderTest, detectWithBOM) {
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding(Encoder::Utf8::byteOrderMark(), strlen(Encoder::Utf8::byteOrderMark())));
  EXPECT_EQ(Encoding::utf16BE, Encoder::detectEncoding(Encoder::Utf16::byteOrderMarkBE(), strlen(Encoder::Utf16::byteOrderMarkBE())));
  EXPECT_EQ(Encoding::utf16LE, Encoder::detectEncoding(Encoder::Utf16::byteOrderMarkLE(), strlen(Encoder::Utf16::byteOrderMarkLE())));
  
  size_t length = 0;
  
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding(asciiU8Bom(), strlen(asciiU8Bom())));
  auto asciiUtf16BE = __toUtf16BE(g_asciiU16, length, true);
  EXPECT_EQ(Encoding::utf16BE, Encoder::detectEncoding(asciiUtf16BE.get(), length));
  auto asciiUtf16LE = __toUtf16LE(g_asciiU16, length, true);
  EXPECT_EQ(Encoding::utf16LE, Encoder::detectEncoding(asciiUtf16LE.get(), length));

  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding(specialU8Bom(), strlen(specialU8Bom())));
  asciiUtf16BE = __toUtf16BE(g_specialU16, length, true);
  EXPECT_EQ(Encoding::utf16BE, Encoder::detectEncoding(asciiUtf16BE.get(), length));
  asciiUtf16LE = __toUtf16LE(g_specialU16, length, true);
  EXPECT_EQ(Encoding::utf16LE, Encoder::detectEncoding(asciiUtf16LE.get(), length));
  
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding(extendedU8Bom(), strlen(extendedU8Bom())));
  asciiUtf16BE = __toUtf16BE(g_extendedU16, length, true);
  EXPECT_EQ(Encoding::utf16BE, Encoder::detectEncoding(asciiUtf16BE.get(), length));
  asciiUtf16LE = __toUtf16LE(g_extendedU16, length, true);
  EXPECT_EQ(Encoding::utf16LE, Encoder::detectEncoding(asciiUtf16LE.get(), length));
}

TEST_F(EncoderTest, detectWithoutBOM) {
  size_t length = 0;
  
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding(g_asciiU8, strlen(g_asciiU8)));
  auto asciiUtf16BE = __toUtf16BE(g_asciiU16, length);
  EXPECT_EQ(Encoding::utf16BE, Encoder::detectEncoding(asciiUtf16BE.get(), length));
  auto asciiUtf16LE = __toUtf16LE(g_asciiU16, length);
  EXPECT_EQ(Encoding::utf16LE, Encoder::detectEncoding(asciiUtf16LE.get(), length));
  
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding(g_specialU8, strlen(g_specialU8)));
  asciiUtf16BE = __toUtf16BE(g_specialU16, length);
  EXPECT_EQ(Encoding::utf16BE, Encoder::detectEncoding(asciiUtf16BE.get(), length));
  asciiUtf16LE = __toUtf16LE(g_specialU16, length);
  EXPECT_EQ(Encoding::utf16LE, Encoder::detectEncoding(asciiUtf16LE.get(), length));
  
  EXPECT_EQ(Encoding::utf8, Encoder::detectEncoding(g_extendedU8, strlen(g_extendedU8)));
  asciiUtf16BE = __toUtf16BE(g_extendedU16, length, true);
  EXPECT_EQ(Encoding::utf16BE, Encoder::detectEncoding(asciiUtf16BE.get(), length));
  asciiUtf16LE = __toUtf16LE(g_extendedU16, length);
  EXPECT_EQ(Encoding::utf16LE, Encoder::detectEncoding(asciiUtf16LE.get(), length));
}


// -- UTF-8 encoder --

TEST_F(EncoderTest, utf8Encode) {
  char buffer[5]{ 0 };
  for (auto& it : utf8Codes) {
    buffer[Encoder::Utf8::encode(it.first, buffer)] = (char)0; // add ending zero to use it as a string
    EXPECT_EQ(it.second, std::string(buffer));
  }
}
TEST_F(EncoderTest, utf8Decode) {
  for (auto& it : utf8Codes) {
    EXPECT_EQ(it.first, Encoder::Utf8::decode(it.second.c_str()));
  }
  EXPECT_EQ((uint32_t)0xFFFDu, Encoder::Utf8::decode("\xFF"));
}
TEST_F(EncoderTest, utf8EncodeDecode) {
  char buffer[4]{ 0 };
  for (uint32_t code = 0; code < 0xFFFFu; code += 64u) {
    EXPECT_TRUE(Encoder::Utf8::encode(code, buffer) >= size_t{1u});
    EXPECT_EQ(code, Encoder::Utf8::decode(buffer));
  }
  for (uint32_t code = 0x10000u; code < 0x10FFFFu; code += 1024u) {
    EXPECT_TRUE(Encoder::Utf8::encode(code, buffer) >= size_t{1u});
    EXPECT_EQ(code, Encoder::Utf8::decode(buffer));
  }
}
TEST_F(EncoderTest, utf8DecodeEncode) {
  char buffer[5]{ 0 };
  for (auto& it : utf8Codes) {
    buffer[Encoder::Utf8::encode(Encoder::Utf8::decode(it.second.c_str()), buffer)] = (char)0; // add ending zero to use it as a string
    EXPECT_EQ(it.second, std::string(buffer));
  }
}


// -- UTF-16 encoder --

TEST_F(EncoderTest, utf16Encode) {
  char16_t buffer[3]{ 0 };
  for (auto& it : utf16Codes) {
    buffer[Encoder::Utf16::encode(it.first, buffer)] = (char16_t)0; // add ending zero to use it as a string
    EXPECT_EQ(it.second, std::u16string(buffer));
  }
}
TEST_F(EncoderTest, utf16Decode) {
  for (auto& it : utf16Codes) {
    EXPECT_EQ((uint32_t)it.first, Encoder::Utf16::decode(it.second.c_str()));
  }
  char16_t invalid[3]{ u'\xD800', u'\xD800', u'\0' };
  EXPECT_EQ((uint32_t)0xFFFDu, Encoder::Utf16::decode(&invalid[0]));
}
TEST_F(EncoderTest, utf16EncodeDecode) {
  char16_t buffer[2]{ 0 };
  for (uint32_t code = 0; code < 0xFFFFu; code += 64u) {
    EXPECT_TRUE(Encoder::Utf16::encode(code, buffer) >= size_t{1u});
    if (code < 0xD800 || code > 0xDFFF)
      EXPECT_EQ(code, Encoder::Utf16::decode(buffer));
    else
      EXPECT_EQ((uint32_t)__P_UNKNOWN_CHAR, Encoder::Utf16::decode(buffer));
  }
  for (uint32_t code = 0x10000u; code < 0x10FFFFu; code += 1024u) {
    EXPECT_TRUE(Encoder::Utf16::encode(code, buffer) >= size_t{1u});
    EXPECT_EQ(code, Encoder::Utf16::decode(buffer));
  }
}
TEST_F(EncoderTest, utf16DecodeEncode) {
  char16_t buffer[3]{ 0 };
  for (auto& it : utf16Codes) {
    buffer[Encoder::Utf16::encode(Encoder::Utf16::decode(it.second.c_str()), buffer)] = (char16_t)0; // add ending zero to use it as a string
    EXPECT_EQ(it.second, std::u16string(buffer));
  }
}


// -- conversions --

TEST_F(EncoderTest, utf16ToUtf8) {
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromUtf16(std::u16string(g_asciiU16)));
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromUtf16(std::u16string(g_specialU16)));
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromUtf16(std::u16string(g_extendedU16)));
}
TEST_F(EncoderTest, utf8ToUtf16) {
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromUtf8(std::string(g_asciiU8)));
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromUtf8(std::string(g_specialU8)));
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromUtf8(std::string(g_extendedU8)));
}


// -- file reader UTF-8 --

TEST_F(EncoderTest, fromFileEncodingKnownUtf8) {
  EXPECT_EQ(std::string{}, Encoder::Utf8::fromFile(nullptr, (size_t)0, Encoding::utf8));
  EXPECT_EQ(std::string{}, Encoder::Utf8::fromFile("", (size_t)0, Encoding::utf8));
  EXPECT_EQ(std::string("a"), Encoder::Utf8::fromFile("a", (size_t)1u, Encoding::utf8));
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromFile(asciiU8Bom(), strlen(asciiU8Bom()), Encoding::utf8));
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromFile(g_asciiU8, strlen(g_asciiU8), Encoding::utf8));
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromFile(specialU8Bom(), strlen(specialU8Bom()), Encoding::utf8));
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromFile(g_specialU8, strlen(g_specialU8), Encoding::utf8));
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromFile(extendedU8Bom(), strlen(extendedU8Bom()), Encoding::utf8));
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromFile(g_extendedU8, strlen(g_extendedU8), Encoding::utf8));
}

TEST_F(EncoderTest, fromFileEncodingInvalidUtf8) {
  size_t length = 0;
  auto utf16Val = __toUtf16BE(g_asciiU16, length);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16LE(g_asciiU16, length);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16BE(g_asciiU16, length, true);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16LE(g_asciiU16, length, true);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  
  utf16Val = __toUtf16BE(g_specialU16, length);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16LE(g_specialU16, length);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16BE(g_specialU16, length, true);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16LE(g_specialU16, length, true);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  
  utf16Val = __toUtf16BE(g_extendedU16, length);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16LE(g_extendedU16, length);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16BE(g_extendedU16, length, true);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
  utf16Val = __toUtf16LE(g_extendedU16, length, true);
  EXPECT_NO_THROW(Encoder::Utf8::fromFile(&utf16Val[0], length, Encoding::utf8)); // incorrect encoding
}

TEST_F(EncoderTest, fromFileDetectUtf8ToUtf8) {
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromFile(asciiU8Bom(), strlen(asciiU8Bom())));
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromFile(g_asciiU8, strlen(g_asciiU8)));
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromFile(specialU8Bom(), strlen(specialU8Bom())));
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromFile(g_specialU8, strlen(g_specialU8)));
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromFile(extendedU8Bom(), strlen(extendedU8Bom())));
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromFile(g_extendedU8, strlen(g_extendedU8)));
}

TEST_F(EncoderTest, fromFileDetectUtf16ToUtf8) {
  size_t length = 0;
  auto utf16Val = __toUtf16BE(g_asciiU16, length);
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_asciiU16, length);
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16BE(g_asciiU16, length, true);
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_asciiU16, length, true);
  EXPECT_EQ(std::string(g_asciiU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  
  utf16Val = __toUtf16BE(g_specialU16, length);
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_specialU16, length);
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16BE(g_specialU16, length, true);
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_specialU16, length, true);
  EXPECT_EQ(std::string(g_specialU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  
  utf16Val = __toUtf16BE(g_extendedU16, length);
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_extendedU16, length);
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16BE(g_extendedU16, length, true);
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_extendedU16, length, true);
  EXPECT_EQ(std::string(g_extendedU8), Encoder::Utf8::fromFile(&utf16Val[0], length));
}


// -- file writer UTF-8 --

TEST_F(EncoderTest, toFileUtf8) {
  size_t fileLength = 0xFFFFFFFFu;
  EXPECT_TRUE(Encoder::Utf8::toFile(nullptr, 0, fileLength, true) == nullptr);
  EXPECT_EQ((size_t)0, fileLength);
  EXPECT_TRUE(Encoder::Utf8::toFile("", 0, fileLength, true) == nullptr);
  EXPECT_EQ((size_t)0, fileLength);
  
  size_t strLength = strlen(g_asciiU8);
  auto fileData = Encoder::Utf8::toFile(g_asciiU8, strLength, fileLength, false);
  ASSERT_TRUE(fileData != nullptr);
  EXPECT_EQ(strLength, fileLength);
  EXPECT_EQ(std::string(g_asciiU8), std::string(&fileData[0], fileLength));
  fileData = Encoder::Utf8::toFile(g_asciiU8, strLength, fileLength, true);
  ASSERT_TRUE(fileData != nullptr);
  EXPECT_EQ(strLength + 3u, fileLength);
  EXPECT_EQ(std::string(asciiU8Bom()), std::string(&fileData[0], fileLength));
  
  strLength = strlen(g_specialU8);
  fileData = Encoder::Utf8::toFile(g_specialU8, strLength, fileLength, false);
  ASSERT_TRUE(fileData != nullptr);
  EXPECT_EQ(strLength, fileLength);
  EXPECT_EQ(std::string(g_specialU8), std::string(&fileData[0], fileLength));
  fileData = Encoder::Utf8::toFile(g_specialU8, strLength, fileLength, true);
  ASSERT_TRUE(fileData != nullptr);
  EXPECT_EQ(strLength + 3u, fileLength);
  EXPECT_EQ(std::string(specialU8Bom()), std::string(&fileData[0], fileLength));
  
  strLength = strlen(g_extendedU8);
  fileData = Encoder::Utf8::toFile(g_extendedU8, strLength, fileLength, false);
  ASSERT_TRUE(fileData != nullptr);
  EXPECT_EQ(strLength, fileLength);
  EXPECT_EQ(std::string(g_extendedU8), std::string(&fileData[0], fileLength));
  fileData = Encoder::Utf8::toFile(g_extendedU8, strLength, fileLength, true);
  ASSERT_TRUE(fileData != nullptr);
  EXPECT_EQ(strLength + 3u, fileLength);
  EXPECT_EQ(std::string(extendedU8Bom()), std::string(&fileData[0], fileLength));
}


// -- file reader UTF-16 --

TEST_F(EncoderTest, fromFileEncodingKnownUtf16BE) {
  size_t length = 0;
  auto utf16Val = __toUtf16BE(g_asciiU16, length);
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16BE));
  utf16Val = __toUtf16BE(g_asciiU16, length, true);
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16BE));
  utf16Val = __toUtf16BE(g_specialU16, length);
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16BE));
  utf16Val = __toUtf16BE(g_specialU16, length, true);
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16BE));
  utf16Val = __toUtf16BE(g_extendedU16, length);
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16BE));
  utf16Val = __toUtf16BE(g_extendedU16, length, true);
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16BE));
}
TEST_F(EncoderTest, fromFileEncodingKnownUtf16LE) {
  size_t length = 0;
  auto utf16Val = __toUtf16LE(g_asciiU16, length);
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16LE));
  utf16Val = __toUtf16LE(g_asciiU16, length, true);
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16LE));
  utf16Val = __toUtf16LE(g_specialU16, length);
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16LE));
  utf16Val = __toUtf16LE(g_specialU16, length, true);
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16LE));
  utf16Val = __toUtf16LE(g_extendedU16, length);
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16LE));
  utf16Val = __toUtf16LE(g_extendedU16, length, true);
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(&utf16Val[0], length, Encoding::utf16LE));
}

TEST_F(EncoderTest, fromFileEncodingInvalidUtf16) {
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(nullptr, (size_t)0, Encoding::utf16BE)); // empty
  EXPECT_NO_THROW(Encoder::Utf16::fromFile("", (size_t)0, Encoding::utf16BE)); // empty
  EXPECT_NO_THROW(Encoder::Utf16::fromFile("a", (size_t)1u, Encoding::utf16BE)); // too short
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(g_asciiU8, strlen(g_asciiU8), Encoding::utf16BE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(g_asciiU8, strlen(g_asciiU8), Encoding::utf16LE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(asciiU8Bom(), strlen(asciiU8Bom()), Encoding::utf16BE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(asciiU8Bom(), strlen(asciiU8Bom()), Encoding::utf16LE)); // incorrect encoding
  
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(g_specialU8, strlen(g_specialU8), Encoding::utf16BE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(g_specialU8, strlen(g_specialU8), Encoding::utf16LE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(specialU8Bom(), strlen(specialU8Bom()), Encoding::utf16BE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(specialU8Bom(), strlen(specialU8Bom()), Encoding::utf16LE)); // incorrect encoding
  
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(g_extendedU8, strlen(g_extendedU8), Encoding::utf16BE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(g_extendedU8, strlen(g_extendedU8), Encoding::utf16LE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(extendedU8Bom(), strlen(extendedU8Bom()), Encoding::utf16BE)); // incorrect encoding
  EXPECT_NO_THROW(Encoder::Utf16::fromFile(extendedU8Bom(), strlen(extendedU8Bom()), Encoding::utf16LE)); // incorrect encoding
}

TEST_F(EncoderTest, fromFileDetectUtf8ToUtf16) {
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(asciiU8Bom(), strlen(asciiU8Bom())));
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(g_asciiU8, strlen(g_asciiU8)));
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(specialU8Bom(), strlen(specialU8Bom())));
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(g_specialU8, strlen(g_specialU8)));
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(extendedU8Bom(), strlen(extendedU8Bom())));
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(g_extendedU8, strlen(g_extendedU8)));
}

TEST_F(EncoderTest, fromFileDetectUtf16ToUtf16) {
  size_t length = 0;
  auto utf16Val = __toUtf16BE(g_asciiU16, length);
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_asciiU16, length);
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16BE(g_asciiU16, length, true);
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_asciiU16, length, true);
  EXPECT_EQ(std::u16string(g_asciiU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  
  utf16Val = __toUtf16BE(g_specialU16, length);
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_specialU16, length);
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16BE(g_specialU16, length, true);
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_specialU16, length, true);
  EXPECT_EQ(std::u16string(g_specialU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  
  utf16Val = __toUtf16BE(g_extendedU16, length);
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_extendedU16, length);
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16BE(g_extendedU16, length, true);
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
  utf16Val = __toUtf16LE(g_extendedU16, length, true);
  EXPECT_EQ(std::u16string(g_extendedU16), Encoder::Utf16::fromFile(&utf16Val[0], length));
}


// -- file writer UTF-16 --

static size_t __str16len(const char16_t* val) noexcept {
  size_t length = 0;
  while (*val) {
    ++val;
    ++length;
  }
  return length;
}

TEST_F(EncoderTest, toFileUtf16BE) {
  size_t fileLength = 0xFFFFFFFFu, byteLength = 0;
  EXPECT_TRUE(Encoder::Utf16::toFileBE(nullptr, 0, fileLength) == nullptr);
  EXPECT_EQ((size_t)0, fileLength);
  EXPECT_TRUE(Encoder::Utf16::toFileBE(u"", 0, fileLength) == nullptr);
  EXPECT_EQ((size_t)0, fileLength);
  
  auto fileData = Encoder::Utf16::toFileBE(g_asciiU16, __str16len(g_asciiU16), fileLength);
  ASSERT_TRUE(fileData != nullptr);
  auto expected = __toUtf16BE(g_asciiU16, byteLength, true);
  ASSERT_EQ(byteLength, fileLength);
  EXPECT_TRUE(memcmp((void*)fileData.get(), (void*)expected.get(), byteLength*sizeof(char)) == 0);

  fileData = Encoder::Utf16::toFileBE(g_specialU16, __str16len(g_specialU16), fileLength);
  ASSERT_TRUE(fileData != nullptr);
  expected = __toUtf16BE(g_specialU16, byteLength, true);
  ASSERT_EQ(byteLength, fileLength);
  EXPECT_TRUE(memcmp((void*)fileData.get(), (void*)expected.get(), byteLength*sizeof(char)) == 0);

  /*auto file = fopen("test_U16BE.txt", "wb");
  fwrite(fileData.get(), 1, fileLength, file);
  fflush(file);
  fclose(file);*/
  
  fileData = Encoder::Utf16::toFileBE(g_extendedU16, __str16len(g_extendedU16), fileLength);
  ASSERT_TRUE(fileData != nullptr);
  expected = __toUtf16BE(g_extendedU16, byteLength, true);
  ASSERT_EQ(byteLength, fileLength);
  EXPECT_TRUE(memcmp((void*)fileData.get(), (void*)expected.get(), byteLength*sizeof(char)) == 0);
}

TEST_F(EncoderTest, toFileUtf16LE) {
  size_t fileLength = 0xFFFFFFFFu, byteLength = 0;
  EXPECT_TRUE(Encoder::Utf16::toFileLE(nullptr, 0, fileLength) == nullptr);
  EXPECT_EQ((size_t)0, fileLength);
  EXPECT_TRUE(Encoder::Utf16::toFileLE(u"", 0, fileLength) == nullptr);
  EXPECT_EQ((size_t)0, fileLength);
  
  auto fileData = Encoder::Utf16::toFileLE(g_asciiU16, __str16len(g_asciiU16), fileLength);
  ASSERT_TRUE(fileData != nullptr);
  auto expected = __toUtf16LE(g_asciiU16, byteLength, true);
  ASSERT_EQ(byteLength, fileLength);
  EXPECT_TRUE(memcmp((void*)fileData.get(), (void*)expected.get(), byteLength*sizeof(char)) == 0);
  
  fileData = Encoder::Utf16::toFileLE(g_specialU16, __str16len(g_specialU16), fileLength);
  ASSERT_TRUE(fileData != nullptr);
  expected = __toUtf16LE(g_specialU16, byteLength, true);
  ASSERT_EQ(byteLength, fileLength);
  EXPECT_TRUE(memcmp((void*)fileData.get(), (void*)expected.get(), byteLength*sizeof(char)) == 0);
  
  fileData = Encoder::Utf16::toFileLE(g_extendedU16, __str16len(g_extendedU16), fileLength);
  ASSERT_TRUE(fileData != nullptr);
  expected = __toUtf16LE(g_extendedU16, byteLength, true);
  ASSERT_EQ(byteLength, fileLength);
  EXPECT_TRUE(memcmp((void*)fileData.get(), (void*)expected.get(), byteLength*sizeof(char)) == 0);
}
