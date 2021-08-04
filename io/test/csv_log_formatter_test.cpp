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
#define GTEST_HAS_TR1_TUPLE 0
#include <gtest/gtest.h>
#include <cstdio>
#include <sstream>
#include <io/csv_log_formatter.h>

using namespace pandora::system;
using namespace pandora::io;

class CsvLogFormatterTest : public testing::Test {
public:
protected:
  //static void SetUpTestCase() {}
  //static void TearDownTestCase() {}
  void SetUp() override {}
  void TearDown() override {}
};

template <char _Sep>
using _CsvFormatter = CsvLogFormatter<_Sep,std::ostringstream,size_t{ 255u },true,DateFormat::none,TimeFormat::none>;


// -- open/close logger --

TEST_F(CsvLogFormatterTest, csvLoggerWithoutHeadersDefaultLevel) {
  Logger<_CsvFormatter<','> > logger(_CsvFormatter<','>(std::ostringstream{}, false));
  EXPECT_EQ(LogLevel::standard, logger.getMinLevel());
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);
  logger.status().clearErrorStatus();
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::failbit);
  EXPECT_FALSE(logger.status().getErrorStatus() & std::ostream::badbit);

  logger.flush();
  EXPECT_EQ(std::string(""), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, csvLoggerWithoutHeaders) {
  Logger<_CsvFormatter<','> > logger(_CsvFormatter<','>(std::ostringstream{}, false), LogLevel::debug);
  EXPECT_EQ(LogLevel::debug, logger.getMinLevel());

  logger.flush();
  EXPECT_EQ(std::string(""), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, csvCommaLoggerWithHeaders) {
  Logger<_CsvFormatter<','> > logger(_CsvFormatter<','>(std::ostringstream{}, true), LogLevel::critical);
  EXPECT_EQ(LogLevel::critical, logger.getMinLevel());

  logger.flush();
  EXPECT_EQ(std::string("TYPE,LEVEL,SOURCE,MESSAGE\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, csvCommaLoggerWithDatedHeaders) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::year_mm_dd,TimeFormat::none> > 
    logger(CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::year_mm_dd,TimeFormat::none>(std::ostringstream{}, true), LogLevel::critical);
  EXPECT_EQ(LogLevel::critical, logger.getMinLevel());

  logger.flush();
  EXPECT_EQ(std::string("DATE,TYPE,LEVEL,SOURCE,MESSAGE\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, csvCommaLoggerWithTimedHeaders) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::none,TimeFormat::h24_mm_ss_milli> > 
    logger(CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::none,TimeFormat::h24_mm_ss_milli>(std::ostringstream{}, true), LogLevel::critical);
  EXPECT_EQ(LogLevel::critical, logger.getMinLevel());

  logger.flush();
  EXPECT_EQ(std::string("TIME,TYPE,LEVEL,SOURCE,MESSAGE\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, csvCommaLoggerWithDateTimeHeaders) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::year_mm_dd,TimeFormat::h24_mm_ss_milli> > 
    logger(CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::year_mm_dd,TimeFormat::h24_mm_ss_milli>(std::ostringstream{}, true), LogLevel::critical);
  EXPECT_EQ(LogLevel::critical, logger.getMinLevel());

  logger.flush();
  EXPECT_EQ(std::string("DATE,TIME,TYPE,LEVEL,SOURCE,MESSAGE\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, csvSemicolonLoggerWithHeaders) {
  Logger<_CsvFormatter<';'> > logger(_CsvFormatter<';'>(std::ostringstream{}, true), LogLevel::critical);
  EXPECT_EQ(LogLevel::critical, logger.getMinLevel());

  logger.flush();
  EXPECT_EQ(std::string("TYPE;LEVEL;SOURCE;MESSAGE\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, csvTabLoggerWithHeaders) {
  Logger<_CsvFormatter<'\t'> > logger(_CsvFormatter<'\t'>(std::ostringstream{}, true), LogLevel::verbose);
  EXPECT_EQ(LogLevel::verbose, logger.getMinLevel());

  logger.flush();
  EXPECT_EQ(std::string("TYPE\tLEVEL\tSOURCE\tMESSAGE\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, csvSpaceLoggerWithHeaders) {
  Logger<_CsvFormatter<' '> > logger(_CsvFormatter<' '>(std::ostringstream{}, true), LogLevel::standard);
  EXPECT_EQ(LogLevel::standard, logger.getMinLevel());

  logger.flush();
  EXPECT_EQ(std::string("TYPE LEVEL SOURCE MESSAGE\n"), logger.status().stream().str());
}


// -- write logs --

TEST_F(CsvLogFormatterTest, logCsvCommaMessageWithDate) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::year_mm_dd,TimeFormat::none> > 
    logger(CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::year_mm_dd,TimeFormat::none>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");

  logger.flush();
  EXPECT_EQ(std::string(",EVENT,1,sherlock(44),this\",\" is not an event;\n"), logger.status().stream().str().substr(size_t{ 10u }));
  printf("%s", logger.status().stream().str().c_str());
}
TEST_F(CsvLogFormatterTest, logCsvCommaMessageWithTime) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::none,TimeFormat::h24_mm_ss_milli> > 
    logger(CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::none,TimeFormat::h24_mm_ss_milli>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");

  logger.flush();
  EXPECT_EQ(std::string(",EVENT,1,sherlock(44),this\",\" is not an event;\n"), logger.status().stream().str().substr(size_t{ 12u }));
  printf("%s", logger.status().stream().str().c_str());
}
TEST_F(CsvLogFormatterTest, logCsvCommaMessageWithDateTime) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::year_mm_dd,TimeFormat::h24_mm_ss_milli> > 
    logger(CsvLogFormatter<',',std::ostringstream,size_t{ 255u },true,DateFormat::year_mm_dd,TimeFormat::h24_mm_ss_milli>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");

  logger.flush();
  EXPECT_EQ(std::string(",EVENT,1,sherlock(44),this\",\" is not an event;\n"), logger.status().stream().str().substr(size_t{ 23u }));
  printf("%s", logger.status().stream().str().c_str());
}

TEST_F(CsvLogFormatterTest, logCsvCommaMessageWithoutHeaders) {
  Logger<_CsvFormatter<','> > logger(_CsvFormatter<','>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d -\t%d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    ",0,nowhere(12),hello world !\n"
    "EVENT,1,sherlock(44),this\",\" is not an event;\n"
    "INFO,2,mib(21),code 257 -\t1 alien to remove\n"
    "WARNING,3,irobot(0),AI will save the world\n"
    "ERROR,4,mars-attacks(8), \n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvSemicolonMessageWithoutHeaders) {
  Logger<_CsvFormatter<';'> > logger(_CsvFormatter<';'>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d -\t%d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    ";0;nowhere(12);hello world !\n"
    "EVENT;1;sherlock(44);this, is not an event\";\"\n"
    "INFO;2;mib(21);code 257 -\t1 alien to remove\n"
    "WARNING;3;irobot(0);AI will save the world\n"
    "ERROR;4;mars-attacks(8); \n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvTabMessageWithoutHeaders) {
  Logger<_CsvFormatter<'\t'> > logger(_CsvFormatter<'\t'>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d -\t%d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    "\t0\tnowhere(12)\thello world !\n"
    "EVENT\t1\tsherlock(44)\tthis, is not an event;\n"
    "INFO\t2\tmib(21)\tcode 257 -\"\t\"1 alien to remove\n"
    "WARNING\t3\tirobot(0)\tAI will save the world\n"
    "ERROR\t4\tmars-attacks(8)\t \n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvSpaceMessageWithoutHeaders) {
  Logger<_CsvFormatter<' '> > logger(_CsvFormatter<' '>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d -\t%d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    " 0 nowhere(12) hello\" \"world\" \"!\n"
    "EVENT 1 sherlock(44) this,\" \"is\" \"not\" \"an\" \"event;\n"
    "INFO 2 mib(21) code\" \"257\" \"-\t1\" \"alien\" \"to\" \"remove\n"
    "WARNING 3 irobot(0) AI\" \"will\" \"save\" \"the\" \"world\n"
    "ERROR 4 mars-attacks(8) \" \"\n"), logger.status().stream().str());
}

TEST_F(CsvLogFormatterTest, logCsvMessageWithHeaders) {
  Logger<_CsvFormatter<','> > logger(_CsvFormatter<','>(std::ostringstream{}, true), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d -\t%d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    "TYPE,LEVEL,SOURCE,MESSAGE\n"
    ",0,nowhere(12),hello world !\n"
    "EVENT,1,sherlock(44),this\",\" is not an event;\n"
    "INFO,2,mib(21),code 257 -\t1 alien to remove\n"
    "WARNING,3,irobot(0),AI will save the world\n"
    "ERROR,4,mars-attacks(8), \n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvSemicolonMessageWithHeaders) {
  Logger<_CsvFormatter<';'> > logger(_CsvFormatter<';'>(std::ostringstream{}, true), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d -\t%d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    "TYPE;LEVEL;SOURCE;MESSAGE\n"
    ";0;nowhere(12);hello world !\n"
    "EVENT;1;sherlock(44);this, is not an event\";\"\n"
    "INFO;2;mib(21);code 257 -\t1 alien to remove\n"
    "WARNING;3;irobot(0);AI will save the world\n"
    "ERROR;4;mars-attacks(8); \n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvTabMessageWithHeaders) {
  Logger<_CsvFormatter<'\t'> > logger(_CsvFormatter<'\t'>(std::ostringstream{}, true), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d -\t%d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    "TYPE\tLEVEL\tSOURCE\tMESSAGE\n"
    "\t0\tnowhere(12)\thello world !\n"
    "EVENT\t1\tsherlock(44)\tthis, is not an event;\n"
    "INFO\t2\tmib(21)\tcode 257 -\"\t\"1 alien to remove\n"
    "WARNING\t3\tirobot(0)\tAI will save the world\n"
    "ERROR\t4\tmars-attacks(8)\t \n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvSpaceMessageWithHeaders) {
  Logger<_CsvFormatter<' '> > logger(_CsvFormatter<' '>(std::ostringstream{}, true), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s;", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d -\t%d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    "TYPE LEVEL SOURCE MESSAGE\n"
    " 0 nowhere(12) hello\" \"world\" \"!\n"
    "EVENT 1 sherlock(44) this,\" \"is\" \"not\" \"an\" \"event;\n"
    "INFO 2 mib(21) code\" \"257\" \"-\t1\" \"alien\" \"to\" \"remove\n"
    "WARNING 3 irobot(0) AI\" \"will\" \"save\" \"the\" \"world\n"
    "ERROR 4 mars-attacks(8) \" \"\n"), logger.status().stream().str());
}

TEST_F(CsvLogFormatterTest, logCsvIgnoredMessage) {
  Logger<_CsvFormatter<','> > logger(_CsvFormatter<','>(std::ostringstream{}, true), LogLevel::critical);
  EXPECT_EQ(LogLevel::critical, logger.getMinLevel());

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
  EXPECT_EQ(std::string(
    "TYPE,LEVEL,SOURCE,MESSAGE\n"
    "INFO,4,abc(42),blabla\n"
    "EVENT,3,def(24),trololololo\n"
    "EVENT,4,def(24),trololololo\n"), logger.status().stream().str());
}

TEST_F(CsvLogFormatterTest, logCsvCommaMessageNotEscaped) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 255u },false,DateFormat::none,TimeFormat::none> > logger(
    CsvLogFormatter<',',std::ostringstream,size_t{ 255u },false,DateFormat::none,TimeFormat::none>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::none, "nowhere", 12, "hello world !");
  logger.log(LogLevel::verbose, LogCategory::EVENT, "sherlock", 44, "this, is not an %s,", "event");
  logger.log(LogLevel::debug, LogCategory::INFO, "mib", 21, "code %d, %d %s to remove", 257, 1, "alien");
  logger.log(LogLevel::standard, LogCategory::WARNING, "irobot", 0, "AI will %s the world", "save");
  logger.log(LogLevel::critical, LogCategory::ERROR, "mars-attacks", 8, " ");

  logger.flush();
  EXPECT_EQ(std::string(
    ",0,nowhere(12),hello world !\n"
    "EVENT,1,sherlock(44),this, is not an event,\n"
    "INFO,2,mib(21),code 257, 1 alien to remove\n"
    "WARNING,3,irobot(0),AI will save the world\n"
    "ERROR,4,mars-attacks(8), \n"), logger.status().stream().str());
}


// -- size overflow --

TEST_F(CsvLogFormatterTest, logCsvCommaPrefixFills) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none> > logger(
    CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::INFO, "nowhere12345", 12, "hello world !");

  logger.flush();
  EXPECT_EQ(std::string(
    "INFO,0,nowhere12345(12),\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvCommaPrefixTooLong) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none> > logger(
    CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::INFO, "nowhere123456", 12, "hello world !");

  logger.flush();
  EXPECT_EQ(std::string(
    "INFO,0,nowhere123456(12)\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvCommaMessageFills) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none> > logger(
    CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::INFO, "nowhere", 12, "hello");

  logger.flush();
  EXPECT_EQ(std::string(
    "INFO,0,nowhere(12),hello\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvCommaMessageTooLong) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none> > logger(
    CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::INFO, "nowhere", 12, "hello world !");

  logger.flush();
  EXPECT_EQ(std::string(
    "INFO,0,nowhere(12),hello\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvCommaMessageTooLongWithParam) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none> > logger(
    CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::INFO, "nowhere", 12, "%s", "hello world !");

  logger.flush();
  EXPECT_EQ(std::string(
    "INFO,0,nowhere(12),hello\n"), logger.status().stream().str());
}
TEST_F(CsvLogFormatterTest, logCsvCommaMessageTooLongWhenEscaped) {
  Logger<CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none> > logger(
    CsvLogFormatter<',',std::ostringstream,size_t{ 25u },true,DateFormat::none,TimeFormat::none>(std::ostringstream{}, false), LogLevel::none);
  EXPECT_EQ(LogLevel::none, logger.getMinLevel());

  logger.log(LogLevel::none, LogCategory::INFO, "nowhere", 12, "1000,");
  logger.log(LogLevel::none, LogCategory::INFO, "nowhere", 12, "10,20");
  logger.log(LogLevel::none, LogCategory::INFO, "nowhere", 12, "1,2,3");
  logger.log(LogLevel::none, LogCategory::INFO, "nowhere", 12, "1,2,300");

  logger.flush();
  EXPECT_EQ(std::string(
    "INFO,0,nowhere(12),1000\n"
    "INFO,0,nowhere(12),10\",\"\n"
    "INFO,0,nowhere(12),1\",\"2\n"
    "INFO,0,nowhere(12),1\",\"2\n"), logger.status().stream().str());
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif
#if !defined(_MSC_VER) && !defined(__clang__) && defined(__GNUG__) && __GNUC__ > 5
# pragma GCC diagnostic pop
#endif
