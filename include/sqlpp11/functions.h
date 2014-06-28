/*
 * Copyright (c) 2013-2014, Roland Bock
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
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

#ifndef SQLPP_FUNCTIONS_H
#define SQLPP_FUNCTIONS_H

#include <sqlpp11/parameter.h>
#include <sqlpp11/parameter_list.h>
#include <sqlpp11/column_types.h>
#include <sqlpp11/in.h>
#include <sqlpp11/is_null.h>
#include <sqlpp11/value_type.h>
#include <sqlpp11/exists.h>
#include <sqlpp11/any.h>
#include <sqlpp11/some.h>
#include <sqlpp11/count.h>
#include <sqlpp11/min.h>
#include <sqlpp11/max.h>
#include <sqlpp11/avg.h>
#include <sqlpp11/sum.h>
#include <sqlpp11/verbatim_table.h> // Csaba Csoma suggests: unsafe_sql instead of verbatim

namespace sqlpp
{
	template<typename T>
		auto value(T t) -> wrap_operand_t<T>
		{
			static_assert(is_wrapped_value_t<wrap_operand_t<T>>::value, "value() is to be called with non-sql-type like int, or string");
			return { t };
		}

	template<typename ValueType> // Csaba Csoma suggests: unsafe_sql instead of verbatim
		struct verbatim_t: public ValueType::template expression_operators<verbatim_t<ValueType>>,
		public alias_operators<verbatim_t<ValueType>>
	{
		using _traits = make_traits<ValueType, ::sqlpp::tag::expression>;
		using _recursive_traits = make_recursive_traits<>;

		verbatim_t(std::string verbatim): _verbatim(verbatim) {}
		verbatim_t(const verbatim_t&) = default;
		verbatim_t(verbatim_t&&) = default;
		verbatim_t& operator=(const verbatim_t&) = default;
		verbatim_t& operator=(verbatim_t&&) = default;
		~verbatim_t() = default;

		std::string _verbatim;
	};

	template<typename Context, typename ValueType>
		struct serializer_t<Context, verbatim_t<ValueType>>
		{
			using T = verbatim_t<ValueType>;

			static Context& _(const T& t, Context& context)
			{
				context << t._verbatim;
				return context;
			}
		};

	template<typename ValueType, typename StringType>
		auto verbatim(StringType s) -> verbatim_t<ValueType>
		{
			return { s };
		}

	template<typename Expression, typename Context>
		auto flatten(const Expression& exp, const Context& context) -> verbatim_t<value_type_of<Expression>>
		{
			static_assert(not make_parameter_list_t<Expression>::type::size::value, "parameters not supported in flattened expressions");
			context.clear();
			serialize(exp, context);
			return { context.str() };
		}

	template<typename Container>
		struct value_list_t // to be used in .in() method
		{
			using _traits = make_traits<value_type_t<typename Container::value_type>, ::sqlpp::tag::expression>;
			using _recursive_traits = make_recursive_traits<>;

			using _container_t = Container;

			value_list_t(_container_t container):
				_container(container)
			{}

			value_list_t(const value_list_t&) = default;
			value_list_t(value_list_t&&) = default;
			value_list_t& operator=(const value_list_t&) = default;
			value_list_t& operator=(value_list_t&&) = default;
			~value_list_t() = default;

			_container_t _container;
		};

	template<typename Context, typename Container>
		struct serializer_t<Context, value_list_t<Container>>
		{
			using T = value_list_t<Container>;

			static Context& _(const T& t, Context& context)
			{
				bool first = true;
				for (const auto& entry: t._container)
				{
					if (first)
						first = false;
					else
						context << ',';

					serialize(value(entry), context);
				}
				return context;
			}
		};

	template<typename Container>
		auto value_list(Container c) -> value_list_t<Container>
		{
			static_assert(is_wrapped_value_t<wrap_operand_t<typename Container::value_type>>::value, "value_list() is to be called with a container of non-sql-type like std::vector<int>, or std::list(string)");
			return { c };
		}

	template<typename T>
		constexpr const char* get_sql_name(const T&) 
		{
			return T::type::_name_t::_get_name();
		}


}

#endif
