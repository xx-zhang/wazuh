cmake_minimum_required(VERSION 3.12.4)

project(syscollectorflatbuffers_unit_test)

set(CMAKE_CXX_FLAGS_DEBUG "-g --coverage")

file(GLOB SYSCOLLECTOR_FLATBUFFERS_UNIT_TEST_SRC
    "*.cpp")

add_executable(${PROJECT_NAME}
    ${SYSCOLLECTOR_FLATBUFFERS_UNIT_TEST_SRC}
)
target_link_libraries(${PROJECT_NAME}
    debug gtestd
    flatbuffers
)

add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME})

