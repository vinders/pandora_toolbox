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

class FixedSizeStringTestData : public testing::Test {
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


// -- contruction/assignment/accessors --

template <typename _CharType>
void _strAccessorsCtorEmpty(const _CharType* emptyValue, const _CharType* filledValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr;
  const FixedSizeString<_STR_MAX_LENGTH, _CharType>& fxStrRef(fxStr);
  EXPECT_EQ(size_t{ 0 }, fxStr.length());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.max_size());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.capacity());
  EXPECT_TRUE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  ASSERT_TRUE(fxStr.c_str() != nullptr);
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));
  ASSERT_TRUE(fxStr.data() != nullptr);
  EXPECT_TRUE(*fxStr.data() == static_cast<_CharType>(0));
  EXPECT_EQ(static_cast<_CharType>(0), fxStr[0]);
  EXPECT_EQ(static_cast<_CharType>(0), fxStrRef[0]);
  EXPECT_THROW(fxStr.at(0), std::out_of_range);
  EXPECT_THROW(fxStrRef.at(0), std::out_of_range);
  EXPECT_EQ(static_cast<_CharType>(0), fxStr.front());
  EXPECT_EQ(static_cast<_CharType>(0), fxStr.back());
  EXPECT_EQ(static_cast<_CharType>(0), fxStrRef.front());
  EXPECT_EQ(static_cast<_CharType>(0), fxStrRef.back());

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr2(emptyValue);
  EXPECT_EQ(size_t{ 0 }, fxStr2.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr2.capacity());
  EXPECT_TRUE(fxStr2.empty());
  EXPECT_FALSE(fxStr2.full());
  ASSERT_TRUE(fxStr2.c_str() != nullptr);
  EXPECT_TRUE(*fxStr2.c_str() == static_cast<_CharType>(0));
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr3(filledValue, size_t{ 0 });
  EXPECT_EQ(size_t{ 0 }, fxStr3.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr3.capacity());
  EXPECT_TRUE(fxStr3.empty());
  EXPECT_FALSE(fxStr3.full());
  ASSERT_TRUE(fxStr3.c_str() != nullptr);
  EXPECT_TRUE(*fxStr3.c_str() == static_cast<_CharType>(0));
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr4(emptyValue, _STR_MAX_LENGTH);
  EXPECT_EQ(size_t{ 0 }, fxStr4.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr4.capacity());
  EXPECT_TRUE(fxStr4.empty());
  EXPECT_FALSE(fxStr4.full());
  ASSERT_TRUE(fxStr4.c_str() != nullptr);
  EXPECT_TRUE(*fxStr4.c_str() == static_cast<_CharType>(0));
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr5(nullptr);
  EXPECT_EQ(size_t{ 0 }, fxStr5.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr5.capacity());
  EXPECT_TRUE(fxStr5.empty());
  EXPECT_FALSE(fxStr5.full());
  ASSERT_TRUE(fxStr5.c_str() != nullptr);
  EXPECT_TRUE(*fxStr5.c_str() == static_cast<_CharType>(0));

  EXPECT_TRUE(fxStr == fxStr2);
  EXPECT_FALSE(fxStr != fxStr2);
  EXPECT_TRUE(fxStr == fxStr3);
  EXPECT_TRUE(fxStr2 == fxStr3);
  EXPECT_TRUE(fxStr == fxStr4);
  EXPECT_EQ(0, fxStr.compare(fxStr2));
  EXPECT_EQ(0, fxStr.compare(fxStr2.c_str()));
}
template <typename _CharType>
void _strAccessorsCtor(const _CharType* value, size_t length, const _CharType* longerValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(value);
  const FixedSizeString<_STR_MAX_LENGTH, _CharType>& fxStrRef(fxStr);
  EXPECT_EQ(length, fxStr.length());
  EXPECT_EQ(length, fxStr.size());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.max_size());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.capacity());
  EXPECT_FALSE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  ASSERT_TRUE(fxStr.c_str() != nullptr);
  EXPECT_TRUE(*fxStr.c_str() == *value);
  EXPECT_TRUE(*(fxStr.c_str() + length) == static_cast<_CharType>(0));
  __if_constexpr (std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)value, length) == 0);
  }
  else __if_constexpr (std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)value, length) == 0);
  }
  ASSERT_TRUE(fxStr.data() != nullptr);
  EXPECT_TRUE(*fxStr.data() == *value);
  EXPECT_TRUE(*(fxStr.data() + length) == static_cast<_CharType>(0));
  EXPECT_EQ(fxStr.data(), fxStr.c_str());
  EXPECT_EQ(*value, fxStr[0]);
  EXPECT_EQ(*value, fxStr.at(0));
  EXPECT_EQ(*value, fxStr.front());
  EXPECT_EQ(value[length - 1], fxStr.back());
  EXPECT_EQ(value[length - 1], fxStr.at(length - 1));
  EXPECT_EQ(static_cast<_CharType>(0), fxStr[length]);
  EXPECT_THROW(fxStr.at(length), std::out_of_range);
  EXPECT_EQ(*value, fxStrRef[0]);
  EXPECT_EQ(*value, fxStrRef.front());
  EXPECT_EQ(value[length - 1], fxStrRef.back());
  EXPECT_EQ(value[length - 1], fxStrRef.at(length - 1));
  EXPECT_EQ(static_cast<_CharType>(0), fxStrRef[length]);
  EXPECT_THROW(fxStrRef.at(length), std::out_of_range);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr2(value, length);
  EXPECT_EQ(length, fxStr2.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr2.capacity());
  EXPECT_FALSE(fxStr2.empty());
  ASSERT_TRUE(fxStr2.c_str() != nullptr);
  EXPECT_TRUE(*fxStr2.c_str() == *value);
  EXPECT_EQ(static_cast<_CharType>(0), fxStr2[length]);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr3(longerValue, length);
  EXPECT_EQ(length, fxStr3.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr3.capacity());
  EXPECT_FALSE(fxStr3.empty());
  ASSERT_TRUE(fxStr3.c_str() != nullptr);
  EXPECT_TRUE(*fxStr3.c_str() == *value);
  EXPECT_EQ(static_cast<_CharType>(0), fxStr3[length]);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr4(value, _STR_MAX_LENGTH);
  EXPECT_EQ(length, fxStr4.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr4.capacity());
  EXPECT_FALSE(fxStr4.empty());
  ASSERT_TRUE(fxStr4.c_str() != nullptr);
  EXPECT_TRUE(*fxStr4.c_str() == *value);
  EXPECT_EQ(static_cast<_CharType>(0), fxStr4[length]);
  
  EXPECT_TRUE(fxStr == fxStr2);
  EXPECT_FALSE(fxStr != fxStr2);
  EXPECT_TRUE(fxStr == fxStr3);
  EXPECT_TRUE(fxStr2 == fxStr3);
  EXPECT_TRUE(fxStr == fxStr4);
  EXPECT_EQ(0, fxStr.compare(fxStr2));
  EXPECT_EQ(0, fxStr.compare(fxStr2.c_str()));
}
template <typename _CharType>
void _strAccessorsCtorFull(const _CharType* value, const _CharType* longerValueSamePrefix) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(value);
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.size());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.max_size());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.capacity());
  EXPECT_FALSE(fxStr.empty());
  EXPECT_TRUE(fxStr.full());
  ASSERT_TRUE(fxStr.c_str() != nullptr);
  EXPECT_EQ(*value , *fxStr.c_str());
  ASSERT_TRUE(fxStr.data() != nullptr);
  EXPECT_EQ(*value , *fxStr.data());
  EXPECT_EQ(static_cast<_CharType>(0), *(fxStr.c_str() + _STR_MAX_LENGTH));
  __if_constexpr (std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr (std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  EXPECT_EQ(*value, fxStr[0]);
  EXPECT_EQ(*value, fxStr.at(0));
  EXPECT_EQ(*value, fxStr.front());
  EXPECT_EQ(value[_STR_MAX_LENGTH - 1u], fxStr.back());
  EXPECT_EQ(value[_STR_MAX_LENGTH - 1u], fxStr.at(_STR_MAX_LENGTH - 1u));
  EXPECT_EQ(static_cast<_CharType>(0), fxStr[_STR_MAX_LENGTH]);
  EXPECT_THROW(fxStr.at(_STR_MAX_LENGTH), std::out_of_range);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr2(value, _STR_MAX_LENGTH);
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr2.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr2.capacity());
  EXPECT_FALSE(fxStr2.empty());
  EXPECT_TRUE(fxStr2.full());
  ASSERT_TRUE(fxStr2.c_str() != nullptr);
  EXPECT_EQ(*value, *fxStr2.c_str());
  EXPECT_EQ(static_cast<_CharType>(0), *(fxStr2.c_str() + _STR_MAX_LENGTH));
  __if_constexpr (std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr2.c_str(), (const char*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr (std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr2.c_str(), (const wchar_t*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  EXPECT_EQ(static_cast<_CharType>(0), fxStr2[_STR_MAX_LENGTH]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr3(longerValueSamePrefix, _STR_MAX_LENGTH + 4u);
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr3.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr3.capacity());
  EXPECT_FALSE(fxStr3.empty());
  EXPECT_TRUE(fxStr3.full());
  ASSERT_TRUE(fxStr3.c_str() != nullptr);
  EXPECT_EQ(*value, *fxStr3.c_str());
  EXPECT_EQ(static_cast<_CharType>(0), *(fxStr3.c_str() + _STR_MAX_LENGTH));
  __if_constexpr (std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr3.c_str(), (const char*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr (std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr3.c_str(), (const wchar_t*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  EXPECT_EQ(static_cast<_CharType>(0), fxStr3[_STR_MAX_LENGTH]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr4(value, _STR_MAX_LENGTH - 4u);
  EXPECT_EQ(_STR_MAX_LENGTH - 4u, fxStr4.length());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr4.capacity());
  EXPECT_FALSE(fxStr4.empty());
  EXPECT_FALSE(fxStr4.full());
  ASSERT_TRUE(fxStr4.c_str() != nullptr);
  EXPECT_EQ(*value, *fxStr4.c_str());
  EXPECT_EQ(static_cast<_CharType>(0), *(fxStr4.c_str() + _STR_MAX_LENGTH - 4u));
  __if_constexpr (std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr4.c_str(), (const char*)value, _STR_MAX_LENGTH - 4u) == 0);
  }
  else __if_constexpr (std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr4.c_str(), (const wchar_t*)value, _STR_MAX_LENGTH - 4u) == 0);
  }
  EXPECT_EQ(static_cast<_CharType>(0), fxStr4[_STR_MAX_LENGTH - 4u]);

  EXPECT_TRUE(fxStr == fxStr2);
  EXPECT_FALSE(fxStr != fxStr2);
  EXPECT_TRUE(fxStr == fxStr3);
  EXPECT_TRUE(fxStr2 == fxStr3);
  EXPECT_FALSE(fxStr == fxStr4);
  EXPECT_TRUE(fxStr != fxStr4);
  EXPECT_EQ(0, fxStr.compare(fxStr2));
  EXPECT_EQ(0, fxStr.compare(fxStr2.c_str()));
  EXPECT_EQ(0, fxStr.compare(fxStr3));
  EXPECT_EQ(0, fxStr.compare(fxStr3.c_str()));
  EXPECT_LT(0, fxStr.compare(fxStr4));
  EXPECT_LT(0, fxStr.compare(fxStr4.c_str()));
}
template <typename _CharType>
void _strCtorSubset(const _CharType* value, const _CharType* maxValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> otherStr(maxValue);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(otherStr, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr2(otherStr, size_t{ 0 }, size_t{ 2u });
  EXPECT_FALSE(fxStr2.empty());
  EXPECT_EQ(size_t{ 2u }, fxStr2.size());
  EXPECT_TRUE(*fxStr2.c_str() == *otherStr.c_str());
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr2.c_str(), (const char*)otherStr.c_str(), size_t{ 2u }) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr2.c_str(), (const wchar_t*)otherStr.c_str(), size_t{ 2u }) == 0);
  }
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr3(otherStr, size_t{ 2u }, size_t{ 3u });
  EXPECT_FALSE(fxStr3.empty());
  EXPECT_EQ(size_t{ 3u }, fxStr3.size());
  EXPECT_TRUE(*fxStr3.c_str() == *(otherStr.c_str() + 2));
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr3.c_str(), (const char*)(otherStr.c_str() + 2), size_t{ 3u }) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr3.c_str(), (const wchar_t*)(otherStr.c_str() + 2), size_t{ 3u }) == 0);
  }

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr4(otherStr, size_t{ 2u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_FALSE(fxStr4.empty());
  EXPECT_EQ(_STR_MAX_LENGTH - 2u, fxStr4.size());
  EXPECT_TRUE(*fxStr4.c_str() == *(otherStr.c_str() + 2));
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr4.c_str(), (const char*)(otherStr.c_str() + 2), _STR_MAX_LENGTH - 2u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr4.c_str(), (const wchar_t*)(otherStr.c_str() + 2), _STR_MAX_LENGTH - 2u) == 0);
  }
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr5(otherStr, _STR_MAX_LENGTH, size_t{ 2u });
  EXPECT_TRUE(fxStr5.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr5.size());
  EXPECT_TRUE(*fxStr5.c_str() == static_cast<_CharType>(0));

  FixedSizeString<_STR_MAX_LENGTH, _CharType> otherSmallStr(value);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr6(otherSmallStr, size_t{ 2u }, _STR_MAX_LENGTH);
  EXPECT_FALSE(fxStr6.empty());
  EXPECT_TRUE(fxStr6.size() < _STR_MAX_LENGTH - 2u);
  EXPECT_TRUE(*fxStr6.c_str() == *(otherSmallStr.c_str() + 2));
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr6.c_str(), (const char*)(otherSmallStr.c_str() + 2), _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr6.c_str(), (const wchar_t*)(otherSmallStr.c_str() + 2), _STR_MAX_LENGTH + 1u) == 0);
  }
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr7(otherSmallStr, _STR_MAX_LENGTH, _STR_MAX_LENGTH);
  EXPECT_TRUE(fxStr7.empty());
}
template <typename _CharType>
void _strCtorCharRepeat(_CharType value) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(size_t{ 0 }, value);
  EXPECT_TRUE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr2(size_t{ 1 }, value);
  EXPECT_FALSE(fxStr2.empty());
  EXPECT_FALSE(fxStr2.full());
  EXPECT_EQ(size_t{ 1 }, fxStr2.size());
  EXPECT_EQ(value, fxStr2[0]);
  EXPECT_EQ(static_cast<_CharType>(0), fxStr2[1]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr3(size_t{ 2 }, value);
  EXPECT_FALSE(fxStr3.empty());
  EXPECT_FALSE(fxStr3.full());
  EXPECT_EQ(size_t{ 2 }, fxStr3.size());
  EXPECT_EQ(value, fxStr3[0]);
  EXPECT_EQ(value, fxStr3[1]);
  EXPECT_EQ(static_cast<_CharType>(0), fxStr3[2]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr4(_STR_MAX_LENGTH, value);
  EXPECT_FALSE(fxStr4.empty());
  EXPECT_TRUE(fxStr4.full());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr4.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i)
    EXPECT_EQ(value, fxStr4[i]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr5(_STR_MAX_LENGTH + 4u, value);
  EXPECT_FALSE(fxStr5.empty());
  EXPECT_TRUE(fxStr5.full());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr5.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i)
    EXPECT_EQ(value, fxStr5[i]);
}

TEST_F(FixedSizeStringTestData, accessorsCtorEmpty) {
  _strAccessorsCtorEmpty<char>("", "abc");
  _strAccessorsCtorEmpty<wchar_t>(L"", L"abc");
  _strAccessorsCtorEmpty<char16_t>(u"", u"abc");
  _strAccessorsCtorEmpty<char32_t>(U"", U"abc");
}
TEST_F(FixedSizeStringTestData, accessorsCtor) {
  _strAccessorsCtor<char>("azerty", size_t{ 6u }, "azertyuiop");
  _strAccessorsCtor<wchar_t>(L"azerty", size_t{ 6u }, L"azertyuiop");
  _strAccessorsCtor<char16_t>(u"azerty", size_t{ 6u }, u"azertyuiop");
  _strAccessorsCtor<char32_t>(U"azerty", size_t{ 6u }, U"azertyuiop");
}
TEST_F(FixedSizeStringTestData, accessorsCtorFull) {
  _strAccessorsCtorFull<char>("0123456789abcdefghij", "0123456789abcdefghijklmo");
  _strAccessorsCtorFull<wchar_t>(L"0123456789abcdefghij", L"0123456789abcdefghijklmo");
  _strAccessorsCtorFull<char16_t>(u"0123456789abcdefghij", u"0123456789abcdefghijklmo");
  _strAccessorsCtorFull<char32_t>(U"0123456789abcdefghij", U"0123456789abcdefghijklmo");
}
TEST_F(FixedSizeStringTestData, ctorSubset) {
  _strCtorSubset<char>("azerty", "0123456789abcdefghij");
  _strCtorSubset<wchar_t>(L"azerty", L"0123456789abcdefghij");
  _strCtorSubset<char16_t>(u"azerty", u"0123456789abcdefghij");
  _strCtorSubset<char32_t>(U"azerty", U"0123456789abcdefghij");
}
TEST_F(FixedSizeStringTestData, ctorCharRepeat) {
  _strCtorCharRepeat<char>('a');
  _strCtorCharRepeat<wchar_t>(L'a');
  _strCtorCharRepeat<char16_t>(u'a');
  _strCtorCharRepeat<char32_t>(U'a');
}

template <typename _CharType>
void _strAssign(const _CharType* emptyValue, const _CharType* value, const _CharType* maxValue, const _CharType* excessValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(value);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_TRUE(*fxStr.c_str() == *value);
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)value, _STR_MAX_LENGTH + 1u) == 0);
  }

  // operator =
  fxStr = nullptr;
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));

  FixedSizeString<_STR_MAX_LENGTH, _CharType> otherStr(value);
  fxStr = otherStr;
  EXPECT_FALSE(fxStr.empty());
  EXPECT_EQ(otherStr.size(), fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *otherStr.c_str());
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)otherStr.c_str(), _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)otherStr.c_str(), _STR_MAX_LENGTH + 1u) == 0);
  }

  FixedSizeString<_STR_MAX_LENGTH, _CharType> moved(emptyValue);
  fxStr = std::move(moved);
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));

  fxStr = value;
  EXPECT_FALSE(fxStr.empty());
  EXPECT_NE(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *value);
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)value, _STR_MAX_LENGTH + 1u) == 0);
  }

  // assign raw
  fxStr.assign(nullptr);
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));
  fxStr.assign(nullptr, size_t{ 0 });
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));
  fxStr.assign(nullptr, _STR_MAX_LENGTH);
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));

  fxStr.assign(emptyValue);
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));
  fxStr.assign(emptyValue, size_t{ 0 });
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));
  fxStr.assign(emptyValue, _STR_MAX_LENGTH);
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));

  fxStr.assign(value);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_TRUE(*fxStr.c_str() == *value);
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)value, _STR_MAX_LENGTH + 1u) == 0);
  }
  fxStr.assign(maxValue);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_TRUE(fxStr.full());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *maxValue);
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)maxValue, _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)maxValue, _STR_MAX_LENGTH + 1u) == 0);
  }
  fxStr.assign(excessValue);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_TRUE(fxStr.full());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *maxValue);
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)maxValue, _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
  EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)maxValue, _STR_MAX_LENGTH + 1u) == 0);
  }
  fxStr.assign(maxValue, size_t{ 0 });
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  fxStr.assign(maxValue, size_t{ 2u });
  EXPECT_FALSE(fxStr.empty());
  EXPECT_EQ(size_t{ 2u }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *maxValue);
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)maxValue, size_t{ 2u }) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)maxValue, size_t{ 2u }) == 0);
  }

  // assign other instance
  fxStr.assign(otherStr);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_EQ(otherStr.size(), fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *otherStr.c_str());
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)otherStr.c_str(), _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)otherStr.c_str(), _STR_MAX_LENGTH + 1u) == 0);
  }
}
template <typename _CharType>
void _strAssignSubset(const _CharType*value, const _CharType* maxValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(value);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> otherStr(maxValue);

  fxStr.assign(otherStr, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));
  fxStr.assign(otherStr, size_t{ 0 }, size_t{ 2u });
  EXPECT_FALSE(fxStr.empty());
  EXPECT_EQ(size_t{ 2u }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *otherStr.c_str());
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)otherStr.c_str(), size_t{ 2u }) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)otherStr.c_str(), size_t{ 2u }) == 0);
  }
  fxStr.assign(otherStr, size_t{ 2u }, size_t{ 3u });
  EXPECT_FALSE(fxStr.empty());
  EXPECT_EQ(size_t{ 3u }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *(otherStr.c_str() + 2));
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)(otherStr.c_str() + 2), size_t{ 3u }) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)(otherStr.c_str() + 2), size_t{ 3u }) == 0);
  }

  fxStr.assign(otherStr, size_t{ 2u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_EQ(_STR_MAX_LENGTH - 2u, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == *(otherStr.c_str() + 2));
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)(otherStr.c_str() + 2), _STR_MAX_LENGTH - 2u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)(otherStr.c_str() + 2), _STR_MAX_LENGTH - 2u) == 0);
  }
  fxStr.assign(otherStr, _STR_MAX_LENGTH, size_t{ 2u });
  EXPECT_TRUE(fxStr.empty());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());
  EXPECT_TRUE(*fxStr.c_str() == static_cast<_CharType>(0));

  FixedSizeString<_STR_MAX_LENGTH, _CharType> otherSmallStr(value);
  fxStr.assign(otherSmallStr, size_t{ 2u }, _STR_MAX_LENGTH);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_TRUE(fxStr.size() < _STR_MAX_LENGTH - 2u);
  EXPECT_TRUE(*fxStr.c_str() == *(otherSmallStr.c_str() + 2));
  __if_constexpr(std::is_same<_CharType, char>::value) {
    EXPECT_TRUE(strncmp((const char*)fxStr.c_str(), (const char*)(otherSmallStr.c_str() + 2), _STR_MAX_LENGTH + 1u) == 0);
  }
  else __if_constexpr(std::is_same<_CharType, wchar_t>::value) {
    EXPECT_TRUE(wcsncmp((const wchar_t*)fxStr.c_str(), (const wchar_t*)(otherSmallStr.c_str() + 2), _STR_MAX_LENGTH + 1u) == 0);
  }
  fxStr.assign(otherSmallStr, _STR_MAX_LENGTH, _STR_MAX_LENGTH);
  EXPECT_TRUE(fxStr.empty());
}
template <typename _CharType>
void _strAssignCharRepeat(_CharType value) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr;
  fxStr.assign(size_t{ 0 }, value);
  EXPECT_TRUE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  EXPECT_EQ(size_t{ 0 }, fxStr.size());

  fxStr.assign(size_t{ 1 }, value);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  EXPECT_EQ(size_t{ 1 }, fxStr.size());
  EXPECT_EQ(value, fxStr[0]);
  EXPECT_EQ(static_cast<_CharType>(0), fxStr[1]);

  fxStr.assign(size_t{ 2 }, value);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  EXPECT_EQ(size_t{ 2 }, fxStr.size());
  EXPECT_EQ(value, fxStr[0]);
  EXPECT_EQ(value, fxStr[1]);
  EXPECT_EQ(static_cast<_CharType>(0), fxStr[2]);

  fxStr.assign(_STR_MAX_LENGTH, value);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_TRUE(fxStr.full());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i)
    EXPECT_EQ(value, fxStr[i]);

  fxStr.assign(_STR_MAX_LENGTH + 4u, value);
  EXPECT_FALSE(fxStr.empty());
  EXPECT_TRUE(fxStr.full());
  EXPECT_EQ(_STR_MAX_LENGTH, fxStr.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i)
    EXPECT_EQ(value, fxStr[i]);
}
template <typename _CharType>
void _strCopyMove(const _CharType* value, const _CharType* maxValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrEmpty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(value);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFull(maxValue);
  EXPECT_TRUE(fxStrEmpty.empty());
  EXPECT_FALSE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  EXPECT_TRUE(fxStrFull.full());

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrEmptyCopy(fxStrEmpty);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrCopy(fxStr);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFullCopy(fxStrFull);
  EXPECT_TRUE(fxStrEmptyCopy.empty());
  EXPECT_FALSE(fxStrCopy.empty());
  EXPECT_FALSE(fxStrCopy.full());
  EXPECT_TRUE(fxStrFullCopy.full());
  EXPECT_TRUE(fxStrEmpty == fxStrEmptyCopy);
  EXPECT_TRUE(fxStr == fxStrCopy);
  EXPECT_TRUE(fxStrFull == fxStrFullCopy);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrEmptyMoved(std::move(fxStrEmpty));
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrMoved(std::move(fxStr));
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFullMoved(std::move(fxStrFull));
  EXPECT_TRUE(fxStrEmptyMoved.empty());
  EXPECT_FALSE(fxStrMoved.empty());
  EXPECT_FALSE(fxStrMoved.full());
  EXPECT_TRUE(fxStrFullMoved.full());
  EXPECT_TRUE(fxStrEmptyMoved == fxStrEmptyCopy);
  EXPECT_TRUE(fxStrMoved == fxStrCopy);
  EXPECT_TRUE(fxStrFullMoved == fxStrFullCopy);

  fxStrEmpty.clear();
  fxStrEmpty = fxStrEmptyCopy;
  fxStr.clear();
  fxStr = fxStrCopy;
  fxStrFull.clear();
  fxStrFull = fxStrFullCopy;
  EXPECT_TRUE(fxStrEmpty.empty());
  EXPECT_FALSE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  EXPECT_TRUE(fxStrFull.full());
  EXPECT_TRUE(fxStrEmpty == fxStrEmptyCopy);
  EXPECT_TRUE(fxStr == fxStrCopy);
  EXPECT_TRUE(fxStrFull == fxStrFullCopy);

  fxStrEmpty.clear();
  fxStrEmpty = std::move(fxStrEmptyMoved);
  fxStr.clear();
  fxStr = std::move(fxStrMoved);
  fxStrFull.clear();
  fxStrFull = std::move(fxStrFullMoved);
  EXPECT_TRUE(fxStrEmpty.empty());
  EXPECT_FALSE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  EXPECT_TRUE(fxStrFull.full());
  EXPECT_TRUE(fxStrEmpty == fxStrEmptyCopy);
  EXPECT_TRUE(fxStr == fxStrCopy);
  EXPECT_TRUE(fxStrFull == fxStrFullCopy);
}
template <typename _CharType>
void _strSwapData(const _CharType* value, const _CharType* maxValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrEmpty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(value);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFull(maxValue);
  EXPECT_TRUE(fxStrEmpty.empty());
  EXPECT_FALSE(fxStr.empty());
  EXPECT_FALSE(fxStr.full());
  EXPECT_TRUE(fxStrFull.full());

  fxStrEmpty.swap(fxStr);
  EXPECT_TRUE(fxStr.empty());
  EXPECT_FALSE(fxStrEmpty.empty());
  EXPECT_FALSE(fxStrEmpty.full());
  EXPECT_TRUE(fxStrEmpty == value);

  fxStr.swap(fxStrFull);
  EXPECT_TRUE(fxStrFull.empty());
  EXPECT_TRUE(fxStr.full());
  EXPECT_TRUE(fxStr == maxValue);
}

template <typename _CharType>
void _strCompareData(const _CharType* emptyValue, const _CharType* value, const _CharType* maxValueLower5th, const _CharType* maxValueGreater) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrEmpty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(value);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFull(maxValueGreater);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFullLow(maxValueLower5th);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrEmpty2;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr2(value);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFull2(maxValueGreater);

  EXPECT_TRUE(fxStrEmpty == fxStrEmpty);
  EXPECT_TRUE(fxStrEmpty == fxStrEmpty2);
  EXPECT_TRUE(fxStrEmpty == nullptr);
  EXPECT_TRUE(fxStrEmpty == emptyValue);
  EXPECT_TRUE(fxStrEmpty.compare(fxStrEmpty) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(nullptr) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(emptyValue) == 0);
  EXPECT_TRUE(fxStrEmpty != fxStr);
  EXPECT_TRUE(fxStrEmpty != fxStrFull);
  EXPECT_TRUE(fxStrEmpty != value);
  EXPECT_TRUE(fxStrEmpty != maxValueGreater);
  EXPECT_TRUE(fxStrEmpty.compare(fxStr) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(fxStrFull) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(value) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(maxValueGreater) < 0);

  EXPECT_TRUE(fxStr == fxStr);
  EXPECT_TRUE(fxStr == fxStr2);
  EXPECT_TRUE(fxStr == value);
  EXPECT_TRUE(fxStr.compare(fxStr) == 0);
  EXPECT_TRUE(fxStr.compare(fxStr2) == 0);
  EXPECT_TRUE(fxStr.compare(value) == 0);
  EXPECT_TRUE(fxStr != fxStrEmpty);
  EXPECT_TRUE(fxStr != fxStrFull);
  EXPECT_TRUE(fxStr != nullptr);
  EXPECT_TRUE(fxStr != emptyValue);
  EXPECT_TRUE(fxStr != maxValueGreater);
  EXPECT_TRUE(fxStr.compare(fxStrEmpty) > 0);
  EXPECT_TRUE(fxStr.compare(fxStrFull) < 0);
  EXPECT_TRUE(fxStr.compare(nullptr) > 0);
  EXPECT_TRUE(fxStr.compare(emptyValue) > 0);
  EXPECT_TRUE(fxStr.compare(maxValueLower5th) > 0);
  EXPECT_TRUE(fxStr.compare(maxValueGreater) < 0);

  EXPECT_TRUE(fxStrFull == fxStrFull);
  EXPECT_TRUE(fxStrFull == fxStrFull2);
  EXPECT_TRUE(fxStrFull == maxValueGreater);
  EXPECT_TRUE(fxStrFull.compare(fxStrFull) == 0);
  EXPECT_TRUE(fxStrFull.compare(fxStrFull2) == 0);
  EXPECT_TRUE(fxStrFull.compare(maxValueGreater) == 0);
  EXPECT_TRUE(fxStrFull != fxStrEmpty);
  EXPECT_TRUE(fxStrFull != fxStr);
  EXPECT_TRUE(fxStrFull != nullptr);
  EXPECT_TRUE(fxStrFull != emptyValue);
  EXPECT_TRUE(fxStrFull != value);
  EXPECT_TRUE(fxStrFull != maxValueLower5th);
  EXPECT_TRUE(fxStrFull.compare(fxStrEmpty) > 0);
  EXPECT_TRUE(fxStrFull.compare(fxStr) > 0);
  EXPECT_TRUE(fxStrFull.compare(nullptr) > 0);
  EXPECT_TRUE(fxStrFull.compare(emptyValue) > 0);
  EXPECT_TRUE(fxStrFull.compare(value) > 0);
  EXPECT_TRUE(fxStrFull.compare(maxValueLower5th) > 0);
}
template <typename _CharType>
void _strCompareSubset(const _CharType* value, const _CharType* maxValueLower5th, const _CharType* maxValueGreater) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrEmpty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr(value);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFull(maxValueGreater);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFullLow(maxValueLower5th);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrEmpty2;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStr2(value);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> fxStrFull2(maxValueGreater);

  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStr2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStr2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStr2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2) < 0);

  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStr2) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStr2) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStr2) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty2) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty2) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty2) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty2) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty2) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty2) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrFullLow) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrFullLow) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrFullLow) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFullLow) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFullLow) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFullLow) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFullLow) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2) < 0);

  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrFullLow) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrFullLow) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFullLow) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFullLow) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFullLow) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFullLow) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFullLow) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStr) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStr) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStr) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr) < 0);

  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, nullptr) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty2.c_str()) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty2.c_str(), size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty2.c_str(), _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty2.c_str(), size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty2.c_str(), _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty2.c_str(), _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStr2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStr2.c_str(), _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStr2.c_str(), _STR_MAX_LENGTH) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2.c_str(), _STR_MAX_LENGTH) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2.c_str(), _STR_MAX_LENGTH) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2.c_str()) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2.c_str(), _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2.c_str(), _STR_MAX_LENGTH) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2.c_str(), _STR_MAX_LENGTH) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2.c_str(), _STR_MAX_LENGTH) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) < 0);

  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStr2.c_str()) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStr2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStr2.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2.c_str()) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2.c_str()) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, nullptr) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, nullptr) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty2.c_str()) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty2.c_str(), size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty2.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty2.c_str(), size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty2.c_str(), _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrFullLow.c_str()) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrFullLow.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrFullLow.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFullLow.c_str()) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFullLow.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFullLow.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFullLow.c_str()) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2.c_str()) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2.c_str()) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2.c_str()) < 0);

  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2.c_str()) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2.c_str()) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2.c_str()) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrFullLow.c_str()) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrFullLow.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFullLow.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFullLow.c_str()) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFullLow.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFullLow.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFullLow.c_str()) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, nullptr) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, nullptr) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty.c_str()) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty.c_str(), size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty.c_str(), size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty.c_str(), _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStr.c_str()) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStr.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStr.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr.c_str()) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr.c_str(), FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr.c_str(), _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr.c_str()) < 0);

  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty2, size_t{ 1 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty2, size_t{ 1 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty2, size_t{ 2 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty2, size_t{ 2 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty2, size_t{ 1 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty2, size_t{ 2 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty2, size_t{ 1 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStr2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStr2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStr2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrEmpty.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2, size_t{ 0 }) < 0);

  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStr2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStr2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStr2, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStr2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStr2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStr2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStr2, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStr2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty2, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty2, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrFullLow, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrFullLow, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrFullLow, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFullLow, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFullLow, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFullLow, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFullLow, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStr.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStr.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2, size_t{ 2u }) < 0);

  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrFull2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrFull2, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFull2, size_t{ 4u }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFull2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFull2, size_t{ 4u }) == 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFull2, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrFullLow, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrFullLow, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrFullLow, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrFullLow, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrFullLow, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrFullLow, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrFullLow, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStrEmpty, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStrEmpty, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStrEmpty, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStrEmpty, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStrEmpty, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStrEmpty, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStrEmpty, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStr, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStr, size_t{ 0 }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStr, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr, size_t{ 0 }) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr, size_t{ 0 }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStr, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStr, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStr, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr, _STR_MAX_LENGTH) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr, _STR_MAX_LENGTH) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, size_t{ 4u }, fxStr, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 0 }, fxStr, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, size_t{ 4u }, fxStr, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 2u }, size_t{ 2u }, fxStr, size_t{ 2u }) == 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH, fxStr, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 0 }, _STR_MAX_LENGTH + 4u, fxStr, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStrFull.compare(size_t{ 4u }, _STR_MAX_LENGTH, fxStr, size_t{ 2u }) > 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr, size_t{ 2u }) < 0);
  EXPECT_TRUE(fxStrFull.compare(_STR_MAX_LENGTH, size_t{ 4u }, fxStr, _STR_MAX_LENGTH) == 0);
}

TEST_F(FixedSizeStringTestData, assignData) {
  _strAssign<char>("", "abc", "0123456789abcdefghij", "0123456789abcdefghijklmo");
  _strAssign<wchar_t>(L"", L"abc", L"0123456789abcdefghij", L"0123456789abcdefghijklmo");
  _strAssign<char16_t>(u"", u"abc", u"0123456789abcdefghij", u"0123456789abcdefghijklmo");
  _strAssign<char32_t>(U"", U"abc", U"0123456789abcdefghij", U"0123456789abcdefghijklmo");
}
TEST_F(FixedSizeStringTestData, assignSubset) {
  _strAssignSubset<char>("abc", "0123456789abcdefghij");
  _strAssignSubset<wchar_t>(L"abc", L"0123456789abcdefghij");
  _strAssignSubset<char16_t>(u"abc", u"0123456789abcdefghij");
  _strAssignSubset<char32_t>(U"abc", U"0123456789abcdefghij");
}
TEST_F(FixedSizeStringTestData, assignCharRepeat) {
  _strAssignCharRepeat<char>('a');
  _strAssignCharRepeat<wchar_t>(L'a');
  _strAssignCharRepeat<char16_t>(u'a');
  _strAssignCharRepeat<char32_t>(U'a');
}
TEST_F(FixedSizeStringTestData, copyMove) {
  _strCopyMove<char>("abc", "0123456789abcdefghij");
  _strCopyMove<wchar_t>(L"abc", L"0123456789abcdefghij");
  _strCopyMove<char16_t>(u"abc", u"0123456789abcdefghij");
  _strCopyMove<char32_t>(U"abc", U"0123456789abcdefghij");
}
TEST_F(FixedSizeStringTestData, swapData) {
  _strSwapData<char>("abc", "0123456789abcdefghij");
  _strSwapData<wchar_t>(L"abc", L"0123456789abcdefghij");
  _strSwapData<char16_t>(u"abc", u"0123456789abcdefghij");
  _strSwapData<char32_t>(U"abc", U"0123456789abcdefghij");
}

TEST_F(FixedSizeStringTestData, compareData) {
  _strCompareData<char>("", "azerty", "azer456789abcdefghij", "azerty6789abcdefghij");
  _strCompareData<wchar_t>(L"", L"azerty", L"azer456789abcdefghij", L"azerty6789abcdefghij");
  _strCompareData<char16_t>(u"", u"azerty", u"azer456789abcdefghij", u"azerty6789abcdefghij");
  _strCompareData<char32_t>(U"", U"azerty", U"azer456789abcdefghij", U"azerty6789abcdefghij");
}
TEST_F(FixedSizeStringTestData, compareSubset) {
  _strCompareSubset<char>("azerty", "azer456789abcdefghij", "azerty6789abcdefghij");
  _strCompareSubset<wchar_t>(L"azerty", L"azer456789abcdefghij", L"azerty6789abcdefghij");
  _strCompareSubset<char16_t>(u"azerty", u"azer456789abcdefghij", u"azerty6789abcdefghij");
  _strCompareSubset<char32_t>(U"azerty", U"azer456789abcdefghij", U"azerty6789abcdefghij");
}


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

TEST_F(FixedSizeStringTestData, clearFill) {
  _strClearFill<char>('a', 'b');
  _strClearFill<wchar_t>(L'a', L'b');
  _strClearFill<char16_t>(u'a', u'b');
  _strClearFill<char32_t>(U'a', U'b');
}
TEST_F(FixedSizeStringTestData, clearResize) {
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

TEST_F(FixedSizeStringTestData, substring) {
  _strSubString<char>("azerty01");
  _strSubString<wchar_t>(L"azerty01");
  _strSubString<char16_t>(u"azerty01");
  _strSubString<char32_t>(U"azerty01");
}
TEST_F(FixedSizeStringTestData, copySubstring) {
  _strCopySubString<char>("azerty01");
  _strCopySubString<wchar_t>(L"azerty01");
  _strCopySubString<char16_t>(u"azerty01");
  _strCopySubString<char32_t>(U"azerty01");
}


// -- search --

TEST_F(FixedSizeStringTestData, findData) {
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
TEST_F(FixedSizeStringTestData, rfindData) {
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

TEST_F(FixedSizeStringTestData, findList) {
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
TEST_F(FixedSizeStringTestData, findNotList) {
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

TEST_F(FixedSizeStringTestData, rfindList) {
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
TEST_F(FixedSizeStringTestData, rfindNotList) {
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

#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic pop
#endif
