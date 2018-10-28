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

#include <atomic>
#include <thread>
#include <iostream>

#include <SFML/Window/Event.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include "display/main_gui.hpp"
#include "display/imgui_helper.hpp"

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

bool display::is_instanciated() {
	return instantiaded;
}

display::main_gui::main_gui()
		: window{sf::VideoMode(1280, 720), "Clonnect"}
		, clk{}
		, frame_flags{ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar}
{
	if (instantiaded.exchange(true)) {
		throw std::logic_error("Tried to create more than one main_gui.");
	}
	textinput_buffer.reserve(2048);

	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);
	ImGui::GetIO().IniFilename = nullptr; // disable .ini saving
}

display::main_gui::~main_gui() {
	ImGui::SFML::Shutdown();
	instantiaded = false;
}

bool display::main_gui::display() {
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

void display::main_gui::update_frame() {
	Scoped(MenuBar()) {
		Scoped(Menu("Options")) {
			Scoped(Menu("Color theme")) {

			};
			if(ImGui::MenuItem("Quit")) {
				window.close();
			}
		};
	};

	Scoped(Child(ImGui::GetID("chat"), ImVec2(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y - 60)))) {
		update_chat_frame();
	};

	// input area
	if (ImGui::InputTextMultiline("text_input", textinput_buffer.data(), textinput_buffer.capacity(), {-1.f, -1.f},
			ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput)) {
		textinput_callback(std::string_view(textinput_buffer.data()));
		textinput_buffer[0] = '\0';
        ImGui::SetKeyboardFocusHere(-1); // auto focus previous widget
	}
}

void display::main_gui::update_chat_frame() {
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
