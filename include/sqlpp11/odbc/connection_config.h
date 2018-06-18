/* 
 * Original work Copyright (c) 2013-2015, Roland Bock
 * Modified work Copyright (c) 2016, Aaron Bishop
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SQLPP11_ODBC_CONNECTION_CONFIG_H
#define SQLPP11_ODBC_CONNECTION_CONFIG_H

#include <string>
#include <iostream>

namespace sqlpp {
	namespace odbc {
		enum class ODBC_Type
		{
			MySQL,
			PostgreSQL,
			SQLite3,
			TSQL
		};

		enum class driver_completion : unsigned short
		{
			no_prompt = 0,
			complete = 1,
			prompt = 2,
			complete_required = 3
		};

		struct connection_config {
			connection_config() : data_source_name(), username(), password(), type(ODBC_Type::TSQL), debug(false) {}
			connection_config(const connection_config&) = default;
			connection_config(connection_config&&) = default;
			
			connection_config(std::string dsn, ODBC_Type t=ODBC_Type::TSQL, std::string vf = {}, bool dbg = false)
			: data_source_name(std::forward<std::string>(dsn)), username(), password(), type(t), debug(dbg) {}
			
			std::string data_source_name;
			std::string username;
			std::string password;
			ODBC_Type type;
			bool debug;
		};

		inline bool operator==(const connection_config& a, const connection_config& b)
		{
			return
				a.data_source_name == b.data_source_name and
				a.username == b.username and
				a.password == b.password and
				a.type == b.type and
				a.debug == b.debug;
		}

		inline bool operator!=(const connection_config& a, const connection_config& b)
		{
			return !(a == b);
		}

		struct driver_connection_config {
			driver_connection_config() : window{nullptr}, connection(), completion(driver_completion::no_prompt), type(ODBC_Type::TSQL), debug(false) {}
			driver_connection_config(const driver_connection_config& ) = default;
			driver_connection_config(driver_connection_config&& ) = default;

			void* window;
			std::string connection;
			driver_completion completion;
 			ODBC_Type type;
 			bool debug;
 		};

		inline bool operator==(const driver_connection_config& a, const driver_connection_config& b)
		{
			return
				a.connection == b.connection and
				a.window == b.window and
				a.completion == b.completion and
				a.type == b.type and
				a.debug == b.debug;
		}

		inline bool operator!=(const driver_connection_config& a, const driver_connection_config& b)
		{
			return !(a == b);
		}
	}
}

#endif //SQLPP11_ODBC_CONNECTION_CONFIG_H
