/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstddef>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <string>
#include <fstream>
#include <system/logger.h>
#if defined(_MSC_VER) && defined(_WINDOWS)
# define __snprintf_truncate(buffer, maxSize, ...) _snprintf_s(buffer, maxSize, _TRUNCATE, __VA_ARGS__)
# define __vsnprintf_truncate(buffer, maxSize, ...) _vsnprintf_s(buffer, maxSize, _TRUNCATE, __VA_ARGS__)
#else
# define __snprintf_truncate(buffer, maxSize, ...) snprintf(buffer, maxSize, __VA_ARGS__)
# define __vsnprintf_truncate(buffer, maxSize, ...) vsnprintf(buffer, maxSize, __VA_ARGS__)
#endif
#if !defined(_CPP_REVISION) || _CPP_REVISION != 14
# define __if_constexpr if constexpr
#else
# define __if_constexpr if
#endif

namespace pandora { 
  namespace io {
    /// @class CsvLogFormatter
    /// @brief CSV table output formatter and writer. Should be injected to Logger instance on creation.
    template <char _Separator = ',',                   ///< CSV column separator
              typename _OutputStream = std::ofstream,  ///< Output stream type: fstream/ofstream/stringstream/ostringstream/osyncstream 
                                                       ///  or any movable class that implements: operator<<, flush, clear, rdstate.
              size_t _BufferSize = size_t{ 255u },     ///< Max log message length (including metadata)
              bool _IsEscapingMessage = true,          ///< Escape characters in message similar to the separator
              pandora::system::DateFormat _DateType = pandora::system::DateFormat::year_mm_dd,      ///< Log date formatting
              pandora::system::TimeFormat _TimeType = pandora::system::TimeFormat::h24_mm_ss_milli> ///< Log time formatting
    class CsvLogFormatter final {
    public:
      using Type = CsvLogFormatter<_Separator,_OutputStream,_BufferSize,_IsEscapingMessage,_DateType,_TimeType>;

      /// @brief Create formatter (inject output stream)
      CsvLogFormatter(_OutputStream&& logStream, bool writeHeaderRow = false)
        : _logStream(std::move(logStream)) {
        if (writeHeaderRow) {
          __if_constexpr (_DateType != pandora::system::DateFormat::none)
            this->_logStream << "DATE" << _Separator;
          __if_constexpr (_TimeType != pandora::system::TimeFormat::none)
            this->_logStream << "TIME" << _Separator;
          __snprintf_truncate(this->_messageBuffer, _BufferSize + 1u, "TYPE%cLEVEL%cSOURCE%cMESSAGE", _Separator, _Separator, _Separator);
          this->_logStream << this->_messageBuffer << std::endl;
        }
      }

      CsvLogFormatter() = default; ///< Formatter with default output (string stream) or no output (file stream)
      CsvLogFormatter(const Type&) = delete;
      CsvLogFormatter(Type&&) = default;
      CsvLogFormatter& operator=(const Type&) = delete;
      CsvLogFormatter& operator=(Type&&) = default;
      ~CsvLogFormatter() = default;

      // -- status --

      inline const _OutputStream& stream() const noexcept { return this->_logStream; } ///< Read stream status or content
      inline void clearErrorStatus() noexcept { this->_logStream.clear(); }                                ///< Reset error flags
      inline std::ios_base::iostate getErrorStatus() const noexcept { return this->_logStream.rdstate(); } ///< Read error flags

    protected:
      // -- operations --

      // Format and write log message and metadata
      inline void write(pandora::system::LogLevel level, pandora::system::LogCategory flag, const char* origin, uint32_t line, const char* format, va_list& args) {
        assert(origin != nullptr && format != nullptr);
        static_assert(_BufferSize > size_t{ 0u }, "Template argument _BufferSize cannot be 0.");
        char categoryBuffer[8];
        
        *(this->_messageBuffer) = static_cast<char>(0);
        size_t totalLength{ 0 };
        __if_constexpr (_DateType != pandora::system::DateFormat::none || _TimeType != pandora::system::TimeFormat::none) {
          totalLength = pandora::system::getCurrentDateTimeString<pandora::system::TimeReference::local, _DateType, _TimeType, _Separator>(this->_messageBuffer, _BufferSize - 1u);
          this->_messageBuffer[totalLength] = _Separator;
          ++totalLength;
        }

        int prefixLength = __snprintf_truncate(this->_messageBuffer + totalLength, _BufferSize - totalLength, "%s%c%u%c%s(%u)%c", 
                                               pandora::system::toString(categoryBuffer,size_t{ 8u },flag), _Separator, static_cast<uint32_t>(level), 
                                               _Separator, origin, line, _Separator);
        totalLength = (prefixLength > 0) ? totalLength + static_cast<size_t>(prefixLength) : strnlen(this->_messageBuffer, _BufferSize);

        if (totalLength < _BufferSize - 1u) {
          int messageLength = __vsnprintf_truncate(this->_messageBuffer + totalLength, _BufferSize - totalLength, format, args);
          if (messageLength < 0)
            messageLength = static_cast<int>(strnlen(this->_messageBuffer + totalLength, _BufferSize - totalLength));

          __if_constexpr (_IsEscapingMessage)
            _escapeString(this->_messageBuffer + totalLength, static_cast<size_t>(messageLength), _BufferSize - 1u - totalLength);
          else
            *(this->_messageBuffer + (totalLength + messageLength)) = static_cast<char>(0);
        }
        this->_logStream << this->_messageBuffer << std::endl;
      }

      // Flush log stream buffer
      inline void flush() noexcept { this->_logStream.flush(); }

    private:
      // Escape same characters as separators
      inline void _escapeString(char* value, size_t length, size_t maxLength) noexcept {
        if (length > maxLength)
          length = maxLength;
        for (size_t i = 0; i < length; ++value, ++i) {
          if (*value == _Separator) { // same character as separator -> escape it
            maxLength -= 2u;
            if (length > maxLength) { // size overflow -> truncate message
              if (i >= maxLength)
                break;
              length = maxLength;
            }

            if (i < length - 1u)
              memmove((void*)(value + 3), (void*)(value + 1), length - 1u - i);
            *(value + 1) = *value;
            *value = '"';
            value += 2;
            *value = '"';
          }
        }
        *value = static_cast<char>(0); // can be after last index -> total size == _BufferSize + 1
      }

      friend class pandora::system::Logger<Type>;

    private:
      char _messageBuffer[_BufferSize + 1u]{ 0 };
      _OutputStream _logStream;
    };


    // -- aliases --

    template <char _Separator = ',', size_t _BufferSize = size_t{ 255u }, bool _IsEscapingMessage = true, 
              pandora::system::DateFormat _DateType = pandora::system::DateFormat::year_mm_dd, pandora::system::TimeFormat _TimeType = pandora::system::TimeFormat::h24_mm_ss_milli>
    using CsvLogFileFormatter = CsvLogFormatter<_Separator,std::ofstream,_BufferSize,_IsEscapingMessage,_DateType,_TimeType>;

    template <char _Separator = ',', size_t _BufferSize = size_t{ 255u }, bool _IsEscapingMessage = true, 
              pandora::system::DateFormat _DateType = pandora::system::DateFormat::year_mm_dd, pandora::system::TimeFormat _TimeType = pandora::system::TimeFormat::h24_mm_ss_milli>
    using CsvLogStringFormatter = CsvLogFormatter<_Separator,std::ostringstream,_BufferSize,_IsEscapingMessage,_DateType,_TimeType>;

    template <char _Separator = ',', size_t _BufferSize = size_t{ 255u }, bool _IsEscapingMessage = true, 
              pandora::system::DateFormat _DateType = pandora::system::DateFormat::year_mm_dd, pandora::system::TimeFormat _TimeType = pandora::system::TimeFormat::h24_mm_ss_milli>
    using CsvFileLogger = pandora::system::Logger<CsvLogFileFormatter<_Separator,_BufferSize,_IsEscapingMessage,_DateType,_TimeType> >;

    template <char _Separator = ',', size_t _BufferSize = size_t{ 255u }, bool _IsEscapingMessage = true, 
              pandora::system::DateFormat _DateType = pandora::system::DateFormat::year_mm_dd, pandora::system::TimeFormat _TimeType = pandora::system::TimeFormat::h24_mm_ss_milli>
    using CsvStringLogger = pandora::system::Logger<CsvLogStringFormatter<_Separator,_BufferSize,_IsEscapingMessage,_DateType,_TimeType> >;

  }
}
#undef __snprintf_truncate
#undef __vsnprintf_truncate
#undef __if_constexpr
