#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
# pragma warning(push)
# pragma warning(disable : 4100)
# pragma warning(disable : 4101)
# pragma warning(disable : 4189)
#endif
#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <gtest/gtest.h>
#include <system/logger.h>

#define _LOG_BUFFER_SIZE size_t{ 255u }

using namespace pandora::system;
using _BasicFormatterNoDate = BasicLogFormatter<std::ostringstream,size_t{ 255u },DateFormat::none,TimeFormat::none>;

class LoggerTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};


// -- formatter mock --

class HtmlLogFormatter final {
public:
  HtmlLogFormatter(std::ostringstream& logStream, bool addHeaders) : _logStream(&logStream) {
    *(this->_logStream) << ((addHeaders) ? "<table>\n<tr><th>Level</th><th>Category</th><th>Source</th><th>Message</th></tr>" : "<table>") << std::endl;;
  }
  ~HtmlLogFormatter() {
    if (this->_logStream != nullptr) {
      *(this->_logStream) << "</table>" << std::endl;
      flush();
    }
  }
  HtmlLogFormatter(const HtmlLogFormatter&) = delete;
  HtmlLogFormatter(HtmlLogFormatter&& rhs) noexcept : _logStream(rhs._logStream) { rhs._logStream = nullptr; }
  HtmlLogFormatter& operator=(const HtmlLogFormatter&) = delete;
  HtmlLogFormatter& operator=(HtmlLogFormatter&& rhs) noexcept { this->_logStream = rhs._logStream; rhs._logStream = nullptr; return *this; }

  // -- status --

  inline void clearErrorStatus() noexcept { this->_logStream->clear(); }
  inline std::ios_base::iostate getErrorStatus() const noexcept { return this->_logStream->rdstate(); }

  // -- operations --

  void write(LogLevel level, LogCategory category, const char* origin, uint32_t line, const char* format, va_list& args) {
    char buffer[_LOG_BUFFER_SIZE + 1u] = { 0 };
    char levelBuffer[12]{ 0 };
    char categoryBuffer[8]{ 0 };

    int writtenLength = snprintf(buffer, _LOG_BUFFER_SIZE, "<tr><td>%s</td><td>%s</td><td>%s(%d)</td><td>", 
                                 toString(levelBuffer, size_t{ 12u }, level), toString(categoryBuffer, size_t{ 8u }, category), origin, line);
    if (writtenLength >= 0 && writtenLength < static_cast<int>(_LOG_BUFFER_SIZE)) {
      vsnprintf(buffer + writtenLength, _LOG_BUFFER_SIZE - static_cast<size_t>(writtenLength), format, args);
      *(this->_logStream) << buffer << "</td></tr>" << std::endl;
    }
  }
  inline void flush() { this->_logStream->flush(); }

private:
  std::ostringstream* _logStream;
};


// -- open/close logger --

TEST_F(LoggerTest, loggerWithoutHeadersDefaultLevel) {
  Logger<BasicLogFormatter<std::ostringstream> > logger(BasicLogFormatter<std::ostringstream>(std::ostringstream{}));
  const Logger<BasicLogFormatter<std::ostringstream> >& loggerRef(logger);
  EXPECT_EQ(LogLevel::standard, logger.getMinLevel());
  logger.setMinLevel(LogLevel::debug);
  EXPECT_EQ(LogLevel::debug, logger.getMinLevel());
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);
  logger.status().clearErrorStatus();
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);

  logger.flush();
  EXPECT_EQ(std::string(""), logger.status().stream().str());
  EXPECT_EQ(std::string(""), loggerRef.status().stream().str());
}
TEST_F(LoggerTest, loggerWithoutHeaders) {
  Logger<BasicLogFormatter<std::ostringstream> > logger(BasicLogFormatter<std::ostringstream>(std::ostringstream{}), LogLevel::verbose);
  EXPECT_EQ(LogLevel::verbose, logger.getMinLevel());
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);
  logger.status().clearErrorStatus();
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);

  logger.flush();
  EXPECT_EQ(std::string(""), logger.status().stream().str());
}
TEST_F(LoggerTest, loggerWithHeaders) {
  Logger<BasicLogFormatter<std::ostringstream> > logger(BasicLogFormatter<std::ostringstream>(std::ostringstream{}, "head-line"), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);
  logger.status().clearErrorStatus();
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);

  logger.flush();
  EXPECT_EQ(std::string("head-line\n"), logger.status().stream().str());
}

TEST_F(LoggerTest, htmlLoggerWithoutHeadersDefaultLevel) {
  std::ostringstream stream;
  {
    Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, false));
    EXPECT_EQ(LogLevel::standard, logger.getMinLevel());
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);
    logger.status().clearErrorStatus();
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);

    logger.flush();
    EXPECT_EQ(std::string("<table>\n"), stream.str());
  }
  EXPECT_EQ(std::string("<table>\n</table>\n"), stream.str());
}
TEST_F(LoggerTest, htmlLoggerWithoutHeaders) {
  std::ostringstream stream;
  {
    Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, false), LogLevel::debug);
    EXPECT_EQ(LogLevel::debug, logger.getMinLevel());
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);
    logger.status().clearErrorStatus();
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);

    logger.flush();
    EXPECT_EQ(std::string("<table>\n"), stream.str());
  }
  EXPECT_EQ(std::string("<table>\n</table>\n"), stream.str());
}
TEST_F(LoggerTest, htmlLoggerWithHeaders) {
  std::ostringstream stream;
  {
    Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, true), LogLevel::critical);
    EXPECT_EQ(LogLevel::critical, logger.getMinLevel());
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);
    logger.status().clearErrorStatus();
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
    EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);

    logger.flush();
    EXPECT_EQ(std::string("<table>\n<tr><th>Level</th><th>Category</th><th>Source</th><th>Message</th></tr>\n"), stream.str());
  }
  EXPECT_EQ(std::string("<table>\n<tr><th>Level</th><th>Category</th><th>Source</th><th>Message</th></tr>\n</table>\n"), stream.str());
}

TEST_F(LoggerTest, movedLogger) {
  Logger<_BasicFormatterNoDate> movedLogger(_BasicFormatterNoDate(std::ostringstream{}), LogLevel::debug);
  std::ostringstream stream;
  Logger<HtmlLogFormatter> movedLoggerHtml(HtmlLogFormatter(stream, false), LogLevel::critical);

  Logger<_BasicFormatterNoDate> logger(std::move(movedLogger));
  Logger<HtmlLogFormatter> loggerHtml(std::move(movedLoggerHtml));

  logger.log(LogLevel::critical, LogCategory::EVENT, "def", 24, "trololololo");
  logger.flush();
  EXPECT_EQ(std::string(" EVENT(lv.4) [def:24]: trololololo\n"), logger.status().stream().str());

  loggerHtml.log(LogLevel::critical, LogCategory::EVENT, "def", 24, "trololololo");
  loggerHtml.flush();
  EXPECT_EQ(std::string("<table>\n<tr><td>critical</td><td>EVENT</td><td>def(24)</td><td>trololololo</td></tr>\n"), stream.str());
}


// -- write logs --

TEST_F(LoggerTest, logMessageWithoutHeadersWithDate) {
  Logger<BasicLogFormatter<std::ostringstream,size_t{255u},DateFormat::year_mm_dd,TimeFormat::h24_mm_ss_milli> > 
    logger(BasicLogFormatter<std::ostringstream,size_t{255u},DateFormat::year_mm_dd,TimeFormat::h24_mm_ss_milli>(std::ostringstream{}), LogLevel::none);
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this is not an %s", "event");

  logger.flush();
  EXPECT_EQ(std::string(" EVENT(lv.1) [sherlock:44]: this is not an event\n"), logger.status().stream().str().substr(size_t{ 23u }));
  printf("%s", logger.status().stream().str().c_str());
}
TEST_F(LoggerTest, logMessageWithoutHeaders) {
  Logger<_BasicFormatterNoDate> logger(_BasicFormatterNoDate(std::ostringstream{}), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this is not an %s", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d - %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    " (lv.0) [nowhere:12]: hello world !\n"
    " EVENT(lv.1) [sherlock:44]: this is not an event\n"
    " INFO(lv.2) [mib:21]: code 257 - 1 alien to remove\n"
    " WARNING(lv.3) [irobot:0]: AI will save the world\n"
    " ERROR(lv.4) [mars-attacks:8]:  \n"), logger.status().stream().str());
}
TEST_F(LoggerTest, logMessageWithHeaders) {
  Logger<_BasicFormatterNoDate> logger(_BasicFormatterNoDate(std::ostringstream{}, "head-line"), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this is not an %s", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d - %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string("head-line\n"
    " (lv.0) [nowhere:12]: hello world !\n"
    " EVENT(lv.1) [sherlock:44]: this is not an event\n"
    " INFO(lv.2) [mib:21]: code 257 - 1 alien to remove\n"
    " WARNING(lv.3) [irobot:0]: AI will save the world\n"
    " ERROR(lv.4) [mars-attacks:8]:  \n"), logger.status().stream().str());
}
TEST_F(LoggerTest, logIgnoredMessage) {
  Logger<_BasicFormatterNoDate> logger(_BasicFormatterNoDate(std::ostringstream{}), LogLevel::critical);

  logger.log(LogLevel::none, LogCategory::INFO, "abc", 42, "blabla");
  logger.log(LogLevel::verbose, LogCategory::INFO, "abc", 42, "blabla");
  logger.log(LogLevel::debug, LogCategory::INFO, "abc", 42, "blabla");
  logger.log(LogLevel::standard, LogCategory::INFO, "abc", 42, "blabla");
  logger.log(LogLevel::critical, LogCategory::INFO, "abc", 42, "blabla");

  logger.setMinLevel(LogLevel::debug);
  EXPECT_EQ(LogLevel::debug, logger.getMinLevel());
  logger.log(LogLevel::none, LogCategory::EVENT, "def", 24, "trololololo");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "def", 24, "trololololo");
  logger.log(LogLevel::debug, LogCategory::EVENT, "def", 24, "trololololo");
  logger.log(LogLevel::standard, LogCategory::EVENT, "def", 24, "trololololo");
  logger.log(LogLevel::critical, LogCategory::EVENT, "def", 24, "trololololo");

  logger.flush();
  EXPECT_EQ(std::string(
    " INFO(lv.4) [abc:42]: blabla\n"
    " EVENT(lv.2) [def:24]: trololololo\n"
    " EVENT(lv.3) [def:24]: trololololo\n"
    " EVENT(lv.4) [def:24]: trololololo\n"), logger.status().stream().str());
}

TEST_F(LoggerTest, logHtmlMessageWithoutHeaders) {
  std::ostringstream stream;
  Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, false), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this is not an %s", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d - %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string("<table>\n"
            "<tr><td>none</td><td></td><td>nowhere(12)</td><td>hello world !</td></tr>\n"
            "<tr><td>verbose</td><td>EVENT</td><td>sherlock(44)</td><td>this is not an event</td></tr>\n"
            "<tr><td>debug</td><td>INFO</td><td>mib(21)</td><td>code 257 - 1 alien to remove</td></tr>\n"
            "<tr><td>standard</td><td>WARNING</td><td>irobot(0)</td><td>AI will save the world</td></tr>\n"
            "<tr><td>critical</td><td>ERROR</td><td>mars-attacks(8)</td><td> </td></tr>\n"), stream.str());
}
TEST_F(LoggerTest, logHtmlMessageWithHeaders) {
  std::ostringstream stream;
  Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, true), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this is not an %s", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d - %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string("<table>\n<tr><th>Level</th><th>Category</th><th>Source</th><th>Message</th></tr>\n"
            "<tr><td>none</td><td></td><td>nowhere(12)</td><td>hello world !</td></tr>\n"
            "<tr><td>verbose</td><td>EVENT</td><td>sherlock(44)</td><td>this is not an event</td></tr>\n"
            "<tr><td>debug</td><td>INFO</td><td>mib(21)</td><td>code 257 - 1 alien to remove</td></tr>\n"
            "<tr><td>standard</td><td>WARNING</td><td>irobot(0)</td><td>AI will save the world</td></tr>\n"
            "<tr><td>critical</td><td>ERROR</td><td>mars-attacks(8)</td><td> </td></tr>\n"), stream.str());
}
TEST_F(LoggerTest, logHtmlIgnoredMessage) {
  std::ostringstream stream;
  Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, false), LogLevel::critical);

  logger.log(LogLevel::none, LogCategory::INFO, "abc", 42, "blabla");
  logger.log(LogLevel::verbose, LogCategory::INFO, "abc", 42, "blabla");
  logger.log(LogLevel::debug, LogCategory::INFO, "abc", 42, "blabla");
  logger.log(LogLevel::standard, LogCategory::INFO, "abc", 42, "blabla");
  logger.log(LogLevel::critical, LogCategory::INFO, "abc", 42, "blabla");

  logger.setMinLevel(LogLevel::standard);
  EXPECT_EQ(LogLevel::standard, logger.getMinLevel());
  logger.log(LogLevel::none, LogCategory::EVENT, "def", 24, "trololololo");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "def", 24, "trololololo");
  logger.log(LogLevel::debug, LogCategory::EVENT, "def", 24, "trololololo");
  logger.log(LogLevel::standard, LogCategory::EVENT, "def", 24, "trololololo");
  logger.log(LogLevel::critical, LogCategory::EVENT, "def", 24, "trololololo");

  logger.flush();
  EXPECT_EQ(std::string("<table>\n"
            "<tr><td>critical</td><td>INFO</td><td>abc(42)</td><td>blabla</td></tr>\n"
            "<tr><td>standard</td><td>EVENT</td><td>def(24)</td><td>trololololo</td></tr>\n"
            "<tr><td>critical</td><td>EVENT</td><td>def(24)</td><td>trololololo</td></tr>\n"), stream.str());
}


// -- size overflow --

TEST_F(LoggerTest, logMessagePrefixTooLong) {
  Logger<BasicLogFormatter<std::ostringstream,size_t{ 19u },DateFormat::none,TimeFormat::none> > 
    logger(BasicLogFormatter<std::ostringstream,size_t{ 19u },DateFormat::none,TimeFormat::none>(std::ostringstream{}), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "now", 12, "hello world !");
  logger.log(LogLevel::none, LogCategory::none, "nowh", 12, "hello world !");
  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");

  logger.flush();
  EXPECT_EQ(std::string(
    " (lv.0) [now:12]: \n"
    " (lv.0) [nowh:12]:\n"
    " (lv.0) [nowhere:1\n"), logger.status().stream().str());
}
TEST_F(LoggerTest, logMessageMessageTooLong) {
  Logger<BasicLogFormatter<std::ostringstream,size_t{ 19u },DateFormat::none,TimeFormat::none> > 
    logger(BasicLogFormatter<std::ostringstream,size_t{ 19u },DateFormat::none,TimeFormat::none>(std::ostringstream{}), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "o", 12, "hello world !");
  logger.log(LogLevel::none, LogCategory::none, "o", 12, "%s", "hello world !");
  logger.log(LogLevel::none, LogCategory::none, "o", 12, " %s", "hello world !");

  logger.flush();
  EXPECT_EQ(std::string(
    " (lv.0) [o:12]: he\n"
    " (lv.0) [o:12]: he\n"
    " (lv.0) [o:12]:  h\n"), logger.status().stream().str());
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif
#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic pop
#endif
