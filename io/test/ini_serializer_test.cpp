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
#include <cstdio>
#include <set>
#include <io/ini_serializer.h>

using namespace pandora::io;

class IniSerializerTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- INI serializer --

TEST_F(IniSerializerTest, toIniInvalidTest) {
  IniSerializer serializer;

  SerializableValue::Object section;
  section["obj"] = SerializableValue(SerializableValue::Object{});
  SerializableValue::Object objWithSubObj;
  objWithSubObj["section"] = SerializableValue(std::move(section));
  EXPECT_ANY_THROW(serializer.toString(objWithSubObj));

  SerializableValue::Object objWithArraySubObj;
  objWithArraySubObj["arr"] = SerializableValue(SerializableValue::Array{ SerializableValue(SerializableValue::Object{}) });
  EXPECT_ANY_THROW(serializer.toString(objWithArraySubObj));

  SerializableValue::Object objWithArraySubArray;
  objWithArraySubArray["arr"] = SerializableValue(SerializableValue::Array{ SerializableValue(SerializableValue::Array{ SerializableValue(0) }) });
  EXPECT_ANY_THROW(serializer.toString(objWithArraySubArray));

  SerializableValue::Object objWithNoKey;
  objWithNoKey["   "] = SerializableValue(0);
  EXPECT_ANY_THROW(serializer.toString(objWithNoKey));
}

TEST_F(IniSerializerTest, toIniEmptyStringTest) {
  IniSerializer serializer;

  SerializableValue::Object emptyVal;
  auto result = serializer.toString(emptyVal);
  EXPECT_EQ(std::string(""), result);
}

TEST_F(IniSerializerTest, toIniBaseTypesTest) {
  SerializableValue::Object root;
  IniSerializer serializer;

  root["val"] = SerializableValue(0);
  auto result = serializer.toString(root);
  EXPECT_EQ(std::string("val=0\n"), result);
  root["val"] = SerializableValue(-42);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=-42\n"), result);

  root["val"] = SerializableValue(-76.127);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=-76.127"), result.substr(0, 11));
  EXPECT_EQ('\n', result.back());

  root["val"] = SerializableValue(true);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=true\n"), result);
  root["val"] = SerializableValue(false);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=false\n"), result);

  root["val"] = SerializableValue("simple text");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=\"simple text\"\n"), result);
  root["val"] = SerializableValue("\ttext value\nwith lines...\r\n");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=\"\ttext value\\nwith lines...\\r\\n\"\n"), result);
  root["val"] = SerializableValue(std::string("string text"));
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=\"string text\"\n"), result);

  root["val"] = SerializableValue(SerializableValue::Array{});
  result = serializer.toString(root);
  EXPECT_EQ(std::string(""), result);
  root["val"] = SerializableValue(SerializableValue::Array{ SerializableValue(7) });
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val[]=7\n"), result);

  root["val"] = SerializableValue(SerializableValue::Object{});
  result = serializer.toString(root);
  EXPECT_EQ(std::string("[val]\n"), result);

  root = {};
  root["int 1"] = SerializableValue(0);
  root["<object;>"] = SerializableValue(SerializableValue::Object{});
  root["str=base"] = SerializableValue("simple text");
  root["str=empty"] = SerializableValue("");
  root["bool;"] = SerializableValue(true);
  root["int2 "] = SerializableValue(-42);
  root["str#lines "] = SerializableValue("\ttext value\nwith lines...\n");
  root["<array>"] = SerializableValue(SerializableValue::Array{ SerializableValue(7) });
  root[" my [string] !"] = SerializableValue(std::string("string text"));
  result = serializer.toString(root);

  std::set<std::string> expectedLines{
    "int 1=0",
    "str\\=base=\"simple text\"",
    "str\\=empty=\"\"",
    "bool\\;=true",
    "int2=-42",
    "str\\#lines=\"\ttext value\\nwith lines...\\n\"",
    "<array>[]=7",
    "my \\[string\\] !=\"string text\"",
    "[<object\\;>]"
  };
  size_t start = 0, end = 0, count = 0;
  while ((end = result.find_first_of('\n', start)) != std::string::npos) {
    std::string part = result.substr(start, end - start);
    if (!part.empty()) {
      bool isFound = expectedLines.find(part) != expectedLines.end();
      EXPECT_TRUE(isFound);
      if (isFound)
        printf("FOUND: %s\n", part.c_str());
      else
        printf("NOT FOUND: %s\n", part.c_str());
      ++count;
    }
    start = end + 1;
  }
  EXPECT_EQ(expectedLines.size(), count);
}

TEST_F(IniSerializerTest, toIniBaseTypesCommentsTest) {
  SerializableValue::Object root;
  IniSerializer serializer;

  root["val"] = SerializableValue(0);
  root.at("val").setComment("comment 1");
  auto result = serializer.toString(root);
  EXPECT_EQ(std::string("val=0 ;comment 1\n"), result);
  root["val"] = SerializableValue(-42);
  root.at("val").setComment(" other comment #;! ");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=-42 ; other comment #;! \n"), result);

  root["val"] = SerializableValue(-76.127);
  root.at("val").setComment("[comment]");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=-76.127"), result.substr(0, 11));
  EXPECT_EQ(std::string("[comment]\n"), result.substr(result.size()-10u));
  EXPECT_EQ('\n', result.back());

  root["val"] = SerializableValue(true);
  root.at("val").setComment("comment\nwith\nlines\n");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=true ;comment\n;with\n;lines\n\n"), result);
  root["val"] = SerializableValue(false);
  root.at("val").setComment(nullptr);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=false\n"), result);

  root["val"] = SerializableValue("simple text");
  root.at("val").setComment(";comment");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=\"simple text\" ;;comment\n"), result);
  root["val"] = SerializableValue("\ttext value\nwith lines...\r\n");
  root.at("val").setComment(" ");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=\"\ttext value\\nwith lines...\\r\\n\" ; \n"), result);
  root["val"] = SerializableValue(std::string("string text"));
  root.at("val").setComment("\"abc\"");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("val=\"string text\" ;\"abc\"\n"), result);

  root["val"] = SerializableValue(SerializableValue::Array{});
  root.at("val").setComment("'abc'");
  result = serializer.toString(root);
  EXPECT_EQ(std::string(";'abc'\n"), result);
  root["val"] = SerializableValue(SerializableValue::Array{ SerializableValue(7) });
  root.at("val").setComment("text\nwith\nlines");
  result = serializer.toString(root);
  EXPECT_EQ(std::string(";text\n;with\n;lines\nval[]=7\n"), result);

  root["val"] = SerializableValue(SerializableValue::Object{});
  root.at("val").setComment("text\nwith\nlines");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("[val]\n;text\n;with\n;lines\n"), result);

  root = {};
  root["int 1"] = SerializableValue(0);
  root.at("int 1").setComment(" ");
  root["<object;>"] = SerializableValue(SerializableValue::Object{});
  root.at("<object;>").setComment("comment !;");
  root["str=base"] = SerializableValue("simple text");
  root["str=empty"] = SerializableValue("");
  root.at("str=empty").setComment("[comment]");
  root["bool;"] = SerializableValue(true);
  root.at("bool;").setComment("= val");
  root["int2 "] = SerializableValue(-42);
  root.at("int2 ").setComment("multiple\nlines");
  root["str#lines "] = SerializableValue("\ttext value\nwith lines...\n");
  root.at("str#lines ").setComment(" OK ");
  SerializableValue arrayVal(7);
  arrayVal.setComment("inlined !");
  root["<array>"] = SerializableValue(SerializableValue::Array{ std::move(arrayVal) });
  root.at("<array>").setComment("single line ");
  root[" my [string] !"] = SerializableValue(std::string("string text"));
  result = serializer.toString(root);

  std::set<std::string> expectedLines{
    "int 1=0 ; ",
    "str\\=base=\"simple text\"",
    "str\\=empty=\"\" ;[comment]",
    "bool\\;=true ;= val",
    "int2=-42 ;multiple",
    ";lines",
    "str\\#lines=\"\ttext value\\nwith lines...\\n\" ; OK ",
    ";single line ",
    "<array>[]=7 ;inlined !",
    "my \\[string\\] !=\"string text\"",
    "[<object\\;>]",
    ";comment !;"
  };
  size_t start = 0, end = 0, count = 0;
  while ((end = result.find_first_of('\n', start)) != std::string::npos) {
    std::string part = result.substr(start, end - start);
    if (!part.empty()) {
      bool isFound = expectedLines.find(part) != expectedLines.end();
      EXPECT_TRUE(isFound);
      if (isFound)
        printf("FOUND: %s\n", part.c_str());
      else
        printf("NOT FOUND: %s\n", part.c_str());
      ++count;
    }
    start = end + 1;
  }
  EXPECT_EQ(expectedLines.size(), count);
}

TEST_F(IniSerializerTest, toIniAllTypesTest) {
  SerializableValue::Object root;
  root["int 1"] = SerializableValue(999);
  root["str=base"] = SerializableValue("simple text");
  root["str=empty"] = SerializableValue("");
  root["bool;"] = SerializableValue(true);
  root.at("bool;").setComment("always true");
  root["int2 "] = SerializableValue(-42);
  root.at("int2 ").setComment("one\ntwo");
  root["str#lines "] = SerializableValue("\ttext value\nwith lines...\n");
  root["<array>"] = SerializableValue(SerializableValue::Array{ SerializableValue("array text"), SerializableValue(7), 
                                                                SerializableValue(-42), SerializableValue(true) });
  SerializableValue::Object section1;
  section1["sub-int 1"] = SerializableValue(0);
  section1["my list1"] = SerializableValue(SerializableValue::Array{ SerializableValue(0), SerializableValue(7), 
                                                                    SerializableValue(42), SerializableValue(-42) });
  section1.at("my list1").setComment("here comes the fun\nbe ready!");
  section1["my list 2"] = SerializableValue(SerializableValue::Array{ SerializableValue("0"), SerializableValue("7"), 
                                                                      SerializableValue("42"), SerializableValue("-42") });
  section1["sub-str 1"] = SerializableValue("0");
  section1.at("sub-str 1").setComment("dummy");
  root["sec 1"] = std::move(section1);
  root.at("sec 1").setComment("this section has a title...");

  root["sec 2 (empty)"] = SerializableValue::Object{};

  SerializableValue::Object section3;
  section3["isOk"] = SerializableValue(false);
  section3["count"] = SerializableValue(57);
  section3["name"] = SerializableValue("alpha");
  root["sec-3"] = std::move(section3);

  IniSerializer serializer;
  auto result = serializer.toString(root);

  std::set<std::string> expectedLines{
    "int 1=999",
    "str\\=base=\"simple text\"",
    "str\\=empty=\"\"",
    "bool\\;=true ;always true",
    "int2=-42 ;one",
    ";two",
    "str\\#lines=\"\ttext value\\nwith lines...\\n\"",
    "<array>[]=\"array text\"",
    "<array>[]=7",
    "<array>[]=-42",
    "<array>[]=true",
    "[sec 1]",
    ";this section has a title...",
    "sub-int 1=0",
    ";here comes the fun",
    ";be ready!",
    "my list1[]=0",
    "my list1[]=7",
    "my list1[]=42",
    "my list1[]=-42",
    "my list 2[]=\"0\"",
    "my list 2[]=\"7\"",
    "my list 2[]=\"42\"",
    "my list 2[]=\"-42\"",
    "sub-str 1=\"0\" ;dummy",
    "[sec 2 (empty)]",
    "[sec-3]",
    "isOk=false",
    "count=57",
    "name=\"alpha\""
  };
  size_t start = 0, end = 0, count = 0;
  while ((end = result.find_first_of('\n', start)) != std::string::npos) {
    std::string part = result.substr(start, end - start);
    if (!part.empty()) {
      bool isFound = expectedLines.find(part) != expectedLines.end();
      EXPECT_TRUE(isFound);
      if (isFound)
        printf("FOUND: %s\n", part.c_str());
      else
        printf("NOT FOUND: %s\n", part.c_str());
      ++count;
    }
    start = end + 1;
  }
  EXPECT_EQ(expectedLines.size(), count);
}


// -- INI deserializer --

TEST_F(IniSerializerTest, fromIniEmptyStringTest) {
  IniSerializer serializer;
  auto result = serializer.fromString(nullptr);
  EXPECT_TRUE(result.empty());
  result = serializer.fromString("");
  EXPECT_TRUE(result.empty());
  result = serializer.fromString("   \r\n\t  #dummy\n  ");
  EXPECT_TRUE(result.empty());
}

TEST_F(IniSerializerTest, fromIniTypesCommSectionsTest) {
  // deserialize raw1 - multiple sections / last empty line / all type+comment+quote possibilites
  const char* raw1 =
    "# title line\n"
    ";comment below title\n"
    "\n"
    "[section1]\n"
    "int1=42\n"
    "int2 = -26  \t\n"
    "int3\t=0;comment\n"
    "int4 = 56 ; yet another comment\n"
    "\n"
    "nb1 = .5\n"
    "nb2=256. ;comment\n"
    "nb3=42.02;comment\n"
    "nb4 =-0.5\n"
    ";comment\n"
    "\n"
    "[section 2]\r\n"
    "#commented=line\n"
    "bool1 = true\n"
    "bool2 = false\n"
    "bool3=true ; comment\r\n"
    "bool-invalid1 = True\n"
    "bool-invalid2 = False\n"
    "[\"section\"3] ;important stuff\n"
    " str with spaces =\tno need for quotes !;comment\n"
    "\\[str\\=special =    \n"
    "empty-line=\n"
    "empty-quotes=\"\"\n"
    "letter-quotes=\"a\"\n"
    "spaces-quotes = \"   \";comment\n"
    "text-quotes = \"this is a normal string !\" ;comment\n"
    "\n"
    "  [section_4 ]\t\n"
    "multi-line = \"this is multi-line\\nLine2...\\r\\nLine 3! \" ;comment\n"
    "apos='why double it?'\n"
    " apos-empty = ''\n"
    "apos-letter = 'x';comment\n"
    "apos-letter = 'z';overwrites\n"
    "apos-text = 'this is \\'not\\' a text' ;comment\n"
    "single-quote = \"bl@ bl@ !\n"
    "single-apos = \'bla bla ! ;comment\n"
    "not-a-number=027-is-not-a-real\\a\\b-number\\t ;yeah!\n"
    "multi-sep= 125 = 126 = 127\n"
    "esc-str= \\\"abc\"\n"
    "\n"
    "array-after[]= 1\n"
    "array-after[] =2;comment\n"
    "\t  array-after[] = 3\r\n"
    "\n"
    "multi-sep[]='not a valid array (key exists)'\n"
    "\n"
    "[empty-section]\n"
    "\n"
    "[not-a-valid-line\n"
    "\n"
    "[section*5]\n"
    "obj = overwritten by next section\n"
    "  [section*5]\t\n"
    "array-before[] = 'a'\n"
    "array-before[] = 'b'\n"
    "array-before[] = 'c'\n"
    "; this is the end of the file\n"
    "\n";

  IniSerializer serializer;
  auto result = serializer.fromString(raw1);
  EXPECT_EQ((size_t)6u, result.size());
  
  ASSERT_TRUE(result.find("section1") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("section1").type());
  EXPECT_EQ((size_t)8u, result.at("section1").size());
  const auto* s1 = result.at("section1").getObject();
  ASSERT_TRUE(s1 != nullptr);
  ASSERT_TRUE(s1->find("int1") != s1->end());
  EXPECT_EQ(SerializableValue::Type::integer, s1->at("int1").type());
  EXPECT_EQ((int32_t)42, s1->at("int1").getInteger());
  ASSERT_TRUE(s1->find("int2") != s1->end());
  EXPECT_EQ(SerializableValue::Type::integer, s1->at("int2").type());
  EXPECT_EQ((int32_t)-26, s1->at("int2").getInteger());
  ASSERT_TRUE(s1->find("int3") != s1->end());
  EXPECT_EQ(SerializableValue::Type::integer, s1->at("int3").type());
  EXPECT_EQ((int32_t)0, s1->at("int3").getInteger());
  ASSERT_TRUE(s1->find("int4") != s1->end());
  EXPECT_EQ(SerializableValue::Type::integer, s1->at("int4").type());
  EXPECT_EQ((int32_t)56, s1->at("int4").getInteger());
  ASSERT_TRUE(s1->find("nb1") != s1->end());
  EXPECT_EQ(SerializableValue::Type::number, s1->at("nb1").type());
  EXPECT_EQ(0.5, s1->at("nb1").getNumber());
  ASSERT_TRUE(s1->find("nb2") != s1->end());
  EXPECT_EQ(SerializableValue::Type::number, s1->at("nb2").type());
  EXPECT_EQ(256.0, s1->at("nb2").getNumber());
  ASSERT_TRUE(s1->find("nb3") != s1->end());
  EXPECT_EQ(SerializableValue::Type::number, s1->at("nb3").type());
  EXPECT_EQ(42.02, s1->at("nb3").getNumber());
  ASSERT_TRUE(s1->find("nb4") != s1->end());
  EXPECT_EQ(SerializableValue::Type::number, s1->at("nb4").type());
  EXPECT_EQ(-0.5, s1->at("nb4").getNumber());
  
  ASSERT_TRUE(result.find("section 2") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("section 2").type());
  EXPECT_EQ((size_t)5u, result.at("section 2").size());
  const auto* s2 = result.at("section 2").getObject();
  ASSERT_TRUE(s2 != nullptr);
  ASSERT_TRUE(s2->find("bool1") != s2->end());
  EXPECT_EQ(SerializableValue::Type::boolean, s2->at("bool1").type());
  EXPECT_EQ(true, s2->at("bool1").getBoolean());
  ASSERT_TRUE(s2->find("bool2") != s2->end());
  EXPECT_EQ(SerializableValue::Type::boolean, s2->at("bool2").type());
  EXPECT_EQ(false, s2->at("bool2").getBoolean());
  ASSERT_TRUE(s2->find("bool3") != s2->end());
  EXPECT_EQ(SerializableValue::Type::boolean, s2->at("bool3").type());
  EXPECT_EQ(true, s2->at("bool3").getBoolean());
  ASSERT_TRUE(s2->find("bool-invalid1") != s2->end());
  EXPECT_EQ(SerializableValue::Type::text, s2->at("bool-invalid1").type());
  EXPECT_STREQ("True", s2->at("bool-invalid1").getText());
  ASSERT_TRUE(s2->find("bool-invalid2") != s2->end());
  EXPECT_EQ(SerializableValue::Type::text, s2->at("bool-invalid2").type());
  EXPECT_STREQ("False", s2->at("bool-invalid2").getText());
  
  ASSERT_TRUE(result.find("\"section\"3") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("\"section\"3").type());
  EXPECT_EQ((size_t)7u, result.at("\"section\"3").size());
  const auto* s3 = result.at("\"section\"3").getObject();
  ASSERT_TRUE(s3 != nullptr);
  ASSERT_TRUE(s3->find("str with spaces") != s3->end());
  EXPECT_EQ(SerializableValue::Type::text, s3->at("str with spaces").type());
  EXPECT_STREQ("no need for quotes !", s3->at("str with spaces").getText());
  ASSERT_TRUE(s3->find("[str=special") != s3->end());
  EXPECT_EQ(SerializableValue::Type::text, s3->at("[str=special").type());
  EXPECT_TRUE(s3->at("[str=special").getText() == nullptr);
  ASSERT_TRUE(s3->find("empty-line") != s3->end());
  EXPECT_EQ(SerializableValue::Type::text, s3->at("empty-line").type());
  EXPECT_TRUE(s3->at("empty-line").getText() == nullptr);
  ASSERT_TRUE(s3->find("empty-quotes") != s3->end());
  EXPECT_EQ(SerializableValue::Type::text, s3->at("empty-quotes").type());
  EXPECT_TRUE(s3->at("empty-quotes").getText() == nullptr);
  ASSERT_TRUE(s3->find("letter-quotes") != s3->end());
  EXPECT_EQ(SerializableValue::Type::text, s3->at("letter-quotes").type());
  EXPECT_STREQ("a", s3->at("letter-quotes").getText());
  ASSERT_TRUE(s3->find("spaces-quotes") != s3->end());
  EXPECT_EQ(SerializableValue::Type::text, s3->at("spaces-quotes").type());
  EXPECT_STREQ("   ", s3->at("spaces-quotes").getText());
  ASSERT_TRUE(s3->find("text-quotes") != s3->end());
  EXPECT_EQ(SerializableValue::Type::text, s3->at("text-quotes").type());
  EXPECT_STREQ("this is a normal string !", s3->at("text-quotes").getText());

  ASSERT_TRUE(result.find("section_4 ") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("section_4 ").type());
  EXPECT_EQ((size_t)11u, result.at("section_4 ").size());
  const auto* s4 = result.at("section_4 ").getObject();
  ASSERT_TRUE(s4 != nullptr);
  ASSERT_TRUE(s4->find("multi-line") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("multi-line").type());
  EXPECT_STREQ("this is multi-line\nLine2...\r\nLine 3! ", s4->at("multi-line").getText());
  ASSERT_TRUE(s4->find("apos") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("apos").type());
  EXPECT_STREQ("why double it?", s4->at("apos").getText());
  ASSERT_TRUE(s4->find("apos-empty") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("apos-empty").type());
  EXPECT_TRUE(s4->at("apos-empty").getText() == nullptr);
  ASSERT_TRUE(s4->find("apos-letter") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("apos-letter").type());
  EXPECT_STREQ("z", s4->at("apos-letter").getText());
  ASSERT_TRUE(s4->find("apos-text") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("apos-text").type());
  EXPECT_STREQ("this is 'not' a text", s4->at("apos-text").getText());
  ASSERT_TRUE(s4->find("single-quote") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("single-quote").type());
  EXPECT_STREQ("\"bl@ bl@ !", s4->at("single-quote").getText());
  ASSERT_TRUE(s4->find("single-apos") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("single-apos").type());
  EXPECT_STREQ("'bla bla !", s4->at("single-apos").getText());
  ASSERT_TRUE(s4->find("not-a-number") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("not-a-number").type());
  EXPECT_STREQ("027-is-not-a-real\a\b-number", s4->at("not-a-number").getText());
  ASSERT_TRUE(s4->find("multi-sep") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("multi-sep").type());
  EXPECT_STREQ("125 = 126 = 127", s4->at("multi-sep").getText());
  ASSERT_TRUE(s4->find("esc-str") != s4->end());
  EXPECT_EQ(SerializableValue::Type::text, s4->at("esc-str").type());
  EXPECT_STREQ("\"abc\"", s4->at("esc-str").getText());
  ASSERT_TRUE(s4->find("array-after") != s4->end());
  EXPECT_EQ(SerializableValue::Type::arrays, s4->at("array-after").type());
  ASSERT_EQ((size_t)3u, s4->at("array-after").size());
  EXPECT_EQ(SerializableValue::Type::integer, (*s4->at("array-after").getArray())[0].type());
  EXPECT_EQ((int32_t)1, (*s4->at("array-after").getArray())[0].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*s4->at("array-after").getArray())[1].type());
  EXPECT_EQ((int32_t)2, (*s4->at("array-after").getArray())[1].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*s4->at("array-after").getArray())[2].type());
  EXPECT_EQ((int32_t)3, (*s4->at("array-after").getArray())[2].getInteger());
  
  ASSERT_TRUE(result.find("empty-section") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("empty-section").type());
  EXPECT_EQ((size_t)0u, result.at("empty-section").size());
  EXPECT_TRUE(result.at("empty-section").getObject() == nullptr);
  
  ASSERT_TRUE(result.find("section*5") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("section*5").type());
  EXPECT_EQ((size_t)1u, result.at("section*5").size());
  const auto* s5 = result.at("section*5").getObject();
  ASSERT_TRUE(s5 != nullptr);
  ASSERT_TRUE(s5->find("array-before") != s5->end());
  EXPECT_EQ(SerializableValue::Type::arrays, s5->at("array-before").type());
  ASSERT_EQ((size_t)3u, s5->at("array-before").size());
  EXPECT_EQ(SerializableValue::Type::text, (*s5->at("array-before").getArray())[0].type());
  EXPECT_STREQ("a", (*s5->at("array-before").getArray())[0].getText());
  EXPECT_EQ(SerializableValue::Type::text, (*s5->at("array-before").getArray())[1].type());
  EXPECT_STREQ("b", (*s5->at("array-before").getArray())[1].getText());
  EXPECT_EQ(SerializableValue::Type::text, (*s5->at("array-before").getArray())[2].type());
  EXPECT_STREQ("c", (*s5->at("array-before").getArray())[2].getText());
}

TEST_F(IniSerializerTest, fromIniNoSectionTest) {
  // deserialize raw2 - properties out of section / no section / fake bool / value+comment on last line
  const char* raw2 =
    "int1=-42\n"
    "str1 = cool string #yeah!\n"
    "arr1[]=\"only value\"\n"
    "str2='apos'\n"
    "str3 = false false is string\n"
    "bool1 = true #TRUE";
  
  IniSerializer serializer;
  auto result = serializer.fromString(raw2);
  EXPECT_EQ((size_t)6u, result.size());
  
  ASSERT_TRUE(result.find("int1") != result.end());
  EXPECT_EQ(SerializableValue::Type::integer, result.at("int1").type());
  EXPECT_EQ((int32_t)-42, result.at("int1").getInteger());
  ASSERT_TRUE(result.find("str1") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("str1").type());
  EXPECT_STREQ("cool string", result.at("str1").getText());
  ASSERT_TRUE(result.find("arr1") != result.end());
  EXPECT_EQ(SerializableValue::Type::arrays, result.at("arr1").type());
  ASSERT_EQ((size_t)1u, result.at("arr1").size());
  EXPECT_EQ(SerializableValue::Type::text, (*result.at("arr1").getArray())[0].type());
  EXPECT_STREQ("only value", (*result.at("arr1").getArray())[0].getText());
  ASSERT_TRUE(result.find("str2") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("str2").type());
  EXPECT_STREQ("apos", result.at("str2").getText());
  ASSERT_TRUE(result.find("str3") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("str3").type());
  EXPECT_STREQ("false false is string", result.at("str3").getText());
  ASSERT_TRUE(result.find("bool1") != result.end());
  EXPECT_EQ(SerializableValue::Type::boolean, result.at("bool1").type());
  EXPECT_EQ(true, result.at("bool1").getBoolean());
}

TEST_F(IniSerializerTest, fromIniMainPropsAndSectionTest) {
  // deserialize raw3 - properties out of section / single section / no-key / same key other section / value on last line
  const char* raw3 =
    "dummy = 42 \n"
    "sectionless-bool= true\n"
    "sectionless str= True\n"
    "= abc\n"
    "  [single-section]  \n"
    "arr 1 [] = value 1\n"
    "arr 1 [] =\r\n"
    "arr 1 [] = abc\r\n"
    "dummy = not-same-section";
  
  IniSerializer serializer;
  auto result = serializer.fromString(raw3);
  EXPECT_EQ((size_t)4u, result.size());
  
  ASSERT_TRUE(result.find("dummy") != result.end());
  EXPECT_EQ(SerializableValue::Type::integer, result.at("dummy").type());
  EXPECT_EQ((int32_t)42, result.at("dummy").getInteger());
  ASSERT_TRUE(result.find("sectionless-bool") != result.end());
  EXPECT_EQ(SerializableValue::Type::boolean, result.at("sectionless-bool").type());
  EXPECT_EQ(true, result.at("sectionless-bool").getBoolean());
  ASSERT_TRUE(result.find("sectionless str") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("sectionless str").type());
  EXPECT_STREQ("True", result.at("sectionless str").getText());
  
  ASSERT_TRUE(result.find("single-section") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("single-section").type());
  EXPECT_EQ((size_t)2u, result.at("single-section").size());
  const auto* raw3Section = result.at("single-section").getObject();
  ASSERT_TRUE(raw3Section != nullptr);
  ASSERT_TRUE(raw3Section->find("arr 1") != raw3Section->end());
  EXPECT_EQ(SerializableValue::Type::arrays, raw3Section->at("arr 1").type());
  ASSERT_TRUE(raw3Section->at("arr 1").getArray() != nullptr);
  ASSERT_EQ((size_t)3u, raw3Section->at("arr 1").getArray()->size());
  EXPECT_EQ(SerializableValue::Type::text, (*raw3Section->at("arr 1").getArray())[0].type());
  EXPECT_STREQ("value 1", (*raw3Section->at("arr 1").getArray())[0].getText());
  EXPECT_EQ(SerializableValue::Type::text, (*raw3Section->at("arr 1").getArray())[1].type());
  EXPECT_TRUE((*raw3Section->at("arr 1").getArray())[1].getText() == nullptr);
  EXPECT_EQ(SerializableValue::Type::text, (*raw3Section->at("arr 1").getArray())[2].type());
  EXPECT_STREQ("abc", (*raw3Section->at("arr 1").getArray())[2].getText());
  ASSERT_TRUE(raw3Section->find("dummy") != raw3Section->end());
  EXPECT_EQ(SerializableValue::Type::text, raw3Section->at("dummy").type());
  EXPECT_STREQ("not-same-section", raw3Section->at("dummy").getText());
}


// -- INI serializer/deserializer --

TEST_F(IniSerializerTest, toFromIniTest) {
  // data to serialize
  SerializableValue::Object root;
  root["int 1"] = SerializableValue(999);
  root["str=base"] = SerializableValue("simple text");
  root["str=empty"] = SerializableValue("");
  root["bool;"] = SerializableValue(true);
  root.at("bool;").setComment("always true");
  root["int2 "] = SerializableValue(-42);
  root.at("int2 ").setComment("one\ntwo");
  root["str#lines "] = SerializableValue("\ttext value\nwith lines...\n");
  root["<array>"] = SerializableValue(SerializableValue::Array{ SerializableValue("array text"), SerializableValue(7), 
                                                                SerializableValue(-42), SerializableValue(true) });
  SerializableValue::Object section1;
  section1["sub-int 1"] = SerializableValue(0);
  section1["my list1"] = SerializableValue(SerializableValue::Array{ SerializableValue(0), SerializableValue(7), 
                                                                     SerializableValue(42), SerializableValue(-42) });
  section1.at("my list1").setComment("here comes the fun\nbe ready!");
  section1["my list 2"] = SerializableValue(SerializableValue::Array{ SerializableValue("0"), SerializableValue("7"), 
                                                                      SerializableValue("-42") });
  section1["sub-str 1"] = SerializableValue("0");
  section1.at("sub-str 1").setComment("dummy");
  root["sec 1"] = std::move(section1);
  root.at("sec 1").setComment("this section has a title...");

  root["sec 2 (empty)"] = SerializableValue::Object{};

  SerializableValue::Object section3;
  section3["isOk"] = SerializableValue(false);
  section3["count"] = SerializableValue(57);
  section3["name"] = SerializableValue("alpha");
  root["sec-3"] = std::move(section3);

  // serialize/deserialize
  IniSerializer serializer;
  std::string serialized = serializer.toString(root);
  auto result = serializer.fromString(serialized.c_str());

  // verify data
  EXPECT_EQ((size_t)10, result.size());

  ASSERT_TRUE(result.find("int 1") != result.end());
  EXPECT_EQ(SerializableValue::Type::integer, result.at("int 1").type());
  EXPECT_EQ((int32_t)999, result.at("int 1").getInteger());
  ASSERT_TRUE(result.find("str=base") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("str=base").type());
  EXPECT_STREQ("simple text", result.at("str=base").getText());
  ASSERT_TRUE(result.find("str=empty") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("str=empty").type());
  EXPECT_TRUE(result.at("str=empty").getText() == nullptr);
  ASSERT_TRUE(result.find("bool;") != result.end());
  EXPECT_EQ(SerializableValue::Type::boolean, result.at("bool;").type());
  EXPECT_EQ(true, result.at("bool;").getBoolean());
  ASSERT_TRUE(result.find("int2") != result.end());
  EXPECT_EQ(SerializableValue::Type::integer, result.at("int2").type());
  EXPECT_EQ((int32_t)-42, result.at("int2").getInteger());
  ASSERT_TRUE(result.find("str#lines") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("str#lines").type());
  EXPECT_STREQ("\ttext value\nwith lines...\n", result.at("str#lines").getText());

  ASSERT_TRUE(result.find("<array>") != result.end());
  EXPECT_EQ(SerializableValue::Type::arrays, result.at("<array>").type());
  const auto* arrayRef = result.at("<array>").getArray();
  ASSERT_TRUE(arrayRef != nullptr);
  ASSERT_EQ((size_t)4u, arrayRef->size());
  EXPECT_EQ(SerializableValue::Type::text, (*arrayRef)[0].type());
  EXPECT_STREQ("array text", (*arrayRef)[0].getText());
  EXPECT_EQ(SerializableValue::Type::integer, (*arrayRef)[1].type());
  EXPECT_EQ((int32_t)7, (*arrayRef)[1].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*arrayRef)[2].type());
  EXPECT_EQ((int32_t)-42, (*arrayRef)[2].getInteger());
  EXPECT_EQ(SerializableValue::Type::boolean, (*arrayRef)[3].type());
  EXPECT_EQ(true, (*arrayRef)[3].getBoolean());

  ASSERT_TRUE(result.find("sec 1") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("sec 1").type());
  EXPECT_EQ((size_t)4u, result.at("sec 1").size());
  const auto* sec1Ref = result.at("sec 1").getObject();
  ASSERT_TRUE(sec1Ref != nullptr);
  ASSERT_TRUE(sec1Ref->find("sub-int 1") != sec1Ref->end());
  EXPECT_EQ(SerializableValue::Type::integer, sec1Ref->at("sub-int 1").type());
  EXPECT_EQ((int32_t)0, sec1Ref->at("sub-int 1").getInteger());
  ASSERT_TRUE(sec1Ref->find("sub-str 1") != sec1Ref->end());
  EXPECT_EQ(SerializableValue::Type::text, sec1Ref->at("sub-str 1").type());
  EXPECT_STREQ("0", sec1Ref->at("sub-str 1").getText());

  ASSERT_TRUE(sec1Ref->find("my list1") != sec1Ref->end());
  EXPECT_EQ(SerializableValue::Type::arrays, sec1Ref->at("my list1").type());
  const auto* s1Array1Ref = sec1Ref->at("my list1").getArray();
  ASSERT_TRUE(s1Array1Ref != nullptr);
  ASSERT_EQ((size_t)4u, s1Array1Ref->size());
  EXPECT_EQ(SerializableValue::Type::integer, (*s1Array1Ref)[0].type());
  EXPECT_EQ((int32_t)0, (*s1Array1Ref)[0].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*s1Array1Ref)[1].type());
  EXPECT_EQ((int32_t)7, (*s1Array1Ref)[1].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*s1Array1Ref)[2].type());
  EXPECT_EQ((int32_t)42, (*s1Array1Ref)[2].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*s1Array1Ref)[3].type());
  EXPECT_EQ((int32_t)-42, (*s1Array1Ref)[3].getInteger());

  ASSERT_TRUE(sec1Ref->find("my list 2") != sec1Ref->end());
  EXPECT_EQ(SerializableValue::Type::arrays, sec1Ref->at("my list 2").type());
  const auto* s1Array2Ref = sec1Ref->at("my list 2").getArray();
  ASSERT_TRUE(s1Array2Ref != nullptr);
  ASSERT_EQ((size_t)3u, s1Array2Ref->size());
  EXPECT_EQ(SerializableValue::Type::text, (*s1Array2Ref)[0].type());
  EXPECT_STREQ("0", (*s1Array2Ref)[0].getText());
  EXPECT_EQ(SerializableValue::Type::text, (*s1Array2Ref)[1].type());
  EXPECT_STREQ("7", (*s1Array2Ref)[1].getText());
  EXPECT_EQ(SerializableValue::Type::text, (*s1Array2Ref)[2].type());
  EXPECT_STREQ("-42", (*s1Array2Ref)[2].getText());

  ASSERT_TRUE(result.find("sec 2 (empty)") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("sec 2 (empty)").type());
  EXPECT_EQ((size_t)0u, result.at("sec 2 (empty)").size());
  EXPECT_TRUE(result.at("sec 2 (empty)").getObject() == nullptr);

  ASSERT_TRUE(result.find("sec-3") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("sec-3").type());
  EXPECT_EQ((size_t)3u, result.at("sec-3").size());
  const auto* sec3Ref = result.at("sec-3").getObject();
  ASSERT_TRUE(sec3Ref != nullptr);
  ASSERT_TRUE(sec3Ref->find("isOk") != sec3Ref->end());
  EXPECT_EQ(SerializableValue::Type::boolean, sec3Ref->at("isOk").type());
  EXPECT_EQ(false, sec3Ref->at("isOk").getBoolean());
  ASSERT_TRUE(sec3Ref->find("count") != sec3Ref->end());
  EXPECT_EQ(SerializableValue::Type::integer, sec3Ref->at("count").type());
  EXPECT_EQ((int32_t)57, sec3Ref->at("count").getInteger());
  ASSERT_TRUE(sec3Ref->find("name") != sec3Ref->end());
  EXPECT_EQ(SerializableValue::Type::text, sec3Ref->at("name").type());
  EXPECT_STREQ("alpha", sec3Ref->at("name").getText());
}
