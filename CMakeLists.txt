# Original work Copyright (c) 2013-2015, Roland Bock
# Modified work Copyright (c) 2016, Aaron Bishop
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
#   Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# 
#   Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.6)
project (sqlpp11-connector-odbc)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

find_package(ODBC REQUIRED)

include_directories(${ODBC_INCLUDE_DIRECTORIES})

message(STATUS "Using ${CMAKE_CXX_COMPILER} (compiler id: ${CMAKE_CXX_COMPILER_ID})")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "-Wall -std=c++11 ${CMAKE_CXX_FLAGS}")
endif ()


set(SQLPP11_INCLUDE_DIR "/usr/include/" CACHE FILEPATH "Path to sqlpp11 includes")

if(NOT EXISTS ${SQLPP11_INCLUDE_DIR}/sqlpp11/exception.h)
  message(SEND_ERROR "Can't find exception.h in ${SQLPP11_INCLUDE_DIR}")
endif()

set(DATE_INCLUDE_DIR "${SQLPP11_INCLUDE_DIR}/sqlpp11" CACHE FILEPATH "Path to Howard Hinnant's date library")

if(NOT EXISTS ${DATE_INCLUDE_DIR}/date.h)
	message(SEND_ERROR "Can't find date.h in ${DATE_INCLUDE_DIR}")
endif()

set(SQLPP11_ODBC_VERSION_MAJOR 0)
set(SQLPP11_ODBC_VERSION_MINOR 4)
set(SQLPP11_ODBC_VERSION_PATCH 0)

if(${SQLPP11_ODBC_VERSION_MINOR} LESS 10)
	set(PROJECT_VERSION_MINOR "0${SQLPP11_ODBC_VERSION_MINOR}")
else()
	set(PROJECT_VERSION_MINOR ${SQLPP11_ODBC_VERSION_MINOR})
endif()

set(PROJECT_VERSION "${SQLPP11_ODBC_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
if(${SQLPP11_ODBC_VERSION_PATCH} GREATER 0)
	if(${PROJECT_VERSION_PATCH} LESS 10)
		set(PROJECT_VERSION_PATCH "0${SQLPP11_ODBC_VERSION_PATCH}")
	else()
		set(PROJECT_VERSION_PATCH ${SQLPP11_ODBC_VERSION_PATCH})
	endif()
	
	set(PROJECT_VERSION "${PROJECT_VERSION}.${PROJECT_VERSION_PATCH}")
	set(SQLPP11_ODBC_VERSION_INT "${SQLPP11_ODBC_VERSION_MINOR}${PROJECT_VERSION_PATCH}")
else()
	set(SQLPP11_ODBC_VERSION_PATCH 0)
	set(SQLPP11_ODBC_VERSION_INT "${SQLPP11_ODBC_VERSION_MINOR}00")
endif()

if(${SQLPP11_ODBC_VERSION_MAJOR} GREATER 0)
	if(${SQLPP11_ODBC_VERSION_MINOR} LESS 10)
		set(SQLPP11_ODBC_VERSION_INT "${SQLPP11_ODBC_VERSION_MAJOR}0${SQLPP11_ODBC_VERSION_INT}")
	else()
		set(SQLPP11_ODBC_VERSION_INT "${SQLPP11_ODBC_VERSION_MAJOR}${SQLPP11_ODBC_VERSION_INT}")
	endif()
endif()

set(SQLPP11_ODBC_VERSION "${PROJECT_VERSION}")

configure_file(version.h.in version.h @ONLY)

message(SEND_MESSAGE "including sqlpp11 from ${SQLPP11_INCLUDE_DIR}")
set(include_dir "${PROJECT_SOURCE_DIR}/sqlpp11")
include_directories(
	${SQLPP11_INCLUDE_DIR}
	${DATE_INCLUDE_DIR}
	${include_dir}
	${PROJECT_SOURCE_DIR}/include/
	)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/sqlpp11" DESTINATION ${DESTDIR}/include)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version.h" DESTINATION ${DESTDIR}/include/sqlpp11/odbc/)

add_subdirectory(src)

if(NOT CMAKE_SQLPP11_CONNECTOR_ODBC_TESTS_IGNORE)
    enable_testing()
    add_subdirectory(tests)
endif()
