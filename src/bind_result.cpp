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
#include <vector>
#include <sqlpp11/exception.h>
#include <sqlpp11/odbc/bind_result.h>
#include <sql.h>
#include <sqlext.h>
#include "detail/prepared_statement_handle.h"
#include "detail/connection_handle.h"
#include <date.h>
#include <cassert>

namespace sqlpp {
	namespace odbc {
		bind_result_t::bind_result_t(const std::shared_ptr<detail::prepared_statement_handle_t>& handle) : _handle(handle){
			if(_handle and _handle->debug){
				std::cerr << "ODBC debug: Constructing bind result, using handle at " << _handle.get() << std::endl;
			}
		}
		
		void bind_result_t::_bind_boolean_result(size_t index, signed char* value, bool* is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding boolean result " << *value << " at index " << index << std::endl;
			}
			SQLLEN ind(0);
			if(!SQL_SUCCEEDED(SQLGetData(_handle->stmt, index+1, SQL_C_BIT, value, sizeof(signed char), &ind))) {
				throw sqlpp::exception("ODBC error: couldn't SQLGetData("+std::to_string(index+1)+",SQL_C_BIT): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
			if(is_null) {
				*is_null = (ind == SQL_NULL_DATA);
			}
		}
		
		void bind_result_t::_bind_floating_point_result(size_t index, double* value, bool* is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding floating_point result " << *value << " at index " << index << std::endl;
			}
			SQLLEN ind(0);
			if(!SQL_SUCCEEDED(SQLGetData(_handle->stmt, index+1, SQL_C_DOUBLE, value, sizeof(double), &ind))) {
				throw sqlpp::exception("ODBC error: couldn't SQLGetData("+std::to_string(index+1)+",SQL_C_DOUBLE): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
			if(is_null) {
				*is_null = (ind == SQL_NULL_DATA);
			}
		}
		
		void bind_result_t::_bind_integral_result(size_t index, int64_t* value, bool* is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding integral result " << *value << " at index " << index << std::endl;
			}
			SQLLEN ind(0);
			if(!SQL_SUCCEEDED(SQLGetData(_handle->stmt, index+1, SQL_C_SBIGINT, value, sizeof(int64_t), &ind))) {
				throw sqlpp::exception("ODBC error: couldn't SQLGetData("+std::to_string(index+1)+",SQL_C_SBIGINT): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
			if(is_null) {
				*is_null = (ind == SQL_NULL_DATA);
			}
		}
		
		void bind_result_t::_bind_text_result(size_t index, const char** value, size_t* len) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding text result at index " << index << std::endl;
			}
			assert(len);
			SQLLEN ind(0);
			*value = new char[256];
			
			if(!SQL_SUCCEEDED(SQLGetData(_handle->stmt, index+1, SQL_C_CHAR,(SQLPOINTER) *value, 256, &ind))) {
				throw sqlpp::exception("ODBC error: couldn't SQLGetData("+std::to_string(index+1)+",SQL_C_CHAR): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
			if(ind == SQL_NULL_DATA || ind == SQL_NO_TOTAL || ind < 0) {
				*len = 0;
			} else {
				*len = ind;
			}
			if(ind > 256) {
				delete[] *value;
				*value = new char[ind];
				if(!SQL_SUCCEEDED(SQLGetData(_handle->stmt, index+1, SQL_C_CHAR,(SQLPOINTER) *value, ind, &ind))) {
					throw sqlpp::exception("ODBC error: couldn't SQLGetData("+std::to_string(index+1)+",SQL_C_CHAR): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
				}
			}
		}
		
		void bind_result_t::_bind_date_result(size_t index, ::sqlpp::chrono::day_point* value, bool* is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding date result at index " << index << std::endl;
			}
			SQL_DATE_STRUCT date_struct = {0};
			SQLLEN ind(0);
			if(!SQL_SUCCEEDED(SQLGetData(_handle->stmt, index+1, SQL_C_TYPE_DATE, &date_struct, sizeof(SQL_DATE_STRUCT), &ind))) {
				throw sqlpp::exception("ODBC error: couldn't SQLGetData("+std::to_string(index+1)+",SQL_C_TYPE_DATE): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
			*is_null = (ind == SQL_NULL_DATA);
			if(!*is_null) {
				*value = ::date::day_point( ::date::year(date_struct.year) / date_struct.month / date_struct.day );
			}
		}
		
		void bind_result_t::_bind_date_time_result(size_t index, ::sqlpp::chrono::microsecond_point* value, bool* is_null) {
			if(_handle->debug) {
				std::cerr << "ODBC debug: binding date_time result at index " << index << std::endl;
			}
			
			SQL_TIMESTAMP_STRUCT timestamp_struct = {0};
			SQLLEN ind(0);
			if(!SQL_SUCCEEDED(SQLGetData(_handle->stmt, index+1, SQL_C_TYPE_TIMESTAMP, &timestamp_struct, sizeof(SQL_TIMESTAMP_STRUCT), &ind))) {
				throw sqlpp::exception("ODBC error: couldn't SQLGetData("+std::to_string(index+1)+",SQL_C_TYPE_TIMESTAMP): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
			*is_null = (ind == SQL_NULL_DATA);
			if(!*is_null) {
				*value = ::date::day_point(::date::year(timestamp_struct.year) / timestamp_struct.month / timestamp_struct.day);
				*value += 
					std::chrono::hours(timestamp_struct.hour) +
					std::chrono::minutes(timestamp_struct.minute) +
					std::chrono::seconds(timestamp_struct.second) +
					std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::nanoseconds(timestamp_struct.fraction));
			}
		}
		
		bool bind_result_t::next_impl() {
			if(_handle->debug) {
				std::cerr << "ODBC debug: accessing next row handle at " << _handle.get() << std::endl;
			}
			SQLRETURN rc;
			while((rc = SQLFetch(_handle->stmt)) == SQL_STILL_EXECUTING){}
			switch(rc) {
				case SQL_NO_DATA:
					return false;
				case SQL_SUCCESS_WITH_INFO:
					std::cerr << "ODBC warning: SQLFetch returned info "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT) << std::endl;
				case SQL_SUCCESS:
					return true;
				case SQL_ERROR:
					throw sqlpp::exception("ODBC error: couldn't SQLFetch(returned SQL_ERROR): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
				case SQL_INVALID_HANDLE:
					throw sqlpp::exception("ODBC error: couldn't SQLFetch(returned SQL_INVALID_HANDLE): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
				default:
					throw sqlpp::exception("ODBC error: couldn't SQLFetch(returned "+std::to_string(rc)+"): "+detail::odbc_error(_handle->stmt, SQL_HANDLE_STMT));
			}
		}
	}
}