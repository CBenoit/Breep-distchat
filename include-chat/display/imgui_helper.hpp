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

#ifndef SYSDIST_SERVER_IMGUI_HELPER_HPP
#define SYSDIST_SERVER_IMGUI_HELPER_HPP

#include <imgui.h>

#define IMGUIGUARD(x) struct x { \
    template <typename... Args>\
    explicit x(Args&&... args) : valid{ImGui::Begin##x(args...)}{}\
    template <typename FuncT>\
    void operator+(FuncT&& f) { if (valid) f();}\
    ~x() { if (valid) ImGui::End##x(); }\
    bool valid;\
}

#define IMGUIGUARD2(x) struct x { \
    template <typename... Args>\
    explicit x(Args&&... args) {ImGui::Begin##x(args...);}\
    template <typename FuncT>\
    void operator+(FuncT&& f) { f();}\
    ~x() { ImGui::End##x(); }\
}

namespace display {
	IMGUIGUARD(MenuBar);

	IMGUIGUARD(Menu);

	IMGUIGUARD2(Child);
}

#undef IMGUIGUARD
#define Scoped(x) x + [&]()

#endif //SYSDIST_SERVER_IMGUI_HELPER_HPP
