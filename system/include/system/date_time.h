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
#pragma once

#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif
#if (!defined(_CPP_REVISION) || _CPP_REVISION != 14) && (defined(_MSC_VER) || !defined(__MINGW32__))
# define __P_FORMAT_SPEC_mdy_VAL "%D"
# define __P_FORMAT_SPEC_mdy_SIZE size_t{ 2u }
# define __P_FORMAT_SPEC_yearmd_VAL "%F"
# define __P_FORMAT_SPEC_yearmd_SIZE size_t{ 2u }
# define __P_FORMAT_SPEC_hms_VAL "%T"
# define __P_FORMAT_SPEC_hms_SIZE size_t{ 2u }
# define __P_FORMAT_SPEC_hms_sub_VAL "%T."
# define __P_FORMAT_SPEC_hms_sub_SIZE size_t{ 3u }
#else
# define __P_FORMAT_SPEC_mdy_VAL "%m/%d/%y"
# define __P_FORMAT_SPEC_mdy_SIZE size_t{ 8u }
# define __P_FORMAT_SPEC_yearmd_VAL "%Y-%m-%d"
# define __P_FORMAT_SPEC_yearmd_SIZE size_t{ 8u }
# define __P_FORMAT_SPEC_hms_VAL "%H:%M:%S"
# define __P_FORMAT_SPEC_hms_SIZE size_t{ 8u }
# define __P_FORMAT_SPEC_hms_sub_VAL "%H:%M:%S."
# define __P_FORMAT_SPEC_hms_sub_SIZE size_t{ 9u }
#endif

namespace pandora {
  namespace system {
    /// @brief Time zone reference
    enum class TimeReference : uint32_t {
      utc = 0u,
      local = 1u
    };
    
    /// @brief Date formatting type
    enum class DateFormat : uint32_t {
      none = 0u,
      weekday,
      month,
      year,
      wkd_d_mth,
      wkd_mth_d,
      weekday_d_month,
      weekday_month_d,
      wkd_d_mth_year,
      wkd_mth_d_year,
      weekday_d_month_year,
      weekday_month_d_year,
      d_month_year,
      month_d_year,
      dd_mm,
      mm_dd,
      dd_mm_yy,
      dd_mm_year,
      mm_dd_yy,
      mm_dd_year,
      yy_mm_dd,
      year_mm_dd
    };
    // Fill date format specifier
    template <DateFormat _DateType>
    inline size_t _getDateFormatSpecifiers(char* out) noexcept {
      assert(out != nullptr);
      __if_constexpr (_DateType == DateFormat::weekday)      { memcpy((void*)out, (void*)"%A", size_t{ 3u }*sizeof(char)); return size_t{ 2u }; }
      else __if_constexpr (_DateType == DateFormat::month)   { memcpy((void*)out, (void*)"%B", size_t{ 3u }*sizeof(char)); return size_t{ 2u }; }
      else __if_constexpr (_DateType == DateFormat::year)    { memcpy((void*)out, (void*)"%Y", size_t{ 3u }*sizeof(char)); return size_t{ 2u }; }
      else __if_constexpr (_DateType == DateFormat::wkd_d_mth)       { memcpy((void*)out, (void*)"%a %d %b", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::wkd_mth_d)       { memcpy((void*)out, (void*)"%a %b %d", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::weekday_d_month) { memcpy((void*)out, (void*)"%A %d %B", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::weekday_month_d) { memcpy((void*)out, (void*)"%A %B %d", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::wkd_d_mth_year)  { memcpy((void*)out, (void*)"%a %d %b %Y", size_t{ 12u }*sizeof(char)); return size_t{ 11u }; }
      else __if_constexpr (_DateType == DateFormat::wkd_mth_d_year)  { memcpy((void*)out, (void*)"%a %b %d %Y", size_t{ 12u }*sizeof(char)); return size_t{ 11u }; }
      else __if_constexpr (_DateType == DateFormat::weekday_d_month_year) { memcpy((void*)out, (void*)"%A %d %B %Y", size_t{ 12u }*sizeof(char)); return size_t{ 11u }; }
      else __if_constexpr (_DateType == DateFormat::weekday_month_d_year) { memcpy((void*)out, (void*)"%A %B %d %Y", size_t{ 12u }*sizeof(char)); return size_t{ 11u }; }
      else __if_constexpr (_DateType == DateFormat::d_month_year) { memcpy((void*)out, (void*)"%d %B %Y", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::month_d_year) { memcpy((void*)out, (void*)"%B %d %Y", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::dd_mm)      { memcpy((void*)out, (void*)"%d/%m", size_t{ 6u }*sizeof(char)); return size_t{ 5u }; }
      else __if_constexpr (_DateType == DateFormat::mm_dd)      { memcpy((void*)out, (void*)"%m/%d", size_t{ 6u }*sizeof(char)); return size_t{ 5u }; }
      else __if_constexpr (_DateType == DateFormat::dd_mm_yy)   { memcpy((void*)out, (void*)"%d/%m/%y", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::dd_mm_year) { memcpy((void*)out, (void*)"%d/%m/%Y", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::mm_dd_yy)   { memcpy((void*)out, (void*)__P_FORMAT_SPEC_mdy_VAL, (__P_FORMAT_SPEC_mdy_SIZE+1u)*sizeof(char)); return __P_FORMAT_SPEC_mdy_SIZE; }
      else __if_constexpr (_DateType == DateFormat::mm_dd_year) { memcpy((void*)out, (void*)"%m/%d/%Y", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::yy_mm_dd)   { memcpy((void*)out, (void*)"%y-%m-%d", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_DateType == DateFormat::year_mm_dd) { memcpy((void*)out, (void*)__P_FORMAT_SPEC_yearmd_VAL, (__P_FORMAT_SPEC_yearmd_SIZE+1u)*sizeof(char)); return __P_FORMAT_SPEC_yearmd_SIZE; }
      else { *out = static_cast<char>(0); return size_t{ 0u }; }
    }
    
    /// @brief Time formatting type
    enum class TimeFormat : uint32_t {
      none = 0u,
      h24_mm,
      h12_mm,
      h24_mm_ss,
      h12_mm_ss,
      h24_mm_ss_milli,
      h24_mm_ss_micro,
      h24_mm_ss_nano
    };
    // Fill time format specifier
    template <TimeFormat _TimeType>
    inline size_t _getTimeFormatSpecifiers(char* out) noexcept {
      assert(out != nullptr);
      __if_constexpr (_TimeType == TimeFormat::none) { *out = static_cast<char>(0); return size_t{ 0u }; }
      else __if_constexpr (_TimeType == TimeFormat::h24_mm)    { memcpy((void*)out, (void*)"%H:%M", size_t{ 6u }*sizeof(char)); return size_t{ 5u }; }
      else __if_constexpr (_TimeType == TimeFormat::h12_mm)    { memcpy((void*)out, (void*)"%H:%M %p", size_t{ 9u }*sizeof(char)); return size_t{ 8u }; }
      else __if_constexpr (_TimeType == TimeFormat::h24_mm_ss) { memcpy((void*)out, (void*)__P_FORMAT_SPEC_hms_VAL, (__P_FORMAT_SPEC_hms_SIZE+1u)*sizeof(char)); return __P_FORMAT_SPEC_hms_SIZE; }
      else __if_constexpr (_TimeType == TimeFormat::h12_mm_ss) { memcpy((void*)out, (void*)"%I:%M:%S %p", size_t{ 12u }*sizeof(char)); return size_t{ 11u }; }
      else { memcpy((void*)out, (void*)__P_FORMAT_SPEC_hms_sub_VAL, (__P_FORMAT_SPEC_hms_sub_SIZE+1u)*sizeof(char)); return __P_FORMAT_SPEC_hms_sub_SIZE; } // HH:MM:SS.sub
    }
   
   
    // -- date/time formatters --
    
    // Convert integer to a fixed length string (left padded with '0')
    template <size_t _Digits>
    inline size_t _toFixedDigitString(char* out, uint32_t value) noexcept {
      char* end = out + _Digits;
      *end = static_cast<char>(0);
      while (--end >= out) {
        *end = '0' + (value % 10u);
        value /= 10u;
      }
      return _Digits;
    }

    /// @brief Convert seconds/nanoseconds to date/time formatted string
    /// @returns Length of formatted string
    template <TimeReference _TimeRef, DateFormat _DateType, TimeFormat _TimeType, char _DateTimeSep = ' '>
    inline size_t formatDateTimeString(char* out, size_t maxLength, time_t sec, long nanosec) noexcept {
      static_assert(_DateTimeSep >= 0x9 || _DateTimeSep < 0, "_DateTimeSep must not be 0 or a control character");
      assert(out != nullptr);
      
      // time zone reference
      struct tm* timeDetails = nullptr;
      __if_constexpr (_TimeRef == TimeReference::local)
        timeDetails = localtime(&sec);
      else
        timeDetails = gmtime(&sec);
      
      // format date/time (>= second)
      char buffer[24];
      size_t dateLength{ 0 };
      __if_constexpr (_DateType != DateFormat::none) {
        dateLength = _getDateFormatSpecifiers<_DateType>(buffer);
        __if_constexpr (_TimeType != TimeFormat::none) {
          buffer[dateLength] = _DateTimeSep;
          ++dateLength;
        }
      }
      __if_constexpr (_TimeType != TimeFormat::none)
        _getTimeFormatSpecifiers<_TimeType>(buffer + dateLength);

      size_t length{ 0 };
      __if_constexpr (_DateType != DateFormat::none || _TimeType != TimeFormat::none)
        length = strftime(out, maxLength, buffer, timeDetails);
      else {
        if (maxLength > size_t{ 0 })
          *out = static_cast<char>(0);
      }
      
      // format milli/micro/nanoseconds
      __if_constexpr (_TimeType == TimeFormat::h24_mm_ss_milli) {
        if (length > size_t{ 0 } && length + 3u < maxLength)
          length += _toFixedDigitString<size_t{ 3u }>(out + length, static_cast<uint32_t>(nanosec/1000000L));
      }
      else __if_constexpr (_TimeType == TimeFormat::h24_mm_ss_micro) {
        if (length > size_t{ 0 } && length + 6u < maxLength)
          length += _toFixedDigitString<size_t{ 6u }>(out + length, static_cast<uint32_t>(nanosec/1000L));
      }
      else __if_constexpr (_TimeType == TimeFormat::h24_mm_ss_nano) {
        if (length > size_t{ 0 } && length + 9u < maxLength)
          length += _toFixedDigitString<size_t{ 9u }>(out + length, static_cast<uint32_t>(nanosec));
      }
      return length;
    }

    /// @brief Get current date/time as a formatted string
    /// @returns Length of formatted string
    template <TimeReference _TimeRef, DateFormat _DateType, TimeFormat _TimeType, char _DateTimeSep = ' '>
    inline size_t getCurrentDateTimeString(char* out, size_t maxLength) noexcept {
      static_assert(_DateTimeSep >= 0x9 || _DateTimeSep < 0, "_DateTimeSep must not be 0 or a control character");
      assert(out != nullptr);
      
#     if (!defined(_CPP_REVISION) || _CPP_REVISION != 14) && defined(TIME_UTC)
        if constexpr (_TimeType == TimeFormat::h24_mm_ss_milli || _TimeType == TimeFormat::h24_mm_ss_micro || _TimeType == TimeFormat::h24_mm_ss_nano) {
          struct timespec currentTime;
          if (timespec_get(&currentTime, TIME_UTC))
            return formatDateTimeString<_TimeRef,_DateType,_TimeType,_DateTimeSep>(out, maxLength, currentTime.tv_sec, currentTime.tv_nsec);
        }
#     endif
      time_t legacyTime;
      time(&legacyTime);
      return formatDateTimeString<_TimeRef,_DateType,_TimeType,_DateTimeSep>(out, maxLength, legacyTime, 0L);
    }
    
  }
}
#undef __if_constexpr
#undef __P_FORMAT_SPEC_mdy_VAL
#undef __P_FORMAT_SPEC_mdy_SIZE
#undef __P_FORMAT_SPEC_yearmd_VAL
#undef __P_FORMAT_SPEC_yearmd_SIZE
#undef __P_FORMAT_SPEC_hms_VAL
#undef __P_FORMAT_SPEC_hms_SIZE
#undef __P_FORMAT_SPEC_hms_sub_VAL
#undef __P_FORMAT_SPEC_hms_sub_SIZE
