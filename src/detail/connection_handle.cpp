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

#include "connection_handle.h"
#include <sqlpp11/odbc/prepared_statement.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sqlpp11/exception.h>
#include <vector>
#include <memory>

namespace sqlpp {
	namespace odbc {
		namespace detail {
			std::string return_code_string(SQLRETURN rc) {
				switch(rc) {
					case 0: return "SQL_SUCCESS";
					case 1: return "SQL_SUCCESS_WITH_INFO";
					case 2: return "SQL_STILL_EXECUTING";
					case -1: return "SQL_NULL_DATA/SQL_ERROR";
					case -2: return "SQL_DATA_AT_EXEC/SQL_INVALID_HANDLE";
					case 99: return "SQL_NEED_DATA";
					case 100: return "SQL_NO_DATA";
					case 101: return "SQL_PARAM_DATA_AVAILABLE";
					default: return "Unknown return code "+std::to_string(rc);
				}
			}

			std::string odbc_error(SQLHANDLE handle, SQLSMALLINT handle_type, SQLRETURN return_code){
				return "Returned "+return_code_string(return_code)+' '+odbc_error(handle, handle_type);
			}
			
			std::string odbc_error(SQLHANDLE handle, SQLSMALLINT handle_type){
				std::vector<std::string> errors;
				errors.reserve(1);
				std::unique_ptr<SQLCHAR[]> buffer(new SQLCHAR[1024]);
				std::vector<SQLCHAR> state(6, '\0');
				SQLINTEGER native_error;
				SQLSMALLINT buffer_len;
				bool get_more(true);
				SQLRETURN rc;
				for(SQLSMALLINT rec_number(1); get_more; rec_number++){
					rc = SQLGetDiagRec(handle_type, handle, rec_number++, state.data(), &native_error, buffer.get(), 1024, &buffer_len);
					errors.push_back("ODBC error STATE: "+std::string((const char*)state.data())+", Native Error: "+std::to_string(native_error));
					switch(rc){
						case SQL_SUCCESS:
						case SQL_SUCCESS_WITH_INFO:
							errors.push_back("Error: "+std::string((const char*)buffer.get(), static_cast<size_t>(buffer_len)));
						case SQL_NO_DATA:
							get_more = false; break;
						case SQL_INVALID_HANDLE:
							std::cerr << "ODBC error: SQLGetDiagRec returned SQL_INVALID_HANDLE (" << handle << ", " << handle_type << ')' << std::endl;
							get_more = false; break;
						case SQL_ERROR:
							std::cerr << "ODBC error: SQLGetDiagRec returned SQL_ERROR" << std::endl;
							get_more = false; break;
						default:
							std::cerr << "ODBC error: SQLGetDiagRec returned " << rc << std::endl;
							get_more = false; break;
					}
				}
				std::string ret;
				size_t ret_size(0);
				for(const auto& s : errors){
					ret_size += s.length()+1;
				}
				ret.reserve(ret_size);
				if(ret_size != 0){
					ret.append(errors.front());
				}else{
					return "Could not retrieve diagnostic information!";
				}
				for(auto itr = errors.begin()+1; itr != errors.end(); itr++){
					ret += '\n';
					ret += *itr;
				}
				return ret;
			}
			
			connection_handle_t::connection_handle_t(bool d, ODBC_Type t)
				: env(nullptr)
				, dbc(nullptr)
				, debug(d)
				, type(t)
			{
				if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env)) || env == nullptr) {
					throw sqlpp::exception("ODBC error: couldn't SQLAllocHandle(SQL_HANDLE_ENV)");
				}else if(!SQL_SUCCEEDED(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))) {
					throw sqlpp::exception("ODBC error: couldn't SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION, SQL_0V_ODBC3): "+odbc_error(env, SQL_HANDLE_ENV));
				}else if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc)) || dbc == nullptr) {
					throw sqlpp::exception("ODBC error: couldn't SQLAllocHandle(SQL_HANDLE_DBC): "+odbc_error(env, SQL_HANDLE_ENV));
				}
			}
			
			sqlpp::odbc::detail::connection_handle_t::~connection_handle_t() {
				if(dbc) {
					SQLDisconnect(dbc);
					SQLFreeHandle(SQL_HANDLE_DBC, dbc);
				}
				if(env) {
					SQLFreeHandle(SQL_HANDLE_ENV, env);
				}
			}

			size_t connection_handle_t::exec_direct(const std::string& statement) {
				SQLHSTMT stmt;
				if(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt))) {
					throw sqlpp::exception("ODBC error: could SQLAllocHandle(SQL_HANDLE_STMT): "+odbc_error(stmt, SQL_HANDLE_STMT));
				}
				auto rc = SQLExecDirect(stmt, (SQLCHAR*)statement.c_str(), statement.length());
				std::string err;
				SQLLEN ret = 0;
				if(!(SQL_SUCCEEDED(rc) || rc == SQL_NO_DATA)){
					err = "ODBC error: couldn't SQLExecDirect("+statement+"): "+odbc_error(stmt, SQL_HANDLE_STMT, rc);
				} else {
					rc = SQLRowCount(stmt, &ret);
					if(!SQL_SUCCEEDED(rc)) {
						err = "ODBC error: couldn't SQLRowCount: "+odbc_error(stmt, SQL_HANDLE_STMT);
					}
				}
				if(!SQL_SUCCEEDED(SQLFreeHandle(SQL_HANDLE_STMT, stmt))) {
					throw sqlpp::exception("ODBC error: couldn't SQLFreeHandle(HSTMT): "+odbc_error(dbc, SQL_HANDLE_DBC));
				}
				stmt = nullptr;
				if(!SQL_SUCCEEDED(rc)) {
					throw sqlpp::exception(err);
				}
				return ret;
			}

		}
	}
}
