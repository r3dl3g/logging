cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project("logging" LANGUAGES CXX VERSION 1.1.0)

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

option(LOGGING_BUILD_STATIC_MODULE_LIB "On to build a static library for each submodule, Off for shared library. default On" ON)
option(LOGGING_CONFIG_INSTALL "On to make an installable standalone build, Off to build as part of a project. Default Off" OFF)
option(LOGGING_CXX_STANDARD "C++ standard to overwrite default cmake standard" ${CMAKE_CXX_STANDARD})
option(LOGGING_TESTS "On to build the tests. Default Off" OFF)

function(DebugPrint MSG)
  if(VERBOSE)
    message(STATUS "${MSG}")
  endif()
endfunction(DebugPrint)

if(NOT TARGET logging)

  set (LOGGING_LIBRARIES logging::logging)

  if (LOGGING_CXX_STANDARD STREQUAL "OFF")
    set (LOGGING_CXX_STANDARD 17)
  endif ()

  set (LOGGING_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${PROJECT_BINARY_DIR}
    )

  set (LOGGING_CXX_FLAGS "-DUSE_LOGGING_LIB")

  if (APPLE)
    set (LOGGING_SYS_LIBRARIES pthread)
    set (LOGGING_INCLUDE_DIRS ${LOGGING_INCLUDE_DIRS} /usr/local/include)
  elseif (ANDROID)
    set(LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DANDROID)
  elseif (EMSCRIPTEN)
    set(LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DUSE_FILE_TIME_POINT -DUSE_EMSCRIPT)
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
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/logging/logging_version.h"
  )

  set(SOURCE_FILES core.cpp
    log_level.cpp
    message_queue.cpp
    record.cpp
    recorder.cpp
  )
  set(INCLUDE_FILES core.h
    core.inl
    dbgstream.h
    formatter.h
    file_logger.h
    logger.h
    log_level.h
    message_queue.h
    recorder.h
    recorder.inl
    record.h
    record.inl
    redirect_stream.h
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
    DebugPrint("LOGGING: Create static Library")
  else()
    DebugPrint("LOGGING: Create shared Library")

    target_link_libraries(logging PRIVATE ${LOGGING_SYS_LIBRARIES})
  endif()

  GENERATE_EXPORT_HEADER(logging
    BASE_NAME LOGGING
    EXPORT_MACRO_NAME LOGGING_EXPORT
    EXPORT_FILE_NAME logging/logging-export.h
    STATIC_DEFINE LOGGING_BUILT_AS_STATIC_LIB
  )

  if (LOGGING_CONFIG_INSTALL)
    set (LOGGING_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/logging)
    set (LOGGING_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}/cmake/logging)

    target_include_directories(logging
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${LOGGING_INSTALL_INCLUDEDIR}>
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/loggingConfigVersion.cmake
        VERSION ${LOGGING_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/loggingConfig.cmake
        INSTALL_DESTINATION ${LOGGING_INSTALL_LIBDIR}
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR
    )

    install(TARGETS logging
        EXPORT loggingTargets
        INCLUDES DESTINATION ${LOGGING_INSTALL_INCLUDEDIR}
    )

    install(FILES
        ${INCLUDE_FILES}
        ${PROJECT_BINARY_DIR}/logging/logging-export.h
        ${PROJECT_BINARY_DIR}/logging/logging_version.h
        LICENSE
        DESTINATION ${LOGGING_INSTALL_INCLUDEDIR}
    )

    install(EXPORT loggingTargets
        FILE loggingTargets.cmake
        NAMESPACE logging::
        DESTINATION ${LOGGING_INSTALL_LIBDIR}
    )

    export(TARGETS logging FILE loggingTargets.cmake)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/loggingConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/loggingConfigVersion.cmake
        DESTINATION ${LOGGING_INSTALL_LIBDIR}
    )

  endif()

  if(LOGGING_TESTS)
    add_subdirectory(tests)
  endif()

endif()
