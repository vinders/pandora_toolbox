#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <gtest/gtest.h>
#include <system/logger.h>

#define _LOG_BUFFER_SIZE size_t{ 255u }

using namespace pandora::system;

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
    std::string levelStr = toString(level);
    std::string catStr = toString(category);

    int writtenLength = snprintf(buffer, _LOG_BUFFER_SIZE, "<tr><td>%s</td><td>%s</td><td>%s(%d)</td><td>", levelStr.c_str(), catStr.c_str(), origin, line);
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
  Logger<BasicLogFormatter<std::ostringstream> > logger(BasicLogFormatter<std::ostringstream>(std::ostringstream{}), LogLevel::trace);
  EXPECT_EQ(LogLevel::trace, logger.getMinLevel());
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
  Logger<BasicLogFormatter<std::ostringstream> > movedLogger(BasicLogFormatter<std::ostringstream>(std::ostringstream{}), LogLevel::debug);
  std::ostringstream stream;
  Logger<HtmlLogFormatter> movedLoggerHtml(HtmlLogFormatter(stream, false), LogLevel::critical);

  Logger<BasicLogFormatter<std::ostringstream> > logger(std::move(movedLogger));
  Logger<HtmlLogFormatter> loggerHtml(std::move(movedLoggerHtml));

  logger.log(LogLevel::critical, LogCategory::event, "def", 24, "trololololo");
  logger.flush();
  EXPECT_EQ(std::string("[event - lv.4]: def(24): trololololo\n"), logger.status().stream().str());

  loggerHtml.log(LogLevel::critical, LogCategory::event, "def", 24, "trololololo");
  loggerHtml.flush();
  EXPECT_EQ(std::string("<table>\n<tr><td>critical</td><td>event</td><td>def(24)</td><td>trololololo</td></tr>\n"), stream.str());
}


// -- write logs --

TEST_F(LoggerTest, logMessageWithoutHeaders) {
  Logger<BasicLogFormatter<std::ostringstream> > logger(BasicLogFormatter<std::ostringstream>(std::ostringstream{}), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::trace, LogCategory::event, "sherlock", 44, "this is not an %s", "event");
  logger.log(LogLevel::debug, LogCategory::info, "mib", 21, "code %d - %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::warning, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::error, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    "[none - lv.0]: nowhere(12): hello world !\n"
    "[event - lv.1]: sherlock(44): this is not an event\n"
    "[info - lv.2]: mib(21): code 257 - 1 alien to remove\n"
    "[warning - lv.3]: irobot(0): AI will save the world\n"
    "[error - lv.4]: mars-attacks(8):  \n"), logger.status().stream().str());
}
TEST_F(LoggerTest, logMessageWithHeaders) {
  Logger<BasicLogFormatter<std::ostringstream> > logger(BasicLogFormatter<std::ostringstream>(std::ostringstream{}, "head-line"), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::trace, LogCategory::event, "sherlock", 44, "this is not an %s", "event");
  logger.log(LogLevel::debug, LogCategory::info, "mib", 21, "code %d - %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::warning, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::error, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string("head-line\n"
    "[none - lv.0]: nowhere(12): hello world !\n"
    "[event - lv.1]: sherlock(44): this is not an event\n"
    "[info - lv.2]: mib(21): code 257 - 1 alien to remove\n"
    "[warning - lv.3]: irobot(0): AI will save the world\n"
    "[error - lv.4]: mars-attacks(8):  \n"), logger.status().stream().str());
}
TEST_F(LoggerTest, logIgnoredMessage) {
  Logger<BasicLogFormatter<std::ostringstream> > logger(BasicLogFormatter<std::ostringstream>(std::ostringstream{}), LogLevel::critical);

  logger.log(LogLevel::none, LogCategory::info, "abc", 42, "blabla");
  logger.log(LogLevel::trace, LogCategory::info, "abc", 42, "blabla");
  logger.log(LogLevel::debug, LogCategory::info, "abc", 42, "blabla");
  logger.log(LogLevel::standard, LogCategory::info, "abc", 42, "blabla");
  logger.log(LogLevel::critical, LogCategory::info, "abc", 42, "blabla");

  logger.setMinLevel(LogLevel::debug);
  EXPECT_EQ(LogLevel::debug, logger.getMinLevel());
  logger.log(LogLevel::none, LogCategory::event, "def", 24, "trololololo");
  logger.log(LogLevel::trace, LogCategory::event, "def", 24, "trololololo");
  logger.log(LogLevel::debug, LogCategory::event, "def", 24, "trololololo");
  logger.log(LogLevel::standard, LogCategory::event, "def", 24, "trololololo");
  logger.log(LogLevel::critical, LogCategory::event, "def", 24, "trololololo");

  logger.flush();
  EXPECT_EQ(std::string(
    "[info - lv.4]: abc(42): blabla\n"
    "[event - lv.2]: def(24): trololololo\n"
    "[event - lv.3]: def(24): trololololo\n"
    "[event - lv.4]: def(24): trololololo\n"), logger.status().stream().str());
}

TEST_F(LoggerTest, logHtmlMessageWithoutHeaders) {
  std::ostringstream stream;
  Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, false), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::trace, LogCategory::event, "sherlock", 44, "this is not an %s", "event");
  logger.log(LogLevel::debug, LogCategory::info, "mib", 21, "code %d - %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::warning, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::error, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string("<table>\n"
            "<tr><td>none</td><td>none</td><td>nowhere(12)</td><td>hello world !</td></tr>\n"
            "<tr><td>trace</td><td>event</td><td>sherlock(44)</td><td>this is not an event</td></tr>\n"
            "<tr><td>debug</td><td>info</td><td>mib(21)</td><td>code 257 - 1 alien to remove</td></tr>\n"
            "<tr><td>standard</td><td>warning</td><td>irobot(0)</td><td>AI will save the world</td></tr>\n"
            "<tr><td>critical</td><td>error</td><td>mars-attacks(8)</td><td> </td></tr>\n"), stream.str());
}
TEST_F(LoggerTest, logHtmlMessageWithHeaders) {
  std::ostringstream stream;
  Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, true), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::trace, LogCategory::event, "sherlock", 44, "this is not an %s", "event");
  logger.log(LogLevel::debug, LogCategory::info, "mib", 21, "code %d - %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::warning, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::error, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string("<table>\n<tr><th>Level</th><th>Category</th><th>Source</th><th>Message</th></tr>\n"
            "<tr><td>none</td><td>none</td><td>nowhere(12)</td><td>hello world !</td></tr>\n"
            "<tr><td>trace</td><td>event</td><td>sherlock(44)</td><td>this is not an event</td></tr>\n"
            "<tr><td>debug</td><td>info</td><td>mib(21)</td><td>code 257 - 1 alien to remove</td></tr>\n"
            "<tr><td>standard</td><td>warning</td><td>irobot(0)</td><td>AI will save the world</td></tr>\n"
            "<tr><td>critical</td><td>error</td><td>mars-attacks(8)</td><td> </td></tr>\n"), stream.str());
}
TEST_F(LoggerTest, logHtmlIgnoredMessage) {
  std::ostringstream stream;
  Logger<HtmlLogFormatter> logger(HtmlLogFormatter(stream, false), LogLevel::critical);

  logger.log(LogLevel::none, LogCategory::info, "abc", 42, "blabla");
  logger.log(LogLevel::trace, LogCategory::info, "abc", 42, "blabla");
  logger.log(LogLevel::debug, LogCategory::info, "abc", 42, "blabla");
  logger.log(LogLevel::standard, LogCategory::info, "abc", 42, "blabla");
  logger.log(LogLevel::critical, LogCategory::info, "abc", 42, "blabla");

  logger.setMinLevel(LogLevel::standard);
  EXPECT_EQ(LogLevel::standard, logger.getMinLevel());
  logger.log(LogLevel::none, LogCategory::event, "def", 24, "trololololo");
  logger.log(LogLevel::trace, LogCategory::event, "def", 24, "trololololo");
  logger.log(LogLevel::debug, LogCategory::event, "def", 24, "trololololo");
  logger.log(LogLevel::standard, LogCategory::event, "def", 24, "trololololo");
  logger.log(LogLevel::critical, LogCategory::event, "def", 24, "trololololo");

  logger.flush();
  EXPECT_EQ(std::string("<table>\n"
            "<tr><td>critical</td><td>info</td><td>abc(42)</td><td>blabla</td></tr>\n"
            "<tr><td>standard</td><td>event</td><td>def(24)</td><td>trololololo</td></tr>\n"
            "<tr><td>critical</td><td>event</td><td>def(24)</td><td>trololololo</td></tr>\n"), stream.str());
}


// -- size overflow --

TEST_F(LoggerTest, logMessagePrefixTooLong) {
  Logger<BasicLogFormatter<std::ostringstream, size_t{ 25u }> > logger(BasicLogFormatter<std::ostringstream, size_t{ 25u }>(std::ostringstream{}), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "now", 12, "hello world !");
  logger.log(LogLevel::none, LogCategory::none, "nowh", 12, "hello world !");
  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");

  logger.flush();
  EXPECT_EQ(std::string(
    "[none - lv.0]: now(12): \n"
    "[none - lv.0]: nowh(12):\n"
    "[none - lv.0]: nowhere(1\n"), logger.status().stream().str());
}
TEST_F(LoggerTest, logMessageMessageTooLong) {
  Logger<BasicLogFormatter<std::ostringstream, size_t{ 25u }> > logger(BasicLogFormatter<std::ostringstream, size_t{ 25u }>(std::ostringstream{}), LogLevel::none);

  logger.log(LogLevel::none, LogCategory::none, "o", 12, "hello world !");
  logger.log(LogLevel::none, LogCategory::none, "o", 12, "%s", "hello world !");
  logger.log(LogLevel::none, LogCategory::none, "o", 12, " %s", "hello world !");

  logger.flush();
  EXPECT_EQ(std::string(
    "[none - lv.0]: o(12): he\n"
    "[none - lv.0]: o(12): he\n"
    "[none - lv.0]: o(12):  h\n"), logger.status().stream().str());
}