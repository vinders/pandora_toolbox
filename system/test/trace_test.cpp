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
#include <cstdio>
#include <system/trace.h>
#include <gtest/gtest.h>

class TraceTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override { valueCallCounter = 0; }
  void TearDown() override {}
  
  const char* value() {
    ++valueCallCounter;
    static std::string val = "dummy";
    return val.c_str();
  }
  const char* valueWithArgument() {
    ++valueCallCounter;
    static std::string val = "%d";
    return val.c_str();
  }

protected:
  int valueCallCounter = 0;
};

// -- trace macros --

TEST_F(TraceTest, standardTrace) {
# if defined(_DEBUG) || !defined(NDEBUG)
    EXPECT_LE(14+13, TRACE("Hello world !")); //prefix >= 14: "[path?]trace_test.cpp(32): "
# else
    TRACE(value());
    EXPECT_EQ(0, valueCallCounter);
# endif
}

TEST_F(TraceTest, standardTraceWithParams) {
# if defined(_DEBUG) || !defined(NDEBUG)
  EXPECT_LE(14+15, TRACE_N("Hello %s %d !", "world", 2));
# else
    TRACE_N(valueWithArgument(), 2);
    EXPECT_EQ(0, valueCallCounter);
# endif
}

TEST_F(TraceTest, errorTrace) {
# if defined(_DEBUG) || !defined(NDEBUG)
  EXPECT_LE(14+13, ERROR_TRACE("Hello world !"));
# else
    ERROR_TRACE(value());
    EXPECT_EQ(0, valueCallCounter);
# endif
}

TEST_F(TraceTest, errorTraceWithParams) {
# if defined(_DEBUG) || !defined(NDEBUG)
  EXPECT_LE(14+15, ERROR_TRACE_N("Hello %s %d !", "world", 2));
# else
    ERROR_TRACE_N(valueWithArgument(), 2);
    EXPECT_EQ(0, valueCallCounter);
# endif
}

TEST_F(TraceTest, fileTrace) {
# if defined(_DEBUG) || !defined(NDEBUG)
  EXPECT_LE(14+13, FILE_TRACE(stdout, "Hello world !"));
# else
    FILE_TRACE(stdout, value());
    EXPECT_EQ(0, valueCallCounter);
# endif
}

TEST_F(TraceTest, fileTraceWithParams) {
# if defined(_DEBUG) || !defined(NDEBUG)
  EXPECT_LE(14+15, FILE_TRACE_N(stdout, "Hello %s %d !", "world", 2));
# else
    FILE_TRACE_N(stdout, valueWithArgument(), 2);
    EXPECT_EQ(0, valueCallCounter);
# endif
}
