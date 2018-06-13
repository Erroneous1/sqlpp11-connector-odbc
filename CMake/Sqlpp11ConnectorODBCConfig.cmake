list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

include(CMakeFindDependencyMacro)
find_dependency(Sqlpp11 REQUIRED)
find_dependency(ODBC REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/Sqlpp11ConnectorODBCTargets.cmake")