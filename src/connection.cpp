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
#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/exception.h>
#include <sqlpp11/functions.h>
#include <sqlpp11/odbc/connection.h>
#include "detail/prepared_statement_handle.h"
#include "detail/connection_handle.h"

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <Windows.h>
#endif

#include <sqlext.h>
#include <sqltypes.h>

namespace sqlpp {
	namespace odbc {
		namespace {
			void execute_statement(SQLHSTMT stmt) {
				if(!SQL_SUCCEEDED(SQLExecute(stmt))) {
					throw sqlpp::exception("ODBC error: couldn't SQLExecute: "+detail::odbc_error(stmt, SQL_HANDLE_STMT));
				}
			}
			size_t odbc_affected(SQLHSTMT stmt) {
				SQLLEN ret;
				if(!SQL_SUCCEEDED(SQLRowCount(stmt, &ret))) {
					throw sqlpp::exception("ODBC error: couldn't SQLRowCount: "+detail::odbc_error(stmt, SQL_HANDLE_STMT));
				}
				return ret;
			}
			
			void set_autocommit(SQLHDBC dbc) {
				if(!SQL_SUCCEEDED(SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, SQLPOINTER(SQL_TRUE), 0))) {
					throw sqlpp::exception("ODBC error: Could not set AUTOCOMMIT to TRUE ("+detail::odbc_error(dbc, SQL_HANDLE_DBC)+')');
				}
			}
		}
		std::shared_ptr<detail::prepared_statement_handle_t> prepare_statement(detail::connection_handle_t& handle, const std::string& statement) {
			if(handle.debug) {
				std::cerr << "ODBC debug: Preparing: " << statement << std::endl;
			}
			SQLHSTMT stmt;
			SQLAllocHandle(SQL_HANDLE_STMT, handle.dbc, &stmt);
			std::shared_ptr<detail::prepared_statement_handle_t> ret = std::make_shared<detail::prepared_statement_handle_t>(stmt, handle.debug);
			if(SQL_SUCCEEDED(SQLPrepare(stmt, (SQLCHAR*)statement.c_str(), statement.length()))){
				return ret;
			} else {
				throw sqlpp::exception("ODBC error: couldn't SQLPrepare " + statement + ": "+detail::odbc_error(stmt, SQL_HANDLE_STMT));
			}
		}

		connection::connection(const connection_config& config)
			: _handle(new detail::connection_handle_t(config.debug, config.type))
		{
				if(_handle->debug) {
					std::cerr << "ODBC debug: connecting to DSN: " << config.data_source_name << std::endl;
				}
				if(!SQL_SUCCEEDED(SQLConnect(_handle->dbc,
											 (SQLCHAR*)config.data_source_name.c_str(), config.data_source_name.length(),
											 config.username.empty() ? nullptr : (SQLCHAR*)config.username.c_str(), config.username.length(),
											 config.password.empty() ? nullptr : (SQLCHAR*)config.password.c_str(), config.password.length()))) {
					std::string err = detail::odbc_error(_handle->dbc, SQL_HANDLE_DBC);
					//Free and nullify so we don't try to disconnect
					if(_handle->dbc) {
						auto dbc = _handle->dbc;
						_handle->dbc = nullptr;
						SQLFreeHandle(SQL_HANDLE_DBC, dbc);
					}
					throw sqlpp::exception("ODBC error: couldn't SQLConnect("+config.data_source_name+"): "+err);
				}
		}

		inline SQLUSMALLINT from_completion(const driver_completion c)
		{
			//Just in case the values are different on your system
			//Otherwise we could just cast. Optimizer might do that anyway.
			switch(c) {
				case driver_completion::prompt:
					return SQL_DRIVER_PROMPT;
				case driver_completion::complete:
					return SQL_DRIVER_COMPLETE;
				case driver_completion::complete_required:
					return SQL_DRIVER_COMPLETE_REQUIRED;
				case driver_completion::no_prompt:
					return SQL_DRIVER_NOPROMPT;
				default:
					return static_cast<SQLUSMALLINT>(c);
			}
		}

		connection::connection(const driver_connection_config& config, std::string& out_connection, size_t out_max)
			: _handle(new detail::connection_handle_t(config.debug, config.type))
		{
				if(_handle->debug) {
					std::cerr << "ODBC debug: connecting to " << config.connection << std::endl;
				}
				out_connection.resize(out_max, '\0');
				SQLSMALLINT out_size = std::min<size_t>(out_max, std::numeric_limits<SQLSMALLINT>::max());
				const bool success = SQL_SUCCEEDED(SQLDriverConnect(_handle->dbc,
											 (SQLHWND)config.window,
											 (SQLCHAR*)config.connection.c_str(), config.connection.length(),
											 (SQLCHAR*)out_connection.c_str(), out_size,
											 &out_size, from_completion(config.completion)));
				out_connection.resize(out_size, '\0');
				if(!success) {
					std::string err = detail::odbc_error(_handle->dbc, SQL_HANDLE_DBC);
					//Free and nullify so we don't try to disconnect
					if(_handle->dbc) {
						auto dbc = _handle->dbc;
						_handle->dbc = nullptr;
						SQLFreeHandle(SQL_HANDLE_DBC, dbc);
					}
					throw sqlpp::exception("ODBC error: couldn't SQLDriverConnect("+config.connection+"): "+err);
				}
		}
		
		connection::~connection() {}
		
		bind_result_t connection::select_impl(const std::string& statement) {
			auto prepared = prepare_statement(*_handle, statement);
			if(!prepared || !*prepared) {
				throw sqlpp::exception("ODBC error: Could not store result set");
			}
			execute_statement(prepared->stmt);
			return bind_result_t(prepared);
		}
		
		bind_result_t connection::run_prepared_select_impl(prepared_statement_t& prepared_statement) {
			return {prepared_statement._handle};
		}
		namespace last_insert_id_ {
			SQLPP_ALIAS_PROVIDER(id);
		}
		size_t connection::last_insert_id(){
			std::string statement;
			switch(_handle->type){
				case ODBC_Type::MySQL:
					statement = "SELECT LAST_INSERT_ID()"; break;
				case ODBC_Type::TSQL:
					statement = "SELECT SCOPE_IDENTITY"; break;
				case ODBC_Type::SQLite3:
					statement = "SELECT last_insert_rowid()"; break;
				case ODBC_Type::PostgreSQL:
					statement = "SELECT LASTVAL()"; break;
				default:
					throw sqlpp::exception("Can't get last insert id for ODBC_Type "+std::to_string(static_cast<int>(_handle->type)));
			}
			auto prepared_statement = prepare_statement(*_handle, statement);
			execute_statement(prepared_statement->stmt);
			int64_t ret;
			bool is_null;
			bind_result_t result(prepared_statement);
			if(!result.next_impl()){
				std::cerr << "ODBC warning: next_impl failed!" << std::endl;
				return 0;
			}
			result._bind_integral_result(0, &ret, &is_null);
			if(is_null) {
				std::cerr << "ODBC warning: NULL returned from " << statement << std::endl;
				return 0;
			}
			return ret;
		}
		
		
		size_t connection::insert_impl(const std::string& statement) {
			auto prepared = prepare_statement(*_handle, statement);
			execute_statement(prepared->stmt);
			
			return last_insert_id();
		}
		
		prepared_statement_t connection::prepare_impl(const std::string& statement) {
			return prepared_statement_t(prepare_statement(*_handle, statement));
		}
		
		size_t connection::run_prepared_insert_impl(prepared_statement_t& prepared_statement) {
			execute_statement(prepared_statement.native_handle());
			return last_insert_id();
		}
		
		size_t connection::run_prepared_execute_impl(prepared_statement_t& prepared_statement) {
			execute_statement(prepared_statement.native_handle());
			return odbc_affected(prepared_statement.native_handle());
		}
		
		size_t connection::execute(const std::string& statement) {
			return _handle->exec_direct(statement);
		}
		
		size_t connection::update_impl(const std::string& statement) {
			auto prepared = prepare_statement(*_handle, statement);
			execute_statement(prepared->stmt);
			return odbc_affected(prepared->stmt);
		}
		
		size_t connection::run_prepared_update_impl(prepared_statement_t& prepared_statement) {
			execute_statement(prepared_statement.native_handle());
			return odbc_affected(prepared_statement.native_handle());
		}
		
		size_t connection::remove_impl(const std::string& statement) {
			auto prepared = prepare_statement(*_handle, statement);
			execute_statement(prepared->stmt);
			return odbc_affected(prepared->stmt);
		}
		
		size_t connection::run_prepared_remove_impl(prepared_statement_t& prepared_statement) {
			execute_statement(prepared_statement.native_handle());
			return odbc_affected(prepared_statement.native_handle());
		}
		
		std::string connection::escape(const std::string& s) const {
			std::string t;
			size_t count(s.size());
			for(auto c : s){
				if(c == '\''){
					count++;
				}
			}
			t.reserve(count);
			for(auto c : s){
				if(c == '\''){
					t.push_back(c);
				}
				t.push_back(c);
			}
			return t;
		}
		
		void connection::start_transaction() {
			if(_transaction_active) {
				throw sqlpp::exception("ODBC error: Cannot have more than one open transaction per connection");
			}
			if(_handle->debug) {
				std::cerr << "ODBC debug: Beginning Transaction\n";
			}
			if(!SQL_SUCCEEDED(SQLSetConnectAttr(_handle->dbc, SQL_ATTR_AUTOCOMMIT, SQLPOINTER(SQL_FALSE), 0))) {
				throw sqlpp::exception("ODBC error: Could not set AUTOCOMMIT to FALSE ("+detail::odbc_error(_handle->dbc, SQL_HANDLE_DBC)+')');
			}
			_transaction_active = true;
		}
		
		void connection::commit_transaction() {
			if(not _transaction_active) {
				throw sqlpp::exception("ODBC error: Cannot commit a finished or failed transaction");
			}
			if(_handle->debug) {
				std::cerr << "ODBC debug: Committing Transaction\n";
			}
			if(!SQL_SUCCEEDED(SQLEndTran(SQL_HANDLE_DBC, _handle->dbc, SQL_COMMIT))) {
				throw sqlpp::exception("ODBC error: Could not SQLEndTran COMMIT("+detail::odbc_error(_handle->dbc, SQL_HANDLE_DBC)+')');
			}
			set_autocommit(_handle->dbc);
			_transaction_active = false;
		}
		
		void connection::rollback_transaction(bool report) {
			if(not _transaction_active) {
				throw sqlpp::exception("ODBC error: Cannot rollback a finished or failed transaction");
			}
			if(report || _handle->debug) {
				std::cerr << "ODBC warning: Rolling back unfinished transaction" << std::endl;
			}
			if(!SQL_SUCCEEDED(SQLEndTran(SQL_HANDLE_DBC, _handle->dbc, SQL_ROLLBACK))) {
				throw sqlpp::exception("ODBC error: Could not SQLEndTran ROLLBACK("+detail::odbc_error(_handle->dbc, SQL_HANDLE_DBC)+')');
			}
			set_autocommit(_handle->dbc);
			_transaction_active = false;
		}
		
		void connection::report_rollback_failure(const std::string message) noexcept {
			std::cerr << "ODBC message: " << message << std::endl;
		}
	}
}
