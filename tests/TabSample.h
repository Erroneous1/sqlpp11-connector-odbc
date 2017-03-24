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
#ifndef TEST_SAMPLE_H
#define TEST_SAMPLE_H

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

// clang-format off

namespace TabFoo_ {
	struct Omega {
		struct _alias_t {
			static constexpr const char _literal[] = "omega";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T omega;
				T& operator()() { return omega; }
				const T& operator()() const { return omega; }
			};
		};
		using _traits = sqlpp::make_traits<sqlpp::bigint>;
	};
	struct Name {
		struct _alias_t {
			static constexpr const char _literal[] = "name";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T name;
				T& operator()() { return name; }
				const T& operator()() const { return name; }
			};
		};
		using _traits = sqlpp::make_traits<sqlpp::varchar>;
	};
}

struct TabFoo : sqlpp::table_t<TabFoo,TabFoo_::Omega,TabFoo_::Name> {
	struct _alias_t {
		static constexpr const char _literal[] =  "tab_foo";
		using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
		
		template<typename T>
		struct _member_t {
			T tab_foo;
			T& operator()() { return tab_foo; }
			const T& operator()() const { return tab_foo; }
		};
	};
};
namespace TabSample_ {
	struct Alpha {
		struct _alias_t {
			static constexpr const char _literal[] =  "alpha";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T alpha;
				T& operator()() { return alpha; }
				const T& operator()() const { return alpha; }
			};
		};
		using _traits = sqlpp::make_traits<sqlpp::bigint, sqlpp::tag::can_be_null>;
// 		using _foreign_key = decltype(TabFoo::omega);
	};
	struct Beta {
		struct _alias_t {
			static constexpr const char _literal[] =  "beta";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T beta;
				T& operator()() { return beta; }
				const T& operator()() const { return beta; }
			};
		};
		using _traits = sqlpp::make_traits<sqlpp::varchar, sqlpp::tag::can_be_null, sqlpp::tag::must_not_update>;
	};
	struct Gamma {
		struct _alias_t {
			static constexpr const char _literal[] =  "gamma";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T gamma;
				T& operator()() { return gamma; }
				const T& operator()() const { return gamma; }
			};
		};
		using _traits = sqlpp::make_traits<sqlpp::boolean, sqlpp::tag::require_insert>;
	};
}

struct TabSample: sqlpp::table_t<TabSample,TabSample_::Alpha,TabSample_::Beta,TabSample_::Gamma> {
	struct _alias_t {
		static constexpr const char _literal[] =  "tab_sample";
		using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
		
		template<typename T>
		struct _member_t {
			T tab_sample;
			T& operator()() { return tab_sample; }
			const T& operator()() const { return tab_sample; }
		};
	};
};

namespace TabBar_ {
	struct Delta {
		struct _alias_t {
			static constexpr const char _literal[] =  "delta";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T delta;
				T& operator()() { return delta; }
				const T& operator()() const { return delta; }
			};
		};
		using _traits = sqlpp::make_traits<sqlpp::day_point, sqlpp::tag::can_be_null>;
	};
	
	struct Epsilon {
		struct _alias_t {
			static constexpr const char _literal[] =  "epsilon";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T epsilon;
				T& operator()() { return epsilon; }
				const T& operator()() const { return epsilon; }
			};
		};
		using _traits = sqlpp::make_traits<sqlpp::time_point, sqlpp::tag::can_be_null>;
	};
	
	struct Zeta {
		struct _alias_t {
			static constexpr const char _literal[] = "zeta";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T zeta;
				T& operator()() { return zeta; }
				const T& operator()() const { return zeta; }
			};
		};
		using _traits = sqlpp::make_traits<sqlpp::time_point, sqlpp::tag::can_be_null>;
	};
	
	struct Eta {
		struct _alias_t {
			static constexpr const char _literal[] = "eta";
			using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
			
			template<typename T>
			struct _member_t {
				T eta;
				T& operator()() { return eta; }
				const T& operator()() const { return eta; }
			};		};
		using _traits = sqlpp::make_traits<sqlpp::time_point, sqlpp::tag::can_be_null>;
	};
}

struct TabBar: sqlpp::table_t<TabBar,TabBar_::Delta,TabBar_::Epsilon,TabBar_::Zeta,TabBar_::Eta> {
	struct _alias_t {
		static constexpr const char _literal[] =  "tab_bar";
		using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
		
		template<typename T>
		struct _member_t {
			T tab_bar;
			T& operator()() { return tab_bar; }
			const T& operator()() const { return tab_bar; }
		};
	};
};

#endif
