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

#include <iostream>
#include <sstream>
#include <string>
#include <date.h>
#include <sqlpp11/exception.h>
#include <sqlpp11/odbc/prepared_statement.h>
#include "detail/prepared_statement_handle.h"
#include "detail/connection_handle.h"
#include <sql.h>
#include <sqlext.h>

#if defined(__CYGWIN__)
#include <cstdio>

// Workaround because cygwin gcc does not define to_string
namespace std
{
	std::string to_string(int i)
	{
		std::unique_ptr<char[]> buffer(new char[12]);
		snprintf(buffer.get(), 12, "%i", i);
		return std::string(buffer.get());
	}
}
#endif

namespace sqlpp {
	namespace odbc {
		void check_bind_result(int result, const char* const type, SQLHSTMT* stmt, detail::prepared_statement_handle_t& handle) {
			switch(result) {
				case SQL_ERROR:
					throw sqlpp::exception("ODBC error: "+std::string(type)+" couldn't bind: "+detail::odbc_error(stmt, SQL_HANDLE_STMT));
				case SQL_INVALID_HANDLE:
					throw sqlpp::exception("ODBC error: "+std::string(type)+" couldn't bind to invalid handle");
				case SQL_SUCCESS:
				case SQL_SUCCESS_WITH_INFO:
					return;
				default:
					throw sqlpp::exception("ODBC error: "+std::string(type)+" unexpected return code: "+std::to_string(result));
			}
		}
		
		prepared_statement_t::prepared_statement_t(std::shared_ptr<detail::prepared_statement_handle_t>&& handle)
		: _handle(std::move(handle)) {
			if(_handle and _handle->debug) {
				std::cerr << "ODBC debug: Constructing prepared_statement, using handle at " << _handle.get() << std::endl;
			}
		}
		
		void prepared_statement_t::_reset() {
			if(_handle->debug) {
				std::cerr << "ODBC debug: resetting prepared_statement" << std::endl;
			}
			auto rc = SQLFreeStmt(_handle->stmt, SQL_RESET_PARAMS);
			if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO){
				throw sqlpp::exception("ODBC error: couldn't reset parameters on prepared_statement: "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
		}
		
		void prepared_statement_t::_bind_boolean_parameter(size_t index, const signed char* value, bool is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding boolean parameter " << (*value ? "true" : "false")
				<< " at index: " << index << ", being " << (is_null ? std::string() : "not") << " null" << std::endl;
			}
			SQLLEN indPtr(is_null ? SQL_NULL_DATA : 0);
			auto rc = SQLBindParameter(_handle->stmt, 
									   index, 
							  SQL_PARAM_INPUT, 
							  SQL_C_CHAR, 
							  SQL_BIT, 
							  1, 
							  0, 
							  (SQLPOINTER)value, 
									   sizeof(signed char), 
									   &indPtr);
			if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
				throw sqlpp::exception("ODBC error: couldn't bind boolean parameter: "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
		}
		
		void prepared_statement_t::_bind_floating_point_parameter(size_t index, const double* value, bool is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding floating_point parameter " << *value
				<< " at index: " << index << ", being " << (is_null ? std::string() : "not") << " null" << std::endl;
			}
			SQLLEN indPtr(is_null ? SQL_NULL_DATA : 0);
			auto rc = SQLBindParameter(_handle->stmt, 
									   index, 
									SQL_PARAM_INPUT, 
									SQL_C_DOUBLE, 
									SQL_DOUBLE, 
									15,
									__DBL_DIG__, 
									(SQLPOINTER)value, 
									   sizeof(double), 
									   &indPtr);
			if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
				throw sqlpp::exception("ODBC error: couldn't bind floating_point parameter: "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
		}
		
		void prepared_statement_t::_bind_integral_parameter(size_t index, const int64_t* value, bool is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding integral parameter " << *value
				<< " at index: " << index << ", being " << (is_null ? std::string() : "not") << " null" << std::endl;
			}
			SQLLEN indPtr(is_null ? SQL_NULL_DATA : 0);
			auto rc = SQLBindParameter(_handle->stmt, 
									   index, 
							  SQL_PARAM_INPUT,
							  SQL_C_SBIGINT,
							  SQL_BIGINT, 
							  20,
							  0, 
							  (SQLPOINTER)value, 
									   sizeof(int64_t), 
									   &indPtr);
			if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
				throw sqlpp::exception("ODBC error: couldn't bind integral parameter: "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
		}
		
		void prepared_statement_t::_bind_text_parameter(size_t index, const std::string* value, bool is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding text parameter " << *value
				<< " at index: " << index << ", being " << (is_null ? std::string() : "not") << " null" << std::endl;
			}
			SQLLEN indPtr(is_null ? SQL_NULL_DATA : 0);
			auto rc = SQLBindParameter(_handle->stmt, 
									   index, 
							  SQL_PARAM_INPUT, 
							  SQL_C_CHAR, 
							  SQL_CHAR,
							  value->length(),
									   0, 
							  (SQLPOINTER)value, 
									   value->size(), 
									   &indPtr);
			if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
				throw sqlpp::exception("ODBC error: couldn't bind text parameter: "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
		}
		
		void prepared_statement_t::_bind_date_parameter(size_t index, const ::sqlpp::chrono::day_point* value, bool is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding date parameter"
				" at index: " << index << ", being " << (is_null ? std::string() : "not") << " null" << std::endl;
			}
			SQLLEN indPtr(is_null ? SQL_NULL_DATA : 0);
			SQL_DATE_STRUCT ymd_value = {0};
			if(!is_null) {
				const auto ymd = ::date::year_month_day{*value};
				ymd_value.year = static_cast<int>(ymd.year());
				ymd_value.month = static_cast<unsigned>(ymd.month());
				ymd_value.day = static_cast<unsigned>(ymd.day());
			}
			
			auto rc = SQLBindParameter(_handle->stmt, 
									   index, 
							  SQL_PARAM_INPUT, 
							  SQL_C_TYPE_DATE, 
							  SQL_TYPE_DATE, 
							  10,
							  0, 
							  (SQLPOINTER)&ymd_value, 
									   sizeof(SQL_DATE_STRUCT), 
									   &indPtr);
			if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
				throw sqlpp::exception("ODBC error: couldn't bind date parameter: "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
		}
		
		void prepared_statement_t::_bind_date_time_parameter(size_t index, const ::sqlpp::chrono::microsecond_point* value, bool is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding date_time parameter"
				" at index: " << index << ", being " << (is_null ? std::string() : "not") << " null" << std::endl;
			}
			SQLLEN indPtr(is_null ? SQL_NULL_DATA : 0);
			SQL_TIMESTAMP_STRUCT ts_value = {0};
			if(!is_null) {
				const auto dp = ::date::floor<::date::days>(*value);
				const auto time = date::make_time(*value - dp);
				const auto ymd = ::date::year_month_day{dp};
				ts_value.year = static_cast<int>(ymd.year());
				ts_value.month = static_cast<unsigned>(ymd.month());
				ts_value.day = static_cast<unsigned>(ymd.day());
				ts_value.hour = time.hours().count();
				ts_value.minute = time.minutes().count();
				ts_value.second = time.seconds().count();
				ts_value.fraction = time.subseconds().count();
			}
			
			auto rc = SQLBindParameter(_handle->stmt, 
									   index, 
							  SQL_PARAM_INPUT, 
							  SQL_C_TYPE_DATE, 
							  SQL_TYPE_TIMESTAMP, 
							  10,
							  0, 
							  (SQLPOINTER)&ts_value, 
									   sizeof(SQL_DATE_STRUCT), 
									   &indPtr);
			if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) {
				throw sqlpp::exception("ODBC error: couldn't bind date parameter: "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
		}
		SQLHSTMT prepared_statement_t::native_handle() {
			return _handle->stmt;
		}
	}
}