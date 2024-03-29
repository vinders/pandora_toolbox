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
#ifndef _P_CI_DISABLE_SLOW_TESTS
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

class FixedSizeStringTestOp : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- operations - double padding --

template <typename _CharType>
void _strDoublePad(_CharType padValue, const _CharType* originalValue, size_t originalSize) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  EXPECT_TRUE(empty.empty());

  empty.pad(size_t{ 0 }, padValue);
  EXPECT_TRUE(empty.empty());
  empty.pad(originalSize, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_FALSE(empty.full());
  EXPECT_EQ(originalSize, empty.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[originalSize]);
  empty.pad(_STR_MAX_LENGTH, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.full());
  EXPECT_EQ(_STR_MAX_LENGTH, empty.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[_STR_MAX_LENGTH]);
  empty.pad(originalSize, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.full());
  EXPECT_EQ(_STR_MAX_LENGTH, empty.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[_STR_MAX_LENGTH]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);

  data.pad(size_t{ 0 }, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);
  data.pad(originalSize - 2u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);
  data.pad(originalSize, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);

  data.pad(originalSize + 4u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize + 4u, data.size());
  EXPECT_FALSE(data == originalValue);
  EXPECT_EQ(padValue, data[0]);
  EXPECT_EQ(padValue, data[1]);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 2u]);
  }
  EXPECT_EQ(padValue, data[originalSize + 2u]);
  EXPECT_EQ(padValue, data[originalSize + 3u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 4u]);

  data.pad(_STR_MAX_LENGTH, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_FALSE(data == originalValue);
  size_t padLength = (_STR_MAX_LENGTH - originalSize) / 2u;
  for (size_t i = 0; i < padLength; ++i) {
    EXPECT_EQ(padValue, data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + padLength]);
  }
  for (size_t i = padLength + originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.pad(_STR_MAX_LENGTH * 2u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_FALSE(data == originalValue);
  for (size_t i = 0; i < padLength; ++i) {
    EXPECT_EQ(padValue, data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + padLength]);
  }
  for (size_t i = padLength + originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);


  data = originalValue;
  data.pad(originalSize + 3u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize + 3u, data.size());
  EXPECT_FALSE(data == originalValue);
  EXPECT_EQ(padValue, data[0]);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 1u]);
  }
  EXPECT_EQ(padValue, data[originalSize + 1u]);
  EXPECT_EQ(padValue, data[originalSize + 2u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 3u]);
}

TEST_F(FixedSizeStringTestOp, doublePad) {
  _strDoublePad<char>(' ', "azerty01", size_t{ 8u });
  _strDoublePad<wchar_t>(L' ', L"azerty01", size_t{ 8u });
  _strDoublePad<char16_t>(u' ', u"azerty01", size_t{ 8u });
  _strDoublePad<char32_t>(U' ', U"azerty01", size_t{ 8u });
}


// -- operations - append --

template <typename _CharType>
void _strAppendData(const _CharType* originalValue, const _CharType* emptyValue, const _CharType* appendShort, const _CharType* appendMax, const _CharType* appendExcess) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> shortData(appendShort);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> maxData(appendMax);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> excessData(appendExcess);
  size_t shortSize = shortData.size();

  data.append(nullptr);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.append(emptyValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.append(appendShort, size_t{ 0 });
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);

  data.append(appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);

  data.append(appendShort, size_t{ 1 });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(appendShort[0], data[originalSize + shortSize]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize + 1]);

  data = originalValue;
  data.append(appendMax);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(appendMax, size_t{ 2u });
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize + 2u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < originalSize + 2u; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 2u]);

  data = originalValue;
  data.append(appendMax, _STR_MAX_LENGTH - originalSize);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.append(appendMax); // already full
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(appendExcess);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(empty);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  empty.append(empty);
  EXPECT_TRUE(empty.empty());

  data.append(shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);

  data = originalValue;
  data.append(maxData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.append(maxData); // already full
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(excessData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data += maxData;
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data += appendMax;
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data += nullptr;
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);

  auto copy1 = data + maxData;
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  EXPECT_TRUE(copy1.full());
  EXPECT_EQ(_STR_MAX_LENGTH, copy1.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
    EXPECT_EQ(originalValue[i], copy1[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], copy1[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), copy1[_STR_MAX_LENGTH]);

  auto copy2 = data + appendMax;
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  EXPECT_TRUE(copy2.full());
  EXPECT_EQ(_STR_MAX_LENGTH, copy2.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
    EXPECT_EQ(originalValue[i], copy2[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], copy2[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), copy2[_STR_MAX_LENGTH]);
}
template <typename _CharType>
void _strAppendSubset(const _CharType* originalValue, const _CharType* appendShort, const _CharType* appendMax, const _CharType* appendExcess) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> shortData(appendShort);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> maxData(appendMax);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> excessData(appendExcess);

  data.append(empty, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.append(empty, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.append(empty, size_t{ 2u }, size_t{ 1u });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());

  data = originalValue;
  data.append(shortData, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.append(shortData, shortData.size());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.append(shortData, shortData.size() + 2u, size_t{ 1u });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());

  data.append(shortData, size_t{ 0 }, shortData.size());
  EXPECT_EQ(originalSize + shortData.size(), data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size()]);
  data = originalValue;
  data.append(shortData, size_t{ 0 }, size_t{ 1u });
  EXPECT_EQ(originalSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < 1u; ++i) {
    EXPECT_EQ(shortData[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data = originalValue;
  data.append(shortData, size_t{ 0 }, size_t{ 5u });
  EXPECT_EQ(originalSize + shortData.size(), data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size()]);
  data = originalValue;
  data.append(shortData, size_t{ 1u }, shortData.size());
  EXPECT_EQ(originalSize + shortData.size() - 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size() - 1u; ++i) {
    EXPECT_EQ(shortData[i + 1u], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size() - 1u]);
  data = originalValue;
  data.append(shortData, size_t{ 1u }, shortData.size() - 1u);
  EXPECT_EQ(originalSize + shortData.size() - 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size() - 1u; ++i) {
    EXPECT_EQ(shortData[i + 1u], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size() - 1u]);

  data = originalValue;
  data.append(maxData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(shortData, size_t{ 0 });
  data.append(maxData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i + originalSize]);
  }
  for (size_t i = originalSize + shortData.size(); i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - originalSize - shortData.size()], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.append(shortData, size_t{ 0 }, size_t{ 1u }); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = originalSize + shortData.size(); i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - originalSize - shortData.size()], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(excessData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(excessData[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
}
template <typename _CharType>
void _strAppendRepeat(const _CharType* originalValue, _CharType value, _CharType value2) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  data.append(size_t{ 0 }, value);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);

  data.append(size_t{ 1u }, value);
  EXPECT_EQ(originalSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);

  data.append(size_t{ 2u }, value2);
  EXPECT_EQ(originalSize + 3u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(value2, data[originalSize + 1u]);
  EXPECT_EQ(value2, data[originalSize + 2u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 3u]);

  data.append(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(value2, data[originalSize + 1u]);
  EXPECT_EQ(value2, data[originalSize + 2u]);
  for (size_t i = originalSize + 3u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.append(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(value2, data[originalSize + 1u]);
  EXPECT_EQ(value2, data[originalSize + 2u]);
  for (size_t i = originalSize + 3u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(_STR_MAX_LENGTH - originalSize, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(_STR_MAX_LENGTH - originalSize - 1u, value);
  EXPECT_EQ(_STR_MAX_LENGTH - 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH - 1u; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH - 1u]);
}
template <typename _CharType>
void _strPushBack(const _CharType* originalValue, _CharType value, _CharType value2) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  EXPECT_TRUE(data.push_back(value));
  EXPECT_EQ(originalSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);

  EXPECT_TRUE(data.push_back(value2));
  EXPECT_EQ(originalSize + 2u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(value2, data[originalSize + 1u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 2u]);

  data.resize(_STR_MAX_LENGTH - 1u, value2);
  EXPECT_EQ(_STR_MAX_LENGTH - 1u, data.size());
  EXPECT_EQ(value2, data[_STR_MAX_LENGTH - 2u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH - 1u]);

  EXPECT_TRUE(data.push_back(value));
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(value2, data[_STR_MAX_LENGTH - 2u]);
  EXPECT_EQ(value, data[_STR_MAX_LENGTH - 1u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  EXPECT_FALSE(data.push_back(value)); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(value2, data[_STR_MAX_LENGTH - 2u]);
  EXPECT_EQ(value, data[_STR_MAX_LENGTH - 1u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
}
template <typename _CharType>
void _strRightPad(_CharType padValue, const _CharType* originalValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  EXPECT_TRUE(empty.empty());

  empty.rpad(size_t{ 0 }, padValue);
  EXPECT_TRUE(empty.empty());
  empty.rpad(size_t{ 8u }, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_FALSE(empty.full());
  EXPECT_EQ(size_t{ 8u }, empty.size());
  for (size_t i = 0; i < size_t{ 8u }; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[8]);
  empty.rpad(_STR_MAX_LENGTH, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.full());
  EXPECT_EQ(_STR_MAX_LENGTH, empty.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[_STR_MAX_LENGTH]);
  empty.rpad(size_t{ 8u }, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.full());
  EXPECT_EQ(_STR_MAX_LENGTH, empty.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[_STR_MAX_LENGTH]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  data.rpad(size_t{ 0 }, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);
  data.rpad(originalSize - 2u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);
  data.rpad(originalSize, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);

  data.rpad(originalSize + 4u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize + 4u, data.size());
  EXPECT_FALSE(data == originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(padValue, data[originalSize]);
  EXPECT_EQ(padValue, data[originalSize + 1u]);
  EXPECT_EQ(padValue, data[originalSize + 2u]);
  EXPECT_EQ(padValue, data[originalSize + 3u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 4u]);

  data.rpad(_STR_MAX_LENGTH, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_FALSE(data == originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.rpad(_STR_MAX_LENGTH * 2u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_FALSE(data == originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
}

TEST_F(FixedSizeStringTestOp, appendData) {
  _strAppendData<char>("azerty01", "", "42", "123456789aze", "123456789azerty");
  _strAppendData<wchar_t>(L"azerty01", L"", L"42", L"123456789aze", L"123456789azerty");
  _strAppendData<char16_t>(u"azerty01", u"", u"42", u"123456789aze", u"123456789azerty");
  _strAppendData<char32_t>(U"azerty01", U"", U"42", U"123456789aze", U"123456789azerty");
}
TEST_F(FixedSizeStringTestOp, appendSubset) {
  _strAppendSubset<char>("azerty01", "42", "123456789aze", "123456789azerty");
  _strAppendSubset<wchar_t>(L"azerty01", L"42", L"123456789aze", L"123456789azerty");
  _strAppendSubset<char16_t>(u"azerty01", u"42", u"123456789aze", u"123456789azerty");
  _strAppendSubset<char32_t>(U"azerty01", U"42", U"123456789aze", U"123456789azerty");
}
TEST_F(FixedSizeStringTestOp, appendRepeat) {
  _strAppendRepeat<char>("azerty01", 'a', 'b');
  _strAppendRepeat<wchar_t>(L"azerty01", L'a', L'b');
  _strAppendRepeat<char16_t>(u"azerty01", u'a', u'b');
  _strAppendRepeat<char32_t>(U"azerty01", U'a', U'b');
}
TEST_F(FixedSizeStringTestOp, pushBack) {
  _strPushBack<char>("azerty01", 'a', 'b');
  _strPushBack<wchar_t>(L"azerty01", L'a', L'b');
  _strPushBack<char16_t>(u"azerty01", u'a', u'b');
  _strPushBack<char32_t>(U"azerty01", U'a', U'b');
}
TEST_F(FixedSizeStringTestOp, rightPad) {
  _strRightPad<char>(' ', "azerty01");
  _strRightPad<wchar_t>(L' ', L"azerty01");
  _strRightPad<char16_t>(u' ', u"azerty01");
  _strRightPad<char32_t>(U' ', U"azerty01");
}


// -- operations - insert --

template <typename _CharType>
void _strInsertData(const _CharType* originalValue, const _CharType* emptyValue, const _CharType* appendShort, const _CharType* appendMax, const _CharType* appendExcess) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> shortData(appendShort);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> maxData(appendMax);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> excessData(appendExcess);
  size_t shortSize = shortData.size();

  data.insert(size_t{ 0 }, nullptr);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.insert(size_t{ 0 }, emptyValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.insert(size_t{ 0 }, appendShort, size_t{ 0 });
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);

  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, appendShort, size_t{ 1u });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(appendShort[0], data[originalSize + shortSize]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize + 1]);

  data = originalValue;
  data.insert(originalSize, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);
  data.insert(originalSize, appendShort, size_t{ 1u });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(appendShort[0], data[originalSize]);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize + 1]);

  data = originalValue;
  data.insert(size_t{ 0 }, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);
  data.insert(size_t{ 0 }, appendShort, size_t{ 1u });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize + 1u, data.size());
  EXPECT_EQ(appendShort[0], data[0]);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 1u]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize + 1]);
  data = originalValue;
  data.insert(size_t{ 4u }, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, appendMax);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 0 }, appendMax);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[_STR_MAX_LENGTH - originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 4u }, appendMax);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[_STR_MAX_LENGTH - originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data.insert(size_t{ 4u }, appendMax); // already full
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = _STR_MAX_LENGTH + 4u - originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - (_STR_MAX_LENGTH + 4u - originalSize)], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(size_t{ 4u }, appendMax, size_t{ 2u });
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize + 2u, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 2u]);
  data.insert(size_t{ 4u }, appendMax, _STR_MAX_LENGTH - originalSize);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u + _STR_MAX_LENGTH - originalSize]);
  }
  for (size_t i = 4u; i < originalSize - 2u; ++i) {
    EXPECT_EQ(originalValue[i], data[i + _STR_MAX_LENGTH - originalSize + 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, appendExcess);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  size_t excessSize = excessData.size();
  data = originalValue;
  data.insert(size_t{ 0 }, appendExcess);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < excessSize; ++i) {
    EXPECT_EQ(appendExcess[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - excessSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + excessSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 7u }, appendExcess);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 7u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - 7u; ++i) {
    EXPECT_EQ(appendExcess[i], data[i + 7u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(size_t{ 0 }, empty);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);

  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);

  data = originalValue;
  data.insert(originalSize, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);
  data.insert(originalSize, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize + shortSize, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
    EXPECT_EQ(appendShort[i], data[originalSize + i + shortSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize + shortSize]);

  data = originalValue;
  data.insert(size_t{ 0 }, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);
  data = originalValue;
  data.insert(size_t{ 4u }, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize]);

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, maxData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 0 }, maxData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[_STR_MAX_LENGTH - originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 4u }, maxData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[_STR_MAX_LENGTH - originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data.insert(size_t{ 4u }, maxData); // already full
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = _STR_MAX_LENGTH + 4u - originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendMax[i - (_STR_MAX_LENGTH + 4u - originalSize)], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(size_t{ 4u }, appendMax, size_t{ 2u });
  data.insert(size_t{ 4u }, maxData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u + _STR_MAX_LENGTH - originalSize]);
  }
  for (size_t i = 4u; i < originalSize - 2u; ++i) {
    EXPECT_EQ(originalValue[i], data[i + _STR_MAX_LENGTH - originalSize + 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, excessData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 0 }, excessData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < excessSize; ++i) {
    EXPECT_EQ(appendExcess[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - excessSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + excessSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 7u }, excessData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 7u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - 7u; ++i) {
    EXPECT_EQ(appendExcess[i], data[i + 7u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
}
template <typename _CharType>
void _strInsertSubset(const _CharType* originalValue, const _CharType* appendShort, const _CharType* appendMax, const _CharType* appendExcess) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> shortData(appendShort);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> maxData(appendMax);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> excessData(appendExcess);

  data.insert(size_t{ 0 }, empty, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.insert(size_t{ 0 }, empty, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.insert(size_t{ 0 }, empty, size_t{ 2u }, size_t{ 1u });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData, shortData.size());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.insert(size_t{ 0 }, shortData, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.insert(size_t{ 0 }, shortData, shortData.size());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.insert(size_t{ 2u }, shortData, shortData.size() + 1u);
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.insert(size_t{ 0 }, shortData, shortData.size() + 2u, size_t{ 1u });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());

  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData, size_t{ 0 }, shortData.size());
  EXPECT_EQ(originalSize + 2u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size()]);
  data = originalValue;
  data.insert(size_t{ 0 }, shortData, size_t{ 0 }, shortData.size() + 1u);
  EXPECT_EQ(originalSize + shortData.size(), data.size());
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortData.size()]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size()]);
  data = originalValue;
  data.insert(size_t{ 4u }, shortData, size_t{ 0 }, shortData.size());
  EXPECT_EQ(originalSize + shortData.size(), data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortData.size()]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size()]);

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData, size_t{ 0 }, size_t{ 1u });
  EXPECT_EQ(originalSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < size_t{ 1u }; ++i) {
    EXPECT_EQ(shortData[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data = originalValue;
  data.insert(size_t{ 0 }, shortData, size_t{ 0 }, size_t{ 1u });
  EXPECT_EQ(originalSize + 1u, data.size());
  for (size_t i = 0; i < size_t{ 1u }; ++i) {
    EXPECT_EQ(shortData[i], data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data = originalValue;
  data.insert(size_t{ 4u }, shortData, size_t{ 0 }, size_t{ 1u });
  EXPECT_EQ(originalSize + 1u, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < size_t{ 1u }; ++i) {
    EXPECT_EQ(shortData[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData, size_t{ 1u }, shortData.size());
  EXPECT_EQ(originalSize + shortData.size() - 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size() - 1u; ++i) {
    EXPECT_EQ(shortData[i + 1u], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size() - 1u]);
  data = originalValue;
  data.insert(size_t{ 4u }, shortData, size_t{ 1u }, shortData.size() - 1u);
  EXPECT_EQ(originalSize + shortData.size() - 1u, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size() - 1u; ++i) {
    EXPECT_EQ(shortData[i + 1u], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortData.size() - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size() - 1u]);

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, maxData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 0 }, maxData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(maxData[i], data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + maxData.size()]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 4u }, maxData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < maxData.size(); ++i) {
    EXPECT_EQ(maxData[i], data[i + size_t{ 4u }]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + maxData.size()]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.append(shortData, size_t{ 0 });
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, maxData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i + originalSize]);
  }
  for (size_t i = originalSize + shortData.size(); i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - originalSize - shortData.size()], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.append(shortData, size_t{ 0 });
  data.insert(size_t{ 4u }, maxData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < maxData.size(); ++i) {
    EXPECT_EQ(maxData[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + maxData.size()]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.insert(size_t{ 4u }, maxData, size_t{ 0 }); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < maxData.size(); ++i) {
    EXPECT_EQ(maxData[i], data[i + 4u]);
  }
  for (size_t i = maxData.size() + 4u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - maxData.size() - 4u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, excessData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(excessData[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 6u }, excessData, size_t{ 0 });
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 6u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 6u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(excessData[i - 6u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
}
template <typename _CharType>
void _strInsertRepeat(const _CharType* originalValue, _CharType value, _CharType value2) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  data.insert(size_t{ 0 }, size_t{ 0 }, value);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);

  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 1u }, value);
  EXPECT_EQ(originalSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 2u }, value2);
  EXPECT_EQ(originalSize + 3u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(value2, data[originalSize + 1u]);
  EXPECT_EQ(value2, data[originalSize + 2u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 3u]);
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(value2, data[originalSize + 1u]);
  EXPECT_EQ(value2, data[originalSize + 2u]);
  for (size_t i = originalSize + 3u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data.insert(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(value2, data[originalSize + 1u]);
  EXPECT_EQ(value2, data[originalSize + 2u]);
  for (size_t i = originalSize + 3u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(size_t{ 0 }, size_t{ 1u }, value);
  EXPECT_EQ(originalSize + 1u, data.size());
  EXPECT_EQ(value, data[0]);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data.insert(size_t{ 0 }, size_t{ 2u }, value2);
  EXPECT_EQ(originalSize + 3u, data.size());
  EXPECT_EQ(value2, data[0]);
  EXPECT_EQ(value2, data[1]);
  EXPECT_EQ(value, data[2]);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 3u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 3u]);
  data.insert(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data.insert(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value2); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value2, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(size_t{ 2u }, size_t{ 1u }, value);
  EXPECT_EQ(originalSize + 1u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  EXPECT_EQ(value, data[2]);
  for (size_t i = 2u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data.insert(size_t{ 2u }, size_t{ 2u }, value2);
  EXPECT_EQ(originalSize + 3u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  EXPECT_EQ(value2, data[2]);
  EXPECT_EQ(value2, data[3]);
  EXPECT_EQ(value, data[4]);
  for (size_t i = 2u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 3u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 3u]);
  data.insert(size_t{ 2u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 2u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data.insert(size_t{ 2u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value2); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 2u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value2, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(size_t{ 0 }, _STR_MAX_LENGTH - originalSize, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + _STR_MAX_LENGTH - originalSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.insert(size_t{ 2u }, _STR_MAX_LENGTH - originalSize, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(value, data[i + 2u]);
  }
  for (size_t i = 2u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + _STR_MAX_LENGTH - originalSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.insert(size_t{ 0 }, _STR_MAX_LENGTH - originalSize - 1u, value);
  EXPECT_EQ(_STR_MAX_LENGTH - 1u, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize - 1u; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + _STR_MAX_LENGTH - originalSize - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH - 1u]);
  data = originalValue;
  data.insert(originalSize, _STR_MAX_LENGTH - originalSize - 1u, value);
  EXPECT_EQ(_STR_MAX_LENGTH - 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH - 1u; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH - 1u]);
}
template <typename _CharType>
void _strLeftPad(_CharType padValue, const _CharType* originalValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  EXPECT_TRUE(empty.empty());

  empty.lpad(size_t{ 0 }, padValue);
  EXPECT_TRUE(empty.empty());
  empty.lpad(size_t{ 8u }, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_FALSE(empty.full());
  EXPECT_EQ(size_t{ 8u }, empty.size());
  for (size_t i = 0; i < size_t{ 8u }; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[8]);
  empty.lpad(_STR_MAX_LENGTH, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.full());
  EXPECT_EQ(_STR_MAX_LENGTH, empty.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[_STR_MAX_LENGTH]);
  empty.lpad(size_t{ 8u }, padValue);
  EXPECT_FALSE(empty.empty());
  EXPECT_TRUE(empty.full());
  EXPECT_EQ(_STR_MAX_LENGTH, empty.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(padValue, empty[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), empty[_STR_MAX_LENGTH]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  data.lpad(size_t{ 0 }, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);
  data.lpad(originalSize - 2u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);
  data.lpad(originalSize, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);

  data.lpad(originalSize + 4u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_FALSE(data.full());
  EXPECT_EQ(originalSize + 4u, data.size());
  EXPECT_FALSE(data == originalValue);
  EXPECT_EQ(padValue, data[0]);
  EXPECT_EQ(padValue, data[1u]);
  EXPECT_EQ(padValue, data[2u]);
  EXPECT_EQ(padValue, data[3u]);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 4u]);

  data.lpad(_STR_MAX_LENGTH, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_FALSE(data == originalValue);
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(padValue, data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + _STR_MAX_LENGTH - originalSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.lpad(_STR_MAX_LENGTH * 2u, padValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_FALSE(data == originalValue);
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(padValue, data[i]);
  }
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + _STR_MAX_LENGTH - originalSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
}

TEST_F(FixedSizeStringTestOp, insertData) {
  _strInsertData<char>("azerty01", "", "42", "123456789aze", "123456789azerty");
  _strInsertData<wchar_t>(L"azerty01", L"", L"42", L"123456789aze", L"123456789azerty");
  _strInsertData<char16_t>(u"azerty01", u"", u"42", u"123456789aze", u"123456789azerty");
  _strInsertData<char32_t>(U"azerty01", U"", U"42", U"123456789aze", U"123456789azerty");
}
TEST_F(FixedSizeStringTestOp, insertSubset) {
  _strInsertSubset<char>("azerty01", "42", "123456789aze", "123456789azerty");
  _strInsertSubset<wchar_t>(L"azerty01", L"42", L"123456789aze", L"123456789azerty");
  _strInsertSubset<char16_t>(u"azerty01", u"42", u"123456789aze", u"123456789azerty");
  _strInsertSubset<char32_t>(U"azerty01", U"42", U"123456789aze", U"123456789azerty");
}
TEST_F(FixedSizeStringTestOp, insertRepeat) {
  _strInsertRepeat<char>("azerty01", 'a', 'b');
  _strInsertRepeat<wchar_t>(L"azerty01", L'a', L'b');
  _strInsertRepeat<char16_t>(u"azerty01", u'a', u'b');
  _strInsertRepeat<char32_t>(U"azerty01", U'a', U'b');
}
TEST_F(FixedSizeStringTestOp, leftPad) {
  _strLeftPad<char>(' ', "azerty01");
  _strLeftPad<wchar_t>(L' ', L"azerty01");
  _strLeftPad<char16_t>(u' ', u"azerty01");
  _strLeftPad<char32_t>(U' ', U"azerty01");
}

#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic pop
#endif
#endif
