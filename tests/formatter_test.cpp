
#include <iomanip>

#include <testing/testing.h>
#include <logging/logger.h>
#include <logging/core.h>
#include <logging/formatter.h>

DEFINE_LOGGING_CORE()

// --------------------------------------------------------------------------
void test_console_formatter () {
  logging::core& core = logging::core::instance();
  core.remove_all_sinks();
  std::ostringstream buffer;
  core.add_sink(&buffer, logging::level::debug, core.get_console_formatter());
  logging::debug() << "test";
  core.flush();
  core.remove_sink(&buffer);

  EXPECT_EQUAL(buffer.str(), std::string("test\n"));
}

// --------------------------------------------------------------------------
void test_no_time_formatter () {
  logging::core& core = logging::core::instance();
  core.remove_all_sinks();
  std::ostringstream buffer;
  core.add_sink(&buffer, logging::level::debug, core.get_no_time_formatter());
  logging::debug() << "test2";
  core.flush();
  core.remove_sink(&buffer);

  EXPECT_EQUAL(buffer.str(), std::string("debug|main|test2\n"));
}

// --------------------------------------------------------------------------
void test_regex () {
  EXPECT_REGEX("0003 12345xyz debug", "0003 .* debug");
}

// --------------------------------------------------------------------------
void test_standard_formatter () {
  logging::core& core = logging::core::instance();
  core.remove_all_sinks();
  std::ostringstream buffer;
  core.add_sink(&buffer, logging::level::debug, core.get_standard_formatter());
  logging::debug() << "test3";
  core.flush();
  core.remove_sink(&buffer);

  EXPECT_REGEX(buffer.str(), std::string("0003\\|.*\\|debug\\|main\\|test3\n"));
}

// --------------------------------------------------------------------------
void test_custom_formatter () {
  logging::core& core = logging::core::instance();
  core.remove_all_sinks();
  std::ostringstream buffer;
  auto fmt = [] (std::ostream& out, const logging::record& e) {
    out << "(" << e.line() << "," << e.level() << "," << e.thread_name() << "," << e.message() << ")";
  };
  core.add_sink(&buffer, logging::level::debug, fmt);
  logging::debug() << "test4";
  core.flush();
  core.remove_sink(&buffer);

  EXPECT_EQUAL(buffer.str(), std::string("(0004,debug,main,test4)"));
}

// --------------------------------------------------------------------------
void test_dynamic_formatter () {
  logging::core& core = logging::core::instance();
  core.remove_all_sinks();
  std::ostringstream buffer;

  using namespace logging;

  auto delem = fmt::constant("'/'");
  core.add_sink(&buffer, level::debug, custom_formatter({fmt::line, fmt::character<':'>, fmt::thread, delem, fmt::level, delem, fmt::message, fmt::endl}));
  logging::debug() << "test5";
  core.flush();
  core.remove_sink(&buffer);

  EXPECT_EQUAL(buffer.str(), std::string("0005:main'/'debug'/'test5\n"));
}

// --------------------------------------------------------------------------
void test_date_and_time_formatter () {
  logging::core& core = logging::core::instance();
  core.remove_all_sinks();
  std::ostringstream buffer;

  std::time_t now = std::time(NULL);
  std::tm t;
#ifdef WIN32
  localtime_s(&t, &now);
#else
  localtime_r(&now, &t);
#endif

  using namespace logging;

  core.add_sink(&buffer, level::debug, custom_formatter({fmt::date, fmt::character<'T'>, fmt::time, fmt::constant(": "), fmt::message, fmt::endl}));
  logging::debug() << "test6";
  core.flush();
  core.remove_sink(&buffer);

  std::ostringstream expected;
  expected << std::setfill('0') << (t.tm_year + 1900) << '-' << std::setw(2) << (t.tm_mon + 1) << '-' << std::setw(2) << t.tm_mday
           << 'T' << std::setw(2) << t.tm_hour << ':' << std::setw(2) << t.tm_min << ':' << std::setw(2) << t.tm_sec << ": test6\n";
  EXPECT_EQUAL(buffer.str(), expected.str());
}

// --------------------------------------------------------------------------
void test_main (const testing::start_params&) {
  testing::log_info("Running " __FILE__);
  run_test(test_console_formatter);
  run_test(test_no_time_formatter);
  run_test(test_regex);
  run_test(test_standard_formatter);
  run_test(test_custom_formatter);
  run_test(test_dynamic_formatter);
  run_test(test_date_and_time_formatter);
}

// --------------------------------------------------------------------------

