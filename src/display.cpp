
#include <display.hpp>
#include <imgui.h>
#include <atomic>
#include <SFML/Window/Event.hpp>
#include <imgui-SFML.h>
#include <thread>
#include <iostream>

namespace {
	std::atomic<bool> instantiaded{false};
}

namespace {
#define IMGUIGUARD(x) struct x { \
	template <typename... Args>\
	explicit x(Args&&... args) : valid{ImGui::Begin##x(args...)}{}\
	template <typename FuncT>\
	void operator+(FuncT&& f) { if (valid) f();}\
	~x() { if (valid) ImGui::End##x(); }\
	bool valid;\
}

IMGUIGUARD(MenuBar);
IMGUIGUARD(Menu);
IMGUIGUARD(Child);

#undef IMGUIGUARD
#define Scoped(x) x + [&]()
}

bool display::is_intiantiated() {
	return instantiaded;
}

display::gui::gui()
		: window{sf::VideoMode(1280, 720), "Clonnect"}
		, clk{}
		, frame_flags{ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar}
{
	if (instantiaded.exchange(true)) {
		throw std::logic_error("Tried to create more than one gui.");
	}
	textinput_buffer.reserve(2048);

	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);
	ImGui::GetIO().IniFilename = nullptr; // disable .ini saving

	add_message("Jean-Louis", "Message 1");
	add_message("Jean-Louis", "Message 2");
	add_message("Marcel", "Message 1");
	add_message("Jean-Louis", "Message 3");
}

display::gui::~gui() {
	ImGui::SFML::Shutdown();
	instantiaded = false;
}

bool display::gui::display() {
	if (!is_open()) {
		return false;
	}

	sf::Event ev{};
	while (window.pollEvent(ev)) {
		if (ev.type == sf::Event::Closed) {
			window.close();
			return false;
		}

		ImGui::SFML::ProcessEvent(ev);
	}

	ImGui::SFML::Update(window, clk.restart());

	ImGui::SetNextWindowSize(ImVec2{static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)});
	ImGui::SetNextWindowPos({});
	ImGui::Begin("Main", nullptr, frame_flags);

	update_frame();

	ImGui::End();
	window.clear();
	ImGui::SFML::Render(window);
	window.display();
	return true;
}

void display::gui::update_frame() {

	Scoped(MenuBar()) {
		Scoped(Menu("Options")) {
//			Scoped(Menu("Color theme")) {
//
//			};
			if(ImGui::MenuItem("Quit")) {
				window.close();
			}
		};
	};

	Scoped(Child(ImGui::GetID("chat"), ImVec2(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y - 60)))) {
		update_chat_frame();
	};

	if (ImGui::InputTextMultiline("text_input", textinput_buffer.data(), textinput_buffer.capacity(), {-1.f, -1.f}
	, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput)) {
		textinput_callback(std::string_view(textinput_buffer.data()));
		textinput_buffer[0] = '\0';
	}

}

void display::gui::update_chat_frame() {
	std::lock_guard lg(msg_mutex);
	for (auto&& msg : messages) {
		ImGui::PushTextWrapPos(0.0f);
		ImGui::TextColored({1.f, 1.f, 1.f, 1.f}, "%s", msg.first.c_str());
		ImGui::SameLine();
		ImGui::Text(": %s", msg.second.c_str());
		ImGui::PopTextWrapPos();
	}
}


#undef Scoped
