# C++ logging framework

A small c++ logging framework.

## Motivation

Goal was to have a independent logging framework that makes use of modern c++ (c++11),
that can easily integrated into own code and into own cmake project files.

## Usage

### In cmake

In your CMakeLists.txt include this project directory and add the library logging to your target:

```cmake

add_subdirectory(logging)

add_definitions(${LOGGING_CXX_FLAGS})

add_executable(MyApp MyApp.cpp)
target_link_libraries(MyApp ${LOGGING_LIBRARIES} ${LOGGING_SYS_LIBRARIES})
set_target_properties(MyApp PROPERTIES CXX_STANDARD ${LOGGING_CXX_STANDARD})

```

### In C++ code

First include logging/logger.h.

```c++

#include <logging/logger.h>

```

To write something to the log use the corresponding logger, use
one of the predefined loggers and just pipe your message in, like for 
any ostream.

```c++

void my_fun () {

  logging::info() << "My first log entry!";

  int i = 4711
  logging::info() << "i = " << i << "!";

}

```

There are several predefined levels of loggers:

  - debug
  - info
  - warn
  - error
  - fatal

And optional, if you define LOGGING_ENABLE_TRACE:

  - trace

```c++

void my_fun () {

  logging::debug() << "Just for debugging purpose...";
  logging::info() << "Normal info";
  logging::warn() << "Not good but not critical";
  logging::error() << "More serious";

  try {
    throw std::runtime_error("Bad mistake");
  } catch (std::excpetion& ex) {
    logging::fatal() << "This is a fatal error: " << ex;
  }

}

```

## Sinks

By default, all logging is done to std::cout.
Additional sinks can be added and also predefined removed.

To add a sink, include \<logging/core.h\> and call

```c++

logging::core::instance().add_sink(ostream&, logging::level, record_formatter&);

```

For convenince there is a predefined file sinks and some predefined record_formatters:

```c++

logging::file_logger log_file("my_logfile.log",
                              logging::level::trace,
                              logging::core::get_standard_formatter());

```

The file_logger registers itself at construction and deregister itself
at destruction.

## Configuration

There is no config file!
Configuration has to be done in source code.

