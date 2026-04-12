cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

file(READ "derivation.nix" derivation_ver)
string(REGEX MATCH "version = \"([0-9\.]+)\"" _ ${derivation_ver})
set(project_version ${CMAKE_MATCH_1})

message("Building logging version: ${project_version}")

project("logging" LANGUAGES CXX VERSION ${project_version})

include (GenerateExportHeader)
include (CMakePackageConfigHelpers)
include (GNUInstallDirs)

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

if(NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND
    NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif()

set (LOGGING_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set (LOGGING_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set (LOGGING_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set (LOGGING_VERSION "${PROJECT_VERSION}")

option(LOGGING_BUILD_STATIC_MODULE_LIB "On to build a static library for each submodule, Off for shared library. default On" OFF)
option(LOGGING_CONFIG_INSTALL "On to make an installable standalone build, Off to build as part of a project. Default Off" ON)
option(LOGGING_BUILD_DEPENDENT_LIBS "On to build dependent lib testlib. Default Off" OFF)
option(LOGGING_TESTS "On to build the tests. Default Off" OFF)
option(LOGGING_NO_THREAD "Run logging core without a background thread. Default Off" OFF)
set(LOGGING_CXX_STANDARD "${CMAKE_CXX_STANDARD}" CACHE STRING "C++ standard to overwrite default cmake standard")

function(DebugPrint MSG)
  if(VERBOSE)
    message(STATUS "${MSG}")
  endif()
endfunction(DebugPrint)

if(NOT TARGET logging)

  set (LOGGING_LIBRARIES logging::logging)

  if (LOGGING_CXX_STANDARD STREQUAL "")
    set (LOGGING_CXX_STANDARD 17)
  endif ()

  set (LOGGING_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/inc
    ${PROJECT_BINARY_DIR}
  )

  set (LOGGING_CXX_FLAGS "-DUSE_LOGGING_LIB")

  if (APPLE)
    set (LOGGING_SYS_LIBRARIES pthread)
    set (LOGGING_INCLUDE_DIRS ${LOGGING_INCLUDE_DIRS} ${CMAKE_INSTALL_PREFIX}/include)
  elseif (ANDROID)
    set(LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DANDROID)
  elseif (EMSCRIPTEN)
    set(LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DUSE_EMSCRIPT -sNO_DISABLE_EXCEPTION_CATCHING -matomics)
    set(LOGGING_BUILD_STATIC_MODULE_LIB ON)
  elseif ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
      ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND NOT (CMAKE_CXX_PLATFORM_ID STREQUAL "Windows")) AND NOT ((CMAKE_CXX_PLATFORM_ID STREQUAL "MinGW")))
    set (LOGGING_SYS_LIBRARIES stdc++fs pthread)
  endif ()

  if (WIN32)
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DWIN32_LEAN_AND_MEAN)
  endif (WIN32)

  if (LOGGING_BUILD_STATIC_MODULE_LIB)
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DLOGGING_BUILT_AS_STATIC_LIB)
    set (LOGGING_LINK STATIC)
  else()
    set (LOGGING_LINK SHARED)
  endif()

  if (LOGGING_NO_THREAD)
  set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DLOGGING_NO_THREAD)
  endif()

  if(CMAKE_CXX_PLATFORM_ID STREQUAL "MinGW")
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DUSE_MINGW)
    set (LOGGING_SYS_LIBRARIES stdc++fs)
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "10.0.0")
      set (LOGGING_SYS_LIBRARIES ${LOGGING_SYS_LIBRARIES} pthread)
    endif()
  endif()

  get_directory_property(hasParent PARENT_DIRECTORY)
  if (hasParent)
    set (LOGGING_SYS_LIBRARIES ${LOGGING_SYS_LIBRARIES} PARENT_SCOPE)
    set (LOGGING_INCLUDE_DIRS ${LOGGING_INCLUDE_DIRS} PARENT_SCOPE)
    set (LOGGING_LIBRARIES ${LOGGING_LIBRARIES} PARENT_SCOPE)
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} PARENT_SCOPE)
    set (LOGGING_CXX_STANDARD ${LOGGING_CXX_STANDARD} PARENT_SCOPE)
  else()
    SET (LOGGING_CONFIG_INSTALL On)
  endif()

  DebugPrint("LOGGING_CXX_FLAGS: ${LOGGING_CXX_FLAGS}")
  DebugPrint("LOGGING_LIBRARIES: ${LOGGING_LIBRARIES}")
  DebugPrint("LOGGING_SYS_LIBRARIES: ${LOGGING_SYS_LIBRARIES}")
  DebugPrint("LOGGING_CXX_STANDARD: ${LOGGING_CXX_STANDARD}")

  include_directories(${LOGGING_INCLUDE_DIRS})

  # header file to pass version settings to the source code
  configure_file (
    "${PROJECT_SOURCE_DIR}/src/version.h.in"
    "${PROJECT_BINARY_DIR}/inc/logging_version.h"
  )

  set(SOURCE_FILES
    src/core.cpp
    src/log_level.cpp
    src/message_queue.cpp
    src/record.cpp
    src/recorder.cpp
  )
  set(INCLUDE_FILES
    src/core.h
    src/core.inl
    src/dbgstream.h
    src/formatter.h
    src/file_logger.h
    src/logger.h
    src/log_level.h
    src/message_queue.h
    src/recorder.h
    src/recorder.inl
    src/record.h
    src/record.inl
    src/redirect_stream.h
  )

  if (NOT ANDROID)
    set(CMAKE_DEBUG_POSTFIX d)
  endif ()

  add_definitions(-Dlogging_EXPORTS ${LOGGING_CXX_FLAGS})

  add_library(logging ${LOGGING_LINK} ${SOURCE_FILES} ${INCLUDE_FILES})
  add_library(logging::logging ALIAS logging)

  set_target_properties(logging PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    FOLDER libraries
    VERSION ${LOGGING_VERSION}
    SOVERSION ${LOGGING_VERSION_MAJOR}
    CXX_STANDARD ${LOGGING_CXX_STANDARD}
  )

  if(LOGGING_BUILD_STATIC_MODULE_LIB)
    message(STATUS "Build LOGGING as static Library")
  else()
    message(STATUS "Build LOGGING as shared Library")

    target_link_libraries(logging PRIVATE ${LOGGING_SYS_LIBRARIES})
  endif()

  GENERATE_EXPORT_HEADER(logging
    BASE_NAME LOGGING
    EXPORT_MACRO_NAME LOGGING_EXPORT
    EXPORT_FILE_NAME inc/logging-export.h
    STATIC_DEFINE LOGGING_BUILT_AS_STATIC_LIB
  )

  if (LOGGING_CONFIG_INSTALL)

  if (CMAKE_CXX_PLATFORM_ID STREQUAL MinGW)
      set(OUTPUT_NAME logging-mingw)
    elseif(EMSCRIPTEN)
      set(OUTPUT_NAME logging-emc)
    else()
      set(OUTPUT_NAME logging)
    endif()

    set (LOGGING_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_INCLUDEDIR}/logging)
    set (LOGGING_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${OUTPUT_NAME})

    target_include_directories(logging
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${LOGGING_INSTALL_INCLUDE_DIR}>
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/loggingConfigVersion.cmake
        VERSION ${LOGGING_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/loggingConfig.cmake
        INSTALL_DESTINATION ${LOGGING_INSTALL_CMAKE_DIR}
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR
    )

    install(TARGETS logging
        EXPORT loggingTargets
        INCLUDES DESTINATION ${LOGGING_INSTALL_INCLUDE_DIR}
    )

    install(FILES
        ${INCLUDE_FILES}
        ${PROJECT_BINARY_DIR}/inc/logging-export.h
        ${PROJECT_BINARY_DIR}/inc/logging_version.h
        LICENSE
        DESTINATION ${LOGGING_INSTALL_INCLUDE_DIR}
    )

    install(EXPORT loggingTargets
        FILE loggingTargets.cmake
        NAMESPACE logging::
        DESTINATION ${LOGGING_INSTALL_CMAKE_DIR}
    )

    export(TARGETS logging FILE loggingTargets.cmake)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/loggingConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/loggingConfigVersion.cmake
        DESTINATION ${LOGGING_INSTALL_CMAKE_DIR}
    )

  endif()

  if(LOGGING_TESTS)
    set(testing_DIR ${CMAKE_PREFIX_PATH}/lib/cmake/testing)
    add_subdirectory(tests)
  endif()

endif()
