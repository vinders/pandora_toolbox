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
#include <pattern/operation_guard.h>

using namespace pandora::pattern;

class OperationGuardTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}

  void SetUp() override {}
  void TearDown() override {}
};


// -- operation guard validation --

TEST_F(OperationGuardTest, incrementDecrement) {
  int value = 5;
  OperationGuard<int, std::plus<int>, 1, std::minus<int>, 1> opGuard(value);
  EXPECT_TRUE(value == 6);
  opGuard.release();
  EXPECT_TRUE(value == 5);
  opGuard.release();
  EXPECT_TRUE(value == 5);

  char value2 = 'a';
  {// scope
    OperationGuard<char, std::plus<char>, 2, std::minus<char>, 2> opGuard2(value2);
    EXPECT_TRUE(value2 == 'c');
  }// end of scope
  EXPECT_TRUE(value2 == 'a');
}

TEST_F(OperationGuardTest, decrementIncrement) {
  char value = 'c';
  OperationGuard<char, std::minus<char>, 2, std::plus<char>, 3> opGuard(value);
  EXPECT_TRUE(value == 'a');
  opGuard.release();
  EXPECT_TRUE(value == 'd');
  opGuard.release();
  EXPECT_TRUE(value == 'd');

  int value2 = 5;
  {// scope
    OperationGuard<int, std::minus<int>, 2, std::plus<int>, 3> opGuard2(value2);
    EXPECT_TRUE(value2 == 3);
  }// end of scope
  EXPECT_TRUE(value2 == 6);
}

TEST_F(OperationGuardTest, multiplyDivide) {
  int value = 5;
  OperationGuard<int, std::multiplies<int>, 2, std::divides<int>, 2> opGuard(value);
  EXPECT_TRUE(value == 10);
  opGuard.release();
  EXPECT_TRUE(value == 5);
  opGuard.release();
  EXPECT_TRUE(value == 5);

  int value2 = 5;
  {// scope
    OperationGuard<int, std::multiplies<int>, 4, std::divides<int>, 4> opGuard2(value2);
    EXPECT_TRUE(value2 == 20);
  }// end of scope
  EXPECT_TRUE(value2 == 5);
}
