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
#include <io/json_serializer.h>

using namespace pandora::io;

class JsonSerializerTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- JSON serializer --

TEST_F(JsonSerializerTest, toJsonEmptyStringTest) {
  JsonSerializer serializer(size_t{ 2u });

  SerializableValue::Object emptyVal;
  auto result = serializer.toString(emptyVal);
  EXPECT_EQ(std::string("{\n}\n"), result);
}

TEST_F(JsonSerializerTest, toJsonBaseTypesTest) {
  SerializableValue::Object root;
  JsonSerializer serializer(size_t{ 2u });
  JsonSerializer serializerI4(size_t{ 4u });

  root["val"] = SerializableValue(0);
  auto result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": 0\n}\n"), result);
  result = serializerI4.toString(root);
  EXPECT_EQ(std::string("{\n    \"val\": 0\n}\n"), result);
  root["val"] = SerializableValue(-42);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": -42\n}\n"), result);
  result = serializerI4.toString(root);
  EXPECT_EQ(std::string("{\n    \"val\": -42\n}\n"), result);

  root["val"] = SerializableValue(-76.127);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": -76.127"), result.substr(0, 18));
  EXPECT_EQ(std::string("\n}\n"), result.substr(result.size()-3u));

  root["val"] = SerializableValue(true);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": true\n}\n"), result);
  root["val"] = SerializableValue(false);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": false\n}\n"), result);

  root["val"] = SerializableValue("simple text");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": \"simple text\"\n}\n"), result);
  root["val"] = SerializableValue("\ttext value\nwith lines...\r\n");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": \"\ttext value\\nwith lines...\\r\\n\"\n}\n"), result);
  root["val"] = SerializableValue(std::string("string\a\b text"));
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": \"string\\a text\"\n}\n"), result);

  root["val"] = SerializableValue(SerializableValue::Array{});
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": [\n  ]\n}\n"), result);
  root["val"] = SerializableValue(SerializableValue::Array{ SerializableValue(7) });
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": [\n    7\n  ]\n}\n"), result);
  result = serializerI4.toString(root);
  EXPECT_EQ(std::string("{\n    \"val\": [\n        7\n    ]\n}\n"), result);
  root["val"] = SerializableValue(SerializableValue::Array{ SerializableValue(7.5) });
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": [\n    7.5"), result.substr(0,20));

  root["val"] = SerializableValue(SerializableValue::Object{});
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": {\n  }\n}\n"), result);

  root = {};
  root["int 1"] = SerializableValue(0);
  root["<object//>"] = SerializableValue(SerializableValue::Object{});
  root["str:base"] = SerializableValue("simple text");
  root["str:empty"] = SerializableValue("");
  root["bool//"] = SerializableValue(true);
  root["int2 "] = SerializableValue(-42);
  root["str\"lines "] = SerializableValue("\ttext value\nwith lines...\n");
  root["<array>"] = SerializableValue(SerializableValue::Array{ SerializableValue(7) });
  root[" my 'string' !"] = SerializableValue(std::string("string text"));
  result = serializer.toString(root);

  std::set<std::string> expectedLines{
    "  \"int 1\": 0",
    "  \"<object//>\": {",
    "  }",
    "  \"str:base\": \"simple text\"",
    "  \"str:empty\": \"\"",
    "  \"bool//\": true",
    "  \"int2 \": -42",
    "  \"str\\\"lines \": \"\ttext value\\nwith lines...\\n\"",
    "  \"<array>\": [",
    "    7",
    "  ]",
    "  \" my 'string' !\": \"string text\""
  };
  size_t start = 0, end = 0, count = 0;
  while ((end = result.find_first_of('\n', start)) != std::string::npos) {
    std::string part = result.substr(start, end - start);
    if (!part.empty()) {
      if (start == 0) {
        EXPECT_EQ(std::string("{"), part);
      }
      else if (end == result.size() - 1) {
        EXPECT_EQ(std::string("}"), part);
      }
      else {
        if (part.back() != '{' && part.back() != '[' 
        && result[end + 1] != '}' && result[end + 3] != '}' && result[end + 3] != ']') {
          EXPECT_EQ(',', part.back());
        }
        auto query = part;
        if (query.back() == ',')
          query.pop_back();

        bool isFound = expectedLines.find(query) != expectedLines.end();
        EXPECT_TRUE(isFound);
        if (isFound)
          printf("FOUND: %s\n", part.c_str());
        else
          printf("NOT FOUND: %s\n", part.c_str());
        ++count;
      }
    }
    start = end + 1;
  }
  EXPECT_EQ(expectedLines.size(), count);
}

TEST_F(JsonSerializerTest, toJsonBaseTypesCommentsTest) {
  SerializableValue::Object root;
  JsonSerializer serializer(size_t{ 2u });

  root["val"] = SerializableValue(0);
  root.at("val").setComment("comment 1");
  auto result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": 0 // comment 1\n}\n"), result);
  root["val"] = SerializableValue(-42);
  root.at("val").setComment(" other comment \":! ");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": -42 //  other comment \":! \n}\n"), result);

  root["val"] = SerializableValue(-76.127);
  root.at("val").setComment("[comment]");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": -76.127"), result.substr(0, 18));
  EXPECT_EQ(std::string("// [comment]\n}\n"), result.substr(result.size()-15u));
  EXPECT_EQ('\n', result.back());

  root["val"] = SerializableValue(true);
  root.at("val").setComment("comment\nwith\nlines\n");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": true // comment\n  // with\n  // lines\n\n}\n"), result);
  root["val"] = SerializableValue(false);
  root.at("val").setComment(nullptr);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": false\n}\n"), result);

  root["val"] = SerializableValue("simple text");
  root.at("val").setComment("//comment");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": \"simple text\" // //comment\n}\n"), result);
  root["val"] = SerializableValue("\ttext value\nwith lines...\r\n");
  root.at("val").setComment(" ");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": \"\ttext value\\nwith lines...\\r\\n\" //  \n}\n"), result);
  root["val"] = SerializableValue(std::string("string text"));
  root.at("val").setComment("\"abc\"");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": \"string text\" // \"abc\"\n}\n"), result);

  root["val"] = SerializableValue(SerializableValue::Array{});
  root.at("val").setComment("'abc'");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": [\n    // 'abc'\n  ]\n}\n"), result);
  root["val"] = SerializableValue(SerializableValue::Array{ SerializableValue(7) });
  root.at("val").setComment("text\nwith\nlines");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": [\n    // text\n    // with\n    // lines\n    7\n  ]\n}\n"), result);

  root["val"] = SerializableValue(SerializableValue::Object{});
  root.at("val").setComment("text\nwith\nlines");
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": {\n    // text\n    // with\n    // lines\n  }\n}\n"), result);

  root = {};
  root["int 1"] = SerializableValue(0);
  root.at("int 1").setComment(" ");
  root["<object;>"] = SerializableValue(SerializableValue::Object{});
  root.at("<object;>").setComment("comment !;");
  root["str=base"] = SerializableValue("simple text");
  root["str=empty"] = SerializableValue("");
  root.at("str=empty").setComment("{comment}");
  root["bool;"] = SerializableValue(true);
  root.at("bool;").setComment(": val");
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
    "  \"int 1\": 0 //  ",
    "  \"<object;>\": {",
    "    // comment !;",
    "  }",
    "  \"bool;\": true // : val",
    "  \"str=base\": \"simple text\"",
    "  \"str=empty\": \"\" // {comment}",
    "  \"int2 \": -42 // multiple",
    "  // lines",
    "  \"str#lines \": \"\ttext value\\nwith lines...\\n\" //  OK ",
    "  \"<array>\": [",
    "    // single line ",
    "    7 // inlined !",
    "  ]",
    "  \" my [string] !\": \"string text\""
  };
  size_t start = 0, end = 0, count = 0;
  while ((end = result.find_first_of('\n', start)) != std::string::npos) {
    std::string part = result.substr(start, end - start);
    if (!part.empty()) {
      if (start == 0) {
        EXPECT_EQ(std::string("{"), part);
      }
      else if (end == result.size() - 1) {
        EXPECT_EQ(std::string("}"), part);
      }
      else {
        size_t commaPos = part.find_first_of(',', 0);
        if (part.back() != '{' && part.back() != '[' && part[2] != '/' 
        && (part.size() < 5 || part[4] != '/') && result[end + 3] != '/'
        && result[end + 1] != '}' && result[end + 3] != '}' && result[end + 3] != ']') {
          EXPECT_TRUE(commaPos != std::string::npos);
          size_t commentPos = part.find_first_of('/', 0);
          if (commentPos != std::string::npos) {
            EXPECT_TRUE(commaPos < commentPos);
          }
        }
        auto query = part;
        if (commaPos != std::string::npos)
          query.erase(commaPos, (size_t)1);

        bool isFound = expectedLines.find(query) != expectedLines.end();
        EXPECT_TRUE(isFound);
        if (isFound)
          printf("FOUND: %s\n", part.c_str());
        else
          printf("NOT FOUND: %s\n", part.c_str());
        ++count;
      }
    }
    start = end + 1;
  }
  EXPECT_EQ(expectedLines.size(), count);
}

TEST_F(JsonSerializerTest, toJsonAllTypesTest) {
  SerializableValue::Object root;
  root["int 1"] = SerializableValue(999);
  root["str:base"] = SerializableValue("simple text");
  root["str:empty"] = SerializableValue("");
  root["bool"] = SerializableValue(true);
  root.at("bool").setComment("always true");
  root["int2 "] = SerializableValue(-42);
  root.at("int2 ").setComment("one\ntwo");
  root["str:lines "] = SerializableValue("\ttext value\nwith lines...\n");
  root["[array]"] = SerializableValue(SerializableValue::Array{ SerializableValue("array text"), SerializableValue(7), 
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

  SerializableValue::Object section4;
  section4["array1"] = SerializableValue(SerializableValue::Array{ 
    SerializableValue::Array{ SerializableValue(0), SerializableValue(7), SerializableValue(42) }, 
    SerializableValue::Array{ SerializableValue(-42), SerializableValue(-7) } });
  section4["array2"] = SerializableValue(SerializableValue::Array{ 
    SerializableValue::Array{ SerializableValue("0"), SerializableValue("7"), SerializableValue("42") }, 
    SerializableValue::Array{ SerializableValue("-42"), SerializableValue("-7") } });
  SerializableValue::Object subObj1;
  subObj1["abc"] = SerializableValue("def");
  subObj1["def"] = SerializableValue("ghi");
  section4["object1"] = std::move(subObj1);
  SerializableValue::Object subObj2;
  subObj2["Tele"] = SerializableValue(52);
  subObj2["Strat"] = SerializableValue(57);
  subObj2["SG"] = SerializableValue(61);
  SerializableValue::Object otherSubObj;
  otherSubObj["a"] = SerializableValue("a");
  otherSubObj["b"] = SerializableValue("b");
  subObj2["other"] = SerializableValue(SerializableValue::Array{ SerializableValue("LP"), SerializableValue("Flying-V"), SerializableValue("Explorer"), std::move(otherSubObj) });
  section4["object2"] = std::move(subObj2);
  root["sec-4{}"] = std::move(section4);

  JsonSerializer serializer(size_t{ 2u });
  auto result = serializer.toString(root);

  std::set<std::string> expectedLines{
    "  \"int 1\": 999",
    "  \"bool\": true // always true",
    "  \"str:empty\": \"\"",
    "  \"str:base\": \"simple text\"",
    "  \"int2 \": -42 // one",
    "  // two",
    "  \"sec 1\": {",
    "    // this section has a title...",
    "    \"sub-int 1\": 0",
    "    \"sub-str 1\": \"0\" // dummy",
    "    \"my list1\": [",
    "      // here comes the fun",
    "      // be ready!",
    "      0",
    "      7",
    "      42",
    "      -42",
    "    ]",
    "    \"my list 2\": [",
    "      \"0\"",
    "      \"7\"",
    "      \"42\"",
    "      \"-42\"",
    "    ]",
    "  }",
    "  \"str:lines \": \"\ttext value\\nwith lines...\\n\"",
    "  \"[array]\": [",
    "    \"array text\"",
    "    7",
    "    -42",
    "    true",
    "  ]",
    "  \"sec 2 (empty)\": {",
    "  }",
    "  \"sec-3\": {",
    "    \"isOk\": false",
    "    \"count\": 57",
    "    \"name\": \"alpha\"",
    "  }",
    "  \"sec-4{}\": {",
    "    \"array1\": [",
    "      [",
    "        0",
    "        7",
    "        42",
    "      ]",
    "      [",
    "        -42",
    "        -7",
    "      ]",
    "    ]",
    "    \"array2\": [",
    "      [",
    "        \"0\"",
    "        \"7\"",
    "        \"42\"",
    "      ]",
    "      [",
    "        \"-42\"",
    "        \"-7\"",
    "      ]",
    "    ]",
    "    \"object1\": {",
    "      \"abc\": \"def\"",
    "      \"def\": \"ghi\"",
    "    }",
    "    \"object2\": {",
    "      \"Tele\": 52",
    "      \"Strat\": 57",
    "      \"SG\": 61",
    "      \"other\": [",
    "        \"LP\"",
    "        \"Flying-V\"",
    "        \"Explorer\"",
    "        {",
    "          \"a\": \"a\"",
    "          \"b\": \"b\"",
    "        }",
    "      ]",
    "    }",
    "  }"
  };
  size_t start = 0, end = 0;
  while ((end = result.find_first_of('\n', start)) != std::string::npos) {
    std::string part = result.substr(start, end - start);
    if (!part.empty()) {
      if (start == 0) {
        EXPECT_EQ(std::string("{"), part);
      }
      else if (end == result.size() - 1) {
        EXPECT_EQ(std::string("}"), part);
      }
      else {
        size_t commaPos = part.find_first_of(',', 0);
        if (part.back() != '{' && part.back() != '[' && part[2] != '/' 
        && (part.size() < 5 || part[4] != '/') && (part.size() < 7 || part[6] != '/')
        && result[end + 1] != '}' && result[end + 3] != '}' && result[end + 3] != ']' && result[end + 3] != '/'
        && (end + 5 >= result.size() || (result[end + 5] != '}' && result[end + 5] != ']')) 
        && (end + 7 >= result.size() || (result[end + 7] != '}' && result[end + 7] != ']'))
        && (end + 9 >= result.size() || (result[end + 9] != '}' && result[end + 9] != ']'))) {
          EXPECT_TRUE(commaPos != std::string::npos);
          size_t commentPos = part.find_first_of('/', 0);
          if (commentPos != std::string::npos) {
            EXPECT_TRUE(commaPos < commentPos);
          }
        }
        std::string query = part;
        if (commaPos != std::string::npos)
          query.erase(commaPos, (size_t)1);

        bool isFound = expectedLines.find(query) != expectedLines.end();
        EXPECT_TRUE(isFound);
        if (isFound)
          printf("FOUND: %s\n", part.c_str());
        else
          printf("NOT FOUND: %s\n", part.c_str());
      }
    }
    start = end + 1;
  }
  // no size check: many duplicates (}, ], ...) -> set contains less entries
}


// -- JSON deserializer --

TEST_F(JsonSerializerTest, fromJsonEmptyStringTest) {
  JsonSerializer serializer;
  auto result = serializer.fromString(nullptr);
  EXPECT_TRUE(result.empty());
  result = serializer.fromString("");
  EXPECT_TRUE(result.empty());
  result = serializer.fromString("   \r\n\t  //dummy\n  ");
  EXPECT_TRUE(result.empty());
}

TEST_F(JsonSerializerTest, fromJsonInvalidTest) {
  JsonSerializer serializer;
  const char* raw1 = "{\n  \"val\": 0, \"val2\": 1\n";
  EXPECT_ANY_THROW(serializer.fromString(raw1));
  const char* raw2 = "{\n  \"val\": 0 \"val2\": 1\n}\n";
  EXPECT_ANY_THROW(serializer.fromString(raw2));
  const char* raw3 = "{\n  \"val\": [ 0, \"val2\": 1\n}\n";
  EXPECT_ANY_THROW(serializer.fromString(raw3));
  const char* raw4 = "{\n  \"val\": 0, val2: 1\n}\n";
  EXPECT_ANY_THROW(serializer.fromString(raw4));
  const char* raw5 = "{\n  \"val\"= 0, \"val2\"= 1\n}\n";
  EXPECT_ANY_THROW(serializer.fromString(raw5));
  const char* raw6 = "\"val\": 0,\n\"val2\": 1\n";
  EXPECT_ANY_THROW(serializer.fromString(raw6));
}

TEST_F(JsonSerializerTest, fromJsonCommentAtEndTest) {
  JsonSerializer serializer;

  const char* raw1 = "{\n  \"bool\": true // always true\n}\n";
  auto result = serializer.fromString(raw1);
  EXPECT_EQ((size_t)1, result.size());
  ASSERT_TRUE(result.find("bool") != result.end());
  EXPECT_EQ(SerializableValue::Type::boolean, result.at("bool").type());
  EXPECT_EQ(true, result.at("bool").getBoolean());

  const char* raw2 = "{\n  \"bool\": true\n}// always true";
  result = serializer.fromString(raw2);
  EXPECT_EQ((size_t)1, result.size());
  ASSERT_TRUE(result.find("bool") != result.end());
  EXPECT_EQ(SerializableValue::Type::boolean, result.at("bool").type());
  EXPECT_EQ(true, result.at("bool").getBoolean());
}

TEST_F(JsonSerializerTest, fromJsonTypesTest) {
  const char* raw =
    "{\n"
    "  /* multi-line\n"
    "     comment */\n"
    "  \"int1\" : 0,\n"
    "  \"int\\\"2\\\"\":-25, // comment\n"
    "  \"nb 1\": 57.252,\n"
    "  \" nb2 \": -0.5  , \n"
    "  \"{nb_[3]}\": 0.0, \n"
    "  \"bool:1\": true ,//comment\n"
    "  \"-bool-2-\": false,\n"
    "  \"MyString1\": 'test: \"string\"',\n"
    "  \"My\\\"String\\\"2\": \"this is a \\\"string\\\"\\r\\nwith 'lines'...\" ,\n"
    "  \"MyString:3\": \"\",//comment\n"
    "  \"<array-empty>\": [],\r\n"
    "  \"<array-empty2>\": [//comment\r\n"
    "  ] , //comment\n"
    "  \"<array1>\": [\n"
    "    0,1,2,3\n"
    "  ],//comment\n"
    "  \"<array2>\": [\n"
    "    \"0\",\n"
    "    \"1\",\n"
    "    {\"two\":\"2\"},\n"
    "    \"3\",\n"
    "  ],\n"
    "  \"obj-empty1\":{} ,\n"
    "  \"obj-empty2\" :{   },\n"
    "  \"obj-empty3\":{\n"
    "  },\n"
    "  \"Obj1\":{\n"
    "    \"abc\": \"\", \n"
    "    \"def\": \"1\", \n"
    "    \"ghi\": 1, \n"
    "  } ,\n"
    "  \"obj2\" : {\n"
    "    \"zero-str\\a\": \"0\", \n"
    "    \"arr\": [ \n"
    "      \"0\",  \n"
    "    ],  \n"
    "    // comment line\n"
    "    \"zero\": 0 // comment -- // comment --\n"
    "    // comment line\n"
    "  }\n"
    "}\n";

  JsonSerializer serializer;
  auto result = serializer.fromString(raw);

  EXPECT_EQ((size_t)19, result.size());

  ASSERT_TRUE(result.find("int1") != result.end());
  EXPECT_EQ(SerializableValue::Type::integer, result.at("int1").type());
  EXPECT_EQ((int32_t)0, result.at("int1").getInteger());
  ASSERT_TRUE(result.find("int\"2\"") != result.end());
  EXPECT_EQ(SerializableValue::Type::integer, result.at("int\"2\"").type());
  EXPECT_EQ((int32_t)-25, result.at("int\"2\"").getInteger());
  ASSERT_TRUE(result.find("nb 1") != result.end());
  EXPECT_EQ(SerializableValue::Type::number, result.at("nb 1").type());
  EXPECT_EQ(57.252, result.at("nb 1").getNumber());
  ASSERT_TRUE(result.find(" nb2 ") != result.end());
  EXPECT_EQ(SerializableValue::Type::number, result.at(" nb2 ").type());
  EXPECT_EQ(-0.5, result.at(" nb2 ").getNumber());
  ASSERT_TRUE(result.find("{nb_[3]}") != result.end());
  EXPECT_EQ(SerializableValue::Type::number, result.at("{nb_[3]}").type());
  EXPECT_EQ(0.0, result.at("{nb_[3]}").getNumber());
  ASSERT_TRUE(result.find("bool:1") != result.end());
  EXPECT_EQ(SerializableValue::Type::boolean, result.at("bool:1").type());
  EXPECT_EQ(true, result.at("bool:1").getBoolean());
  ASSERT_TRUE(result.find("-bool-2-") != result.end());
  EXPECT_EQ(SerializableValue::Type::boolean, result.at("-bool-2-").type());
  EXPECT_EQ(false, result.at("-bool-2-").getBoolean());
  ASSERT_TRUE(result.find("MyString1") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("MyString1").type());
  EXPECT_STREQ("test: \"string\"", result.at("MyString1").getText());
  ASSERT_TRUE(result.find("My\"String\"2") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("My\"String\"2").type());
  EXPECT_STREQ("this is a \"string\"\r\nwith 'lines'...", result.at("My\"String\"2").getText());
  ASSERT_TRUE(result.find("MyString:3") != result.end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("MyString:3").type());
  EXPECT_EQ((size_t)0, result.at("MyString:3").size());
  EXPECT_TRUE(result.at("MyString:3").getText() == nullptr);

  ASSERT_TRUE(result.find("<array-empty>") != result.end());
  EXPECT_EQ(SerializableValue::Type::arrays, result.at("<array-empty>").type());
  EXPECT_EQ((size_t)0, result.at("<array-empty>").size());
  EXPECT_TRUE(result.at("<array-empty>").getArray() == nullptr);
  ASSERT_TRUE(result.find("<array-empty2>") != result.end());
  EXPECT_EQ(SerializableValue::Type::arrays, result.at("<array-empty2>").type());
  EXPECT_EQ((size_t)0, result.at("<array-empty2>").size());
  EXPECT_TRUE(result.at("<array-empty2>").getArray() == nullptr);
  ASSERT_TRUE(result.find("obj-empty1") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("obj-empty1").type());
  EXPECT_EQ((size_t)0, result.at("obj-empty1").size());
  EXPECT_TRUE(result.at("obj-empty1").getObject() == nullptr);
  ASSERT_TRUE(result.find("obj-empty2") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("obj-empty2").type());
  EXPECT_EQ((size_t)0, result.at("obj-empty2").size());
  EXPECT_TRUE(result.at("obj-empty2").getObject() == nullptr);
  ASSERT_TRUE(result.find("obj-empty3") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("obj-empty3").type());
  EXPECT_EQ((size_t)0, result.at("obj-empty3").size());
  EXPECT_TRUE(result.at("obj-empty3").getObject() == nullptr);

  ASSERT_TRUE(result.find("<array1>") != result.end());
  EXPECT_EQ(SerializableValue::Type::arrays, result.at("<array1>").type());
  EXPECT_EQ((size_t)4, result.at("<array1>").size());
  EXPECT_TRUE(result.at("<array1>").getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::integer, (*result.at("<array1>").getArray())[0].type());
  EXPECT_EQ((int32_t)0, (*result.at("<array1>").getArray())[0].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*result.at("<array1>").getArray())[1].type());
  EXPECT_EQ((int32_t)1, (*result.at("<array1>").getArray())[1].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*result.at("<array1>").getArray())[2].type());
  EXPECT_EQ((int32_t)2, (*result.at("<array1>").getArray())[2].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*result.at("<array1>").getArray())[3].type());
  EXPECT_EQ((int32_t)3, (*result.at("<array1>").getArray())[3].getInteger());

  ASSERT_TRUE(result.find("<array2>") != result.end());
  EXPECT_EQ(SerializableValue::Type::arrays, result.at("<array2>").type());
  EXPECT_EQ((size_t)4, result.at("<array2>").size());
  EXPECT_TRUE(result.at("<array2>").getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::text, (*result.at("<array2>").getArray())[0].type());
  EXPECT_STREQ("0", (*result.at("<array2>").getArray())[0].getText());
  EXPECT_EQ(SerializableValue::Type::text, (*result.at("<array2>").getArray())[1].type());
  EXPECT_STREQ("1", (*result.at("<array2>").getArray())[1].getText());
  EXPECT_EQ(SerializableValue::Type::object, (*result.at("<array2>").getArray())[2].type());
  EXPECT_EQ((size_t)1u, (*result.at("<array2>").getArray())[2].size());
  ASSERT_TRUE((*result.at("<array2>").getArray())[2].getObject() != nullptr);
  ASSERT_TRUE((*result.at("<array2>").getArray())[2].getObject()->find("two") != (*result.at("<array2>").getArray())[2].getObject()->end());
  EXPECT_EQ(SerializableValue::Type::text, (*result.at("<array2>").getArray())[2].getObject()->at("two").type());
  EXPECT_STREQ("2", (*result.at("<array2>").getArray())[2].getObject()->at("two").getText());
  EXPECT_EQ(SerializableValue::Type::text, (*result.at("<array2>").getArray())[3].type());
  EXPECT_STREQ("3", (*result.at("<array2>").getArray())[3].getText());

  ASSERT_TRUE(result.find("Obj1") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("Obj1").type());
  EXPECT_EQ((size_t)3, result.at("Obj1").size());
  EXPECT_TRUE(result.at("Obj1").getObject() != nullptr);
  ASSERT_TRUE(result.at("Obj1").getObject()->find("abc") != result.at("Obj1").getObject()->end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("Obj1").getObject()->at("abc").type());
  EXPECT_TRUE(result.at("Obj1").getObject()->at("abc").getText() == nullptr);
  ASSERT_TRUE(result.at("Obj1").getObject()->find("def") != result.at("Obj1").getObject()->end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("Obj1").getObject()->at("def").type());
  EXPECT_STREQ("1", result.at("Obj1").getObject()->at("def").getText());
  ASSERT_TRUE(result.at("Obj1").getObject()->find("ghi") != result.at("Obj1").getObject()->end());
  EXPECT_EQ(SerializableValue::Type::integer, result.at("Obj1").getObject()->at("ghi").type());
  EXPECT_EQ((int32_t)1, result.at("Obj1").getObject()->at("ghi").getInteger());

  ASSERT_TRUE(result.find("obj2") != result.end());
  EXPECT_EQ(SerializableValue::Type::object, result.at("obj2").type());
  EXPECT_EQ((size_t)3, result.at("obj2").size());
  EXPECT_TRUE(result.at("obj2").getObject() != nullptr);
  ASSERT_TRUE(result.at("obj2").getObject()->find("zero-str\a") != result.at("obj2").getObject()->end());
  EXPECT_EQ(SerializableValue::Type::text, result.at("obj2").getObject()->at("zero-str\a").type());
  EXPECT_STREQ("0", result.at("obj2").getObject()->at("zero-str\a").getText());
  ASSERT_TRUE(result.at("obj2").getObject()->find("arr") != result.at("obj2").getObject()->end());
  EXPECT_EQ(SerializableValue::Type::arrays, result.at("obj2").getObject()->at("arr").type());
  EXPECT_EQ((size_t)1u, result.at("obj2").getObject()->at("arr").size());
  ASSERT_TRUE(result.at("obj2").getObject()->at("arr").getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::text, (*result.at("obj2").getObject()->at("arr").getArray())[0].type());
  EXPECT_STREQ("0", (*result.at("obj2").getObject()->at("arr").getArray())[0].getText());
  ASSERT_TRUE(result.at("obj2").getObject()->find("zero") != result.at("obj2").getObject()->end());
  EXPECT_EQ(SerializableValue::Type::integer, result.at("obj2").getObject()->at("zero").type());
  EXPECT_EQ((int32_t)0, result.at("obj2").getObject()->at("zero").getInteger());
}


// -- JSON serializer/deserializer --

TEST_F(JsonSerializerTest, toFromJsonTest) {
  // data to serialize
  SerializableValue::Object root;
  root["int 1"] = SerializableValue(999);
  root["str:base"] = SerializableValue("simple text");
  root["str:empty"] = SerializableValue("");
  root["bool"] = SerializableValue(true);
  root.at("bool").setComment("always true");
  root["int2 "] = SerializableValue(-42);
  root.at("int2 ").setComment("one\ntwo");
  root["str:lines "] = SerializableValue("\ttext value\nwith lines...\n");
  root["[array]"] = SerializableValue(SerializableValue::Array{ SerializableValue("array text"), SerializableValue(7), 
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

  SerializableValue::Object section4;
  section4["array1"] = SerializableValue(SerializableValue::Array{ 
    SerializableValue::Array{ SerializableValue(0), SerializableValue(7), SerializableValue(42) }, 
    SerializableValue::Array{ SerializableValue(-42), SerializableValue(-7) } });
  section4["array2"] = SerializableValue(SerializableValue::Array{ 
    SerializableValue::Array{ SerializableValue("0"), SerializableValue("7"), SerializableValue("42") }, 
    SerializableValue::Array{ SerializableValue("-42"), SerializableValue("-7") } });
  SerializableValue::Object subObj1;
  subObj1["abc"] = SerializableValue("def");
  subObj1["def"] = SerializableValue("ghi");
  section4["object1"] = std::move(subObj1);
  SerializableValue::Object subObj2;
  subObj2["Tele"] = SerializableValue(52);
  subObj2["Strat"] = SerializableValue(57);
  subObj2["SG"] = SerializableValue(61);
  subObj2["other"] = SerializableValue(SerializableValue::Array{ SerializableValue("LP"), SerializableValue("Flying-V"), SerializableValue("Explorer") });
  section4["object2"] = std::move(subObj2);
  root["sec-4{}"] = std::move(section4);

  // serialize/deserialize
  JsonSerializer serializer(size_t{ 2u });
  std::string serialized = serializer.toString(root);
  try {
    auto result = serializer.fromString(serialized.c_str());

    // verify data
    EXPECT_EQ((size_t)11, result.size());

    ASSERT_TRUE(result.find("int 1") != result.end());
    EXPECT_EQ(SerializableValue::Type::integer, result.at("int 1").type());
    EXPECT_EQ((int32_t)999, result.at("int 1").getInteger());
    ASSERT_TRUE(result.find("str:base") != result.end());
    EXPECT_EQ(SerializableValue::Type::text, result.at("str:base").type());
    EXPECT_STREQ("simple text", result.at("str:base").getText());
    ASSERT_TRUE(result.find("str:empty") != result.end());
    EXPECT_EQ(SerializableValue::Type::text, result.at("str:empty").type());
    EXPECT_TRUE(result.at("str:empty").getText() == nullptr);
    ASSERT_TRUE(result.find("bool") != result.end());
    EXPECT_EQ(SerializableValue::Type::boolean, result.at("bool").type());
    EXPECT_EQ(true, result.at("bool").getBoolean());
    ASSERT_TRUE(result.find("int2 ") != result.end());
    EXPECT_EQ(SerializableValue::Type::integer, result.at("int2 ").type());
    EXPECT_EQ((int32_t)-42, result.at("int2 ").getInteger());
    ASSERT_TRUE(result.find("str:lines ") != result.end());
    EXPECT_EQ(SerializableValue::Type::text, result.at("str:lines ").type());
    EXPECT_STREQ("\ttext value\nwith lines...\n", result.at("str:lines ").getText());

    ASSERT_TRUE(result.find("[array]") != result.end());
    EXPECT_EQ(SerializableValue::Type::arrays, result.at("[array]").type());
    const auto* arrayRef = result.at("[array]").getArray();
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

    ASSERT_TRUE(result.find("sec-4{}") != result.end());
    EXPECT_EQ(SerializableValue::Type::object, result.at("sec-4{}").type());
    EXPECT_EQ((size_t)4u, result.at("sec-4{}").size());
    const auto* sec4Ref = result.at("sec-4{}").getObject();
    ASSERT_TRUE(sec4Ref != nullptr);

    ASSERT_TRUE(sec4Ref->find("array1") != sec4Ref->end());
    EXPECT_EQ(SerializableValue::Type::arrays, sec4Ref->at("array1").type());
    const auto* s4Array1Ref = sec4Ref->at("array1").getArray();
    ASSERT_TRUE(s4Array1Ref != nullptr);
    ASSERT_EQ((size_t)2u, s4Array1Ref->size());
    EXPECT_EQ(SerializableValue::Type::arrays, (*s4Array1Ref)[0].type());
    ASSERT_EQ((size_t)3u, (*s4Array1Ref)[0].size());
    const auto* s4a1a1 = (*s4Array1Ref)[0].getArray();
    ASSERT_TRUE(s4a1a1 != nullptr);
    EXPECT_EQ(SerializableValue::Type::integer, (*s4a1a1)[0].type());
    EXPECT_EQ((int32_t)0, (*s4a1a1)[0].getInteger());
    EXPECT_EQ(SerializableValue::Type::integer, (*s4a1a1)[1].type());
    EXPECT_EQ((int32_t)7, (*s4a1a1)[1].getInteger());
    EXPECT_EQ(SerializableValue::Type::integer, (*s4a1a1)[2].type());
    EXPECT_EQ((int32_t)42, (*s4a1a1)[2].getInteger());
    EXPECT_EQ(SerializableValue::Type::arrays, (*s4Array1Ref)[1].type());
    ASSERT_EQ((size_t)2u, (*s4Array1Ref)[1].size());
    const auto* s4a1a2 = (*s4Array1Ref)[1].getArray();
    ASSERT_TRUE(s4a1a2 != nullptr);
    EXPECT_EQ(SerializableValue::Type::integer, (*s4a1a2)[0].type());
    EXPECT_EQ((int32_t)-42, (*s4a1a2)[0].getInteger());
    EXPECT_EQ(SerializableValue::Type::integer, (*s4a1a2)[1].type());
    EXPECT_EQ((int32_t)-7, (*s4a1a2)[1].getInteger());

    ASSERT_TRUE(sec4Ref->find("array2") != sec4Ref->end());
    EXPECT_EQ(SerializableValue::Type::arrays, sec4Ref->at("array2").type());
    const auto* s4Array2Ref = sec4Ref->at("array2").getArray();
    ASSERT_TRUE(s4Array2Ref != nullptr);
    ASSERT_EQ((size_t)2u, s4Array2Ref->size());
    EXPECT_EQ(SerializableValue::Type::arrays, (*s4Array2Ref)[0].type());
    ASSERT_EQ((size_t)3u, (*s4Array2Ref)[0].size());
    const auto* s4a2a1 = (*s4Array2Ref)[0].getArray();
    ASSERT_TRUE(s4a2a1 != nullptr);
    EXPECT_EQ(SerializableValue::Type::text, (*s4a2a1)[0].type());
    EXPECT_STREQ("0", (*s4a2a1)[0].getText());
    EXPECT_EQ(SerializableValue::Type::text, (*s4a2a1)[1].type());
    EXPECT_STREQ("7", (*s4a2a1)[1].getText());
    EXPECT_EQ(SerializableValue::Type::text, (*s4a2a1)[2].type());
    EXPECT_STREQ("42", (*s4a2a1)[2].getText());
    EXPECT_EQ(SerializableValue::Type::arrays, (*s4Array2Ref)[1].type());
    ASSERT_EQ((size_t)2u, (*s4Array2Ref)[1].size());
    const auto* s4a2a2 = (*s4Array2Ref)[1].getArray();
    ASSERT_TRUE(s4a2a2 != nullptr);
    EXPECT_EQ(SerializableValue::Type::text, (*s4a2a2)[0].type());
    EXPECT_STREQ("-42", (*s4a2a2)[0].getText());
    EXPECT_EQ(SerializableValue::Type::text, (*s4a2a2)[1].type());
    EXPECT_STREQ("-7", (*s4a2a2)[1].getText());

    ASSERT_TRUE(sec4Ref->find("object1") != sec4Ref->end());
    EXPECT_EQ(SerializableValue::Type::object, sec4Ref->at("object1").type());
    const auto* s4o1 = sec4Ref->at("object1").getObject();
    ASSERT_TRUE(s4o1 != nullptr);
    ASSERT_EQ((size_t)2u, s4o1->size());
    ASSERT_TRUE(s4o1->find("abc") != s4o1->end());
    EXPECT_EQ(SerializableValue::Type::text, s4o1->at("abc").type());
    EXPECT_STREQ("def", s4o1->at("abc").getText());
    ASSERT_TRUE(s4o1->find("def") != s4o1->end());
    EXPECT_EQ(SerializableValue::Type::text, s4o1->at("def").type());
    EXPECT_STREQ("ghi", s4o1->at("def").getText());

    ASSERT_TRUE(sec4Ref->find("object2") != sec4Ref->end());
    EXPECT_EQ(SerializableValue::Type::object, sec4Ref->at("object2").type());
    const auto* s4o2 = sec4Ref->at("object2").getObject();
    ASSERT_TRUE(s4o2 != nullptr);
    ASSERT_EQ((size_t)4u, s4o2->size());
    ASSERT_TRUE(s4o2->find("Tele") != s4o2->end());
    EXPECT_EQ(SerializableValue::Type::integer, s4o2->at("Tele").type());
    EXPECT_EQ((int32_t)52, s4o2->at("Tele").getInteger());
    ASSERT_TRUE(s4o2->find("Strat") != s4o2->end());
    EXPECT_EQ(SerializableValue::Type::integer, s4o2->at("Strat").type());
    EXPECT_EQ((int32_t)57, s4o2->at("Strat").getInteger());
    ASSERT_TRUE(s4o2->find("SG") != s4o2->end());
    EXPECT_EQ(SerializableValue::Type::integer, s4o2->at("SG").type());
    EXPECT_EQ((int32_t)61, s4o2->at("SG").getInteger());
    ASSERT_TRUE(s4o2->find("other") != s4o2->end());
    EXPECT_EQ(SerializableValue::Type::arrays, s4o2->at("other").type());
    const auto* s4o2a1 = s4o2->at("other").getArray();
    ASSERT_TRUE(s4o2a1 != nullptr);
    EXPECT_EQ(SerializableValue::Type::text, (*s4o2a1)[0].type());
    EXPECT_STREQ("LP", (*s4o2a1)[0].getText());
    EXPECT_EQ(SerializableValue::Type::text, (*s4o2a1)[1].type());
    EXPECT_STREQ("Flying-V", (*s4o2a1)[1].getText());
    EXPECT_EQ(SerializableValue::Type::text, (*s4o2a1)[2].type());
    EXPECT_STREQ("Explorer", (*s4o2a1)[2].getText());
  }
  catch (const std::exception& exc) {
    printf("VALUE:\n%s\n", serialized.c_str());
    ASSERT_STREQ("no error", exc.what());
  }
}
