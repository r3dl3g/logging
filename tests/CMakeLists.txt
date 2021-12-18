cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project("logging-test" CXX)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/testing testing)

include_directories(${PROJECT_BINARY_DIR} ${TESTING_INCLUDE_DIRS})

enable_testing()

set(tests
    formatter_test
)

add_definitions(${LOGGING_CXX_FLAGS})

foreach(test ${tests})
    add_executable(${test} WIN32 ${test}.cpp)
    target_link_libraries(${test} testing ${LOGGING_LIBRARIES} ${LOGGING_SYS_LIBRARIES})
    add_test(${test} ${CMAKE_CURRENT_BINARY_DIR}/${test})
    set_target_properties(${test} PROPERTIES
                          FOLDER tests
                          CXX_STANDARD ${LOGGING_CXX_STANDARD})
endforeach(test)
