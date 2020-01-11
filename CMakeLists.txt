cmake_minimum_required(VERSION 2.8)


# Options:
#    LEXBOR_OPTIMIZATION_LEVEL           default: -O2
#    LEXBOR_C_FLAGS                      default: see this file
#    LEXBOR_CXX_FLAGS                    default: see this file
#    LEXBOR_WITHOUT_THREADS              default: ON; Not used now, for the future
#    LEXBOR_BUILD_SHARED                 default: ON; Create shaded library
#    LEXBOR_BUILD_STATIC                 default: ON; Create static library
#    LEXBOR_BUILD_EXAMPLES               default: OFF; Build all examples
#    LEXBOR_BUILD_TESTS                  default: OFF; Build tests
#    LEXBOR_BUILD_TESTS_CPP              default: ON; Build C++ tests.
#                                         Used with LEXBOR_BUILD_TESTS
#    LEXBOR_BUILD_UTILS                  default: OFF; Build utils/helpers for project.
#    LEXBOR_BUILD_WITH_ASAN              default: OFF; Build with address sanitizer if possible
#    LEXBOR_INSTALL_HEADERS              default: ON; The header files will be installed
#                                         if set to ON
#    LEXBOR_MAKE_DISTRO_NUM              default: 1; For packaging
#    LEXBOR_PRINT_MODULE_DEPENDENCIES    default: OFF; Prints dependencies between modules

# For build with ASAN use CMake flag:
#     -DLEXBOR_BUILD_WITH_ASAN=ON
#     or
#     -DCMAKE_C_FLAGS='-O0 -g -fsanitize=address'
#     or
#     -DLEXBOR_C_FLAGS='-O0 -g -fsanitize=address'


set(PRODUCT_NAME "grammar")
project(${PRODUCT_NAME})

set(PROJECT_NAME "lexbor")

message(STATUS "Project name: ${PROJECT_NAME}")

################
## Options
#########################
option(LEXBOR_WITHOUT_THREADS "Build without Threads" ON)
option(LEXBOR_BUILD_SHARED "Build shared library" ON)
option(LEXBOR_BUILD_STATIC "Build static library" ON)
option(LEXBOR_BUILD_EXAMPLES "Build examples" OFF)
option(LEXBOR_BUILD_TESTS "Build tests" OFF)
option(LEXBOR_BUILD_TESTS_CPP "Build C++ tests" ON)
option(LEXBOR_BUILD_UTILS "Build utils" OFF)
option(LEXBOR_BUILD_WITH_ASAN "Build with address sanitizer" OFF)
option(LEXBOR_INSTALL_HEADERS "Install header files" ON)
option(LEXBOR_MAKE_PACKAGES_FILES "Create files for build packages" OFF)
option(LEXBOR_PRINT_MODULE_DEPENDENCIES "Prints dependencies" OFF)

IF(NOT LEXBOR_MAKE_DISTRO_NUM)
    set(LEXBOR_MAKE_DISTRO_NUM "1")
ENDIF()

################
## Version and path
#########################
set(LEXBOR_BASE "core")
set(LEXBOR_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/source" CACHE STRING "")
set(LEXBOR_SOURCE_LEXBOR "${LEXBOR_SOURCE}/lexbor" CACHE STRING "")
set(LEXBOR_DIR_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib" CACHE STRING "")
set(LEXBOR_DIR_HEADER "${LEXBOR_SOURCE}" CACHE STRING "")
set(LEXBOR_DIR_HEADER_LEXBOR "${LEXBOR_SOURCE}/lexbor" CACHE STRING "")
set(LEXBOR_SOURCE_PORT_DIR "${LEXBOR_SOURCE_LEXBOR}/ports" CACHE STRING "")
set(LEXBOR_DIR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "")

set(LEXBOR_VERSION_FILEPATH "${CMAKE_CURRENT_SOURCE_DIR}/version" CACHE STRING "")

set(LEXBOR_INSTALL_DLL_EXE_DIR "bin")

################
## RPATH
#########################
include(GNUInstallDirs)

IF(APPLE)
    set(CMAKE_MACOSX_RPATH ON)

    set(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
ENDIF(APPLE)

################
## lib param
#########################
IF(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR "lib")
ENDIF()

################
## CMake Includes
#########################
include("${CMAKE_CURRENT_SOURCE_DIR}/config.cmake")

################
## Features
#########################
include("${CMAKE_CURRENT_SOURCE_DIR}/feature.cmake")

################
## Project Version
#########################
GET_LEXBOR_VERSION(LEXBOR_VERSION_MAJOR LEXBOR_VERSION_MINOR
                   LEXBOR_VERSION_PATCH LEXBOR_VERSION_STRING)

message(STATUS "Grammar version: ${LEXBOR_VERSION_STRING}")

################
## Includes
#########################
include_directories(${LEXBOR_DIR_HEADER})

################
## Export LEXBOR_INCLUDE_DIRS and LEXBOR_LIBRARIES to global CACHE
#########################
set(LEXBOR_INCLUDES ${LEXBOR_DIR_HEADER} CACHE STRING "Include paths for ${PROJECT_NAME}")
set(LEXBOR_LIBRARIES ${LEXBOR_LIB_NAME} CACHE STRING "Libraries to link for ${PROJECT_NAME}")

################
## Sources
#########################
GET_MODULES_LIST(LEXBOR_MODULES "${LEXBOR_SOURCE}")

################
## Check libraries
#########################
IF(LEXBOR_BUILD_SHARED)
    find_library(LEXBOR_REQ_LIB_CORE_PATH NAMES "lexbor-core")
    IF(NOT LEXBOR_REQ_LIB_CORE_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-core")
    ENDIF()

    find_library(LEXBOR_REQ_LIB_HTML_PATH NAMES "lexbor-html")
    IF(NOT LEXBOR_REQ_LIB_HTML_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-html")
    ENDIF()

    find_library(LEXBOR_REQ_LIB_DOM_PATH NAMES "lexbor-dom")
    IF(NOT LEXBOR_REQ_LIB_DOM_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-dom")
    ENDIF()

    find_library(LEXBOR_REQ_LIB_TAG_PATH NAMES "lexbor-tag")
    IF(NOT LEXBOR_REQ_LIB_TAG_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-tag")
    ENDIF()

    find_library(LEXBOR_REQ_LIB_NS_PATH NAMES "lexbor-ns")
    IF(NOT LEXBOR_REQ_LIB_NS_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-ns")
    ENDIF()
ENDIF()

IF(LEXBOR_BUILD_STATIC)
    find_library(LEXBOR_REQ_LIB_CORE_STATIC_PATH NAMES "lexbor-core_static")
    IF(NOT LEXBOR_REQ_LIB_CORE_STATIC_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-core_static")
    ENDIF()

    find_library(LEXBOR_REQ_LIB_HTML_STATIC_PATH NAMES "lexbor-html_static")
    IF(NOT LEXBOR_REQ_LIB_HTML_STATIC_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-html_static")
    ENDIF()

    find_library(LEXBOR_REQ_LIB_DOM_STATIC_PATH NAMES "lexbor-dom_static")
    IF(NOT LEXBOR_REQ_LIB_DOM_STATIC_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-dom_static")
    ENDIF()

    find_library(LEXBOR_REQ_LIB_TAG_STATIC_PATH NAMES "lexbor-tag_static")
    IF(NOT LEXBOR_REQ_LIB_TAG_STATIC_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-tag_static")
    ENDIF()

    find_library(LEXBOR_REQ_LIB_NS_STATIC_PATH NAMES "lexbor-ns_static")
    IF(NOT LEXBOR_REQ_LIB_NS_STATIC_PATH)
        message(FATAL_ERROR "Required library not found: liblexbor-ns_static")
    ENDIF()
ENDIF()

find_path(LEXBOR_REQ_INC_PATH NAMES "lexbor/core/base.h")
IF(NOT LEXBOR_REQ_INC_PATH)
    message(FATAL_ERROR "Required headers not found: lexbor/core/*")
ENDIF()

include_directories("${LEXBOR_REQ_INC_PATH}")

find_path(LEXBOR_REQ_INC_PATH NAMES "lexbor/html/base.h")
IF(NOT LEXBOR_REQ_INC_PATH)
    message(FATAL_ERROR "Required headers not found: lexbor/html/*")
ENDIF()

include_directories("${LEXBOR_REQ_INC_PATH}")

################
## Build all modules in one liblexbor library
#########################
set(LEXBOR_LIB_NAME "${PRODUCT_NAME}")
set(LEXBOR_LIB_NAME_STATIC "${PRODUCT_NAME}_static")

## Get all source files from modules
FOREACH(module ${LEXBOR_MODULES})
    GET_MODULE_RESURSES(headers sources "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})
    LIST(APPEND LEXBOR_SOURCES ${sources})
ENDFOREACH()

## First, need to add target for shared and static library
IF(LEXBOR_BUILD_SHARED)
    add_library(${LEXBOR_LIB_NAME} SHARED ${LEXBOR_SOURCES})
ENDIF()

IF(LEXBOR_BUILD_STATIC)
    add_library(${LEXBOR_LIB_NAME_STATIC} STATIC ${LEXBOR_SOURCES})
ENDIF()

FOREACH(module ${LEXBOR_MODULES})
    IF(LEXBOR_INSTALL_HEADERS)
        INSTALL_MODULE_HEADERS("${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})
    ENDIF()

    INCLUDE_MODULE_CONFIG(${PROJECT_NAME} ${module} "${LEXBOR_SOURCE_LEXBOR}/${module}")
    GET_MODULE_DEPENDENCIES(${PROJECT_NAME} ${module} module_deps)
    GET_MODULE_VERSION(major minor patch version_string
                       "${LEXBOR_SOURCE}" "${PROJECT_NAME}" ${module})

    MODULE_PRINT(${module} ${version_string} "${module_deps}")
ENDFOREACH()

## Create dynamic/static library
IF(LEXBOR_BUILD_SHARED)
    ADD_MODULE_LIBRARY("" ${LEXBOR_LIB_NAME} ${LEXBOR_VERSION_STRING}
                       ${LEXBOR_VERSION_MAJOR} "${LEXBOR_SOURCES}")
    target_link_libraries(${LEXBOR_LIB_NAME} "${LEXBOR_REQ_LIB_CORE_PATH}")
    target_link_libraries(${LEXBOR_LIB_NAME} "${LEXBOR_REQ_LIB_HTML_PATH}")
    target_link_libraries(${LEXBOR_LIB_NAME} "${LEXBOR_REQ_LIB_DOM_PATH}")
    target_link_libraries(${LEXBOR_LIB_NAME} "${LEXBOR_REQ_LIB_TAG_PATH}")
    target_link_libraries(${LEXBOR_LIB_NAME} "${LEXBOR_REQ_LIB_NS_PATH}")
ENDIF()

IF(LEXBOR_BUILD_STATIC)
    ADD_MODULE_LIBRARY("" ${LEXBOR_LIB_NAME_STATIC} ${LEXBOR_VERSION_STRING}
                       ${LEXBOR_VERSION_MAJOR} "${LEXBOR_SOURCES}")
    target_link_libraries(${LEXBOR_LIB_NAME_STATIC} "${LEXBOR_REQ_LIB_CORE_STATIC_PATH}")
    target_link_libraries(${LEXBOR_LIB_NAME_STATIC} "${LEXBOR_REQ_LIB_HTML_STATIC_PATH}")
    target_link_libraries(${LEXBOR_LIB_NAME_STATIC} "${LEXBOR_REQ_LIB_DOM_STATIC_PATH}")
    target_link_libraries(${LEXBOR_LIB_NAME_STATIC} "${LEXBOR_REQ_LIB_TAG_STATIC_PATH}")
    target_link_libraries(${LEXBOR_LIB_NAME_STATIC} "${LEXBOR_REQ_LIB_NS_STATIC_PATH}")
ENDIF()

################
## Sets C_FLAGS
#########################
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LEXBOR_OPTIMIZATION_LEVEL} ${LEXBOR_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LEXBOR_OPTIMIZATION_LEVEL} ${LEXBOR_CXX_FLAGS}")
message(STATUS "CFLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "CXXFLAGS: ${CMAKE_CXX_FLAGS}")

################
## Check features
#########################
FEATURE_CHECK_ASAN(LEXBOR_HAVE_ASAN)
IF(${LEXBOR_HAVE_ASAN})
    add_definitions(-DLEXBOR_HAVE_ADDRESS_SANITIZER)
ENDIF()

################
## Tests
#########################
IF(LEXBOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
ENDIF()

################
## Examples
#########################
IF(LEXBOR_BUILD_EXAMPLES)
    add_subdirectory(examples)
ENDIF()

################
## Utils
#########################
IF(LEXBOR_BUILD_UTILS)
    add_subdirectory(utils)
ENDIF()
