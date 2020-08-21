cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project("logging" CXX)

# see: http://stackoverflow.com/questions/23684789/cmake-build-multiple-executables-in-one-project-with-static-library

include (GenerateExportHeader)

option(LOGGING_BUILD_STATIC_MODULE_LIB "On to build a static library for each submodule, Off for shared library. default On" ON)

if(NOT TARGET logging-obj)

    set (logging_include
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_BINARY_DIR}
    )

    if (APPLE)
      set (logging_include ${logging_include} /usr/local/include)
    endif (APPLE)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set (logging_sys_libraries stdc++fs pthread)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (logging_sys_libraries stdc++fs pthread)
    endif()

    if (LOGGING_BUILD_STATIC_MODULE_LIB)
        set (logging_libraries logging-static)
        set (logging_defines "-DLOGGING_BUILT_AS_STATIC_LIB" "-DUSE_LOGGING_LIB")
    else()
        set (logging_libraries logging)
        set (logging_defines "-DUSE_LOGGING_LIB")
    endif()

    get_directory_property(hasParent PARENT_DIRECTORY)
    if (hasParent)
      set (logging_sys_libraries ${logging_sys_libraries} PARENT_SCOPE)
      set (logging_include ${logging_include} PARENT_SCOPE)
      set (logging_libraries ${logging_libraries} PARENT_SCOPE)
      set (logging_defines ${logging_defines} PARENT_SCOPE)
    endif()

    message(STATUS "logging_defines: ${logging_defines}")
    message(STATUS "logging_libraries: ${logging_libraries}")
    message(STATUS "logging_sys_libraries: ${logging_sys_libraries}")

    set (LOGGING_VERSION_MAJOR 1)
    set (LOGGING_VERSION_MINOR 0)
    set (LOGGING_VERSION_PATCH 1)
    set (LOGGING_VERSION "${LOGGING_VERSION_MAJOR}.${LOGGING_VERSION_MINOR}.${LOGGING_VERSION_PATCH}")

    set (LOGGING_INCLUDE_INSTALL_DIR include/logging)
    set (LOGGING_LIBRARY_INSTALL_DIR lib)

    include_directories(${logging_include})

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

    add_definitions(-DEXPERIMENTAL -Dlogging_obj_EXPORTS ${logging_defines})

    add_library(logging-obj OBJECT ${SOURCE_FILES})

    set_target_properties(logging-obj PROPERTIES
                          POSITION_INDEPENDENT_CODE ON
                          FOLDER libraries
                          CXX_STANDARD 11)

    if(LOGGING_BUILD_STATIC_MODULE_LIB)
        add_library(logging-static STATIC $<TARGET_OBJECTS:logging-obj>)
        install(TARGETS logging-static DESTINATION ${LOGGING_LIBRARY_INSTALL_DIR})
    else()
        add_library(logging SHARED $<TARGET_OBJECTS:logging-obj>)

        target_link_libraries(logging PRIVATE ${logging_sys_libraries})

        set_target_properties(logging PROPERTIES
                              POSITION_INDEPENDENT_CODE ON
                              FOLDER libraries
                              VERSION ${LOGGING_VERSION}
                              SOVERSION ${LOGGING_VERSION_MAJOR})
        install(TARGETS logging DESTINATION ${LOGGING_LIBRARY_INSTALL_DIR})
    endif()

    GENERATE_EXPORT_HEADER(logging-obj
        BASE_NAME LOGGING
        EXPORT_MACRO_NAME LOGGING_EXPORT
        EXPORT_FILE_NAME logging-export.h
        STATIC_DEFINE LOGGING_BUILT_AS_STATIC_LIB
    )

    install(FILES ${INCLUDE_FILES}
                  ${PROJECT_BINARY_DIR}/logging-export.h
                  ${PROJECT_BINARY_DIR}/logging_version.h
            DESTINATION ${LOGGING_INCLUDE_INSTALL_DIR})

endif()
