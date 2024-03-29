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

class FixedSizeStringTestOp2 : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- operations - replace --

template <typename _CharType>
void _strReplaceData(const _CharType* originalValue, const _CharType* emptyValue, const _CharType* appendShort, const _CharType* appendMax, const _CharType* appendExcess) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> shortData(appendShort);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> maxData(appendMax);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> excessData(appendExcess);
  size_t shortSize = shortData.size();

  data.replace(size_t{ 0 }, size_t{ 0 }, nullptr);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.replace(size_t{ 0 }, size_t{ 0 }, emptyValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.replace(size_t{ 0 }, size_t{ 0 }, appendShort, size_t{ 0 });
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data = originalValue;
  data.replace(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, nullptr);
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
  data = originalValue;
  data.replace(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, emptyValue);
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
  data = originalValue;
  data.replace(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, appendShort, size_t{ 0 });
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
  data = originalValue;
  data.replace(size_t{ 2u }, size_t{ 2u }, nullptr);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize - 2u, data.size());
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 2u]);
  data = originalValue;
  data.replace(size_t{ 2u }, size_t{ 2u }, emptyValue);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize - 2u, data.size());
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 2u]);
  data = originalValue;
  data.replace(size_t{ 2u }, size_t{ 2u }, appendShort, size_t{ 0 });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize - 2u, data.size());
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 2u]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 0 }, appendShort);
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
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortSize, appendShort, size_t{ 1u });
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
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, appendShort);
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
  data.replace(originalSize, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, appendShort);
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
  data.replace(originalSize, size_t{ 1u }, appendShort + 1, size_t{ 1u });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(appendShort[1], data[originalSize]);
  for (size_t i = 1; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize + 1]);

  data = originalValue;
  data.replace(size_t{ 0 }, size_t{ 0 }, appendShort);
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
  data.replace(size_t{ 0 }, size_t{ 0 }, appendShort, size_t{ 1u });
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
  data.replace(size_t{ 4u }, size_t{ 0 }, appendShort);
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
  data.replace(size_t{ 0 }, size_t{ 2u }, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 2u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  for (size_t i = 2u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 2u]);
  data.replace(size_t{ 0 }, size_t{ 2u }, appendShort + 1u, size_t{ 1u });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 3u, data.size());
  EXPECT_EQ(appendShort[1], data[0]);
  for (size_t i = 2u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortSize - 3u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 3u]);
  data = originalValue;
  data.replace(size_t{ 0 }, originalSize - 1u, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(shortSize + 1u, data.size());
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  EXPECT_EQ(originalValue[originalSize - 1u], data[shortSize]);
  EXPECT_EQ(static_cast<_CharType>(0), data[shortSize + 1u]);
  data = originalValue;
  data.replace(size_t{ 4u }, size_t{ 2u }, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 2u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 4u]);
  }
  for (size_t i = 6u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortSize - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 2u]);
  
  data = originalValue;
  data.replace(size_t{ 0 }, size_t{ 4u }, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 4u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i - 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 4u]);
  data.replace(size_t{ 0 }, size_t{ 4u }, appendShort + 1u, size_t{ 1u });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 7u, data.size());
  EXPECT_EQ(appendShort[1], data[0]);
  for (size_t i = 6u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortSize - 7u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 7u]);
  data = originalValue;
  data.replace(size_t{ 4u }, size_t{ 3u }, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 3u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 4u]);
  }
  for (size_t i = 7u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortSize - 3u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 3u]);

  data = originalValue;
  data.replace(size_t{ 0 }, originalSize, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortSize]);
  data.replace(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, appendShort, size_t{ 1u });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(size_t{ 1u }, data.size());
  EXPECT_EQ(appendShort[0], data[0]);
  EXPECT_EQ(static_cast<_CharType>(0), data[1]);
  data = originalValue;
  data.replace(size_t{ 4u }, originalSize, appendShort);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(shortSize + 4u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortSize + 4u]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 0 }, appendMax);
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
  data.replace(size_t{ 0 }, size_t{ 0 }, appendMax);
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
  data.replace(size_t{ 4u }, size_t{ 0 }, appendMax);
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
  data.replace(size_t{ 4u }, size_t{ 0 }, appendMax); // already full
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
  data.replace(size_t{ 0 }, size_t{ 4u }, appendMax);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_STR_MAX_LENGTH - 4u, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[_STR_MAX_LENGTH - originalSize + i - 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH - 4u]);
  data = originalValue;
  data.replace(size_t{ 4u }, size_t{ 4u }, appendMax);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_STR_MAX_LENGTH - 4u, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH - 4u]);
  data.replace(size_t{ 4u }, size_t{ 4u }, appendMax);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[_STR_MAX_LENGTH - originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.replace(size_t{ 4u }, size_t{ 3u }, appendMax, size_t{ 2u });
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize - 1u, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = 7u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 1u]);
  data.replace(size_t{ 4u }, originalSize, appendMax, _STR_MAX_LENGTH - originalSize);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_STR_MAX_LENGTH + 4u - originalSize, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH + 4u - originalSize]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 1 }, appendExcess);
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
  data.replace(size_t{ 0 }, size_t{ 1 }, appendExcess);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < excessSize; ++i) {
    EXPECT_EQ(appendExcess[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - excessSize; ++i) {
    EXPECT_EQ(originalValue[i + 1u], data[i + excessSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.replace(size_t{ 7u }, size_t{ 1 }, appendExcess);
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
  data.replace(size_t{ 0 }, size_t{ 0 }, empty);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.replace(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, empty);
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
  data = originalValue;
  data.replace(size_t{ 2u }, size_t{ 2u }, empty);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize - 2u, data.size());
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 2u]);
  
  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 0 }, shortData);
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
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData);
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
  data.replace(originalSize, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData);
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
  data.replace(size_t{ 0 }, size_t{ 0 }, shortData);
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
  data.replace(size_t{ 4u }, size_t{ 0 }, shortData);
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
  data.replace(size_t{ 0 }, size_t{ 2u }, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 2u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  for (size_t i = 2u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 2u]);
  data = originalValue;
  data.replace(size_t{ 0 }, originalSize - 1u, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(shortSize + 1u, data.size());
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  EXPECT_EQ(originalValue[originalSize - 1u], data[shortSize]);
  EXPECT_EQ(static_cast<_CharType>(0), data[shortSize + 1u]);
  data = originalValue;
  data.replace(size_t{ 4u }, size_t{ 2u }, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 2u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 4u]);
  }
  for (size_t i = 6u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortSize - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 2u]);

  data = originalValue;
  data.replace(size_t{ 0 }, size_t{ 4u }, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 4u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[shortSize + i - 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 4u]);
  data = originalValue;
  data.replace(size_t{ 4u }, size_t{ 3u }, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(originalSize + shortSize - 3u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 4u]);
  }
  for (size_t i = 7u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortSize - 3u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortSize - 3u]);

  data = originalValue;
  data.replace(size_t{ 0 }, originalSize, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(shortSize, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortSize]);
  data = originalValue;
  data.replace(size_t{ 4u }, originalSize, shortData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(shortSize + 4u, data.size());
  EXPECT_TRUE(data != originalValue);
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortSize; ++i) {
    EXPECT_EQ(appendShort[i], data[i + 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortSize + 4u]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 0 }, maxData);
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
  data.replace(size_t{ 0 }, size_t{ 0 }, maxData);
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
  data.replace(size_t{ 4u }, size_t{ 0 }, maxData);
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
  data.replace(size_t{ 4u }, size_t{ 0 }, maxData); // already full
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
  data.replace(size_t{ 0 }, size_t{ 4u }, maxData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_STR_MAX_LENGTH - 4u, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[_STR_MAX_LENGTH - originalSize + i - 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH - 4u]);
  data = originalValue;
  data.replace(size_t{ 4u }, size_t{ 4u }, maxData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_STR_MAX_LENGTH - 4u, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH - 4u]);
  data.replace(size_t{ 4u }, size_t{ 4u }, maxData);
  EXPECT_FALSE(data.empty());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[i + 4u]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(appendMax[i], data[_STR_MAX_LENGTH - originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 1 }, excessData);
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
  data.replace(size_t{ 0 }, size_t{ 1 }, excessData);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data.full());
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < excessSize; ++i) {
    EXPECT_EQ(appendExcess[i], data[i]);
  }
  for (size_t i = 0; i < _STR_MAX_LENGTH - excessSize; ++i) {
    EXPECT_EQ(originalValue[i + 1u], data[i + excessSize]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.replace(size_t{ 7u }, size_t{ 1 }, excessData);
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
void _strReplaceWithSubset(const _CharType* originalValue, const _CharType* appendShort, const _CharType* appendMax, const _CharType* appendExcess) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> shortData(appendShort);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> maxData(appendMax);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> excessData(appendExcess);

  data.replace(size_t{ 0 }, size_t{ 0 }, empty, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.replace(size_t{ 0 }, size_t{ 0 }, empty, size_t{ 2u }, size_t{ 1u });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.replace(size_t{ 0 }, size_t{ 0 }, empty, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.replace(size_t{ 4u }, originalSize - 5u, empty, size_t{ 2u }, size_t{ 1u });
  EXPECT_EQ(size_t{ 5u }, data.size());
  EXPECT_EQ(originalValue[3], data[3]);
  EXPECT_EQ(originalValue[originalSize - 1u], data[4]);
  EXPECT_EQ(static_cast<_CharType>(0), data[5]);
  data.replace(size_t{ 0 }, originalSize, empty, size_t{ 2u }, size_t{ 1u });
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);

  data = originalValue;
  data.replace(size_t{ 0 }, size_t{ 0 }, shortData, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(originalSize, data.size());
  data.replace(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData, size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data.empty());
  data = originalValue;
  data.replace(size_t{ 0 }, shortData.size(), shortData, shortData.size(), size_t{ 1u });
  EXPECT_EQ(originalSize - shortData.size(), data.size());
  for (size_t i = shortData.size(); i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - shortData.size()]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - shortData.size()]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, shortData.size(), shortData, size_t{ 0 }, shortData.size());
  EXPECT_EQ(originalSize + shortData.size(), data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[originalSize + i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + shortData.size()]);
  data = originalValue;
  data.replace(size_t{ 0 }, shortData.size(), shortData, size_t{ 0 }, shortData.size() + 1u);
  EXPECT_EQ(originalSize, data.size());
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i]);
  }
  for (size_t i = shortData.size(); i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data = originalValue;
  data.replace(size_t{ 4u }, size_t{ 3u }, shortData, size_t{ 0 }, shortData.size());
  EXPECT_EQ(originalSize - 3u + shortData.size(), data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i + 4u]);
  }
  for (size_t i = 7u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 3u + shortData.size()]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 3u + shortData.size()]);
  data = originalValue;
  data.replace(size_t{ 2u }, originalSize - 2u, shortData, size_t{ 0 }, shortData.size());
  EXPECT_EQ(shortData.size() + 2u, data.size());
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i + 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortData.size() + 2u]);
  data = originalValue;
  data.replace(size_t{ 2u }, size_t{ 150u }, shortData, size_t{ 0 }, size_t{ 1u });
  EXPECT_EQ(size_t{ 3u }, data.size());
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(shortData[0], data[2u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[3u]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 4u }, maxData, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.append(shortData);
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 4u }, maxData, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
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
  data.replace(size_t{ 4u }, size_t{ 0 }, maxData, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
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
  data = originalValue;
  data.append(originalValue);
  data.replace(size_t{ 4u }, size_t{ 2u }, maxData, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 0; i < maxData.size(); ++i) {
    EXPECT_EQ(maxData[i], data[i + 4u]);
  }
  for (size_t i = 6u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 2u + maxData.size()]);
  }
  for (size_t i = maxData.size() + originalSize - 2u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(originalValue[i- maxData.size() - originalSize + 2u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.append(originalValue);
  data.replace(size_t{ 10u }, size_t{ 2u }, maxData, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < size_t{ 10u }; ++i) {
    EXPECT_EQ(originalValue[i - originalSize], data[i]);
  }
  for (size_t i = 10u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - 10u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data.replace(size_t{ 10u }, size_t{ 2u }, maxData, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < size_t{ 10u }; ++i) {
    EXPECT_EQ(originalValue[i - originalSize], data[i]);
  }
  for (size_t i = 10u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(maxData[i - 10u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 4u }, excessData, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(excessData[i - originalSize], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data = originalValue;
  data.replace(size_t{ 6u }, size_t{ 2u }, excessData, size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
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
void _strReplaceWithRepeat(const _CharType* originalValue, _CharType value, _CharType value2) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  data.replace(size_t{ 0 }, size_t{ 0 }, size_t{ 0 }, value);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 0 }, value);
  EXPECT_EQ(originalSize, data.size());
  EXPECT_TRUE(data == originalValue);
  data.replace(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 0 }, value);
  EXPECT_EQ(size_t{ 0 }, data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
  data = originalValue;
  data.replace(size_t{ 4u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 0 }, value);
  EXPECT_EQ(size_t{ 4u }, data.size());
  for (size_t i = 0; i < size_t{ 4u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[4u]);
  data = originalValue;
  data.replace(size_t{ 2u }, size_t{ 2u }, size_t{ 0 }, value);
  EXPECT_EQ(originalSize - 2u, data.size());
  for (size_t i = 0; i < size_t{ 2u }; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 2u]);

  data = originalValue;
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 0 }, size_t{ 1u }, value);
  EXPECT_EQ(originalSize + 1u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 2u }, size_t{ 2u }, value2);
  EXPECT_EQ(originalSize + 3u, data.size());
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  EXPECT_EQ(value, data[originalSize]);
  EXPECT_EQ(value2, data[originalSize + 1u]);
  EXPECT_EQ(value2, data[originalSize + 2u]);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 3u]);
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value);
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
  data.replace(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 2u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value); // already full
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
  data.replace(size_t{ 0 }, size_t{ 0 }, size_t{ 1u }, value);
  EXPECT_EQ(originalSize + 1u, data.size());
  EXPECT_EQ(value, data[0]);
  for (size_t i = 0; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data.replace(size_t{ 0 }, size_t{ 2u }, size_t{ 2u }, value2);
  EXPECT_EQ(originalSize + 1u, data.size());
  EXPECT_EQ(value2, data[0]);
  EXPECT_EQ(value2, data[1]);
  for (size_t i = 1u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 3u]);
  data.replace(size_t{ 0 }, size_t{ 1u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data.replace(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value2); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value2, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.replace(size_t{ 2u }, size_t{ 2u }, size_t{ 1u }, value);
  EXPECT_EQ(originalSize - 1u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  EXPECT_EQ(value, data[2]);
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize -+ 1u]);
  data.replace(size_t{ 2u }, size_t{ 0 }, size_t{ 2u }, value2);
  EXPECT_EQ(originalSize + 1u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  EXPECT_EQ(value2, data[2]);
  EXPECT_EQ(value2, data[3]);
  EXPECT_EQ(value, data[4]);
  for (size_t i = 4u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize + 1u]);
  data.replace(size_t{ 2u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 2u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
  data.replace(size_t{ 2u }, size_t{ 2u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, value2); // already full
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 2u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(value2, data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
}
template <typename _CharType>
void _strReplaceIteration(const _CharType* originalValue, const _CharType* appendShort, const _CharType* appendMax, const _CharType* appendExcess) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_FALSE(data.empty());
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  FixedSizeString<_STR_MAX_LENGTH, _CharType> shortData(appendShort);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> maxData(appendMax);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> excessData(appendExcess);

  data.replace(nullptr, &data.back(), shortData);
  EXPECT_TRUE(data == originalValue);
  data.replace(&data.front(), nullptr, shortData);
  EXPECT_TRUE(data == originalValue);
  data.replace(&data.back(), &data.front(), shortData);
  EXPECT_TRUE(data == originalValue);
  data.replace(&data[1], &data.front(), shortData);
  EXPECT_TRUE(data == originalValue);
  data.replace(&shortData.front(), &shortData.back(), shortData);
  EXPECT_TRUE(data == originalValue);

  data.replace(&data[0], &data[data.size()], empty);
  EXPECT_TRUE(data.empty());
  data = originalValue;
  data.replace(&data[0], &data[data.size()], shortData);
  EXPECT_TRUE(data == shortData);
  data = originalValue;
  data.replace(&data[0], &data[data.size()], maxData);
  EXPECT_TRUE(data == maxData);
  data = originalValue;
  data.replace(&data[0], &data[data.size()], excessData + excessData);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < excessData.size(); ++i) {
    EXPECT_EQ(appendExcess[i], data[i]);
  }
  for (size_t i = excessData.size(); i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - excessData.size()], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.replace(&data[2], &data[data.size()], empty);
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  EXPECT_EQ(static_cast<_CharType>(0), data[2]);
  data = originalValue;
  data.replace(&data[2], &data[data.size()], shortData);
  EXPECT_EQ(shortData.size() + 2u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i + 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortData.size() + 2u]);
  data = originalValue;
  data.replace(&data[2], &data[data.size()], excessData + excessData);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 0; i < excessData.size(); ++i) {
    EXPECT_EQ(appendExcess[i], data[i + 2u]);
  }
  for (size_t i = excessData.size() + 2u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - excessData.size() - 2u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.replace(&data[2], &data[3], empty);
  EXPECT_EQ(originalSize - 1u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 3u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 1u]);
  data = originalValue;
  data.replace(&data[2], &data[3], shortData);
  EXPECT_EQ(shortData.size() + originalSize - 1u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(shortData[i], data[i + 2u]);
  }
  for (size_t i = 3u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortData.size() - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortData.size() + originalSize - 1u]);
  data = originalValue;
  data.replace(&data[2], &data[3], excessData + excessData);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 0; i < excessData.size(); ++i) {
    EXPECT_EQ(appendExcess[i], data[i + 2u]);
  }
  for (size_t i = excessData.size() + 2u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - excessData.size() - 2u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.replace(nullptr, &data.back(), appendShort);
  EXPECT_TRUE(data == originalValue);
  data.replace(&data.front(), nullptr, appendShort);
  EXPECT_TRUE(data == originalValue);
  data.replace(&data.back(), &data.front(), appendShort);
  EXPECT_TRUE(data == originalValue);
  data.replace(&data[1], &data.front(), appendShort);
  EXPECT_TRUE(data == originalValue);
  data.replace(&shortData.front(), &shortData.back(), appendShort);
  EXPECT_TRUE(data == originalValue);

  data.replace(&data.front(), &data[data.size()], nullptr);
  EXPECT_TRUE(data.empty());
  data = originalValue;
  data.replace(&data[0], &data[data.size()], appendShort);
  EXPECT_TRUE(data == appendShort);
  data = originalValue;
  data.replace(&data[0], &data[data.size()], appendMax);
  EXPECT_TRUE(data == appendMax);
  data = originalValue;
  data.replace(&data[originalSize - 1u], &data[_STR_MAX_LENGTH], excessData);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize - 1u; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize - 1u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - originalSize + 1u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);

  data = originalValue;
  data.replace(&data[2], &data[data.size()], nullptr);
  EXPECT_EQ(size_t{ 2u }, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  EXPECT_EQ(static_cast<_CharType>(0), data[2]);
  data = originalValue;
  data.replace(&data[2], &data[data.size()], appendShort);
  EXPECT_EQ(shortData.size() + 2u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(appendShort[i], data[i + 2u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortData.size() + 2u]);

  data = originalValue;
  data.replace(&data[2], &data[3], appendShort);
  EXPECT_EQ(shortData.size() + originalSize - 1u, data.size());
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1], data[1]);
  for (size_t i = 0; i < shortData.size(); ++i) {
    EXPECT_EQ(appendShort[i], data[i + 2u]);
  }
  for (size_t i = 3u; i < originalSize; ++i) {
    EXPECT_EQ(originalValue[i], data[i + shortData.size() - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[shortData.size() + originalSize - 1u]);
  data = originalValue;
  data.replace(&data[originalSize - 2u], &data[originalSize - 1u], excessData);
  EXPECT_EQ(_STR_MAX_LENGTH, data.size());
  for (size_t i = 0; i < originalSize - 2u; ++i) {
    EXPECT_EQ(originalValue[i], data[i]);
  }
  for (size_t i = originalSize - 2u; i < _STR_MAX_LENGTH; ++i) {
    EXPECT_EQ(appendExcess[i - originalSize + 2u], data[i]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[_STR_MAX_LENGTH]);
}

TEST_F(FixedSizeStringTestOp2, replaceData) {
  _strReplaceData<char>("azerty01", "", "42", "123456789aze", "123456789azerty");
  _strReplaceData<wchar_t>(L"azerty01", L"", L"42", L"123456789aze", L"123456789azerty");
  _strReplaceData<char16_t>(u"azerty01", u"", u"42", u"123456789aze", u"123456789azerty");
  _strReplaceData<char32_t>(U"azerty01", U"", U"42", U"123456789aze", U"123456789azerty");
}
TEST_F(FixedSizeStringTestOp2, replaceWithSubset) {
  _strReplaceWithSubset<char>("azerty01", "42", "123456789aze", "123456789azerty");
  _strReplaceWithSubset<wchar_t>(L"azerty01", L"42", L"123456789aze", L"123456789azerty");
  _strReplaceWithSubset<char16_t>(u"azerty01", u"42", u"123456789aze", u"123456789azerty");
  _strReplaceWithSubset<char32_t>(U"azerty01", U"42", U"123456789aze", U"123456789azerty");
}
TEST_F(FixedSizeStringTestOp2, replaceWithRepeat) {
  _strReplaceWithRepeat<char>("azerty01", 'a', 'b');
  _strReplaceWithRepeat<wchar_t>(L"azerty01", L'a', L'b');
  _strReplaceWithRepeat<char16_t>(u"azerty01", u'a', u'b');
  _strReplaceWithRepeat<char32_t>(U"azerty01", U'a', U'b');
}
TEST_F(FixedSizeStringTestOp2, replaceIteration) {
  _strReplaceIteration<char>("azerty01", "42", "123456789aze", "123456789azerty");
  _strReplaceIteration<wchar_t>(L"azerty01", L"42", L"123456789aze", L"123456789azerty");
  _strReplaceIteration<char16_t>(u"azerty01", u"42", u"123456789aze", u"123456789azerty");
  _strReplaceIteration<char32_t>(U"azerty01", U"42", U"123456789aze", U"123456789azerty");
}


// -- operations - erase --

template <typename _CharType>
void _strEraseData(const _CharType* originalValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);

  empty.erase(size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);
  empty.erase(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);
  empty.erase(size_t{ 2u }, size_t{ 1u });
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  data.erase(FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos, size_t{ 1 });
  EXPECT_TRUE(data == originalValue);
  data.erase(size_t{ 0 }, size_t{ 0 });
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);

  data.erase(size_t{ 0 }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
  data = originalValue;
  data.erase(size_t{ 2u }, FixedSizeString<_STR_MAX_LENGTH, _CharType>::npos);
  for (size_t pos = 0; pos < 2u; ++pos) {
    EXPECT_EQ(originalValue[pos], data[pos]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[2]);
  data = originalValue;
  data.erase(size_t{ 2u }, size_t{ 1u });
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1u], data[1u]);
  for (size_t pos = 3u; pos < originalSize; ++pos) {
    EXPECT_EQ(originalValue[pos], data[pos - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 1u]);
}
template <typename _CharType>
void _strEraseIteration(const _CharType* originalValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);

  empty.erase(&empty[0]);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);
  empty.erase(&empty[2u]);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);

  empty.erase(&empty[0], &empty[0]);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);
  empty.erase(&empty[0], &empty[_STR_MAX_LENGTH]);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);
  empty.erase(&empty[2u], &empty[3u]);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  data.erase(&data[data.size()]);
  EXPECT_TRUE(data == originalValue);
  data.erase(&data[0]);
  EXPECT_EQ(originalSize - 1u, data.size());
  for (size_t pos = 1u; pos < originalSize; ++pos) {
    EXPECT_EQ(originalValue[pos], data[pos - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 1u]);
  data = originalValue;
  data.erase(&data[2u]);
  EXPECT_EQ(originalSize - 1u, data.size());
  for (size_t pos = 0; pos < 2u; ++pos) {
    EXPECT_EQ(originalValue[pos], data[pos]);
  }
  for (size_t pos = 3u; pos < originalSize; ++pos) {
    EXPECT_EQ(originalValue[pos], data[pos - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 1u]);

  data = originalValue;
  data.erase(&data[data.size()], &data[_STR_MAX_LENGTH]);
  EXPECT_TRUE(data == originalValue);
  data.erase(&data[2u], &data[0]);
  EXPECT_TRUE(data == originalValue);
  data.erase(&data[0], &data[0]);
  EXPECT_TRUE(data == originalValue);
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize]);
  data.erase(&data[0], &data[data.size()]);
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
  data = originalValue;
  data.erase(&data[2u], &data[_STR_MAX_LENGTH]);
  for (size_t pos = 0; pos < 2u; ++pos) {
    EXPECT_EQ(originalValue[pos], data[pos]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[2]);
  data = originalValue;
  data.erase(&data[2u], &data[3u]);
  EXPECT_EQ(originalValue[0], data[0]);
  EXPECT_EQ(originalValue[1u], data[1u]);
  for (size_t pos = 3u; pos < originalSize; ++pos) {
    EXPECT_EQ(originalValue[pos], data[pos - 1u]);
  }
  EXPECT_EQ(static_cast<_CharType>(0), data[originalSize - 1u]);
}
template <typename _CharType>
void _strPopBack(const _CharType* originalValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> empty;
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);

  EXPECT_FALSE(empty.pop_back());
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(static_cast<_CharType>(0), empty[0]);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_TRUE(data == originalValue);
  size_t originalSize = data.size();

  for (size_t i = originalSize; i > 1; --i) {
    EXPECT_TRUE(data.pop_back());
    for (size_t pos = 0; pos < i - 1u; ++pos) {
      EXPECT_EQ(originalValue[pos], data[pos]);
    }
    EXPECT_EQ(static_cast<_CharType>(0), data[i - 1u]);
  }
  EXPECT_TRUE(data.pop_back());
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
  EXPECT_FALSE(data.pop_back());
  EXPECT_TRUE(data.empty());
  EXPECT_EQ(static_cast<_CharType>(0), data[0]);
}
template <typename _CharType>
void _strTrimData(const _CharType* originalValue, const _CharType* ltrimmedValue, const _CharType* rtrimmedValue, const _CharType* trimmedValue) {
  FixedSizeString<_STR_MAX_LENGTH, _CharType> data(originalValue);
  EXPECT_TRUE(data == originalValue);

  FixedSizeString<_STR_MAX_LENGTH, _CharType> trimmed(trimmedValue);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> ltrimmed(ltrimmedValue);
  FixedSizeString<_STR_MAX_LENGTH, _CharType> rtrimmed(rtrimmedValue);

  data.trim();
  EXPECT_TRUE(data == trimmedValue);
  EXPECT_EQ(trimmed.size(), data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[data.size()]);

  data = originalValue;
  EXPECT_TRUE(data == originalValue);
  data.ltrim();
  EXPECT_TRUE(data == ltrimmedValue);
  EXPECT_EQ(ltrimmed.size(), data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[data.size()]);

  data = originalValue;
  EXPECT_TRUE(data == originalValue);
  data.rtrim();
  EXPECT_TRUE(data == rtrimmedValue);
  EXPECT_EQ(rtrimmed.size(), data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[data.size()]);

  data = originalValue;
  EXPECT_TRUE(data == originalValue);
  data.ltrim();
  data.rtrim();
  EXPECT_TRUE(data == trimmedValue);
  EXPECT_EQ(trimmed.size(), data.size());
  EXPECT_EQ(static_cast<_CharType>(0), data[data.size()]);
}

TEST_F(FixedSizeStringTestOp2, eraseData) {
  _strEraseData<char>("azerty01");
  _strEraseData<wchar_t>(L"azerty01");
  _strEraseData<char16_t>(u"azerty01");
  _strEraseData<char32_t>(U"azerty01");
}
TEST_F(FixedSizeStringTestOp2, eraseIteration) {
  _strEraseIteration<char>("azerty01");
  _strEraseIteration<wchar_t>(L"azerty01");
  _strEraseIteration<char16_t>(u"azerty01");
  _strEraseIteration<char32_t>(U"azerty01");
}
TEST_F(FixedSizeStringTestOp2, popBack) {
  _strPopBack<char>("azerty01");
  _strPopBack<wchar_t>(L"azerty01");
  _strPopBack<char16_t>(u"azerty01");
  _strPopBack<char32_t>(U"azerty01");
}
TEST_F(FixedSizeStringTestOp2, trimData) {
  _strTrimData<char>("    azerty01      ", "azerty01      ", "    azerty01", "azerty01");
  _strTrimData<wchar_t>(L"    azerty01      ", L"azerty01      ", L"    azerty01", L"azerty01");
  _strTrimData<char16_t>(u"    azerty01      ", u"azerty01      ", u"    azerty01", u"azerty01");
  _strTrimData<char32_t>(U"    azerty01      ", U"azerty01      ", U"    azerty01", U"azerty01");
  _strTrimData<char>("azerty01      ", "azerty01      ", "azerty01", "azerty01");
  _strTrimData<wchar_t>(L"azerty01      ", L"azerty01      ", L"azerty01", L"azerty01");
  _strTrimData<char16_t>(u"azerty01      ", u"azerty01      ", u"azerty01", u"azerty01");
  _strTrimData<char32_t>(U"azerty01      ", U"azerty01      ", U"azerty01", U"azerty01");
  _strTrimData<char>("    azerty01", "azerty01", "    azerty01", "azerty01");
  _strTrimData<wchar_t>(L"    azerty01", L"azerty01", L"    azerty01", L"azerty01");
  _strTrimData<char16_t>(u"    azerty01", u"azerty01", u"    azerty01", u"azerty01");
  _strTrimData<char32_t>(U"    azerty01", U"azerty01", U"    azerty01", U"azerty01");
  _strTrimData<char>("", "", "", "");
  _strTrimData<wchar_t>(L"", L"", L"", L"");
  _strTrimData<char16_t>(u"", u"", u"", u"");
  _strTrimData<char32_t>(U"", U"", U"", U"");
}

#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic pop
#endif
#endif
