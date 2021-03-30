cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project("logging" CXX)

# see: http://stackoverflow.com/questions/23684789/cmake-build-multiple-executables-in-one-project-with-static-library

include (GenerateExportHeader)
include (CMakePackageConfigHelpers)
include (GNUInstallDirs)

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

option(LOGGING_BUILD_STATIC_MODULE_LIB "On to build a static library for each submodule, Off for shared library. default On" ON)
option(LOGGING_CONFIG_INSTALL "On to make an installable standalone build, Off to build as part of a project. Default Off" OFF)
option(LOGGING_CXX_STANDARD "C++ standard to overwrite default cmake standard" ${CMAKE_CXX_STANDARD})

function(DebugPrint MSG)
  if(VERBOSE)
    message(STATUS "${MSG}")
  endif()
endfunction(DebugPrint)

if(NOT TARGET logging)

  set (LOGGING_VERSION_MAJOR 1)
  set (LOGGING_VERSION_MINOR 0)
  set (LOGGING_VERSION_PATCH 1)
  set (LOGGING_VERSION "${LOGGING_VERSION_MAJOR}.${LOGGING_VERSION_MINOR}.${LOGGING_VERSION_PATCH}")

  set (LOGGING_LIBRARIES logging)

  if (LOGGING_CXX_STANDARD STREQUAL "OFF")
    set (LOGGING_CXX_STANDARD 17)
  endif ()

  set (LOGGING_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}
    )

  set (LOGGING_CXX_FLAGS "-DUSE_LOGGING_LIB")

  if (APPLE)
    set (LOGGING_SYS_LIBRARIES pthread)
    set (LOGGING_INCLUDE_DIRS ${LOGGING_INCLUDE_DIRS} /usr/local/include)
  elseif (ANDROID)
    set(LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} -DANDROID)
  elseif ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
          ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND NOT (CMAKE_CXX_PLATFORM_ID STREQUAL "Windows")))
    set (LOGGING_SYS_LIBRARIES stdc++fs pthread)
  endif ()


  if (WIN32)
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} "-DWIN32_LEAN_AND_MEAN")
  endif (WIN32)

  if (LOGGING_BUILD_STATIC_MODULE_LIB)
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} "-DLOGGING_BUILT_AS_STATIC_LIB")
    set (LOGGING_LINK STATIC)
  else()
    set (LOGGING_LINK SHARED)
  endif()

  if(CMAKE_CXX_PLATFORM_ID STREQUAL "MinGW")
    set (LOGGING_CXX_FLAGS ${LOGGING_CXX_FLAGS} "-DUSE_MINGW")
    set (LOGGING_SYS_LIBRARIES stdc++fs pthread)
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
    "${PROJECT_BINARY_DIR}/logging_version.h"
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
    EXPORT_FILE_NAME logging-export.h
    STATIC_DEFINE LOGGING_BUILT_AS_STATIC_LIB
  )

  if (LOGGING_CONFIG_INSTALL)
    set (INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/logging)
    set (LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/logging)

    target_include_directories(logging
                               PUBLIC
                               "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                               "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>"
    )

    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/loggingConfigVersion.cmake
                                     VERSION ${LOGGING_VERSION}
                                     COMPATIBILITY SameMajorVersion)

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
                                  "${CMAKE_CURRENT_BINARY_DIR}/loggingConfig.cmake"
                                  INSTALL_DESTINATION ${LIBRARY_INSTALL_DIR})

    install(TARGETS logging
            EXPORT loggingTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR}
    )

    install(FILES ${INCLUDE_FILES}
      ${PROJECT_BINARY_DIR}/logging-export.h
      ${PROJECT_BINARY_DIR}/logging_version.h
      DESTINATION ${INCLUDE_INSTALL_DIR})

    install(EXPORT loggingTargets
            FILE loggingTargets.cmake
            NAMESPACE logging::
            DESTINATION ${LIBRARY_INSTALL_DIR}
    )

    export(TARGETS logging FILE loggingTargets.cmake)

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/loggingConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/loggingConfigVersion.cmake"
            DESTINATION ${LIBRARY_INSTALL_DIR})

  endif()

endif()
