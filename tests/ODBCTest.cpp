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
#include <date.h>

#include <iostream>
#include <vector>

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
	odbc::connection_config config;
 	config.data_source_name = argv[1];
	config.database = argv[2];
	config.username = argv[3];
	config.password = argv[4];
	std::map<std::string,odbc::connection_config::ODBC_Type> odbc_types({
		{"MySQL",odbc::connection_config::ODBC_Type::MySQL},
		{"PostgreSQL",odbc::connection_config::ODBC_Type::PostgreSQL},
		{"SQLite3",odbc::connection_config::ODBC_Type::SQLite3},
		{"TSQL",odbc::connection_config::ODBC_Type::TSQL}
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
	config.type = mType->second;
	config.debug = true;
	std::unique_ptr<odbc::connection> db;
	try
	{
		db.reset(new odbc::connection(config));
	}
	catch(const sqlpp::exception& )
	{
		std::cerr << "For testing, you'll need to create a database sqlpp_sample with a table tab_sample, as shown in tests/TabSample.sql" << std::endl;
		throw;
	}
	db->execute(R"(DROP TABLE IF EXISTS tab_sample)");
	db->execute(R"(DROP TABLE IF EXISTS tab_foo)");
	db->execute(R"(DROP TABLE IF EXISTS tab_bar)");
	db->execute(R"(CREATE TABLE tab_foo (
		omega bigint(20) AUTO_INCREMENT,
		PRIMARY KEY (omega)
	))");
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
	size_t omega = db->insert(insert_into(foo).default_values());
	
	std::cout << "Inserted " << omega << std::endl;
	(*db)(insert_into(tab).set(tab.alpha = (int64_t)omega, tab.gamma = true, tab.beta = "cheesecake"));
	(*db)(insert_into(tab).set(tab.gamma = false, tab.beta = "blueberry muffin"));

	for(const auto& row : (*db)(select(all_of(tab)).from(tab).unconditionally()))
	{
		printResultsSample(row);
	};
	auto date_time = std::chrono::system_clock::time_point() 
		+ std::chrono::microseconds(3723123456);
	auto dp = date::floor<date::days>(date_time);
	auto date = date::year_month_day{dp};
	auto time = date::make_time(date_time-dp);
	std::cout << "Using today(" << date << ' ' << time << ')' << std::endl;
	
	(*db)(insert_into(bar).set(
		bar.delta = dp,
		bar.epsilon = date_time,
		bar.zeta = date_time,
		bar.eta = date_time));
	for(const auto& row : (*db)(select(all_of(bar)).from(bar).unconditionally()))
	{
		printResultsBar(row);
	}

	return 0;
}
