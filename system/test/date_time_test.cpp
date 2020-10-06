#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstdio>
#include <gtest/gtest.h>
#include <system/date_time.h>

using namespace pandora::system;

class DateTimeTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- date/time formatters --

TEST_F(DateTimeTest, intToFixedDigitString) {
  char buffer[8]{ 'a' };
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 0u));
  EXPECT_STREQ("000", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 1u));
  EXPECT_STREQ("001", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 9u));
  EXPECT_STREQ("009", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 10u));
  EXPECT_STREQ("010", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 11u));
  EXPECT_STREQ("011", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 42u));
  EXPECT_STREQ("042", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 99u));
  EXPECT_STREQ("099", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 100u));
  EXPECT_STREQ("100", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 101u));
  EXPECT_STREQ("101", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 127u));
  EXPECT_STREQ("127", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 999u));
  EXPECT_STREQ("999", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 1000u));
  EXPECT_STREQ("000", buffer);
  EXPECT_EQ(size_t{ 3u }, _toFixedDigitString<size_t{ 3u }>(buffer, 2999u));
  EXPECT_STREQ("999", buffer);
}

#define _FORMAT_DATE_TIME(date,time,sep,size,seconds,nano) formatDateTimeString<TimeReference::utc, DateFormat:: date, TimeFormat:: time, sep>(buffer, size_t{ size }, time_t{ seconds }, nano)
TEST_F(DateTimeTest, formatToDateTimeString) {
  char buffer[64]{ 'a' };
  size_t length = _FORMAT_DATE_TIME(none, none, ' ', 64u, 0u, 0L);
  EXPECT_EQ(size_t{ 0 }, length);
  length = _FORMAT_DATE_TIME(mm_dd_yy, none, ' ', 64u, 0u, 0L);
  EXPECT_EQ(size_t{ 8u }, length);
  EXPECT_STREQ("01/01/70", buffer);
  length = _FORMAT_DATE_TIME(none, h24_mm_ss, ' ', 64u, 0u, 0L);
  EXPECT_EQ(size_t{ 8u }, length);
  EXPECT_STREQ("00:00:00", buffer);
  length = _FORMAT_DATE_TIME(mm_dd_yy, h24_mm_ss, ' ', 64u, 0u, 0L);
  EXPECT_EQ(size_t{ 17u }, length);
  EXPECT_STREQ("01/01/70 00:00:00", buffer);
  length = _FORMAT_DATE_TIME(mm_dd_yy, h24_mm_ss, ',', 64u, 0u, 0L);
  EXPECT_EQ(size_t{ 17u }, length);
  EXPECT_STREQ("01/01/70,00:00:00", buffer);
  
  length = _FORMAT_DATE_TIME(weekday, h12_mm, ' ', 64u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 17u }, length);
  EXPECT_STREQ("Thursday 01:02 AM", buffer);
  length = _FORMAT_DATE_TIME(month, h12_mm, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 16u }, length);
  EXPECT_STREQ("January 01:02 AM", buffer);
  length = _FORMAT_DATE_TIME(year, h12_mm, ' ', 64u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 13u }, length);
  EXPECT_STREQ("1970 01:02 AM", buffer);
  length = _FORMAT_DATE_TIME(wkd_d_mth, none, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 10u }, length);
  EXPECT_STREQ("Thu  1 Jan", buffer);
  length = _FORMAT_DATE_TIME(wkd_mth_d, h24_mm, ' ', 64u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 16u }, length);
  EXPECT_STREQ("Thu Jan  1 01:02", buffer);
  length = _FORMAT_DATE_TIME(wkd_d_mth_year, none, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 15u }, length);
  EXPECT_STREQ("Thu  1 Jan 1970", buffer);
  length = _FORMAT_DATE_TIME(wkd_mth_d_year, h24_mm, ' ', 64u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 21u }, length);
  EXPECT_STREQ("Thu Jan  1 1970 01:02", buffer);
  length = _FORMAT_DATE_TIME(weekday_d_month, h24_mm, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 25u }, length);
  EXPECT_STREQ("Thursday  1 January 01:02", buffer);
  length = _FORMAT_DATE_TIME(weekday_month_d, h24_mm, ' ', 64u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 25u }, length);
  EXPECT_STREQ("Thursday January  1 01:02", buffer);
  length = _FORMAT_DATE_TIME(weekday_d_month_year, h12_mm_ss, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 36u }, length);
  EXPECT_STREQ("Thursday  1 January 1970 01:02:03 AM", buffer);
  length = _FORMAT_DATE_TIME(weekday_month_d_year, h12_mm_ss, ' ', 64u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 36u }, length);
  EXPECT_STREQ("Thursday January  1 1970 01:02:03 AM", buffer);
  length = _FORMAT_DATE_TIME(d_month_year, h12_mm_ss, '-', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 27u }, length);
  EXPECT_STREQ(" 1 January 1970-01:02:03 AM", buffer);
  length = _FORMAT_DATE_TIME(month_d_year, h12_mm_ss, ';', 64u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 27u }, length);
  EXPECT_STREQ("January  1 1970;01:02:03 AM", buffer);
  
  length = _FORMAT_DATE_TIME(dd_mm, h24_mm_ss, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 14u }, length);
  EXPECT_STREQ("01/01 01:02:03", buffer);
  length = _FORMAT_DATE_TIME(mm_dd, h24_mm_ss, ' ', 64u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 14u }, length);
  EXPECT_STREQ("01/01 01:02:03", buffer);
  length = _FORMAT_DATE_TIME(dd_mm_yy, h24_mm_ss_milli, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 21u }, length);
  EXPECT_STREQ("01/01/70 01:02:03.000", buffer);
  length = _FORMAT_DATE_TIME(mm_dd_yy, h24_mm_ss_micro, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 24u }, length);
  EXPECT_STREQ("01/01/70 01:02:03.000000", buffer);
  length = _FORMAT_DATE_TIME(yy_mm_dd, h24_mm_ss_nano, ' ', 64u, 3723u, 0L);
  EXPECT_EQ(size_t{ 27u }, length);
  EXPECT_STREQ("70-01-01 01:02:03.000000000", buffer);
# if (!defined(_CPP_REVISION) || _CPP_REVISION != 14) && defined(TIME_UTC)
    length = _FORMAT_DATE_TIME(dd_mm_year, h24_mm_ss_milli, ' ', 64u, 3723u, 123456789L);
    EXPECT_EQ(size_t{ 23u }, length);
    EXPECT_STREQ("01/01/1970 01:02:03.123", buffer);
    length = _FORMAT_DATE_TIME(mm_dd_year, h24_mm_ss_micro, ' ', 64u, 3723u, 123456789L);
    EXPECT_EQ(size_t{ 26u }, length);
    EXPECT_STREQ("01/01/1970 01:02:03.123456", buffer);
    length = _FORMAT_DATE_TIME(year_mm_dd, h24_mm_ss_nano, ' ', 64u, 3723u, 123456789L);
    EXPECT_EQ(size_t{ 29u }, length);
    EXPECT_STREQ("1970-01-01 01:02:03.123456789", buffer);
# endif

  length = _FORMAT_DATE_TIME(year_mm_dd, h24_mm_ss_nano, ' ', 2u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 0 }, length);
  length = _FORMAT_DATE_TIME(year_mm_dd, h24_mm_ss_nano, ' ', 16u, 3723u, 123456789L);
  EXPECT_EQ(size_t{ 0 }, length);
}
#undef _FORMAT_DATE_TIME

#define _CURRENT_DATE_TIME(zone,date,time,sep,size) getCurrentDateTimeString<TimeReference:: zone, DateFormat:: date, TimeFormat:: time, sep>(buffer, size_t{ size })
TEST_F(DateTimeTest, currentDateTimeString) {
  char buffer[64]{ 'a' };
  size_t length = _CURRENT_DATE_TIME(utc, year_mm_dd, h24_mm_ss, ' ', 2u);
  EXPECT_EQ(size_t{ 0 }, length);
  length = _CURRENT_DATE_TIME(utc, year_mm_dd, h24_mm_ss, ' ', 64u);
  EXPECT_EQ(size_t{ 19u }, length);
  length = _CURRENT_DATE_TIME(local, year_mm_dd, h24_mm_ss, ' ', 64u);
  EXPECT_EQ(size_t{ 19u }, length);
  length = _CURRENT_DATE_TIME(utc, year_mm_dd, h24_mm_ss_nano, ' ', 64u);
  EXPECT_EQ(size_t{ 29u }, length);
  length = _CURRENT_DATE_TIME(local, year_mm_dd, h24_mm_ss_nano, ' ', 64u);
  EXPECT_EQ(size_t{ 29u }, length);
}
#undef _CURRENT_DATE_TIME
