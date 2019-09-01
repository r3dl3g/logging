cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project("logging" CXX)

# see: http://stackoverflow.com/questions/23684789/cmake-build-multiple-executables-in-one-project-with-static-library

include (GenerateExportHeader)

set(LOGGING_BUILD_STATIC_MODULE_LIBS ON CACHE BOOL "On to build a static library for each submodule, Off for shared library. default On")

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

    if (LOGGING_BUILD_STATIC_MODULE_LIBS)
        set (logging_libraries logging-static)
        set (logging_defines "-DLOGGING_BUILT_AS_STATIC")
    else()
        set (logging_libraries logging)
    endif()

    set (logging_sys_libraries ${logging_sys_libraries} PARENT_SCOPE)
    set (logging_include ${logging_include} PARENT_SCOPE)
    set (logging_libraries ${logging_libraries} PARENT_SCOPE)
    set (logging_defines ${logging_defines} PARENT_SCOPE)

    message(STATUS "logging_defines: ${logging_defines}")
    message(STATUS "logging_libraries: ${logging_libraries}")
    message(STATUS "logging_sys_libraries: ${logging_sys_libraries}")

    set (LOGGING_VERSION_MAJOR 1)
    set (LOGGING_VERSION_MINOR 0)
    set (LOGGING_VERSION_PATCH 0)
    set (LOGGING_VERSION "${LOGGING_VERSION_MAJOR}.${LOGGING_VERSION_MINOR}.${LOGGING_VERSION_PATCH}")

    include_directories(${logging_include})

    # header file to pass version settings to the source code
    configure_file (
        "${PROJECT_SOURCE_DIR}/version.h.in"
        "${PROJECT_BINARY_DIR}/logging_version.h"
    )

    file(GLOB SOURCE_FILES "*.h" "*.cpp" "*.inl")

    if (WIN32)
      add_definitions(-DWIN32_LEAN_AND_MEAN)
    endif (WIN32)

    add_definitions(-DEXPERIMENTAL -Dlogging_obj_EXPORTS ${logging_defines})

    add_library(logging-obj OBJECT ${SOURCE_FILES})

    set_target_properties(logging-obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

    if(LOGGING_BUILD_STATIC_MODULE_LIBS)
        add_library(logging-static STATIC $<TARGET_OBJECTS:logging-obj>)
    else()
        add_library(logging SHARED $<TARGET_OBJECTS:logging-obj>)

        target_link_libraries(logging PRIVATE ${logging_sys_libraries})

        set_target_properties(logging PROPERTIES
                                     POSITION_INDEPENDENT_CODE ON
                                     VERSION ${LOGGING_VERSION}
                                     SOVERSION ${LOGGING_VERSION_MAJOR})
    endif()

    GENERATE_EXPORT_HEADER(logging-obj
        BASE_NAME LOGGING
        EXPORT_MACRO_NAME LOGGING_EXPORT
        EXPORT_FILE_NAME logging-export.h
        STATIC_DEFINE LOGGING_BUILT_AS_STATIC
    )

endif()
