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

  root["val"] = SerializableValue(0);
  auto result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": 0\n}\n"), result);
  root["val"] = SerializableValue(-42);
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": -42\n}\n"), result);

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
  root["val"] = SerializableValue(std::string("string text"));
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": \"string text\"\n}\n"), result);

  root["val"] = SerializableValue(SerializableValue::Array{});
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": [\n  ]\n}\n"), result);
  root["val"] = SerializableValue(SerializableValue::Array{ SerializableValue(7) });
  result = serializer.toString(root);
  EXPECT_EQ(std::string("{\n  \"val\": [\n    7\n  ]\n}\n"), result);

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
        if (part.back() == ',')
          part.pop_back();

        bool isFound = expectedLines.find(part) != expectedLines.end();
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
        if (part.back() != '{' && part.back() != '[' && part[2] != '/' && (part.size() < 5 || part[4] != '/')
        && result[end + 1] != '}' && result[end + 3] != '}' && result[end + 3] != ']') {
          EXPECT_TRUE(commaPos != std::string::npos);
          size_t commentPos = part.find_first_of('/', 0);
          if (commentPos != std::string::npos) {
            EXPECT_TRUE(commaPos < commentPos);
          }
        }
        if (commaPos != std::string::npos)
          part.erase(commaPos, (size_t)1);

        bool isFound = expectedLines.find(part) != expectedLines.end();
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
  subObj2["other"] = SerializableValue(SerializableValue::Array{ SerializableValue("LP"), SerializableValue("Flying-V"), SerializableValue("Explorer") });
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
        && result[end + 1] != '}' && result[end + 3] != '}' && result[end + 3] != ']' 
        && (end + 5 >= result.size() || (result[end + 5] != '}' && result[end + 5] != ']')) 
        && (end + 7 >= result.size() || (result[end + 7] != '}' && result[end + 7] != ']'))) {
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

}

TEST_F(JsonSerializerTest, fromJsonTypesCommSectionsTest) {
  // avec et sans { } autour de tout
}

TEST_F(JsonSerializerTest, fromJsonNoSectionTest) {

}

TEST_F(JsonSerializerTest, fromJsonMainPropsAndSectionTest) {

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
  subObj2["other"] = SerializableValue(SerializableValue::Array{ SerializableValue("LP"), SerializableValue("Flying-V"), SerializableValue("Explorer") });
  section4["object2"] = std::move(subObj2);
  root["sec-4{}"] = std::move(section4);

  // serialize/deserialize
  JsonSerializer serializer(size_t{ 2u });
  std::string serialized = serializer.toString(root);
  auto result = serializer.fromString(serialized.c_str());

  // verify data
  EXPECT_EQ((size_t)11, result.size());

  //...
}
