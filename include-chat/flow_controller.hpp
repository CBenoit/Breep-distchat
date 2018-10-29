#ifndef SYSDIST_FLOW_CONTROLLER_HPP
#define SYSDIST_FLOW_CONTROLLER_HPP

/*************************************************************************************
 * MIT License                                                                       *
 *                                                                                   *
 * Copyright (c) 2018 TiWinDeTea                                                     *
 *                                                                                   *
 * Permission is hereby granted, free of charge, to any person obtaining a copy      *
 * of this software and associated documentation files (the "Software"), to deal     *
 * in the Software without restriction, including without limitation the rights      *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell         *
 * copies of the Software, and to permit persons to whom the Software is             *
 * furnished to do so, subject to the following conditions:                          *
 *                                                                                   *
 * The above copyright notice and this permission notice shall be included in all    *
 * copies or substantial portions of the Software.                                   *
 *                                                                                   *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR        *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,          *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE       *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER            *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,     *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE     *
 * SOFTWARE.                                                                         *
 *                                                                                   *
 *************************************************************************************/


#include <utility>
#include <exception>

#define CAT_IMPL(x, y) x##y
#define CAT(x, y) CAT_IMPL(x,y)

#ifdef __COUNTER__
#define ANONYMOUS(x) CAT(CAT(CAT(CAT(x,__),__LINE__),__),__COUNTER__)
#else
#define ANONYMOUS(x) CAT(CAT(x,__),__LINE__)
#endif

namespace details {

	template<typename T>
	struct always_do {

		always_do(T&& t) : var(std::forward<T>(t)) {}

		~always_do() {
			var();
		}

		T var;
	};

	template<typename T, bool OnExcept>
	struct sometimes_do {
		sometimes_do(T&& t) : var(std::forward<T>(t)), exception_count(std::uncaught_exceptions()) {}

		~sometimes_do() {
			if constexpr (OnExcept) {
				if (exception_count < std::uncaught_exceptions()) {
					var();
				}
			} else {
				if (exception_count == std::uncaught_exceptions()) {
					var();
				}
			}
		}

		T var;
		int exception_count;
	};

	struct always_do_struct {
	};
	struct except_do_struct {
	};
	struct noexcept_do_struct {
	};

	template<typename T>
	always_do<T> operator+(always_do_struct, T&& f) {
		return {std::forward<T>(f)};
	}

	template<typename T>
	sometimes_do<T, true> operator+(except_do_struct, T&& f) {
		return {std::forward<T>(f)};
	}

	template<typename T>
	sometimes_do<T, false> operator+(noexcept_do_struct, T&& f) {
		return {std::forward<T>(f)};
	}

}

#define ON_SCOPE_EXIT auto ANONYMOUS(ON_SCOPE_EXIT_VAR) = details::always_do_struct{} + [&] ()
#define ON_SCOPE_EXIT_EXCEPTION auto ANONYMOUS(ON_SCOPE_EXIT_EXCEPTION_VAR) = details::except_do_struct{} + [&] ()
#define ON_SCOPE_EXIT_NOEXCEPT auto ANONYMOUS(ON_SCOPE_EXIT_NOEXCEPT_VAR) = details::noexcept_do_struct{} + [&] ()

#endif //SYSDIST_FLOW_CONTROLLER_HPP