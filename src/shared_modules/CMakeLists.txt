cmake_minimum_required(VERSION 3.12.4)

project(shared_modules)

if (NOT SRC_FOLDER)
    get_filename_component(SRC_FOLDER     ${CMAKE_SOURCE_DIR}/../ ABSOLUTE)
endif()

if (NOT SHARED_MODULES)
    get_filename_component(SHARED_MODULES ${CMAKE_SOURCE_DIR} ABSOLUTE)
endif()

if (NOT BUILD_DIRECTORY)
    get_filename_component(BUILD_DIRECTORY ${CMAKE_SOURCE_DIR}/build ABSOLUTE)
endif()

# Build modules on demand

option(BUILD_ROUTER "Build the router module" OFF)
option(BUILD_HTTP_REQUEST "Build the http-request module" OFF)
option(BUILD_INDEXER_CONNECTOR "Build the indexer_connector module" OFF)
option(BUILD_CONTENT_MANAGER "Build the content_manager module" OFF)

if (BUILD_HTTP_REQUEST OR BUILD_INDEXER_CONNECTOR OR BUILD_CONTENT_MANAGER)
    add_subdirectory(${SHARED_MODULES}/http-request ${BUILD_DIRECTORY}/http-request)
endif()

if (BUILD_ROUTER OR BUILD_CONTENT_MANAGER)
    add_subdirectory(${SHARED_MODULES}/router ${BUILD_DIRECTORY}/router)
endif()

if (BUILD_CONTENT_MANAGER)
    add_subdirectory(${SHARED_MODULES}/content_manager ${BUILD_DIRECTORY}/content_manager)
endif()

if (BUILD_INDEXER_CONNECTOR)
    add_subdirectory(${SHARED_MODULES}/indexer_connector ${BUILD_DIRECTORY}/indexer_connector)
endif()

