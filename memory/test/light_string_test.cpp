#include <gtest/gtest.h>
#include <memory/light_string.h>

using namespace pandora::memory;

class LightStringTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- LightString --

TEST_F(LightStringTest, stringAccessorsCtors) {
  LightString empty;
  EXPECT_TRUE(empty.c_str() != nullptr);
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_STREQ("", empty.c_str());
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_EQ(size_t{0}, empty.length());
  EXPECT_TRUE(empty.empty());
  
  LightString empty2;
  EXPECT_STREQ("", empty2.c_str());
  EXPECT_TRUE(empty == empty2);
  EXPECT_FALSE(empty != empty2);
  EXPECT_TRUE(empty == "");
  EXPECT_FALSE(empty != "");
  
  LightString preallocEmpty(size_t{0});
  EXPECT_STREQ("", preallocEmpty.c_str());
  EXPECT_TRUE(preallocEmpty.c_str() != nullptr);
  EXPECT_TRUE(preallocEmpty.data() == nullptr);
  EXPECT_EQ(size_t{0}, preallocEmpty.size());
  EXPECT_EQ(size_t{0}, preallocEmpty.length());
  EXPECT_TRUE(preallocEmpty.empty());
  
  LightString prealloc(size_t{16u});
  EXPECT_TRUE(prealloc.c_str() != nullptr);
  EXPECT_TRUE(prealloc.data() != nullptr);
  EXPECT_EQ(size_t{16u}, prealloc.size());
  EXPECT_EQ(size_t{16u}, prealloc.length());
  EXPECT_FALSE(prealloc.empty());
  EXPECT_EQ('\0', prealloc.c_str()[0]);
  prealloc.data()[0] = 'a';
  EXPECT_STREQ(prealloc.c_str(), "a");
  
  LightString val1("abc");
  EXPECT_TRUE(val1.c_str() != nullptr);
  EXPECT_TRUE(val1.data() != nullptr);
  EXPECT_EQ(size_t{3u}, val1.size());
  EXPECT_EQ(size_t{3u}, val1.length());
  EXPECT_FALSE(val1.empty());
  EXPECT_EQ('a', val1.c_str()[0]);
  EXPECT_STREQ(val1.c_str(), "abc");
  EXPECT_FALSE(val1 == empty2);
  EXPECT_TRUE(val1 != empty2);
  EXPECT_TRUE(val1 == "abc");
  EXPECT_FALSE(val1 != "abc");
  
  LightString val2("abc", size_t{3});
  EXPECT_TRUE(val2.c_str() != nullptr);
  EXPECT_TRUE(val2.data() != nullptr);
  EXPECT_EQ(size_t{3u}, val2.size());
  EXPECT_EQ(size_t{3u}, val2.length());
  EXPECT_FALSE(val2.empty());
  EXPECT_EQ('a', val2.c_str()[0]);
  EXPECT_STREQ(val2.c_str(), "abc");
  EXPECT_TRUE(val2 == val1);
  EXPECT_FALSE(val2 != val1);
  EXPECT_TRUE(val2 == "abc");
  EXPECT_FALSE(val2 != "abc");
  
  LightString valTrunc("abc", size_t{2});
  EXPECT_TRUE(valTrunc.c_str() != nullptr);
  EXPECT_TRUE(valTrunc.data() != nullptr);
  EXPECT_EQ(size_t{2u}, valTrunc.size());
  EXPECT_EQ(size_t{2u}, valTrunc.length());
  EXPECT_FALSE(valTrunc.empty());
  EXPECT_EQ('a', valTrunc.c_str()[0]);
  EXPECT_STREQ(valTrunc.c_str(), "ab");
  EXPECT_FALSE(valTrunc == val1);
  EXPECT_TRUE(valTrunc != val1);
  EXPECT_FALSE(valTrunc == "abc");
  EXPECT_TRUE(valTrunc != "abc");
  
  LightString copied(valTrunc);
  EXPECT_TRUE(copied.c_str() != nullptr);
  EXPECT_TRUE(copied.data() != nullptr);
  EXPECT_EQ(size_t{2u}, copied.size());
  EXPECT_EQ(size_t{2u}, copied.length());
  EXPECT_FALSE(copied.empty());
  EXPECT_EQ('a', copied.c_str()[0]);
  EXPECT_STREQ(copied.c_str(), "ab");
  EXPECT_FALSE(valTrunc.empty());
  EXPECT_STREQ(valTrunc.c_str(), "ab");
  val2 = copied;
  EXPECT_TRUE(val2.c_str() != nullptr);
  EXPECT_TRUE(val2.data() != nullptr);
  EXPECT_EQ(size_t{2u}, val2.size());
  EXPECT_EQ(size_t{2u}, val2.length());
  EXPECT_FALSE(val2.empty());
  EXPECT_EQ('a', val2.c_str()[0]);
  EXPECT_STREQ(val2.c_str(), "ab");
  EXPECT_FALSE(copied.empty());
  EXPECT_STREQ(copied.c_str(), "ab");
  
  LightString moved(std::move(valTrunc));
  EXPECT_TRUE(moved.c_str() != nullptr);
  EXPECT_TRUE(moved.data() != nullptr);
  EXPECT_EQ(size_t{2u}, moved.size());
  EXPECT_EQ(size_t{2u}, moved.length());
  EXPECT_FALSE(moved.empty());
  EXPECT_EQ('a', moved.c_str()[0]);
  EXPECT_STREQ(moved.c_str(), "ab");
  EXPECT_TRUE(valTrunc.empty());
  EXPECT_TRUE(valTrunc.c_str() != nullptr);
  valTrunc = std::move(moved);
  EXPECT_TRUE(valTrunc.c_str() != nullptr);
  EXPECT_TRUE(valTrunc.data() != nullptr);
  EXPECT_EQ(size_t{2u}, valTrunc.size());
  EXPECT_EQ(size_t{2u}, valTrunc.length());
  EXPECT_FALSE(valTrunc.empty());
  EXPECT_EQ('a', valTrunc.c_str()[0]);
  EXPECT_STREQ(valTrunc.c_str(), "ab");
  EXPECT_TRUE(moved.empty());
  EXPECT_TRUE(moved.c_str() != nullptr);
}

TEST_F(LightStringTest, stringOperators) {
  LightString empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(empty.assign("456789"));
  EXPECT_TRUE(empty == "456789");
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.assign("def"));
  EXPECT_TRUE(empty == "def");
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.assign(nullptr));
  EXPECT_TRUE(empty == "");
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(empty.assign(""));
  EXPECT_TRUE(empty == "");
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(empty.assign("abc", size_t{3u}));
  EXPECT_TRUE(empty == "abc");
  EXPECT_FALSE(empty.empty());
  
  empty = "def";
  EXPECT_TRUE(empty == "def");
  EXPECT_FALSE(empty.empty());
  
  LightString val1("abc");
  EXPECT_FALSE(val1.empty());
  EXPECT_TRUE(val1.assign("456789"));
  EXPECT_TRUE(val1 == "456789");
  EXPECT_FALSE(val1.empty());
  EXPECT_TRUE(val1.assign("def"));
  EXPECT_TRUE(val1 == "def");
  EXPECT_FALSE(val1.empty());
  EXPECT_TRUE(val1.assign(nullptr));
  EXPECT_TRUE(val1 == "");
  EXPECT_TRUE(val1.empty());
  EXPECT_TRUE(val1.assign(""));
  EXPECT_TRUE(val1 == "");
  EXPECT_TRUE(val1.empty());
  EXPECT_TRUE(val1.assign("abc", size_t{3u}));
  EXPECT_TRUE(val1 == "abc");
  EXPECT_FALSE(val1.empty());
  
  LightString empty2;
  EXPECT_TRUE(empty2.empty());
  EXPECT_TRUE(empty2.append(nullptr));
  EXPECT_TRUE(empty2.empty());
  EXPECT_TRUE(empty2.append("456789"));
  EXPECT_TRUE(empty2 == "456789");
  EXPECT_FALSE(empty2.empty());
  EXPECT_TRUE(empty2.append("def", size_t{3u}));
  EXPECT_TRUE(empty2 == "456789def");
  EXPECT_FALSE(empty2.empty());
  EXPECT_TRUE(empty2.append(""));
  EXPECT_TRUE(empty2 == "456789def");
  EXPECT_FALSE(empty2.empty());
  EXPECT_TRUE(empty2.append("abc", size_t{2u}));
  EXPECT_TRUE(empty2 == "456789defab");
  EXPECT_FALSE(empty2.empty());
  
  LightString val2("abc");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append(nullptr));
  EXPECT_TRUE(val2 == "abc");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append("456789"));
  EXPECT_TRUE(val2 == "abc456789");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append("def", size_t{3u}));
  EXPECT_TRUE(val2 == "abc456789def");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append(""));
  EXPECT_TRUE(val2 == "abc456789def");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append("abc", size_t{2u}));
  EXPECT_TRUE(val2 == "abc456789defab");
  EXPECT_FALSE(val2.empty());
  
  val2 += "zzz";
  EXPECT_TRUE(val2 == "abc456789defabzzz");
  EXPECT_FALSE(val2.empty());
  
  LightString concat = val1 + val2;
  EXPECT_TRUE(concat == "abcabc456789defabzzz");
  EXPECT_FALSE(concat.empty());
  LightString concat2 = val2 + val1.c_str();
  EXPECT_TRUE(concat2 == "abc456789defabzzzabc");
  EXPECT_FALSE(concat2.empty());
}

// -- LightWString --

TEST_F(LightStringTest, wstringAccessorsCtors) {
  LightWString empty;
  EXPECT_TRUE(empty.c_str() != nullptr);
  EXPECT_TRUE(empty.data() == nullptr);
  EXPECT_EQ(size_t{0}, empty.size());
  EXPECT_EQ(size_t{0}, empty.length());
  EXPECT_TRUE(empty.empty());
  
  LightWString empty2;
  EXPECT_TRUE(empty == empty2);
  EXPECT_FALSE(empty != empty2);
  EXPECT_TRUE(empty == L"");
  EXPECT_FALSE(empty != L"");
  
  LightWString preallocEmpty(size_t{0});
  EXPECT_TRUE(preallocEmpty.c_str() != nullptr);
  EXPECT_TRUE(preallocEmpty.data() == nullptr);
  EXPECT_EQ(size_t{0}, preallocEmpty.size());
  EXPECT_EQ(size_t{0}, preallocEmpty.length());
  EXPECT_TRUE(preallocEmpty.empty());
  
  LightWString prealloc(size_t{16u});
  EXPECT_TRUE(prealloc.c_str() != nullptr);
  EXPECT_TRUE(prealloc.data() != nullptr);
  EXPECT_EQ(size_t{16u}, prealloc.size());
  EXPECT_EQ(size_t{16u}, prealloc.length());
  EXPECT_FALSE(prealloc.empty());
  EXPECT_EQ(L'\0', prealloc.c_str()[0]);
  prealloc.data()[0] = L'a';
  EXPECT_STREQ(prealloc.c_str(), L"a");
  
  LightWString val1(L"abc");
  EXPECT_TRUE(val1.c_str() != nullptr);
  EXPECT_TRUE(val1.data() != nullptr);
  EXPECT_EQ(size_t{3u}, val1.size());
  EXPECT_EQ(size_t{3u}, val1.length());
  EXPECT_FALSE(val1.empty());
  EXPECT_EQ(L'a', val1.c_str()[0]);
  EXPECT_STREQ(val1.c_str(), L"abc");
  EXPECT_FALSE(val1 == empty2);
  EXPECT_TRUE(val1 != empty2);
  EXPECT_TRUE(val1 == L"abc");
  EXPECT_FALSE(val1 != L"abc");
  
  LightWString val2(L"abc", size_t{3});
  EXPECT_TRUE(val2.c_str() != nullptr);
  EXPECT_TRUE(val2.data() != nullptr);
  EXPECT_EQ(size_t{3u}, val2.size());
  EXPECT_EQ(size_t{3u}, val2.length());
  EXPECT_FALSE(val2.empty());
  EXPECT_EQ(L'a', val2.c_str()[0]);
  EXPECT_STREQ(val2.c_str(), L"abc");
  EXPECT_TRUE(val2 == val1);
  EXPECT_FALSE(val2 != val1);
  EXPECT_TRUE(val2 == L"abc");
  EXPECT_FALSE(val2 != L"abc");
  
  LightWString valTrunc(L"abc", size_t{2});
  EXPECT_TRUE(valTrunc.c_str() != nullptr);
  EXPECT_TRUE(valTrunc.data() != nullptr);
  EXPECT_EQ(size_t{2u}, valTrunc.size());
  EXPECT_EQ(size_t{2u}, valTrunc.length());
  EXPECT_FALSE(valTrunc.empty());
  EXPECT_EQ(L'a', valTrunc.c_str()[0]);
  EXPECT_STREQ(valTrunc.c_str(), L"ab");
  EXPECT_FALSE(valTrunc == val1);
  EXPECT_TRUE(valTrunc != val1);
  EXPECT_FALSE(valTrunc == L"abc");
  EXPECT_TRUE(valTrunc != L"abc");
  
  LightWString copied(valTrunc);
  EXPECT_TRUE(copied.c_str() != nullptr);
  EXPECT_TRUE(copied.data() != nullptr);
  EXPECT_EQ(size_t{2u}, copied.size());
  EXPECT_EQ(size_t{2u}, copied.length());
  EXPECT_FALSE(copied.empty());
  EXPECT_EQ(L'a', copied.c_str()[0]);
  EXPECT_STREQ(copied.c_str(), L"ab");
  EXPECT_FALSE(valTrunc.empty());
  EXPECT_STREQ(valTrunc.c_str(), L"ab");
  val2 = copied;
  EXPECT_TRUE(val2.c_str() != nullptr);
  EXPECT_TRUE(val2.data() != nullptr);
  EXPECT_EQ(size_t{2u}, val2.size());
  EXPECT_EQ(size_t{2u}, val2.length());
  EXPECT_FALSE(val2.empty());
  EXPECT_EQ(L'a', val2.c_str()[0]);
  EXPECT_STREQ(val2.c_str(), L"ab");
  EXPECT_FALSE(copied.empty());
  EXPECT_STREQ(copied.c_str(), L"ab");
  
  LightWString moved(std::move(valTrunc));
  EXPECT_TRUE(moved.c_str() != nullptr);
  EXPECT_TRUE(moved.data() != nullptr);
  EXPECT_EQ(size_t{2u}, moved.size());
  EXPECT_EQ(size_t{2u}, moved.length());
  EXPECT_FALSE(moved.empty());
  EXPECT_EQ(L'a', moved.c_str()[0]);
  EXPECT_STREQ(moved.c_str(), L"ab");
  EXPECT_TRUE(valTrunc.empty());
  EXPECT_TRUE(valTrunc.c_str() != nullptr);
  valTrunc = std::move(moved);
  EXPECT_TRUE(valTrunc.c_str() != nullptr);
  EXPECT_TRUE(valTrunc.data() != nullptr);
  EXPECT_EQ(size_t{2u}, valTrunc.size());
  EXPECT_EQ(size_t{2u}, valTrunc.length());
  EXPECT_FALSE(valTrunc.empty());
  EXPECT_EQ(L'a', valTrunc.c_str()[0]);
  EXPECT_STREQ(valTrunc.c_str(), L"ab");
  EXPECT_TRUE(moved.empty());
  EXPECT_TRUE(moved.c_str() != nullptr);
}

TEST_F(LightStringTest, wstringOperators) {
  LightWString empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(empty.assign(L"456789"));
  EXPECT_TRUE(empty == L"456789");
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.assign(L"def"));
  EXPECT_TRUE(empty == L"def");
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.assign(nullptr));
  EXPECT_TRUE(empty == L"");
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(empty.assign(L""));
  EXPECT_TRUE(empty == L"");
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(empty.assign(L"abc", size_t{3u}));
  EXPECT_TRUE(empty == L"abc");
  EXPECT_FALSE(empty.empty());
  
  empty = L"def";
  EXPECT_TRUE(empty == L"def");
  EXPECT_FALSE(empty.empty());
  
  LightWString val1(L"abc");
  EXPECT_FALSE(val1.empty());
  EXPECT_TRUE(val1.assign(L"456789"));
  EXPECT_TRUE(val1 == L"456789");
  EXPECT_FALSE(val1.empty());
  EXPECT_TRUE(val1.assign(L"def"));
  EXPECT_TRUE(val1 == L"def");
  EXPECT_FALSE(val1.empty());
  EXPECT_TRUE(val1.assign(nullptr));
  EXPECT_TRUE(val1 == L"");
  EXPECT_TRUE(val1.empty());
  EXPECT_TRUE(val1.assign(L""));
  EXPECT_TRUE(val1 == L"");
  EXPECT_TRUE(val1.empty());
  EXPECT_TRUE(val1.assign(L"abc", size_t{3u}));
  EXPECT_TRUE(val1 == L"abc");
  EXPECT_FALSE(val1.empty());
  
  LightWString empty2;
  EXPECT_TRUE(empty2.empty());
  EXPECT_TRUE(empty2.append(nullptr));
  EXPECT_TRUE(empty2.empty());
  EXPECT_TRUE(empty2.append(L"456789"));
  EXPECT_TRUE(empty2 == L"456789");
  EXPECT_FALSE(empty2.empty());
  EXPECT_TRUE(empty2.append(L"def", size_t{3u}));
  EXPECT_TRUE(empty2 == L"456789def");
  EXPECT_FALSE(empty2.empty());
  EXPECT_TRUE(empty2.append(L""));
  EXPECT_TRUE(empty2 == L"456789def");
  EXPECT_FALSE(empty2.empty());
  EXPECT_TRUE(empty2.append(L"abc", size_t{2u}));
  EXPECT_TRUE(empty2 == L"456789defab");
  EXPECT_FALSE(empty2.empty());
  
  LightWString val2(L"abc");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append(nullptr));
  EXPECT_TRUE(val2 == L"abc");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append(L"456789"));
  EXPECT_TRUE(val2 == L"abc456789");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append(L"def", size_t{3u}));
  EXPECT_TRUE(val2 == L"abc456789def");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append(L""));
  EXPECT_TRUE(val2 == L"abc456789def");
  EXPECT_FALSE(val2.empty());
  EXPECT_TRUE(val2.append(L"abc", size_t{2u}));
  EXPECT_TRUE(val2 == L"abc456789defab");
  EXPECT_FALSE(val2.empty());
  
  val2 += L"zzz";
  EXPECT_TRUE(val2 == L"abc456789defabzzz");
  EXPECT_FALSE(val2.empty());
  
  LightWString concat = val1 + val2;
  EXPECT_TRUE(concat == L"abcabc456789defabzzz");
  EXPECT_FALSE(concat.empty());
  LightWString concat2 = val2 + val1.c_str();
  EXPECT_TRUE(concat2 == L"abc456789defabzzzabc");
  EXPECT_FALSE(concat2.empty());
}
