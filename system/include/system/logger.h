/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
*******************************************************************************/
#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cassert>
#include <string>
#include <fstream>
#include "./preprocessor_tools.h"
#if defined(_MSC_VER) && defined(_WINDOWS)
# define __snprintf_truncate(buffer, ...) _snprintf_s(buffer, _TRUNCATE, __VA_ARGS__)
# define __vsnprintf_truncate(buffer, ...) _vsnprintf_s(buffer, _TRUNCATE, __VA_ARGS__)
#else
# define __snprintf_truncate(buffer, ...) snprintf(buffer, __VA_ARGS__)
# define __vsnprintf_truncate(buffer, ...) vsnprintf(buffer, __VA_ARGS__)
#endif

namespace pandora {
  namespace system {
    template <typename,size_t> class BasicLogFormatter;

    /// @brief Log level type
    enum class LogLevel: uint32_t {
      none = 0u,
      trace = 1u,
      debug = 2u,
      standard = 3u,
      critical = 4u
    };
    _P_SERIALIZABLE_ENUM(LogLevel, none, critical, standard, debug, trace);

    /// @brief Log content category
    enum class LogCategory: uint32_t {
      none = 0u,
      event = 1u,
      info = 2u,
      warning = 3u,
      error = 4u,
    };
    _P_SERIALIZABLE_ENUM(LogCategory, none, event, info, warning, error);


    // -- log utility --

    /// @class Logger
    /// @brief Simple log writing utility. Not protected against concurrent access (can be used in Locked pattern for such situations)
    template <typename _OutputFormatter                   ///< Output formatter: any movable class that implements:
                       = BasicLogFormatter                ///  void write(LogLevel, LogCategory, const char* orig, uint32_t line, const char* format, va_list&),
                         <std::ofstream,size_t{ 255u }> > ///  void write(LogLevel, LogCategory, const wchar_t* orig, uint32_t line, const wchar_t* format, va_list&),
                                                          ///  void flush().
    class Logger final {
    public:
      /// @brief Create logger utility (inject output formatter with stream + set initial output level)
      Logger(_OutputFormatter&& formatter, LogLevel minWriteLevel = LogLevel::standard)
        : _output(std::move(formatter)), _level(minWriteLevel) {}
      Logger() = default; ///< Empty logger utility: default output (string stream) or no output (file stream)
      Logger(const Logger<_OutputFormatter>&) = delete;
      Logger(Logger<_OutputFormatter>&&) = default;
      Logger& operator=(const Logger<_OutputFormatter>&) = delete;
      Logger& operator=(Logger<_OutputFormatter>&&) = default;
      ~Logger() = default;

      // -- minimum active log level --

      inline LogLevel getMinLevel() const noexcept { return this->_level; }      ///< get minimum output log level
      inline void setMinLevel(LogLevel level) noexcept { this->_level = level; } ///< set minimum output log level

      // -- status --

      inline _OutputFormatter& status() noexcept { return this->_output; }
      inline const _OutputFormatter& status() const noexcept { return this->_output; }

      // -- operations --
     
      /// @brief Log new entry (only written if 'level' >= minimum active level)
      /// @param[in] level  Log level for current message (if less important than configured output level, message ignored)
      /// @param[in] flag   Type of message
      /// @param[in] origin Source location of the message (example: __FILE__ or method name)
      /// @param[in] line   Line number in source file (example: __LINE__)
      /// @param[in] format Log message with printf-like format, followed by optional arguments
#     ifdef __GNUC__
        __attribute__((format(printf, 6, 7)))
#     endif
      void log(LogLevel level, LogCategory flag, const char* origin, uint32_t line, const char* format, ...) {
        if (level >= this->_level) {
          va_list args;
          va_start(args, format);
          _output.write(level, flag, origin, line, format, args);
          va_end(args);
        }
      }

      /// @brief Flush log stream buffer
      inline void flush() noexcept { this->_output.flush(); }
    
    
    private:
      _OutputFormatter _output;
      LogLevel _level = LogLevel::standard;
    };


    // -- log output formatting --

    /// @class BasicLogFormatter
    /// @brief Simple log output formatter and writer. Should be injected to Logger instance on creation.
    /// @note  Use this class as an example to create custom formatters (csv, html, xml...)
    template <typename _OutputStream = std::ofstream,  ///< Output stream type: fstream/ofstream/stringstream/ostringstream/osyncstream 
                                                       ///  or any movable class that implements: operator<<, flush, clear, rdstate.
              size_t _BufferSize = size_t{ 255u }>     ///< Max log message length (including metadata)
    class BasicLogFormatter final {
    public:
      /// @brief Create formatter (inject output stream)
      BasicLogFormatter(_OutputStream&& logStream) 
        : _logStream(std::move(logStream)) {}
      /// @brief Create formatter (inject output stream) + write header before logs
      BasicLogFormatter(_OutputStream&& logStream, const char* outputHeader) 
        : _logStream(std::move(logStream)) {
        if (outputHeader != nullptr)
          this->_logStream << outputHeader << std::endl;
      }
      BasicLogFormatter() = default; ///< Formatter with default output (string stream) or no output (file stream)
      BasicLogFormatter(const BasicLogFormatter<_OutputStream,_BufferSize>&) = delete;
      BasicLogFormatter(BasicLogFormatter<_OutputStream,_BufferSize>&&) = default;
      BasicLogFormatter& operator=(const BasicLogFormatter<_OutputStream,_BufferSize>&) = delete;
      BasicLogFormatter& operator=(BasicLogFormatter<_OutputStream,_BufferSize>&&) = default;
      ~BasicLogFormatter() = default;

      // -- status --

      inline const _OutputStream& stream() const noexcept { return this->_logStream; } ///< Read stream status or content
      inline void clearErrorStatus() noexcept { this->_logStream.clear(); }                                ///< Reset error flags
      inline std::ios_base::iostate getErrorStatus() const noexcept { return this->_logStream.rdstate(); } ///< Read error flags

    protected:
      // -- operations --

      // Format and write log message and metadata
      inline void write(LogLevel level, LogCategory flag, const char* origin, uint32_t line, const char* format, va_list& args) {
        assert(origin != nullptr && format != nullptr);
        *(this->_messageBuffer) = '\0';

        const auto categoryLabel = toString(flag);
        int writtenLength = __snprintf_truncate(this->_messageBuffer, _BufferSize, "[%s - lv.%u]: %s(%u): ", categoryLabel.c_str(), static_cast<uint32_t>(level), origin, line);
        if (writtenLength < 0)
          writtenLength = static_cast<int>(strnlen(this->_messageBuffer, _BufferSize));

        if (static_cast<size_t>(writtenLength) < _BufferSize - 1u)
          __vsnprintf_truncate(this->_messageBuffer + writtenLength, _BufferSize - static_cast<size_t>(writtenLength), format, args);

        this->_messageBuffer[_BufferSize - 1u] = static_cast<char>(0);
        this->_logStream << this->_messageBuffer << std::endl;
      }
      // Flush log stream buffer
      inline void flush() noexcept { this->_logStream.flush(); }

      friend class Logger<BasicLogFormatter<_OutputStream,_BufferSize> >;

    private:
      char _messageBuffer[_BufferSize + 1u]{ 0 };
      _OutputStream _logStream;
    };


    // -- aliases --

    template <size_t _BufferSize = size_t{ 255u } >
    using LogFileFormatter = BasicLogFormatter<std::ofstream,_BufferSize>;

    template <size_t _BufferSize = size_t{ 255u } >
    using LogStringFormatter = BasicLogFormatter<std::ostringstream, _BufferSize>;

    template <size_t _BufferSize = size_t{ 255u } >
    using FileLogger = Logger<LogFileFormatter<_BufferSize> >;

    template <size_t _BufferSize = size_t{ 255u } >
    using StringLogger = Logger<LogStringFormatter<_BufferSize> >;

  }
}
#undef __snprintf_truncate
#undef __vsnprintf_truncate
