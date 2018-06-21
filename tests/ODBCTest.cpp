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

#include "TabSample.h"
#include <sqlpp11/sqlpp11.h>
#include <sqlpp11/odbc/odbc.h>
#include <date/date.h>

#include <iostream>
#include <vector>

#include <cassert>

template<typename ResultRow>
void printResultsSample(const ResultRow& row) {
	std::cout << "alpha:\t";
	if(row.alpha.is_null()) 
		std::cout << "NULL";
	else
		std::cout << row.alpha;
	std::cout << ", beta:\t";
	if(row.beta.is_null())
		std::cout << "NULL";
	else
		std::cout << row.beta;
	std::cout << ", gamma:\t";
	if(row.gamma.is_null())
		std::cout << "NULL";
	else
		std::cout << row.gamma;
	std::cout << std::endl;
}

template<typename ResultRow>
void printResultsBar(const ResultRow& row) {
	std::cout << "delta:\t";
	if(row.delta.is_null())
		std::cout << "NULL";
	else
		std::cout << row.delta;
	std::cout << ", epsilon:\t";
	if(row.epsilon.is_null())
		std::cout << "NULL";
	else
		std::cout << row.epsilon;
	std::cout << ", zeta:\t";
	if(row.zeta.is_null())
		std::cout << "NULL";
	else
		std::cout << row.zeta;
	std::cout << ", eta:\t";
	if(row.eta.is_null())
		std::cout << "NULL";
	else
		std::cout << row.eta;
	std::cout << std::endl;
}

namespace odbc = sqlpp::odbc;
int main(int argc, const char **argv)
{
	if(argc != 6)
	{
		std::cout << "Usage: ODBCTest data_source_name database username password type" << std::endl;
		return 1;
	}
	try {
		odbc::connection_config config;
		config.data_source_name = argv[1];
		const std::string database = argv[2];
		config.username = argv[3];
		config.password = argv[4];
		std::map<std::string,std::pair<odbc::ODBC_Type,const char*>> odbc_types({
			{"MySQL",{odbc::ODBC_Type::MySQL,"BIGINT NOT NULL AUTO_INCREMENT"}},
			{"PostgreSQL",{odbc::ODBC_Type::PostgreSQL,"SERIAL"}},
			{"SQLite3",{odbc::ODBC_Type::SQLite3,"BIGINT NOT NULL AUTOINCREMENT"}},
			{"TSQL",{odbc::ODBC_Type::TSQL, "BIGINT NOT NULL AUTO_INCREMENT"}}
		});
		auto mType = odbc_types.find(argv[5]);
		if(mType == odbc_types.end())
		{
			std::cout << "Unknown type: " << argv[5] << ", valid types are:" << std::endl;
			for(auto t : odbc_types)
			{
				std::cout << '\t' << t.first << std::endl;
			}
		}
		if(mType->second.first == odbc::ODBC_Type::SQLite3) {
			std::cout << "SQLite3 does not allow for proper DATE and TIMESTAMP datatypes. Testing cancelled\n";
			return 1;
		}
		config.type = mType->second.first;
		std::string auto_increment_column = mType->second.second;
		config.debug = true;
		std::unique_ptr<odbc::connection> db;
		db.reset(new odbc::connection(config));
		if(!database.empty())
			db->execute("USE "+database);
		db->execute(R"(DROP TABLE IF EXISTS tab_sample)");
		db->execute(R"(DROP TABLE IF EXISTS tab_foo)");
		db->execute(R"(DROP TABLE IF EXISTS tab_bar)");
		db->execute("CREATE TABLE tab_foo (\n\
			omega "+auto_increment_column+",\n\
			name VARCHAR(32),\n\
			PRIMARY KEY (omega)\n\
		)");
		db->execute(R"(CREATE TABLE tab_sample (
				alpha bigint(20) DEFAULT NULL,
				beta varchar(255) DEFAULT NULL,
				gamma bool DEFAULT NULL,
				FOREIGN KEY (alpha) REFERENCES tab_foo(omega)
				))");
		db->execute(R"(CREATE TABLE tab_bar (
				delta date DEFAULT NULL,
				epsilon datetime DEFAULT NULL,
				zeta timestamp,
				eta time DEFAULT NULL
			))");

		TabFoo foo;
		TabSample tab;
		TabBar bar;
		// clear the table
		size_t omega = db->insert(insert_into(foo).set(foo.name = "test"));
		
		std::cout << "Inserted " << omega << std::endl;
		(*db)(insert_into(tab).set(tab.alpha = (int64_t)omega, tab.gamma = true, tab.beta = "cheesecake"));
		(*db)(insert_into(tab).set(tab.gamma = false, tab.beta = "blueberry muffin"));

		{
			auto result = (*db)(select(all_of(tab)).from(tab).unconditionally());
			auto size = result.size();
			std::cout << "Select returned " << size << " rows\n";
			assert(size == 2);
			for(const auto& row : result)
			{
				printResultsSample(row);
				--size;
			};
			assert(size == 0);
		}
		auto date_time = std::chrono::system_clock::time_point() 
			+ std::chrono::microseconds(3723123456);
		auto dp = date::floor<date::days>(date_time);
		auto date = date::year_month_day{dp};
		auto time = date::make_time(date_time-dp);
		std::cout << "Using today(" << date << ' ' << time << ')' << std::endl;
		
		db->start_transaction();
		
		(*db)(insert_into(bar).set(
			bar.delta = dp,
			bar.epsilon = date_time,
			bar.zeta = date_time,
			bar.eta = date_time));
		auto&& select_bar = (*db)(select(all_of(bar)).from(bar).unconditionally());
		assert(!select_bar.empty());
		for(const auto& row : select_bar)
		{
			printResultsBar(row);
		}
		
		db->rollback_transaction(true);
		
		select_bar = (*db)(select(all_of(bar)).from(bar).unconditionally());
		assert(select_bar.empty());
	} catch(const std::exception& e) {
		std::cerr << "Encountered error: " << e.what() << '\n';
		return 2;
	}
	return 0;
}
