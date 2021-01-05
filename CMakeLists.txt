cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project("logging" CXX)

# see: http://stackoverflow.com/questions/23684789/cmake-build-multiple-executables-in-one-project-with-static-library

include (GenerateExportHeader)

option(LOGGING_BUILD_STATIC_MODULE_LIB "On to build a static library for each submodule, Off for shared library. default On" ON)
option(LOGGING_STANDALONE_BUILD "On to make an installable standalone build, Off to build as part of a project. Default Off" OFF)
option(LOGGING_CXX_STANDARD "C++ standard to overwrite default cmake standard" ${CMAKE_CXX_STANDARD})

if(NOT TARGET logging-obj)

  set (LOGGING_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}
    )

  if (APPLE)
    set (LOGGING_INCLUDE_DIRS ${LOGGING_INCLUDE_DIRS} /usr/local/include)
  endif (APPLE)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set (LOGGING_SYS_LIBRARIES stdc++fs pthread)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (LOGGING_SYS_LIBRARIES stdc++fs pthread)
  endif()

  if (LOGGING_BUILD_STATIC_MODULE_LIB)
    set (LOGGING_LIBRARIES logging-static)
    set (LOGGING_CXX_FLAGS "-DLOGGING_BUILT_AS_STATIC_LIB" "-DUSE_LOGGING_LIB")
  else()
    set (LOGGING_LIBRARIES logging)
    set (LOGGING_CXX_FLAGS "-DUSE_LOGGING_LIB")
  endif()

  if(CMAKE_CXX_PLATFORM_ID STREQUAL "MinGW")
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} "-DUSE_MINGW")
  endif()

  if (LOGGING_CXX_STANDARD STREQUAL "OFF")
    set (LOGGING_CXX_STANDARD 17)
  endif ()

  get_directory_property(hasParent PARENT_DIRECTORY)
  if (hasParent)
    set (LOGGING_SYS_LIBRARIES ${LOGGING_SYS_LIBRARIES} PARENT_SCOPE)
    set (LOGGING_INCLUDE_DIRS ${LOGGING_INCLUDE_DIRS} PARENT_SCOPE)
    set (LOGGING_LIBRARIES ${LOGGING_LIBRARIES} PARENT_SCOPE)
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} PARENT_SCOPE)
    set (LOGGING_CXX_STANDARD ${LOGGING_CXX_STANDARD} PARENT_SCOPE)
  else()
    SET (LOGGING_STANDALONE_BUILD On)
  endif()

  message(STATUS "LOGGING_CXX_FLAGS: ${LOGGING_CXX_FLAGS}")
  message(STATUS "LOGGING_LIBRARIES: ${LOGGING_LIBRARIES}")
  message(STATUS "LOGGING_SYS_LIBRARIES: ${LOGGING_SYS_LIBRARIES}")
  message(STATUS "LOGGING_CXX_STANDARD: ${LOGGING_CXX_STANDARD}")

  set (LOGGING_VERSION_MAJOR 1)
  set (LOGGING_VERSION_MINOR 0)
  set (LOGGING_VERSION_PATCH 1)
  set (LOGGING_VERSION "${LOGGING_VERSION_MAJOR}.${LOGGING_VERSION_MINOR}.${LOGGING_VERSION_PATCH}")

  set (LOGGING_INCLUDE_INSTALL_DIR include/logging)
  set (LOGGING_LIBRARY_INSTALL_DIR lib)

  include_directories(${LOGGING_INCLUDE_DIRS})

  # header file to pass version settings to the source code
  configure_file (
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/logging_version.h"
    )

  set(SOURCE_FILES core.cpp
    log_level.cpp
    message_queue.cpp
    record.cpp
    recorder.cpp)
  set(INCLUDE_FILES core.h
    core.inl
    dbgstream.h
    file_logger.h
    logger.h
    log_level.h
    message_queue.h
    recorder.h
    recorder.inl
    record.h
    record.inl
    redirect_stream.h)

  if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
  endif (WIN32)

  set(CMAKE_DEBUG_POSTFIX d)

  add_definitions(-DEXPERIMENTAL -Dlogging_obj_EXPORTS ${LOGGING_CXX_FLAGS})

  add_library(logging-obj OBJECT ${SOURCE_FILES} ${INCLUDE_FILES})

  set_target_properties(logging-obj PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    FOLDER libraries
    CXX_STANDARD ${LOGGING_CXX_STANDARD})

  if(LOGGING_BUILD_STATIC_MODULE_LIB)
    add_library(logging-static STATIC $<TARGET_OBJECTS:logging-obj>)
    if (LOGGING_STANDALONE_BUILD)
      install(TARGETS logging-static DESTINATION ${LOGGING_LIBRARY_INSTALL_DIR})
    endif()
  else()
    add_library(logging SHARED $<TARGET_OBJECTS:logging-obj>)

    target_link_libraries(logging PRIVATE ${LOGGING_SYS_LIBRARIES})

    set_target_properties(logging PROPERTIES
      POSITION_INDEPENDENT_CODE ON
      FOLDER libraries
      VERSION ${LOGGING_VERSION}
      SOVERSION ${LOGGING_VERSION_MAJOR})
    if (LOGGING_STANDALONE_BUILD)
      install(TARGETS logging DESTINATION ${LOGGING_LIBRARY_INSTALL_DIR})
    endif()
  endif()

  GENERATE_EXPORT_HEADER(logging-obj
    BASE_NAME LOGGING
    EXPORT_MACRO_NAME LOGGING_EXPORT
    EXPORT_FILE_NAME logging-export.h
    STATIC_DEFINE LOGGING_BUILT_AS_STATIC_LIB
    )

  if (LOGGING_STANDALONE_BUILD)
    install(FILES ${INCLUDE_FILES}
      ${PROJECT_BINARY_DIR}/logging-export.h
      ${PROJECT_BINARY_DIR}/logging_version.h
      DESTINATION ${LOGGING_INCLUDE_INSTALL_DIR})
  endif()

endif()
