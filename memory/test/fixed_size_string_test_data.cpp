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

#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic pop
#endif
