# Find the ODBC driver manager includes and library.
# 
# This module defines
# ODBC_INCLUDE_DIRECTORIES, where to find sql.h
# ODBC_LIBRARIES, the libraries to link against to use ODBC

set(ODBC_INCLUDE_DIR "${ODBC_INCLUDE_DIR}" CACHE PATH "Directory to include for ODBC headers")
set(ODBC_LIBRARY_DIR "${ODBC_LIBRARY_DIR}" CACHE PATH "Directory containing ODBC library")

set(SQL_HEADERS sql.h sqlext.h sqltypes.h)

if(WIN32)

  set(ODBC_WIN_ARCH "${ODBC_WIN_ARCH}" CACHE PATH "ODBC Target Architecture")
  if(NOT ODBC_WIN_ARCH)
    set(ODBC_WIN_ARCH "x86")
  endif()
  
  set(PROGENV "ProgramFiles(x86)")
  set(PROG32 $ENV{${PROGENV}})
  if(NOT PROG32)
    set(PROG32 $ENV{PROGRAMFILES})
  endif(NOT PROG32)
  find_path(ODBC_INCLUDE_DIRECTORIES 
    NAMES ${SQL_HEADERS}
    HINTS ${ODBC_INCLUDE_DIR}
    "${PROG32}/Windows Kits/10/Include/10.0.16299.0/um"
    "${PROG32}/Windows Kits/10/Include/10.0.15063.0/um"
    "${PROG32}/Windows Kits/8.1/Include/um"
    "${PROG32}/Microsoft SDKs/Windows/v7.0/include"
    "${PROG32}/Microsoft SDKs/Windows/v6.0a/include"
    "C:/Program Files/ODBC/include"
    "C:/ODBC/include"
    DOC "Specify the directory containing sql.h."
  )
  
  find_library(ODBC_LIBRARY
    NAMES odbc odbc32
	HINTS ${ODBC_LIBRARY_DIR}
    "${PROG32}/Windows Kits/10/Lib/10.0.16299.0/um/${ODBC_WIN_ARCH}"
	"${PROG32}/Windows Kits/10/Lib/10.0.15063.0/um/${ODBC_WIN_ARCH}"
	"${PROG32}/Windows Kits/8.1/Lib/winv6.3/um/${ODBC_WIN_ARCH}"
	"${PROG32}/Microsoft SDKs/Windows/v7.0/Lib"
	"${PROG32}/Microsoft SDKs/Windows/v6.0a/Lib"
    "C:/Program Files/ODBC/lib"
    "C:/ODBC/lib/debug"
    DOC "Specify the ODBC driver manager library here."
  )
else()
  find_path(ODBC_INCLUDE_DIRECTORIES
    NAMES ${SQL_HEADERS}
    HINTS ${ODBC_INCLUDE_DIR}
    /usr/include
    /usr/include/odbc
    /usr/local/include
    /usr/local/include/odbc
    /usr/local/odbc/include
  )
  
  find_library(ODBC_LIBRARY
    NAMES iodbc unixodbc
	HINTS ${ODBC_LIBRARY_DIR}
    /usr/lib
    /usr/lib/odbc
    /usr/local/lib
    /usr/local/lib/odbc
    /usr/local/odbc/lib
    DOC "Specify the ODBC driver manager library here."
  )
endif(WIN32)

if(ODBC_LIBRARY AND ODBC_INCLUDE_DIRECTORIES)
  set(ODBC_LIBRARIES ${ODBC_LIBRARY})
endif(ODBC_LIBRARY AND ODBC_INCLUDE_DIRECTORIES)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODBC
    REQUIRED_VARS ODBC_INCLUDE_DIRECTORIES ODBC_LIBRARIES
    FAIL_MESSAGE "Could not find ODBC directories"
)

