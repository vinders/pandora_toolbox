#include <gtest/gtest.h>
#include <string>
#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Warray-bounds"
# pragma GCC diagnostic ignored "-Wstringop-overflow"
# pragma GCC diagnostic ignored "-Wrestrict"
#endif
#define _P_SKIP_ASSERTS 1
#include <memory/fixed_size_string.h>

#define _STR_MAX_LENGTH size_t{ 20u }

using namespace pandora::memory;

class FixedSizeStringTestData2 : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif


#ifndef _P_CI_DISABLE_SLOW_TESTS

// -- change size --

template <typename _CharType>
void _strClearFill(_CharType value, _CharType value2) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data;
  EXPECT_TRUE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.fill(value);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.clear();
  EXPECT_TRUE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.push_back(value2);
  data.push_back(value2);

  data.fill(value);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.fill(value2);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value2, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.clear();
  EXPECT_TRUE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 0 }, data.size());
}
template <typename _CharType>
void _strClearResize(_CharType value, _CharType value2) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data;
  EXPECT_TRUE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.resize(size_t{ 10u }, value);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 10u }, data.size());
  for (size_t i = 0; i < size_t{ 10u }; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[10]);

  data.clear();
  EXPECT_TRUE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 0 }, data.size());

  data.resize(size_t{ 10u }, value);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 10u }, data.size());
  for (size_t i = 0; i < size_t{ 10u }; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[10]);

  data.resize(_STR_MAX_LENGTH, value2);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 10u }; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  for (size_t i = 10u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value2, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.resize(_STR_MAX_LENGTH + 10u, value2);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 10u }; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  for (size_t i = 10u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value2, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.resize(size_t{ 4u }, value2);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 4u }, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[4]);

  data.resize(size_t{ 0 }, value2);
  EXPECT_TRUE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(size_t{ 0 }, data.size());
}

TEST_F(FixedSizeStringTestData2, clearFill) {
  _strClearFill<char>('a', 'b');
  _strClearFill<wchar_t>(L'a', L'b');
  _strClearFill<char16_t>(u'a', u'b');
  _strClearFill<char32_t>(U'a', U'b');
}
TEST_F(FixedSizeStringTestData2, clearResize) {
  _strClearResize<char>('a', 'b');
  _strClearResize<wchar_t>(L'a', L'b');
  _strClearResize<char16_t>(u'a', u'b');
  _strClearResize<char32_t>(U'a', U'b');
}


// -- extract --

template <typename _CharType>
void _strSubString(const _CharType* value) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);
  EXPECT_EQ(empty, empty.substr(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos));
  EXPECT_EQ(empty, empty.substr(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 2u }));
  EXPECT_EQ(empty, empty.substr(empty.size()));
  EXPECT_EQ(empty, empty.substr(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(empty, empty.substr(size_t{ 0 }));
  EXPECT_EQ(empty, empty.substr(size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(empty, empty.substr(size_t{ 2u }));
  EXPECT_EQ(empty, empty.substr(size_t{ 2u }, size_t{ 1u }));
  
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(value);
  size_t originalSize = data.size();
  EXPECT_TRUE(data == value);
  EXPECT_EQ(empty, data.substr(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos));
  EXPECT_EQ(empty, data.substr(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 2u }));
  EXPECT_EQ(empty, data.substr(data.size()));
  EXPECT_EQ(empty, data.substr(size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(data, data.substr(size_t{ 0 }));
  FixedSizeString<_STR_MAX_LENGTH, _CharType> buffer = data.substr(size_t{ 0 }, size_t{ 2u });
  EXPECT_EQ(size_t{ 2u }, buffer.size());
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(data[i], buffer[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), buffer[2u]);
  buffer = data.substr(size_t{ 2u });
  EXPECT_EQ(originalSize - 2u, buffer.size());
  for (size_t i = 2u; i < originalSize; ++i) {
    EXPECT_EQ(data[i], buffer[i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), buffer[originalSize - 2u]);
  buffer = data.substr(size_t{ 2u }, size_t{ 1u });
  EXPECT_EQ(size_t{ 1u }, buffer.size());
  EXPECT_EQ(data[2u], buffer[0]);
  EXPECT_EQ(static_cast<_CharType>(0), buffer[1u]);

  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);
  EXPECT_TRUE(data == value);
}
template <typename _CharType>
void _strCopySubString(const _CharType* value) {
  _CharType buffer[_STR_MAX_LENGTH]{ 0 };

  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);

  EXPECT_EQ(size_t{ 0 }, empty.copy(buffer, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, empty.copy(buffer, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 2u }));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, empty.copy(buffer, empty.size()));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, empty.copy(buffer, size_t{ 0 }, size_t{ 0 }));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, empty.copy(buffer, size_t{ 0 }));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, empty.copy(buffer, size_t{ 0 }, size_t{ 2u }));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, empty.copy(buffer, size_t{ 2u }));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, empty.copy(buffer, size_t{ 2u }, size_t{ 1u }));
  EXPECT_TRUE(empty == buffer);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(value);
  size_t originalSize = data.size();
  EXPECT_TRUE(data == value);
  EXPECT_EQ(size_t{ 0 }, data.copy(buffer, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, data.copy(buffer, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 2u }));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, data.copy(buffer, data.size()));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(size_t{ 0 }, data.copy(buffer, size_t{ 0 }, size_t{ 0 }));
  EXPECT_TRUE(empty == buffer);
  EXPECT_EQ(originalSize, data.copy(buffer, size_t{ 0 }));
  EXPECT_TRUE(data == buffer);
  EXPECT_EQ(size_t{ 2u }, data.copy(buffer, size_t{ 0 }, size_t{ 2u }));
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(data[i], buffer[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), buffer[2u]);
  EXPECT_EQ(originalSize - 2u, data.copy(buffer, size_t{ 2u }));
  for (size_t i = 2u; i < originalSize; ++i) {
    EXPECT_EQ(data[i], buffer[i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), buffer[originalSize - 2u]);
  EXPECT_EQ(size_t{ 1u }, data.copy(buffer, size_t{ 2u }, size_t{ 1u }));
  EXPECT_EQ(data[2u], buffer[0]);
  EXPECT_EQ(static_cast<_CharType>(0), buffer[1u]);

  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);
  EXPECT_TRUE(data == value);
}

TEST_F(FixedSizeStringTestData2, substring) {
  _strSubString<char>("azerty01");
  _strSubString<wchar_t>(L"azerty01");
  _strSubString<char16_t>(u"azerty01");
  _strSubString<char32_t>(U"azerty01");
}
TEST_F(FixedSizeStringTestData2, copySubstring) {
  _strCopySubString<char>("azerty01");
  _strCopySubString<wchar_t>(L"azerty01");
  _strCopySubString<char16_t>(u"azerty01");
  _strCopySubString<char32_t>(U"azerty01");
}


// -- search --

TEST_F(FixedSizeStringTestData2, findData) {
  FixedSizeString<_STR_MAX_LENGTH, char> data("azerty01z");
  FixedSizeString<_STR_MAX_LENGTH, wchar_t> wdata(L"azerty01z");
  size_t notFound = FixedSizeString<_STR_MAX_LENGTH, char>::npos;
  size_t wnotFound = FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos;

  EXPECT_EQ(notFound, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("bzrty"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("az"), size_t{ 2u }));
  EXPECT_EQ(notFound, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 4u }));
  EXPECT_EQ(notFound, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("ze"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("ze"), size_t{ 1u }));
  EXPECT_EQ(notFound, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("ze"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("z"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("z"), size_t{ 8u }));
  EXPECT_EQ(notFound, data.find(FixedSizeString < _STR_MAX_LENGTH, char>("z"), size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"bzrty"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"az"), FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 4u }));
  EXPECT_EQ(wnotFound, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 9u }));

  EXPECT_EQ(notFound, data.find("azerty01zazerty01z", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find("bzrty", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find("az", size_t{ 2u }));
  EXPECT_EQ(notFound, data.find("az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.find("azerty01z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find("az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find("ab", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(notFound, data.find("ab", size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(notFound, data.find("az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find("01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find("01", size_t{ 4u }));
  EXPECT_EQ(notFound, data.find("01", size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, data.find("ze", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find("ze", size_t{ 1u }));
  EXPECT_EQ(notFound, data.find("ze", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find("z", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find("z", size_t{ 8u }));
  EXPECT_EQ(notFound, data.find("z", size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.find(L"azerty01zazerty01z", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find(L"bzrty", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find(L"az", size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find(L"az", FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.find(L"azerty01z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find(L"az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find(L"ab", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.find(L"ab", size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find(L"az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find(L"01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find(L"01", size_t{ 4u }));
  EXPECT_EQ(wnotFound, wdata.find(L"01", size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find(L"ze", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find(L"ze", size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.find(L"ze", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find(L"z", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find(L"z", size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find(L"z", size_t{ 9u }));

  EXPECT_EQ(notFound, data.find('b', size_t{ 0 }));
  EXPECT_EQ(notFound, data.find('a', size_t{ 2u }));
  EXPECT_EQ(notFound, data.find('a', FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.find('a', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find('0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find('0', size_t{ 4u }));
  EXPECT_EQ(notFound, data.find('0', size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, data.find('z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find('z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, data.find('z', size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find('z', size_t{ 8u }));
  EXPECT_EQ(notFound, data.find('z', size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.find(L'b', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find(L'a', size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find(L'a', FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.find(L'a', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find(L'0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find(L'0', size_t{ 4u }));
  EXPECT_EQ(wnotFound, wdata.find(L'0', size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find(L'z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find(L'z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find(L'z', size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find(L'z', size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find(L'z', size_t{ 9u }));
}
TEST_F(FixedSizeStringTestData2, rfindData) {
  FixedSizeString<_STR_MAX_LENGTH, char> data("azerty01z");
  FixedSizeString<_STR_MAX_LENGTH, wchar_t> wdata(L"azerty01z");
  size_t notFound = FixedSizeString<_STR_MAX_LENGTH, char>::npos;
  size_t wnotFound = FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos;

  EXPECT_EQ(notFound, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("bzrty"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("az"), size_t{ 8u }));
  EXPECT_EQ(notFound, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 1u }));
  EXPECT_EQ(notFound, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("ze"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("ze"), size_t{ 1u }));
  EXPECT_EQ(notFound, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("ze"), size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("z"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("z"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.rfind(FixedSizeString < _STR_MAX_LENGTH, char>("z"), size_t{ 8u }));

  EXPECT_EQ(wnotFound, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"bzrty"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"az"), FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.rfind(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 9u }));

  EXPECT_EQ(notFound, data.rfind("azerty01zazerty01z", size_t{ 0 }));
  EXPECT_EQ(notFound, data.rfind("bzrty", size_t{ 0 }));
  EXPECT_EQ(notFound, data.rfind("az", size_t{ 8u }));
  EXPECT_EQ(notFound, data.rfind("az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.rfind("azerty01z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.rfind("az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.rfind("ab", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(notFound, data.rfind("ab", size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(notFound, data.rfind("az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.rfind("01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.rfind("01", size_t{ 1u }));
  EXPECT_EQ(notFound, data.rfind("01", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.rfind("z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.rfind("z", size_t{ 1 }));
  EXPECT_EQ(size_t{ 1u }, data.rfind("ze", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.rfind("ze", size_t{ 1u }));
  EXPECT_EQ(notFound, data.rfind("ze", size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, data.rfind("z", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.rfind("z", size_t{ 0 }));
  EXPECT_EQ(notFound, data.rfind("z", size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.rfind(L"azerty01zazerty01z", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.rfind(L"bzrty", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.rfind(L"az", size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.rfind(L"az", FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.rfind(L"azerty01z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.rfind(L"az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.rfind(L"ab", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.rfind(L"ab", size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.rfind(L"az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.rfind(L"01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.rfind(L"01", size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.rfind(L"01", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.rfind(L"z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(L"z", size_t{ 1 }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(L"ze", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(L"ze", size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.rfind(L"ze", size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(L"z", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.rfind(L"z", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.rfind(L"z", size_t{ 9u }));

  EXPECT_EQ(notFound, data.rfind('b', size_t{ 0 }));
  EXPECT_EQ(notFound, data.rfind('a', size_t{ 9u }));
  EXPECT_EQ(notFound, data.rfind('a', FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.rfind('a', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.rfind('0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.rfind('0', size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, data.rfind('0', size_t{ 2u }));
  EXPECT_EQ(notFound, data.rfind('0', size_t{ 3u }));
  EXPECT_EQ(size_t{ 8u }, data.rfind('z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.rfind('z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 1u }, data.rfind('z', size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.rfind('z', size_t{ 0 }));
  EXPECT_EQ(notFound, data.rfind('z', size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.rfind(L'b', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.rfind(L'a', size_t{ 9u }));
  EXPECT_EQ(wnotFound, wdata.rfind(L'a', FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.rfind(L'a', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.rfind(L'0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.rfind(L'0', size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, wdata.rfind(L'0', size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.rfind(L'0', size_t{ 3u }));
  EXPECT_EQ(size_t{ 8u }, wdata.rfind(L'z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(L'z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 1u }, wdata.rfind(L'z', size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.rfind(L'z', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.rfind(L'z', size_t{ 9u }));
}

TEST_F(FixedSizeStringTestData2, findList) {
  FixedSizeString<_STR_MAX_LENGTH, char> data("azerty01z");
  FixedSizeString<_STR_MAX_LENGTH, wchar_t> wdata(L"azerty01z");
  size_t notFound = FixedSizeString<_STR_MAX_LENGTH, char>::npos;
  size_t wnotFound = FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos;

  EXPECT_EQ(notFound, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>(""), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("clmkj-!:,;"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 3u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("ytrb"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("bzrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("az"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("za"), size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("ab"), size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("ba"), size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("azerty01z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("ab"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("01"), size_t{ 4u }));
  EXPECT_EQ(notFound, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("05"), size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("ze"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("ez"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("z"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("z"), size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_first_of(FixedSizeString<_STR_MAX_LENGTH, char>("z"), size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L""), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"clmkj-!:,;"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 3u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ytrb"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"bzrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"za"), size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ab"), size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ba"), size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azerty01z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ab"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 4u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"05"), size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ez"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 9u }));

  EXPECT_EQ(notFound, data.find_first_of("", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_of("clmkj-!:,;", size_t{ 0 }));
  EXPECT_EQ(size_t{ 3u }, data.find_first_of("ytrb", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_of("bzrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of("az", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of("za", size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_of("ab", size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_of("ba", size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_of("az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.find_first_of("azerty01z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_of("ab", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_of("az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_of("ab", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(notFound, data.find_first_of("ba", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(notFound, data.find_first_of("az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_first_of("01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_first_of("01", size_t{ 4u }));
  EXPECT_EQ(notFound, data.find_first_of("05", size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_of("ze", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_of("ez", size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of("z", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of("z", size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_first_of("z", size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.find_first_of(L"", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L"clmkj-!:,;", size_t{ 0 }));
  EXPECT_EQ(size_t{ 3u }, wdata.find_first_of(L"ytrb", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_of(L"bzrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(L"az", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(L"za", size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L"ab", size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L"ba", size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L"az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_of(L"azerty01z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_of(L"ab", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_of(L"az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_of(L"ab", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L"ba", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L"az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_first_of(L"01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_first_of(L"01", size_t{ 4u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L"05", size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_of(L"ze", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_of(L"ez", size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(L"z", size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(L"z", size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L"z", size_t{ 9u }));

  EXPECT_EQ(notFound, data.find_first_of('b', size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_of('a', size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_of('a', FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.find_first_of('a', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_first_of('0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_first_of('0', size_t{ 4u }));
  EXPECT_EQ(notFound, data.find_first_of('0', size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_of('z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_of('z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of('z', size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_of('z', size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_first_of('z', size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.find_first_of(L'b', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L'a', size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L'a', FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_of(L'a', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_first_of(L'0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_first_of(L'0', size_t{ 4u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L'0', size_t{ 7u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_of(L'z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_of(L'z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(L'z', size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_of(L'z', size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_first_of(L'z', size_t{ 9u }));
}
TEST_F(FixedSizeStringTestData2, findNotList) {
  FixedSizeString<_STR_MAX_LENGTH, char> data("azerty01z");
  FixedSizeString<_STR_MAX_LENGTH, wchar_t> wdata(L"azerty01z");
  size_t notFound = FixedSizeString<_STR_MAX_LENGTH, char>::npos;
  size_t wnotFound = FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos;

  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>(""), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("clmkj-!:,;"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("azerty01z"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("10ytreza"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("bzrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("azrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("azerty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("az"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("za"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("ab"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("ba"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("azerty0z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("ab"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 4u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("01"), size_t{ 4u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("01"), size_t{ 6u }));
  EXPECT_EQ(size_t{ 7u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("05"), size_t{ 6u }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("ze"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 3u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("ez"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("z"), size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("z"), size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("z"), size_t{ 9u }));

  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L""), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"clmkj-!:,;"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azerty01z"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"10ytreza"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"bzrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azerty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"za"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ab"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ba"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azerty0z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ab"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 4u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 4u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 6u }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"05"), size_t{ 6u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 3u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ez"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 2u }));
  EXPECT_EQ(notFound, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 9u }));

  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of("", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of("clmkj-!:,;", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_not_of("azerty01z", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_not_of("10ytreza", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of("bzrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of("azrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_first_not_of("azerty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of("az", size_t{ 2u }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of("za", size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of("ab", size_t{ 2u }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_not_of("ba", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_not_of("az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, data.find_first_not_of("azerty0z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_not_of("ab", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of("az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_not_of("aze", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of("az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of("01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 4u }, data.find_first_not_of("01", size_t{ 4u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_not_of("01", size_t{ 6u }));
  EXPECT_EQ(size_t{ 7u }, data.find_first_not_of("05", size_t{ 6u }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of("ze", size_t{ 0 }));
  EXPECT_EQ(size_t{ 3u }, data.find_first_not_of("ez", size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of("z", size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_not_of("z", size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_first_not_of("z", size_t{ 9u }));

  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(L"", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(L"clmkj-!:,;", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L"azerty01z", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L"10ytreza", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(L"bzrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(L"azrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_first_not_of(L"azerty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(L"az", size_t{ 2u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(L"za", size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(L"ab", size_t{ 2u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_not_of(L"ba", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L"az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, wdata.find_first_not_of(L"azerty0z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_not_of(L"ab", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(L"az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_not_of(L"aze", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(L"az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(L"01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 4u }, wdata.find_first_not_of(L"01", size_t{ 4u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_not_of(L"01", size_t{ 6u }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_first_not_of(L"05", size_t{ 6u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(L"ze", size_t{ 0 }));
  EXPECT_EQ(size_t{ 3u }, wdata.find_first_not_of(L"ez", size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(L"z", size_t{ 2u }));
  EXPECT_EQ(notFound, wdata.find_first_not_of(L"z", size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L"z", size_t{ 9u }));

  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of('b', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_first_not_of('a', size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_not_of('a', FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 1u }, data.find_first_not_of('a', size_t{ 1u }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of('0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_first_not_of('0', size_t{ 6u }));
  EXPECT_EQ(size_t{ 8u }, data.find_first_not_of('1', size_t{ 7u }));
  EXPECT_EQ(size_t{ 0 }, data.find_first_not_of('z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of('z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.find_first_not_of('z', size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_not_of('z', size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_first_not_of('z', size_t{ 9u }));
  data = "aaaa";
  EXPECT_EQ(notFound, data.find_first_not_of('a', size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_first_not_of('a', size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_first_not_of('a', size_t{ 5u }));

  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(L'b', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_not_of(L'a', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L'a', FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 1u }, wdata.find_first_not_of(L'a', size_t{ 1u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_first_not_of(L'0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_first_not_of(L'0', size_t{ 6u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_first_not_of(L'1', size_t{ 7u }));
  EXPECT_EQ(size_t{ 0u }, wdata.find_first_not_of(L'z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(L'z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_first_not_of(L'z', size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L'z', size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L'z', size_t{ 9u }));
  wdata = L"aaaa";
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L'a', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L'a', size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_first_not_of(L'a', size_t{ 5u }));
}

TEST_F(FixedSizeStringTestData2, rfindList) {
  FixedSizeString<_STR_MAX_LENGTH, char> data("azerty01z");
  FixedSizeString<_STR_MAX_LENGTH, wchar_t> wdata(L"azerty01z");
  size_t notFound = FixedSizeString<_STR_MAX_LENGTH, char>::npos;
  size_t wnotFound = FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos;

  EXPECT_EQ(size_t{ 8u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("bzrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("brty"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("azerty01z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("ab"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("01"), size_t{ 3u }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("e"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("ze"), size_t{ 1u }));
  EXPECT_EQ(notFound, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("ze"), size_t{ 9u }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("a"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("a"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>(""), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_of(FixedSizeString < _STR_MAX_LENGTH, char>("cbkx!"), size_t{ 0 }));

  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"bzrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"brty"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"az"), FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"azerty01z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ab"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 3u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"e"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 9u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"a"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"a"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L""), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(FixedSizeString < _STR_MAX_LENGTH, wchar_t>(L"cbkx!"), size_t{ 0 }));

  EXPECT_EQ(size_t{ 8u }, data.find_last_of("bzrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, data.find_last_of("brty", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_of("01", size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_last_of("az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of("azerty01z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of("az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_of("ab", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(notFound, data.find_last_of("bc", size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_last_of("az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_of("01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_of("01", size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_last_of("01", size_t{ 3u }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of("z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_last_of("z", size_t{ 1 }));
  EXPECT_EQ(size_t{ 2u }, data.find_last_of("e", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_last_of("ze", size_t{ 1u }));
  EXPECT_EQ(notFound, data.find_last_of("ze", size_t{ 9u }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_of("a", size_t{ 2u }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_of("a", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_of("", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_of("cbkx!", size_t{ 0 }));

  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(L"bzrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, wdata.find_last_of(L"brty", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L"01", size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L"az", FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(L"azerty01z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(L"az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_of(L"ab", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L"bc", size_t{ 0 }, size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L"az", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_of(L"01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_of(L"01", size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L"01", size_t{ 3u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(L"z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_last_of(L"z", size_t{ 1 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_last_of(L"e", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_last_of(L"ze", size_t{ 1u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L"ze", size_t{ 9u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_of(L"a", size_t{ 2u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_of(L"a", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L"", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L"cbkx!", size_t{ 0 }));

  EXPECT_EQ(notFound, data.find_last_of('b', size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_of('a', size_t{ 9u }));
  EXPECT_EQ(notFound, data.find_last_of('a', FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 0 }, data.find_last_of('a', size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of('z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_of('0', size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_of('0', size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_last_of('0', size_t{ 3u }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of('z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, data.find_last_of('z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 1u }, data.find_last_of('z', size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_of('z', size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_of('z', size_t{ 9u }));

  EXPECT_EQ(wnotFound, wdata.find_last_of(L'b', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L'a', size_t{ 9u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L'a', FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_of(L'a', size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(L'z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_of(L'0', size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_of(L'0', size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L'0', size_t{ 3u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(L'z', size_t{ 0 }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_last_of(L'z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 1u }, wdata.find_last_of(L'z', size_t{ 2u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_of(L'z', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_of(L'z', size_t{ 9u }));
}
TEST_F(FixedSizeStringTestData2, rfindNotList) {
  FixedSizeString<_STR_MAX_LENGTH, char> data("azerty01z");
  FixedSizeString<_STR_MAX_LENGTH, wchar_t> wdata(L"azerty01z");
  size_t notFound = FixedSizeString<_STR_MAX_LENGTH, char>::npos;
  size_t wnotFound = FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos;

  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>(""), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("clmkj-!:,;"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("azerty01z"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("10ytreza"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("bzrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("azrty1"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("azrty10"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("zerty10"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("1z"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("z1"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("zb"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("bz"), size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("azerty0z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("ab"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("01"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("01"), size_t{ 6u }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("ze"), size_t{ 6u }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("15"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("51"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("z"), size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("a"), size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, char>("a"), size_t{ 9u }));

  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L""), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"clmkj-!:,;"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azerty01z"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"10ytreza"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"bzrty"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azrty1"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azrty10"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"zerty10"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"1z"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"z1"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"zb"), size_t{ 2u }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"bz"), size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"az"), FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"azerty0z"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ab"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"az"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"01"), size_t{ 6u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"ze"), size_t{ 6u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"15"), size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"51"), size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"z"), size_t{ 2u }));
  EXPECT_EQ(notFound, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"a"), size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(FixedSizeString<_STR_MAX_LENGTH, wchar_t>(L"a"), size_t{ 9u }));

  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of("", size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of("clmkj-!:,;", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_not_of("azerty01z", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_not_of("10ytreza", size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of("bzrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of("azrty1", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, data.find_last_not_of("azrty10", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_not_of("zerty10", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of("1z", size_t{ 2u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of("z1", size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of("zb", size_t{ 2u }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of("bz", size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_not_of("az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of("azerty0z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of("ab", size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of("az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 4u }, data.find_last_not_of("z10y", size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of("z10y", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of("z10y", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of("01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, data.find_last_not_of("01", size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, data.find_last_not_of("01", size_t{ 6u }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_not_of("ze", size_t{ 6u }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of("15", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of("51", size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of("z", size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_last_not_of("a", size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_last_not_of("a", size_t{ 9u }));

  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(L"", size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(L"clmkj-!:,;", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L"azerty01z", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L"10ytreza", size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(L"bzrty", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(L"azrty1", size_t{ 0 }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_last_not_of(L"azrty10", size_t{ 0 }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_not_of(L"zerty10", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(L"1z", size_t{ 2u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(L"z1", size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(L"zb", size_t{ 2u }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(L"bz", size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L"az", FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(L"azerty0z", size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(L"ab", size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(L"az", size_t{ 0 }));
  EXPECT_EQ(size_t{ 4u }, wdata.find_last_not_of(L"z10y", size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(L"z10y", size_t{ 0 }, size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(L"z10y", size_t{ 0 }, size_t{ 0 }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(L"01", size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, wdata.find_last_not_of(L"01", size_t{ 1u }));
  EXPECT_EQ(size_t{ 2u }, wdata.find_last_not_of(L"01", size_t{ 6u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_not_of(L"ze", size_t{ 6u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(L"15", size_t{ 0 }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(L"51", size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(L"z", size_t{ 2u }));
  EXPECT_EQ(notFound, wdata.find_last_not_of(L"a", size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L"a", size_t{ 9u }));

  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of('b', size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of('z', size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_not_of('a', FixedSizeString<_STR_MAX_LENGTH, char>::npos));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of('z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, data.find_last_not_of('0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, data.find_last_not_of('0', size_t{ 2u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of('1', size_t{ 1u }));
  EXPECT_EQ(size_t{ 0 }, data.find_last_not_of('z', size_t{ 7u }));
  EXPECT_EQ(size_t{ 7u }, data.find_last_not_of('z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, data.find_last_not_of('z', size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_last_not_of('a', size_t{ 8u }));
  EXPECT_EQ(notFound, data.find_last_not_of('a', size_t{ 9u }));
  data = "aaaa";
  EXPECT_EQ(notFound, data.find_last_not_of('a', size_t{ 0 }));
  EXPECT_EQ(notFound, data.find_last_not_of('a', size_t{ 2u }));
  EXPECT_EQ(notFound, data.find_last_not_of('a', size_t{ 5u }));

  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(L'b', size_t{ 0 }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(L'z', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L'a', FixedSizeString<_STR_MAX_LENGTH, wchar_t>::npos));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(L'z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 8u }, wdata.find_last_not_of(L'0', size_t{ 0 }));
  EXPECT_EQ(size_t{ 5u }, wdata.find_last_not_of(L'0', size_t{ 2u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(L'1', size_t{ 1u }));
  EXPECT_EQ(size_t{ 0 }, wdata.find_last_not_of(L'z', size_t{ 7u }));
  EXPECT_EQ(size_t{ 7u }, wdata.find_last_not_of(L'z', size_t{ 1u }));
  EXPECT_EQ(size_t{ 6u }, wdata.find_last_not_of(L'z', size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L'a', size_t{ 8u }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L'a', size_t{ 9u }));
  wdata = L"aaaa";
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L'a', size_t{ 0 }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L'a', size_t{ 2u }));
  EXPECT_EQ(wnotFound, wdata.find_last_not_of(L'a', size_t{ 5u }));
}

#else
  
TEST_F(FixedSizeStringTestData2, baseData2Test) {
  FixedSizeString<_STR_MAX_LENGTH, char> fxStr("test str");
  fxStr.pop_back();
  EXPECT_EQ(0, fxStr.compare("test st"));
  fxStr.resize(size_t{ 10u }, ' ');
  EXPECT_EQ(0, fxStr.compare("test st   "));
  fxStr.resize(size_t{ 2u }, ' ');
  EXPECT_EQ(0, fxStr.compare("te"));
}

#endif

#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic pop
#endif
