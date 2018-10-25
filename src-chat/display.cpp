
#include <display.hpp>
#include <imgui.h>
#include <atomic>
#include <SFML/Window/Event.hpp>
#include <imgui-SFML.h>
#include <thread>
#include <iostream>

namespace {
	std::atomic<bool> instantiaded{false};
	const std::hash<std::string> str_hasher{};

	constexpr display::colors all_colors[] = {
			display::colors::red,
			display::colors::blue,
			display::colors::green,
			display::colors::cyan,
			display::colors::pink,
			display::colors::yellow
	};


	ImVec4 imgui_color(display::colors color) {
		return {
				static_cast<float>(static_cast<unsigned int>(color) >> 16u & 0xFF) / 255.f,
				static_cast<float>(static_cast<unsigned int>(color) >> 8u  & 0xFF) / 255.f,
				static_cast<float>(static_cast<unsigned int>(color) >> 0u  & 0xFF) / 255.f,
				1.f
		};
	}

	ImVec4 author_color(const std::string& str) {
		return imgui_color(all_colors[str_hasher(str) % std::size(all_colors)]);
	}
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

	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);
	ImGui::GetIO().IniFilename = nullptr; // disable .ini saving
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

		if ((ev.type != sf::Event::KeyPressed && ev.type != sf::Event::KeyReleased) || ev.key.code != sf::Keyboard::Unknown)
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

	auto print_message = [](const std::tuple<int, std::string, std::string>& tpl) {
		ImGui::PushTextWrapPos();
		ImGui::TextColored(author_color(std::get<1>(tpl)), "%s:", std::get<1>(tpl).data());
		ImGui::SameLine();
		ImGui::TextColored(imgui_color(colors::white), "%s", std::get<2>(tpl).data());
		ImGui::PopTextWrapPos();
	};

	auto print_system_message = [](const std::tuple<int, std::string>& tpl) {
		ImGui::PushTextWrapPos();
		ImGui::TextColored(imgui_color(colors::system), "%s", std::get<1>(tpl).data());
		ImGui::PopTextWrapPos();
	};

	std::lock_guard lg(msg_mutex);
	auto msg_it = messages.cbegin();
	auto sys_it = system_messages.cbegin();

	while(msg_it != messages.cend() && sys_it != system_messages.cend()) {
		if (std::get<int>(*msg_it) < std::get<int>(*sys_it)) {
			print_message(*msg_it++);
		} else {
			print_system_message(*sys_it++);
		}
	}

	while (msg_it != messages.cend()) {
		print_message(*msg_it++);
	}

	while (sys_it != system_messages.cend()) {
		print_system_message(*sys_it++);
	}
}


#undef Scoped
