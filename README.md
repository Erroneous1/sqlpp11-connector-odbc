sqlpp11-connector-odbc
======================
ODBC connector for sqlpp11

Sample Code:
------------
```C++
namespace odbc = sqlpp::odbc;
int main() {
 	odbc::connection_config config;
 	//DSN between [ ] in odbc.ini for unixodbc
 	config.data_source_name = "MyDSN";
 	//Database to use. Leave empty to execute your own USE statement
	config.database = "my_schema";
	//Username for data source
	config.username = "user";
	//Authentication for data source
	config.password = "password";
	
	//Valid types currently include MySQL, PostgreSQL, SQLite3, or TSQL
	//This is used to get the last insert ID, used by insert function
	config.type = odbc::connection_config::ODBC_Type::MySQL;
	config.debug = true;
	odbc::connection db(config);
	...
}
```

Requirements:
-------------
__Compiler:__
sqlpp11-connector-odbc makes use of C++11 and requires a recent compiler and STL. The following compilers are known to compile the test programs:
  * g++-7.2.0 on Arch Linux as of 2017-10-25
  * Visual Studio on Windows 7 using LLVM toolset as of 2017-10-29

__C++ SQL Layer:__
sqlpp11-connector-odbc depends on sqlpp11 (https://github.com/rbock/sqlpp11).

__ODBC:__
Appropriate ODBC library (like unixodbc or Windows's ODBC library), and ODBC connector capable of ODBC 3.0 or higher. Tested with unixodbc 2.3.4.

__Threading:__
Using the same `sqlpp::odbc::connection` object on multiple threads is not safe. Instead, pass the `sqlpp::odbc::connection_config` to create a new connection (which may be safe depending on your ODBC connector) or use mutexes.

__Wide Strings:__
While sqlpp11-connector-odbc should compile on Windows (possibly with some modifications), sqlpp11 uses std::string and std::ostream.

__Windows:__
To build on Windows for 64 bit, first build and install sqlpp11:
```sh
mkdir sqlpp11/build
cd sqlpp11/build
cmake .. -G"Visual Studio 15 2017 Win64" -T"LLVM-vs2014" -DHinnantDate_ROOT_DIR="C:/Users/myuser/source/repos/date/include"
# Use VS 2017 to build INSTALL project
# As admin, run the Post Build script for INSTALL project
```
Then you can build sqlpp11-connector-odbc the same way.