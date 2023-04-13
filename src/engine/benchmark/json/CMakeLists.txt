# Copyright (C) 2015-2021, Wazuh Inc.
#
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    GIT_TAG v3.11.2
    DOWNLOAD_ONLY YES
    )

# Configuration regex test
add_executable(json_bench
json_bench.cpp
)

# Build benchmarks
target_link_libraries(json_bench benchmark::benchmark_main json)

target_include_directories(json_bench
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${nlohmann_json_SOURCE_DIR}/include
    )
