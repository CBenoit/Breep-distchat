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

namespace display {
    IMGUIGUARD(MenuBar);
    IMGUIGUARD(Menu);
    IMGUIGUARD(Child);
}

#undef IMGUIGUARD
#define Scoped(x) x + [&]()

#endif //SYSDIST_SERVER_IMGUI_HELPER_HPP
