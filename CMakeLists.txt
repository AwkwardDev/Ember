# Copyright (c) 2014, 2015, 2016, 2017 Ember
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.1.3)
project(Ember)

option(BUILD_OPT_TOOLS "Build optional tools" ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/bin/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/bin/lib)
set(CMAKE_CXX_STANDARD 17)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	add_definitions(-D_HAS_AUTO_PTR_ETC=1)               # Until Boost conforms with C++17
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17") # Until CMake handles this properly

	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.10)
		message(FATAL_ERROR "You need VS2017 or newer to compile Ember on Windows!")
	endif()
endif()

option(DISABLE_EMBER_THREADS "Disable multithreading for debugging" OFF)

if(DISABLE_EMBER_THREADS)
	add_definitions(-DDEBUG_NO_THREADS)
endif()

find_package(Threads REQUIRED)

# todo, breaks with 64-bit
# http://stackoverflow.com/questions/9742003/platform-detection-in-cmake
if (WIN32)
    macro(get_WIN32_WINNT version)
        if (CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if ("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif ("${verMajor}" MATCHES "10")
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif(CMAKE_SYSTEM_VERSION)
    endmacro(get_WIN32_WINNT)

    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
endif(WIN32)

##############################
#            Boost           #
##############################
add_definitions(-DBOOST_ALL_NO_LIB)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.64.0 REQUIRED COMPONENTS program_options filesystem locale system)
include_directories(${Boost_INCLUDE_DIRS})

##############################
#             Git            #
##############################
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
git_describe(VERSION --tags --dirty=-d)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/libs/shared/shared/Version.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp" @ONLY)
list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp" Version.h)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp")

##############################
#            Botan           #
##############################
find_package(Botan 1.11.0 REQUIRED)
include_directories(${BOTAN_INCLUDE_DIRS})

##############################
#     MySQL Connector C++    #
##############################
find_package(MySQLConnectorCPP REQUIRED)
include_directories(${MYSQLCCPP_INCLUDE_DIRS})

##############################
#         FlatBuffers        #
##############################
find_package(FlatBuffers REQUIRED)
include_directories(${FLATBUFFERS_INCLUDE_DIR})
include(${CMAKE_MODULE_PATH}/BuildFlatBuffers.cmake)

##############################
#            zlib            #
##############################
find_package(zlib 1.2.8 REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

##############################
#            PCRE            #
##############################
find_package(PCRE 8.39 REQUIRED)
include_directories(${PCRE_INCLUDE_DIR})

##############################
#         Google Test        #
##############################
ADD_SUBDIRECTORY(deps/gtest-1.7.0)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	option(gtest_force_shared_crt ON)
endif()

add_definitions(-DDB_MYSQL) #temporary!

include(BuildDBCLoaders) # temp, maybe

add_subdirectory(spark-schemas)
add_subdirectory(tests)
add_subdirectory(src)
