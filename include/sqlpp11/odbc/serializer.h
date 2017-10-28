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

#ifndef SQLPP11_ODBC_SERIALIZER_H
#define SQLPP11_ODBC_SERIALIZER_H

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <Windows.h>
#endif

#include <sql.h>
#include <sqlpp11/data_types/text/concat.h>
#include <sqlpp11/insert_value_list.h>
#include <sqlpp11/connection.h>

namespace sqlpp
{
	namespace odbc {
		struct serializer_t;
	}
	#if ODBCVER < 0x0300
	struct assert_no_with_t
	{
		using type = std::false_type;
		
		template <typename T = void>
		static void _()
		{
			static_assert(wrong_t<T>::value, "ODBC versions prior to 3.0 unsupported");
		}
	};
	#endif
	namespace odbc {
		struct serializer_t {
			serializer_t(const connection& db) : _db(db), _count(1) {}
			
			template <typename T>
			std::ostream& operator<<(T t) {
				return _os << t;
			}
			
			std::string escape(std::string arg);
			
			std::string str() const {
				return _os.str();
			}
			
			size_t count() const {
				return _count;
			}
			
			void pop_count() {
				++_count;
			}
			
			const connection& _db;
			std::stringstream _os;
			size_t _count;
		};
	}
	template <typename First, typename... Args>
	struct serializer_t<odbc::serializer_t, concat_t<First, Args...>>
	{
		using _serialize_check = consistent_t;
		using T = concat_t<First, Args...>;
		
		static odbc::serializer_t& _(const T& t, odbc::serializer_t& context)
		{
			context << "{fn CONCAT(";
			interpret_tuple(t._args, ',', context);
			context << ")}";
			return context;
		}
	};
	template <>
	struct serializer_t<odbc::serializer_t, insert_default_values_data_t>
	{
		using _serialize_check = consistent_t;
		using T = insert_default_values_data_t;
		
		static odbc::serializer_t& _(const T& t, odbc::serializer_t& context)
		{
			context << " () VALUES()";
			return context;
		}
	};
}

#endif //SQLPP11_ODBC_SERIALIZER_H