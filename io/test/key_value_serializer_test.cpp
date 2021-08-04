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
#include <io/key_value_serializer.h>

using namespace pandora::io;

class KeyValueSerializerTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- SerializableValue --

TEST_F(KeyValueSerializerTest, emptyValueTest) {
  SerializableValue empty;
  EXPECT_EQ(SerializableValue::Type::integer, empty.type());
  EXPECT_EQ((int32_t)0, empty.getInteger());
  EXPECT_TRUE(empty.comment() == nullptr);
}

TEST_F(KeyValueSerializerTest, commentTest) {
  SerializableValue str("abc");
  EXPECT_EQ(SerializableValue::Type::text, str.type());
  EXPECT_EQ((size_t)3, str.size());
  EXPECT_STREQ("abc", str.getText());
  EXPECT_TRUE(str.comment() == nullptr);
  
  str.setComment(nullptr);
  EXPECT_EQ(SerializableValue::Type::text, str.type());
  EXPECT_EQ((size_t)3, str.size());
  EXPECT_STREQ("abc", str.getText());
  EXPECT_TRUE(str.comment() == nullptr);
  
  str.setComment("");
  EXPECT_EQ(SerializableValue::Type::text, str.type());
  EXPECT_EQ((size_t)3, str.size());
  EXPECT_STREQ("abc", str.getText());
  EXPECT_TRUE(str.comment() == nullptr);
  
  str.setComment("my first comment");
  EXPECT_EQ(SerializableValue::Type::text, str.type());
  EXPECT_EQ((size_t)3, str.size());
  EXPECT_STREQ("abc", str.getText());
  ASSERT_TRUE(str.comment() != nullptr);
  EXPECT_STREQ("my first comment", str.comment());
  
  str.setComment(nullptr);
  EXPECT_EQ(SerializableValue::Type::text, str.type());
  EXPECT_EQ((size_t)3, str.size());
  EXPECT_STREQ("abc", str.getText());
  EXPECT_TRUE(str.comment() == nullptr);
}

TEST_F(KeyValueSerializerTest, typedValueTest) {
  SerializableValue integer(0);
  EXPECT_EQ(SerializableValue::Type::integer, integer.type());
  EXPECT_EQ((size_t)1, integer.size());
  EXPECT_EQ((int32_t)0, integer.getInteger());
  EXPECT_FALSE(integer.empty());
  EXPECT_TRUE(integer);
  EXPECT_TRUE(integer.comment() == nullptr);
  SerializableValue integer2(42);
  EXPECT_EQ(SerializableValue::Type::integer, integer2.type());
  EXPECT_EQ((size_t)1, integer2.size());
  EXPECT_EQ((int32_t)42, integer2.getInteger());
  EXPECT_FALSE(integer2.empty());
  EXPECT_TRUE(integer2);
  EXPECT_TRUE(integer2.comment() == nullptr);
  SerializableValue integer3(-5210);
  EXPECT_EQ(SerializableValue::Type::integer, integer3.type());
  EXPECT_EQ((size_t)1, integer3.size());
  EXPECT_EQ((int32_t)-5210, integer3.getInteger());
  EXPECT_FALSE(integer3.empty());
  EXPECT_TRUE(integer3);
  EXPECT_TRUE(integer3.comment() == nullptr);
  EXPECT_TRUE(integer2 == integer2);
  EXPECT_TRUE(integer2 != integer3);
  
  SerializableValue number(0.0);
  EXPECT_EQ(SerializableValue::Type::number, number.type());
  EXPECT_EQ((size_t)1, number.size());
  EXPECT_EQ(0.0, number.getNumber());
  EXPECT_FALSE(number.empty());
  EXPECT_TRUE(number);
  EXPECT_TRUE(number.comment() == nullptr);
  SerializableValue number2(580.027);
  EXPECT_EQ(SerializableValue::Type::number, number2.type());
  EXPECT_EQ((size_t)1, number2.size());
  EXPECT_EQ(580.027, number2.getNumber());
  EXPECT_FALSE(number2.empty());
  EXPECT_TRUE(number2);
  EXPECT_TRUE(number2.comment() == nullptr);
  SerializableValue number3(-0.5);
  EXPECT_EQ(SerializableValue::Type::number, number3.type());
  EXPECT_EQ((size_t)1, number3.size());
  EXPECT_EQ(-0.5, number3.getNumber());
  EXPECT_FALSE(number3.empty());
  EXPECT_TRUE(number3);
  EXPECT_TRUE(number3.comment() == nullptr);
  SerializableValue numberF(0.25f);
  EXPECT_EQ(SerializableValue::Type::number, numberF.type());
  EXPECT_EQ((size_t)1, numberF.size());
  EXPECT_EQ(0.25, numberF.getNumber());
  EXPECT_FALSE(numberF.empty());
  EXPECT_TRUE(numberF);
  EXPECT_TRUE(numberF.comment() == nullptr);
  EXPECT_TRUE(number2 == number2);
  EXPECT_TRUE(number2 != number3);
  
  SerializableValue bool1(true);
  EXPECT_EQ(SerializableValue::Type::boolean, bool1.type());
  EXPECT_EQ((size_t)1, bool1.size());
  EXPECT_EQ(true, bool1.getBoolean());
  EXPECT_FALSE(bool1.empty());
  EXPECT_TRUE(bool1);
  EXPECT_TRUE(bool1.comment() == nullptr);
  SerializableValue bool2(false);
  EXPECT_EQ(SerializableValue::Type::boolean, bool2.type());
  EXPECT_EQ((size_t)1, bool2.size());
  EXPECT_EQ(false, bool2.getBoolean());
  EXPECT_FALSE(bool2.empty());
  EXPECT_TRUE(bool2);
  EXPECT_TRUE(bool2.comment() == nullptr);
  EXPECT_TRUE(bool2 == bool2);
  EXPECT_TRUE(bool1 != bool2);
  
  SerializableValue str1(nullptr);
  EXPECT_EQ(SerializableValue::Type::text, str1.type());
  EXPECT_EQ((size_t)0, str1.size());
  EXPECT_TRUE(str1.getText() == nullptr);
  EXPECT_TRUE(str1.empty());
  EXPECT_FALSE(str1);
  EXPECT_TRUE(str1.comment() == nullptr);
  SerializableValue str2("");
  EXPECT_EQ(SerializableValue::Type::text, str2.type());
  EXPECT_EQ((size_t)0, str2.size());
  EXPECT_TRUE(str2.getText() == nullptr);
  EXPECT_TRUE(str2.empty());
  EXPECT_FALSE(str2);
  EXPECT_TRUE(str2.comment() == nullptr);
  SerializableValue str3(" ");
  EXPECT_EQ(SerializableValue::Type::text, str3.type());
  EXPECT_EQ((size_t)1, str3.size());
  EXPECT_STREQ(" ", str3.getText());
  EXPECT_FALSE(str3.empty());
  EXPECT_TRUE(str3);
  EXPECT_TRUE(str3.comment() == nullptr);
  SerializableValue str4("@_very-long string !\n with new-lines...");
  EXPECT_EQ(SerializableValue::Type::text, str4.type());
  EXPECT_EQ((size_t)39, str4.size());
  EXPECT_STREQ("@_very-long string !\n with new-lines...", str4.getText());
  EXPECT_FALSE(str4.empty());
  EXPECT_TRUE(str4);
  EXPECT_TRUE(str4.comment() == nullptr);
  EXPECT_TRUE(str3 == str3);
  EXPECT_TRUE(str3 != str4);
  
  SerializableValue str5(std::string{});
  EXPECT_EQ(SerializableValue::Type::text, str5.type());
  EXPECT_EQ((size_t)0, str5.size());
  EXPECT_TRUE(str5.getText() == nullptr);
  EXPECT_TRUE(str5.empty());
  EXPECT_FALSE(str5);
  EXPECT_TRUE(str5.comment() == nullptr);
  SerializableValue str6(std::string(" "));
  EXPECT_EQ(SerializableValue::Type::text, str6.type());
  EXPECT_EQ((size_t)1, str6.size());
  EXPECT_STREQ(" ", str6.getText());
  EXPECT_FALSE(str6.empty());
  EXPECT_TRUE(str6);
  EXPECT_TRUE(str6.comment() == nullptr);
  SerializableValue str7(std::string("@_very-long string !\n with new-lines..."));
  EXPECT_EQ(SerializableValue::Type::text, str7.type());
  EXPECT_EQ((size_t)39, str7.size());
  EXPECT_STREQ("@_very-long string !\n with new-lines...", str7.getText());
  EXPECT_FALSE(str7.empty());
  EXPECT_TRUE(str7);
  EXPECT_TRUE(str7.comment() == nullptr);
  
  SerializableValue::Array sourceArray;
  SerializableValue arr1(std::move(sourceArray));
  EXPECT_EQ(SerializableValue::Type::arrays, arr1.type());
  EXPECT_EQ((size_t)0, arr1.size());
  EXPECT_TRUE(arr1.getArray() == nullptr);
  EXPECT_TRUE(arr1.empty());
  EXPECT_FALSE(arr1);
  EXPECT_TRUE(arr1.comment() == nullptr);
  sourceArray = SerializableValue::Array{ SerializableValue(5) };
  SerializableValue arr2(std::move(sourceArray));
  EXPECT_EQ(SerializableValue::Type::arrays, arr2.type());
  ASSERT_EQ((size_t)1, arr2.size());
  ASSERT_TRUE(arr2.getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::integer, (*arr2.getArray())[0].type());
  EXPECT_EQ((int32_t)5, (*arr2.getArray())[0].getInteger());
  EXPECT_FALSE(arr2.empty());
  EXPECT_TRUE(arr2);
  EXPECT_TRUE(arr2.comment() == nullptr);
  sourceArray = SerializableValue::Array{ SerializableValue(1), SerializableValue(8), SerializableValue(42) };
  SerializableValue arr3(std::move(sourceArray));
  EXPECT_EQ(SerializableValue::Type::arrays, arr3.type());
  ASSERT_EQ((size_t)3, arr3.size());
  ASSERT_TRUE(arr3.getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::integer, (*arr3.getArray())[0].type());
  EXPECT_EQ((int32_t)1, (*arr3.getArray())[0].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*arr3.getArray())[1].type());
  EXPECT_EQ((int32_t)8, (*arr3.getArray())[1].getInteger());
  EXPECT_EQ(SerializableValue::Type::integer, (*arr3.getArray())[2].type());
  EXPECT_EQ((int32_t)42, (*arr3.getArray())[2].getInteger());
  EXPECT_FALSE(arr3.empty());
  EXPECT_TRUE(arr3);
  EXPECT_TRUE(arr3.comment() == nullptr);
  sourceArray = SerializableValue::Array{ SerializableValue(5.7), SerializableValue(42), SerializableValue("test") };
  SerializableValue arr4(std::move(sourceArray));
  EXPECT_EQ(SerializableValue::Type::arrays, arr4.type());
  ASSERT_EQ((size_t)3, arr4.size());
  ASSERT_TRUE(arr4.getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*arr4.getArray())[0].type());
  EXPECT_EQ(5.7, (*arr4.getArray())[0].getNumber());
  EXPECT_EQ(SerializableValue::Type::integer, (*arr4.getArray())[1].type());
  EXPECT_EQ((int32_t)42, (*arr4.getArray())[1].getInteger());
  EXPECT_EQ(SerializableValue::Type::text, (*arr4.getArray())[2].type());
  EXPECT_STREQ("test", (*arr4.getArray())[2].getText());
  EXPECT_FALSE(arr4.empty());
  EXPECT_TRUE(arr4);
  EXPECT_TRUE(arr4.comment() == nullptr);
  EXPECT_TRUE(arr4 == arr4);
  EXPECT_TRUE(arr3 != arr4);
  
  SerializableValue::Object sourceObj;
  SerializableValue obj1(std::move(sourceObj));
  EXPECT_EQ(SerializableValue::Type::object, obj1.type());
  EXPECT_EQ((size_t)0, obj1.size());
  EXPECT_TRUE(obj1.getObject() == nullptr);
  EXPECT_TRUE(obj1.empty());
  EXPECT_FALSE(obj1);
  EXPECT_TRUE(obj1.comment() == nullptr);
  sourceObj = {};
  sourceObj["int1"] = SerializableValue(5);
  SerializableValue obj2(std::move(sourceObj));
  EXPECT_EQ(SerializableValue::Type::object, obj2.type());
  ASSERT_EQ((size_t)1, obj2.size());
  ASSERT_TRUE(obj2.getObject() != nullptr);
  EXPECT_EQ(SerializableValue::Type::integer, (*obj2.getObject()).at("int1").type());
  EXPECT_EQ((int32_t)5, (*obj2.getObject()).at("int1").getInteger());
  EXPECT_FALSE(obj2.empty());
  EXPECT_TRUE(obj2);
  EXPECT_TRUE(obj2.comment() == nullptr);
  sourceObj = {};
  sourceObj["double"] = SerializableValue(5.7);
  sourceObj["answer"] = SerializableValue(42);
  sourceObj["reason"] = SerializableValue("test");
  SerializableValue obj3(std::move(sourceObj));
  EXPECT_EQ(SerializableValue::Type::object, obj3.type());
  ASSERT_EQ((size_t)3, obj3.size());
  ASSERT_TRUE(obj3.getObject() != nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*obj3.getObject()).at("double").type());
  EXPECT_EQ(5.7, (*obj3.getObject()).at("double").getNumber());
  EXPECT_EQ(SerializableValue::Type::integer, (*obj3.getObject()).at("answer").type());
  EXPECT_EQ((int32_t)42, (*obj3.getObject()).at("answer").getInteger());
  EXPECT_EQ(SerializableValue::Type::text, (*obj3.getObject()).at("reason").type());
  EXPECT_STREQ("test", (*obj3.getObject()).at("reason").getText());
  EXPECT_FALSE(obj3.empty());
  EXPECT_TRUE(obj3);
  EXPECT_TRUE(obj3.comment() == nullptr);
  EXPECT_TRUE(obj2 == obj2);
  EXPECT_TRUE(obj2 != obj3);
}

TEST_F(KeyValueSerializerTest, movedValueTest) {
  SerializableValue integer(0);
  SerializableValue movedInteger(std::move(integer));
  EXPECT_EQ(SerializableValue::Type::integer, movedInteger.type());
  EXPECT_EQ((size_t)1, movedInteger.size());
  EXPECT_EQ((int32_t)0, movedInteger.getInteger());
  EXPECT_TRUE(movedInteger.comment() == nullptr);
  integer = std::move(movedInteger);
  EXPECT_EQ(SerializableValue::Type::integer, integer.type());
  EXPECT_EQ((size_t)1, integer.size());
  EXPECT_EQ((int32_t)0, integer.getInteger());
  EXPECT_TRUE(integer.comment() == nullptr);
  
  SerializableValue integer2(42);
  integer2.setComment("abc");
  SerializableValue movedInteger2(std::move(integer2));
  EXPECT_EQ(SerializableValue::Type::integer, movedInteger2.type());
  EXPECT_EQ((size_t)1, movedInteger2.size());
  EXPECT_EQ((int32_t)42, movedInteger2.getInteger());
  EXPECT_STREQ("abc", movedInteger2.comment());
  integer2 = std::move(movedInteger2);
  EXPECT_EQ(SerializableValue::Type::integer, integer2.type());
  EXPECT_EQ((size_t)1, integer2.size());
  EXPECT_EQ((int32_t)42, integer2.getInteger());
  EXPECT_STREQ("abc", integer2.comment());
  
  SerializableValue number(0.0);
  SerializableValue movedNb(std::move(number));
  EXPECT_EQ(SerializableValue::Type::number, movedNb.type());
  EXPECT_EQ((size_t)1, movedNb.size());
  EXPECT_EQ(0.0, movedNb.getNumber());
  EXPECT_TRUE(movedNb.comment() == nullptr);
  number = std::move(movedNb);
  EXPECT_EQ(SerializableValue::Type::number, number.type());
  EXPECT_EQ((size_t)1, number.size());
  EXPECT_EQ(0.0, number.getNumber());
  EXPECT_TRUE(number.comment() == nullptr);
  
  SerializableValue number2(-0.5);
  number2.setComment("abc");
  SerializableValue movedNb2(std::move(number2));
  EXPECT_EQ(SerializableValue::Type::number, movedNb2.type());
  EXPECT_EQ((size_t)1, movedNb2.size());
  EXPECT_EQ(-0.5, movedNb2.getNumber());
  EXPECT_STREQ("abc", movedNb2.comment());
  number2 = std::move(movedNb2);
  EXPECT_EQ(SerializableValue::Type::number, number2.type());
  EXPECT_EQ((size_t)1, number2.size());
  EXPECT_EQ(-0.5, number2.getNumber());
  EXPECT_STREQ("abc", number2.comment());
  
  SerializableValue bool1(true);
  SerializableValue movedBool1(std::move(bool1));
  EXPECT_EQ(SerializableValue::Type::boolean, movedBool1.type());
  EXPECT_EQ((size_t)1, movedBool1.size());
  EXPECT_EQ(true, movedBool1.getBoolean());
  EXPECT_TRUE(movedBool1.comment() == nullptr);
  bool1 = std::move(movedBool1);
  EXPECT_EQ(SerializableValue::Type::boolean, bool1.type());
  EXPECT_EQ((size_t)1, bool1.size());
  EXPECT_EQ(true, bool1.getBoolean());
  EXPECT_TRUE(bool1.comment() == nullptr);
  
  SerializableValue bool2(false);
  bool2.setComment("abc");
  SerializableValue movedBool2(std::move(bool2));
  EXPECT_EQ(SerializableValue::Type::boolean, movedBool2.type());
  EXPECT_EQ((size_t)1, movedBool2.size());
  EXPECT_EQ(false, movedBool2.getBoolean());
  EXPECT_STREQ("abc", movedBool2.comment());
  bool2 = std::move(movedBool2);
  EXPECT_EQ(SerializableValue::Type::boolean, bool2.type());
  EXPECT_EQ((size_t)1, bool2.size());
  EXPECT_EQ(false, bool2.getBoolean());
  EXPECT_STREQ("abc", bool2.comment());
  
  SerializableValue str1(nullptr);
  SerializableValue movedStr1(std::move(str1));
  EXPECT_EQ(SerializableValue::Type::text, movedStr1.type());
  EXPECT_EQ((size_t)0, movedStr1.size());
  EXPECT_TRUE(movedStr1.getText() == nullptr);
  EXPECT_TRUE(movedStr1.comment() == nullptr);
  str1 = std::move(movedStr1);
  EXPECT_EQ(SerializableValue::Type::text, str1.type());
  EXPECT_EQ((size_t)0, str1.size());
  EXPECT_TRUE(str1.getText() == nullptr);
  EXPECT_TRUE(str1.comment() == nullptr);
  
  SerializableValue str2("@_very-long string !\n with new-lines...");
  str2.setComment("abc");
  SerializableValue movedStr2(std::move(str2));
  EXPECT_EQ(SerializableValue::Type::text, movedStr2.type());
  EXPECT_EQ((size_t)39, movedStr2.size());
  EXPECT_STREQ("@_very-long string !\n with new-lines...", movedStr2.getText());
  EXPECT_STREQ("abc", movedStr2.comment());
  str2 = std::move(movedStr2);
  EXPECT_EQ(SerializableValue::Type::text, str2.type());
  EXPECT_EQ((size_t)39, str2.size());
  EXPECT_STREQ("@_very-long string !\n with new-lines...", str2.getText());
  EXPECT_STREQ("abc", str2.comment());
  
  SerializableValue::Array sourceArray;
  SerializableValue arr1(std::move(sourceArray));
  SerializableValue movedArr1(std::move(arr1));
  EXPECT_EQ(SerializableValue::Type::arrays, movedArr1.type());
  EXPECT_EQ((size_t)0, movedArr1.size());
  EXPECT_TRUE(movedArr1.getArray() == nullptr);
  EXPECT_TRUE(movedArr1.comment() == nullptr);
  arr1 = std::move(movedArr1);
  EXPECT_EQ(SerializableValue::Type::arrays, arr1.type());
  EXPECT_EQ((size_t)0, arr1.size());
  EXPECT_TRUE(arr1.getArray() == nullptr);
  EXPECT_TRUE(arr1.comment() == nullptr);
  
  sourceArray = SerializableValue::Array{ SerializableValue(5.7), SerializableValue(42), SerializableValue("test") };
  SerializableValue arr2(std::move(sourceArray));
  arr2.setComment("abc");
  SerializableValue movedArr2(std::move(arr2));
  EXPECT_EQ(SerializableValue::Type::arrays, movedArr2.type());
  ASSERT_EQ((size_t)3, movedArr2.size());
  ASSERT_TRUE(movedArr2.getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*movedArr2.getArray())[0].type());
  EXPECT_EQ(5.7, (*movedArr2.getArray())[0].getNumber());
  EXPECT_EQ(SerializableValue::Type::integer, (*movedArr2.getArray())[1].type());
  EXPECT_EQ((int32_t)42, (*movedArr2.getArray())[1].getInteger());
  EXPECT_EQ(SerializableValue::Type::text, (*movedArr2.getArray())[2].type());
  EXPECT_STREQ("test", (*movedArr2.getArray())[2].getText());
  EXPECT_STREQ("abc", movedArr2.comment());
  arr2 = std::move(movedArr2);
  EXPECT_EQ(SerializableValue::Type::arrays, arr2.type());
  ASSERT_EQ((size_t)3, arr2.size());
  ASSERT_TRUE(arr2.getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*arr2.getArray())[0].type());
  EXPECT_EQ(5.7, (*arr2.getArray())[0].getNumber());
  EXPECT_EQ(SerializableValue::Type::integer, (*arr2.getArray())[1].type());
  EXPECT_EQ((int32_t)42, (*arr2.getArray())[1].getInteger());
  EXPECT_EQ(SerializableValue::Type::text, (*arr2.getArray())[2].type());
  EXPECT_STREQ("test", (*arr2.getArray())[2].getText());
  EXPECT_STREQ("abc", arr2.comment());
  
  SerializableValue::Object sourceObj;
  SerializableValue obj1(std::move(sourceObj));
  SerializableValue movedObj1(std::move(obj1));
  EXPECT_EQ(SerializableValue::Type::object, movedObj1.type());
  EXPECT_EQ((size_t)0, movedObj1.size());
  EXPECT_TRUE(movedObj1.getObject() == nullptr);
  EXPECT_TRUE(movedObj1.comment() == nullptr);
  obj1 = std::move(movedObj1);
  EXPECT_EQ(SerializableValue::Type::object, obj1.type());
  EXPECT_EQ((size_t)0, obj1.size());
  EXPECT_TRUE(obj1.getObject() == nullptr);
  EXPECT_TRUE(obj1.comment() == nullptr);
  
  sourceObj = {};
  sourceObj["double"] = SerializableValue(5.7);
  sourceObj["answer"] = SerializableValue(42);
  sourceObj["reason"] = SerializableValue("test");
  SerializableValue obj2(std::move(sourceObj));
  obj2.setComment("abc");
  SerializableValue movedObj2(std::move(obj2));
  EXPECT_EQ(SerializableValue::Type::object, movedObj2.type());
  ASSERT_EQ((size_t)3, movedObj2.size());
  ASSERT_TRUE(movedObj2.getObject() != nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*movedObj2.getObject()).at("double").type());
  EXPECT_EQ(5.7, (*movedObj2.getObject()).at("double").getNumber());
  EXPECT_EQ(SerializableValue::Type::integer, (*movedObj2.getObject()).at("answer").type());
  EXPECT_EQ((int32_t)42, (*movedObj2.getObject()).at("answer").getInteger());
  EXPECT_EQ(SerializableValue::Type::text, (*movedObj2.getObject()).at("reason").type());
  EXPECT_STREQ("test", (*movedObj2.getObject()).at("reason").getText());
  EXPECT_STREQ("abc", movedObj2.comment());
  obj2 = std::move(movedObj2);
  EXPECT_EQ(SerializableValue::Type::object, obj2.type());
  ASSERT_EQ((size_t)3, obj2.size());
  ASSERT_TRUE(obj2.getObject() != nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*obj2.getObject()).at("double").type());
  EXPECT_EQ(5.7, (*obj2.getObject()).at("double").getNumber());
  EXPECT_EQ(SerializableValue::Type::integer, (*obj2.getObject()).at("answer").type());
  EXPECT_EQ((int32_t)42, (*obj2.getObject()).at("answer").getInteger());
  EXPECT_EQ(SerializableValue::Type::text, (*obj2.getObject()).at("reason").type());
  EXPECT_STREQ("test", (*obj2.getObject()).at("reason").getText());
  EXPECT_STREQ("abc", obj2.comment());
}

TEST_F(KeyValueSerializerTest, convertTest) {
  int8_t otherInt1 = (int8_t)8;
  SerializableValue intConv1(otherInt1);
  EXPECT_EQ(SerializableValue::Type::integer, intConv1.type());
  EXPECT_EQ((int32_t)8, intConv1.getInteger());
  uint16_t otherInt2 = (uint16_t)16;
  SerializableValue intConv2(otherInt2);
  EXPECT_EQ(SerializableValue::Type::integer, intConv2.type());
  EXPECT_EQ((int32_t)16, intConv2.getInteger());
  
  SerializableValue int1(0);
  EXPECT_EQ((int32_t)0, int1.getInteger());
  EXPECT_EQ(0.0, int1.getNumber());
  EXPECT_EQ(false, int1.getBoolean());
  EXPECT_TRUE(int1.getText() == nullptr);
  EXPECT_TRUE(int1.getArray() == nullptr);
  EXPECT_TRUE(int1.getObject() == nullptr);
  SerializableValue int2(42);
  EXPECT_EQ((int32_t)42, int2.getInteger());
  EXPECT_EQ(42.0, int2.getNumber());
  EXPECT_EQ(true, int2.getBoolean());
  EXPECT_TRUE(int2.getText() == nullptr);
  EXPECT_TRUE(int2.getArray() == nullptr);
  EXPECT_TRUE(int2.getObject() == nullptr);
  SerializableValue int3(-42);
  EXPECT_EQ((int32_t)-42, int3.getInteger());
  EXPECT_EQ(-42.0, int3.getNumber());
  EXPECT_EQ(true, int3.getBoolean());
  EXPECT_TRUE(int3.getText() == nullptr);
  EXPECT_TRUE(int3.getArray() == nullptr);
  EXPECT_TRUE(int3.getObject() == nullptr);
  
  SerializableValue nb1(0.0);
  EXPECT_EQ((int32_t)0, nb1.getInteger());
  EXPECT_EQ(0.0, nb1.getNumber());
  EXPECT_EQ(false, nb1.getBoolean());
  EXPECT_TRUE(nb1.getText() == nullptr);
  EXPECT_TRUE(nb1.getArray() == nullptr);
  EXPECT_TRUE(nb1.getObject() == nullptr);
  SerializableValue nb2(42.02);
  EXPECT_EQ((int32_t)42, nb2.getInteger());
  EXPECT_EQ(42.02, nb2.getNumber());
  EXPECT_EQ(true, nb2.getBoolean());
  EXPECT_TRUE(nb2.getText() == nullptr);
  EXPECT_TRUE(nb2.getArray() == nullptr);
  EXPECT_TRUE(nb2.getObject() == nullptr);
  SerializableValue nb3(-42.02f);
  EXPECT_EQ((int32_t)-42, nb3.getInteger());
  EXPECT_EQ(-42.02f, nb3.getNumber());
  EXPECT_EQ(true, nb3.getBoolean());
  EXPECT_TRUE(nb3.getText() == nullptr);
  EXPECT_TRUE(nb3.getArray() == nullptr);
  EXPECT_TRUE(nb3.getObject() == nullptr);
  
  SerializableValue bool1(false);
  EXPECT_EQ((int32_t)0, bool1.getInteger());
  EXPECT_EQ(0.0, bool1.getNumber());
  EXPECT_EQ(false, bool1.getBoolean());
  EXPECT_STREQ("false", bool1.getText());
  EXPECT_TRUE(bool1.getArray() == nullptr);
  EXPECT_TRUE(bool1.getObject() == nullptr);
  SerializableValue bool2(true);
  EXPECT_EQ((int32_t)1, bool2.getInteger());
  EXPECT_EQ(1.0, bool2.getNumber());
  EXPECT_EQ(true, bool2.getBoolean());
  EXPECT_STREQ("true", bool2.getText());
  EXPECT_TRUE(bool2.getArray() == nullptr);
  EXPECT_TRUE(bool2.getObject() == nullptr);
  
  SerializableValue str1(nullptr);
  EXPECT_EQ((int32_t)0, str1.getInteger());
  EXPECT_EQ(0.0, str1.getNumber());
  EXPECT_EQ(false, str1.getBoolean());
  EXPECT_TRUE(str1.getText() == nullptr);
  EXPECT_TRUE(str1.getArray() == nullptr);
  EXPECT_TRUE(str1.getObject() == nullptr);
  SerializableValue str2("-42");
  EXPECT_EQ((int32_t)-42, str2.getInteger());
  EXPECT_EQ(-42.0, str2.getNumber());
  EXPECT_EQ(true, str2.getBoolean());
  EXPECT_STREQ("-42", str2.getText());
  EXPECT_TRUE(str2.getArray() == nullptr);
  EXPECT_TRUE(str2.getObject() == nullptr);
  SerializableValue str3(std::string("text\nwith\nlines"));
  EXPECT_EQ((int32_t)0, str3.getInteger());
  EXPECT_EQ(0.0, str3.getNumber());
  EXPECT_EQ(true, str3.getBoolean());
  EXPECT_STREQ("text\nwith\nlines", str3.getText());
  EXPECT_TRUE(str3.getArray() == nullptr);
  EXPECT_TRUE(str3.getObject() == nullptr);
  
  SerializableValue arr1(SerializableValue::Array{});
  EXPECT_EQ((int32_t)0, arr1.getInteger());
  EXPECT_EQ(0.0, arr1.getNumber());
  EXPECT_EQ(false, arr1.getBoolean());
  EXPECT_TRUE(arr1.getText() == nullptr);
  EXPECT_TRUE(arr1.getArray() == nullptr);
  EXPECT_TRUE(arr1.getObject() == nullptr);
  SerializableValue arr2(SerializableValue::Array{ SerializableValue(42) });
  EXPECT_EQ((int32_t)0, arr2.getInteger());
  EXPECT_EQ(0.0, arr2.getNumber());
  EXPECT_EQ(true, arr2.getBoolean());
  EXPECT_TRUE(arr2.getText() == nullptr);
  EXPECT_TRUE(arr2.getArray() != nullptr);
  EXPECT_TRUE(arr2.getObject() == nullptr);
  SerializableValue arr3(SerializableValue::Array{ SerializableValue("test"), SerializableValue(42.02), SerializableValue(7) });
  EXPECT_EQ((int32_t)0, arr3.getInteger());
  EXPECT_EQ(0.0, arr3.getNumber());
  EXPECT_EQ(true, arr3.getBoolean());
  EXPECT_TRUE(arr3.getText() == nullptr);
  EXPECT_TRUE(arr3.getArray() != nullptr);
  EXPECT_TRUE(arr3.getObject() == nullptr);
  
  SerializableValue obj1(SerializableValue::Object{});
  EXPECT_EQ((int32_t)0, obj1.getInteger());
  EXPECT_EQ(0.0, obj1.getNumber());
  EXPECT_EQ(false, obj1.getBoolean());
  EXPECT_TRUE(obj1.getText() == nullptr);
  EXPECT_TRUE(obj1.getArray() == nullptr);
  EXPECT_TRUE(obj1.getObject() == nullptr);
  
  SerializableValue::Object srcObj2;
  srcObj2["int"] = SerializableValue(42);
  SerializableValue obj2(std::move(srcObj2));
  EXPECT_EQ((int32_t)0, obj2.getInteger());
  EXPECT_EQ(0.0, obj2.getNumber());
  EXPECT_EQ(true, obj2.getBoolean());
  EXPECT_TRUE(obj2.getText() == nullptr);
  EXPECT_TRUE(obj2.getArray() == nullptr);
  EXPECT_TRUE(obj2.getObject() != nullptr);
  
  SerializableValue::Object srcObj3;
  srcObj3["str"] = SerializableValue("test");
  srcObj3["nb"] = SerializableValue(42.02);
  srcObj3["int"] = SerializableValue(7);
  SerializableValue obj3(std::move(srcObj3));
  EXPECT_EQ((int32_t)0, obj3.getInteger());
  EXPECT_EQ(0.0, obj3.getNumber());
  EXPECT_EQ(true, obj3.getBoolean());
  EXPECT_TRUE(obj3.getText() == nullptr);
  EXPECT_TRUE(obj3.getArray() == nullptr);
  EXPECT_TRUE(obj3.getObject() != nullptr);
}

TEST_F(KeyValueSerializerTest, nestedDataTest) {
  SerializableValue::Object subObj1Data;
  subObj1Data["lv1a.a"] = SerializableValue("test");
  subObj1Data["lv1a.b"] = SerializableValue(42.02);
  SerializableValue subObj1(std::move(subObj1Data));
  subObj1.setComment("subObj1");
  SerializableValue::Object subObj2Data;
  subObj2Data["lv1b.a"] = SerializableValue("abc");
  subObj2Data["lv1b.b"] = SerializableValue(7);
  subObj2Data["lv1b.c"] = SerializableValue(0.5);
  subObj2Data["lv1b.d"] = SerializableValue(SerializableValue::Array{ SerializableValue(5.5), SerializableValue(8) });
  SerializableValue subObj2(std::move(subObj2Data));
  SerializableValue subObj3(SerializableValue::Object{});
  SerializableValue::Array subArray1{ SerializableValue(42), SerializableValue(0) };
  SerializableValue::Array subArray2{ SerializableValue(59.94), SerializableValue("ntsc") };
  SerializableValue::Array subArray3{};
  
  SerializableValue::Object subArrayObj1Data;
  subArrayObj1Data["str1"] = SerializableValue("test1");
  subArrayObj1Data["str2"] = SerializableValue("test2");
  SerializableValue subArrayObj1(std::move(subArrayObj1Data));
  subArrayObj1.setComment("subArrayObj1");
  SerializableValue::Object subArrayObj2Data;
  subArrayObj2Data["nb"] = SerializableValue(42.02);
  subArrayObj2Data["int"] = SerializableValue(-1);
  SerializableValue subArrayObj2(std::move(subArrayObj2Data));
  SerializableValue::Array subArrayArray1{ SerializableValue(7), SerializableValue("abc") };
  
  SerializableValue::Array arrayWithObj{ SerializableValue("test"), SerializableValue(std::move(subArrayObj1)), SerializableValue(7), 
                                         SerializableValue(std::move(subArrayObj2)), SerializableValue(std::move(subArrayArray1)) };
  
  SerializableValue::Object mainObj;
  mainObj["str"] = SerializableValue("title");
  mainObj["val"] = SerializableValue(22);
  mainObj["arr1"] = SerializableValue(std::move(subArray1));
  mainObj["obj1"] = std::move(subObj1);
  mainObj["arrWithObj"] = SerializableValue(std::move(arrayWithObj));
  mainObj["arr2"] = SerializableValue(std::move(subArray2));
  mainObj["obj2"] = std::move(subObj2);
  mainObj["arr3"] = SerializableValue(std::move(subArray3));
  mainObj["obj3"] = std::move(subObj3);
  
  // root values
  SerializableValue finalData(std::move(mainObj));
  EXPECT_EQ(SerializableValue::Type::object, finalData.type());
  ASSERT_EQ((size_t)9, finalData.size());
  ASSERT_TRUE(finalData.getObject() != nullptr);
  EXPECT_TRUE(finalData.comment() == nullptr);
  
  EXPECT_EQ(SerializableValue::Type::text, (*finalData.getObject()).at("str").type());
  EXPECT_STREQ("title", (*finalData.getObject()).at("str").getText());
  EXPECT_TRUE((*finalData.getObject()).at("str").comment() == nullptr);
  
  EXPECT_EQ(SerializableValue::Type::integer, (*finalData.getObject()).at("val").type());
  EXPECT_EQ((int32_t)22, (*finalData.getObject()).at("val").getInteger());
  EXPECT_TRUE((*finalData.getObject()).at("val").comment() == nullptr);
  
  // array
  EXPECT_EQ(SerializableValue::Type::arrays, (*finalData.getObject()).at("arr1").type());
  EXPECT_TRUE((*finalData.getObject()).at("arr1").comment() == nullptr);
  ASSERT_EQ((size_t)2, (*finalData.getObject()).at("arr1").size());
  ASSERT_TRUE((*finalData.getObject()).at("arr1").getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::integer, (*(*finalData.getObject()).at("arr1").getArray())[0].type());
  EXPECT_EQ((int32_t)42, (*(*finalData.getObject()).at("arr1").getArray())[0].getInteger());
  EXPECT_TRUE((*(*finalData.getObject()).at("arr1").getArray())[0].comment() == nullptr);
  EXPECT_EQ(SerializableValue::Type::integer, (*(*finalData.getObject()).at("arr1").getArray())[1].type());
  EXPECT_EQ((int32_t)0, (*(*finalData.getObject()).at("arr1").getArray())[1].getInteger());
  EXPECT_TRUE((*(*finalData.getObject()).at("arr1").getArray())[1].comment() == nullptr);
  
  // object
  EXPECT_EQ(SerializableValue::Type::object, (*finalData.getObject()).at("obj1").type());
  ASSERT_TRUE((*finalData.getObject()).at("obj1").comment() != nullptr);
  EXPECT_STREQ("subObj1", (*finalData.getObject()).at("obj1").comment());
  ASSERT_EQ((size_t)2, (*finalData.getObject()).at("obj1").size());
  ASSERT_TRUE((*finalData.getObject()).at("obj1").getObject() != nullptr);
  EXPECT_EQ(SerializableValue::Type::text, (*(*finalData.getObject()).at("obj1").getObject()).at("lv1a.a").type());
  EXPECT_STREQ("test", (*(*finalData.getObject()).at("obj1").getObject()).at("lv1a.a").getText());
  EXPECT_TRUE((*(*finalData.getObject()).at("obj1").getObject()).at("lv1a.a").comment() == nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*(*finalData.getObject()).at("obj1").getObject()).at("lv1a.b").type());
  EXPECT_EQ(42.02, (*(*finalData.getObject()).at("obj1").getObject()).at("lv1a.b").getNumber());
  EXPECT_TRUE((*(*finalData.getObject()).at("obj1").getObject()).at("lv1a.b").comment() == nullptr);
  
  // array of objects/arrays
  EXPECT_EQ(SerializableValue::Type::arrays, (*finalData.getObject()).at("arrWithObj").type());
  EXPECT_TRUE((*finalData.getObject()).at("arrWithObj").comment() == nullptr);
  ASSERT_EQ((size_t)5, (*finalData.getObject()).at("arrWithObj").size());
  ASSERT_TRUE((*finalData.getObject()).at("arrWithObj").getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::text, (*(*finalData.getObject()).at("arrWithObj").getArray())[0].type());
  EXPECT_STREQ("test", (*(*finalData.getObject()).at("arrWithObj").getArray())[0].getText());
  EXPECT_TRUE((*(*finalData.getObject()).at("arrWithObj").getArray())[0].comment() == nullptr);
  EXPECT_EQ(SerializableValue::Type::integer, (*(*finalData.getObject()).at("arrWithObj").getArray())[2].type());
  EXPECT_EQ((int32_t)7, (*(*finalData.getObject()).at("arrWithObj").getArray())[2].getInteger());
  EXPECT_TRUE((*(*finalData.getObject()).at("arrWithObj").getArray())[2].comment() == nullptr);
  {
    auto& objRef = (*(*finalData.getObject()).at("arrWithObj").getArray())[1];
    EXPECT_EQ(SerializableValue::Type::object, objRef.type());
    ASSERT_TRUE(objRef.comment() != nullptr);
    EXPECT_STREQ("subArrayObj1", objRef.comment());
    ASSERT_EQ((size_t)2, objRef.size());
    ASSERT_TRUE(objRef.getObject() != nullptr);
    EXPECT_EQ(SerializableValue::Type::text, (*objRef.getObject()).at("str1").type());
    EXPECT_STREQ("test1", (*objRef.getObject()).at("str1").getText());
    EXPECT_TRUE((*objRef.getObject()).at("str1").comment() == nullptr);
    EXPECT_EQ(SerializableValue::Type::text, (*objRef.getObject()).at("str2").type());
    EXPECT_STREQ("test2", (*objRef.getObject()).at("str2").getText());
    EXPECT_TRUE((*objRef.getObject()).at("str2").comment() == nullptr);
  }
  {
    auto& objRef = (*(*finalData.getObject()).at("arrWithObj").getArray())[3];
    EXPECT_EQ(SerializableValue::Type::object, objRef.type());
    EXPECT_TRUE(objRef.comment() == nullptr);
    ASSERT_EQ((size_t)2, objRef.size());
    ASSERT_TRUE(objRef.getObject() != nullptr);
    EXPECT_EQ(SerializableValue::Type::number, (*objRef.getObject()).at("nb").type());
    EXPECT_EQ(42.02, (*objRef.getObject()).at("nb").getNumber());
    EXPECT_TRUE((*objRef.getObject()).at("nb").comment() == nullptr);
    EXPECT_EQ(SerializableValue::Type::integer, (*objRef.getObject()).at("int").type());
    EXPECT_EQ((int32_t)-1, (*objRef.getObject()).at("int").getInteger());
    EXPECT_TRUE((*objRef.getObject()).at("int").comment() == nullptr);
  }
  {
    auto& arrayRef = (*(*finalData.getObject()).at("arrWithObj").getArray())[4];
    EXPECT_EQ(SerializableValue::Type::arrays, arrayRef.type());
    EXPECT_TRUE(arrayRef.comment() == nullptr);
    ASSERT_EQ((size_t)2, arrayRef.size());
    ASSERT_TRUE(arrayRef.getArray() != nullptr);
    EXPECT_EQ(SerializableValue::Type::integer, (*arrayRef.getArray())[0].type());
    EXPECT_EQ((int32_t)7, (*arrayRef.getArray())[0].getInteger());
    EXPECT_TRUE((*arrayRef.getArray())[0].comment() == nullptr);
    EXPECT_EQ(SerializableValue::Type::text, (*arrayRef.getArray())[1].type());
    EXPECT_STREQ("abc", (*arrayRef.getArray())[1].getText());
    EXPECT_TRUE((*arrayRef.getArray())[1].comment() == nullptr);
  }
  
  // array
  EXPECT_EQ(SerializableValue::Type::arrays, (*finalData.getObject()).at("arr2").type());
  EXPECT_TRUE((*finalData.getObject()).at("arr2").comment() == nullptr);
  ASSERT_EQ((size_t)2, (*finalData.getObject()).at("arr2").size());
  ASSERT_TRUE((*finalData.getObject()).at("arr2").getArray() != nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*(*finalData.getObject()).at("arr2").getArray())[0].type());
  EXPECT_EQ(59.94, (*(*finalData.getObject()).at("arr2").getArray())[0].getNumber());
  EXPECT_TRUE((*(*finalData.getObject()).at("arr2").getArray())[0].comment() == nullptr);
  EXPECT_EQ(SerializableValue::Type::text, (*(*finalData.getObject()).at("arr2").getArray())[1].type());
  EXPECT_STREQ("ntsc", (*(*finalData.getObject()).at("arr2").getArray())[1].getText());
  EXPECT_TRUE((*(*finalData.getObject()).at("arr2").getArray())[1].comment() == nullptr);
  
  // object with array
  EXPECT_EQ(SerializableValue::Type::object, (*finalData.getObject()).at("obj2").type());
  EXPECT_TRUE((*finalData.getObject()).at("obj2").comment() == nullptr);
  ASSERT_EQ((size_t)4, (*finalData.getObject()).at("obj2").size());
  ASSERT_TRUE((*finalData.getObject()).at("obj2").getObject() != nullptr);
  EXPECT_EQ(SerializableValue::Type::text, (*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.a").type());
  EXPECT_STREQ("abc", (*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.a").getText());
  EXPECT_TRUE((*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.a").comment() == nullptr);
  EXPECT_EQ(SerializableValue::Type::integer, (*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.b").type());
  EXPECT_EQ((int32_t)7, (*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.b").getInteger());
  EXPECT_TRUE((*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.b").comment() == nullptr);
  EXPECT_EQ(SerializableValue::Type::number, (*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.c").type());
  EXPECT_EQ(0.5, (*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.c").getNumber());
  EXPECT_TRUE((*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.c").comment() == nullptr);
  {
    auto& arrayRef = (*(*finalData.getObject()).at("obj2").getObject()).at("lv1b.d");
    EXPECT_EQ(SerializableValue::Type::arrays, arrayRef.type());
    EXPECT_TRUE(arrayRef.comment() == nullptr);
    ASSERT_EQ((size_t)2, arrayRef.size());
    ASSERT_TRUE(arrayRef.getArray() != nullptr);
    EXPECT_EQ(SerializableValue::Type::number, (*arrayRef.getArray())[0].type());
    EXPECT_EQ(5.5, (*arrayRef.getArray())[0].getNumber());
    EXPECT_TRUE((*arrayRef.getArray())[0].comment() == nullptr);
    EXPECT_EQ(SerializableValue::Type::integer, (*arrayRef.getArray())[1].type());
    EXPECT_EQ((int32_t)8, (*arrayRef.getArray())[1].getInteger());
    EXPECT_TRUE((*arrayRef.getArray())[1].comment() == nullptr);
  }
  
  // empty array
  EXPECT_EQ(SerializableValue::Type::arrays, (*finalData.getObject()).at("arr3").type());
  EXPECT_EQ((size_t)0, (*finalData.getObject()).at("arr3").size());
  EXPECT_TRUE((*finalData.getObject()).at("arr3").getArray() == nullptr);
  
  // empty object
  EXPECT_EQ(SerializableValue::Type::object, (*finalData.getObject()).at("obj3").type());
  EXPECT_EQ((size_t)0, (*finalData.getObject()).at("obj3").size());
  EXPECT_TRUE((*finalData.getObject()).at("obj3").getObject() == nullptr);
}
