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

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project (sqlpp11-connector-odbc VERSION 0.6.0 LANGUAGES CXX)

set(SQLPP11_ODBC_VERSION_MAJOR 0)
set(SQLPP11_ODBC_VERSION_MINOR 6)
set(SQLPP11_ODBC_VERSION_PATCH 0)
include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/SetVersion.cmake")

option(SQLPP11_ODBC_DISABLE_SHARED "Disable making sqlpp11-connector-odbc shared library" Off)
option(SQLPP11_ODBC_DISABLE_STATIC "Disable making sqlpp11-connector-odbc static library" Off)

find_package(Sqlpp11 REQUIRED)
include_directories(${HinnantDate_INCLUDE_DIR} $<TARGET_PROPERTY:sqlpp11,INTERFACE_INCLUDE_DIRECTORIES>)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/")
find_package(ODBC REQUIRED)

include_directories(${ODBC_INCLUDE_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/include)

enable_testing()

if(NOT CMAKE_SQLPP11_CONNECTOR_ODBC_TESTS_IGNORE)
    add_subdirectory(tests)
endif()

add_library(sqlpp11-odbc INTERFACE)

target_link_libraries(sqlpp11-odbc INTERFACE sqlpp11 ${ODBC_LIBRARIES})

install(TARGETS sqlpp11-odbc EXPORT Sqlpp11ConnectorODBCTargets)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/sqlpp11" DESTINATION include)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version.h" DESTINATION include/sqlpp11/odbc/)

if(NOT SQLPP11_ODBC_DISABLE_SHARED)
  add_library(sqlpp11-odbc-shared SHARED
	src/connection.cpp
	src/bind_result.cpp
	src/prepared_statement.cpp
	src/detail/connection_handle.cpp)
  target_include_directories(sqlpp11-odbc-shared INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )
  target_link_libraries(sqlpp11-odbc-shared sqlpp11-odbc)
  set_target_properties(sqlpp11-odbc-shared PROPERTIES OUTPUT_NAME sqlpp11-odbc)
  install(TARGETS sqlpp11-odbc-shared EXPORT Sqlpp11ConnectorODBCTargets DESTINATION lib)
endif()
if(NOT SQLPP11_ODBC_DISABLE_STATIC)
  add_library(sqlpp11-odbc-static STATIC
	src/connection.cpp
	src/bind_result.cpp
	src/prepared_statement.cpp
	src/detail/connection_handle.cpp)
  target_include_directories(sqlpp11-odbc-static INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )
  target_link_libraries(sqlpp11-odbc-static sqlpp11-odbc)
  set_target_properties(sqlpp11-odbc-static PROPERTIES OUTPUT_NAME sqlpp11-odbc)
  install(TARGETS sqlpp11-odbc-static EXPORT Sqlpp11ConnectorODBCTargets DESTINATION lib)
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/CMake/Sqlpp11ConnectorODBCConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

export(EXPORT Sqlpp11ConnectorODBCTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/CMake/Sqlpp11ConnectorODBCTargets.cmake"
)

configure_file(CMake/Sqlpp11ConnectorODBCConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/CMake/Sqlpp11ConnectorODBCConfig.cmake"
  COPYONLY
)

set(ConfigPackageLocation lib/CMake/Sqlpp11ConnectorODBC)
install(EXPORT Sqlpp11ConnectorODBCTargets
  FILE
    Sqlpp11ConnectorODBCTargets.cmake
  DESTINATION
    ${ConfigPackageLocation}
)

install(
  FILES
    "CMake/Sqlpp11ConnectorODBCConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CMake/Sqlpp11ConnectorODBCConfigVersion.cmake"
    "CMake/Modules/FindODBC.cmake"
  DESTINATION
    ${ConfigPackageLocation}
)
