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
#include <display/main_gui.hpp>
#include <imgui_internal.h>


#include "display/main_gui.hpp"
#include "display/imgui_helper.hpp"
#include "display/imgui_theming.hpp"

namespace {
	std::atomic<bool> instantiaded{false};
}

ImVec4 display::imgui_color(colors color, float alpha) {
	return {
			static_cast<float>(static_cast<unsigned int>(color) >> 16u & 0xFF) / 255.f,
			static_cast<float>(static_cast<unsigned int>(color) >> 8u & 0xFF) / 255.f,
			static_cast<float>(static_cast<unsigned int>(color) >> 0u & 0xFF) / 255.f,
			alpha
	};
}

bool display::is_instanciated() {
	return instantiaded;
}

display::main_gui::main_gui()
		: window{sf::VideoMode(1280, 720), "Clonnect"}, clk{},
		  frame_flags{ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar} {
	if (instantiaded.exchange(true)) {
		throw std::logic_error("Tried to create more than one main_gui.");
	}
	textinput_buffer.reserve(2048);

	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);
	ImGui::GetIO().IniFilename = nullptr; // disable .ini saving

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.f;

	current_theme = theme::dark_itamago;
	current_theme();
}

display::main_gui::~main_gui() {
	ImGui::SFML::Shutdown();
	instantiaded = false;
}

bool display::main_gui::display() {
	if (!is_open()) {
		return false;
	}
	if (new_theme) {
		current_theme = new_theme;
		new_theme();
		new_theme = nullptr;
	}

	sf::Event ev{};
	while (window.pollEvent(ev)) {
		if (ev.type == sf::Event::Closed) {
			window.close();
			return false;
		}

		if ((ev.type != sf::Event::KeyPressed && ev.type != sf::Event::KeyReleased) ||
		    ev.key.code != sf::Keyboard::Unknown)
			ImGui::SFML::ProcessEvent(ev);
	}

	ImGui::SFML::Update(window, clk.restart());

	ImGui::SetNextWindowSize(ImVec2{static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)});
	ImGui::SetNextWindowPos({});

	ImGui::Begin("##Main", nullptr, frame_flags);
	update_frame();
	ImGui::End();

	window.clear();
	ImGui::SFML::Render(window);
	window.display();

	return true;
}

void display::main_gui::update_frame() {
	update_menu_bar();

	Scoped(Child(ImGui::GetID("Chat_area"), ImVec2{-(peer_name_max_x_size + 20.f), 0})) {
		update_chat_area();
	};
	ImGui::SameLine();
	ImGui::VerticalSeparator();
	ImGui::SameLine();
	Scoped(Child(ImGui::GetID("Peers_area"))) {
		update_peers_area();
	};
}

void display::main_gui::update_chat_area() {
	bool can_send_msgs = false;
	Scoped(Child(ImGui::GetID("chat"),
	             ImVec2(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y - 60)))) {
		std::lock_guard lg(msg_mutex);
		if (focused_user) {
			auto msgs = messages.find(focused_user.value());
			if (msgs != messages.end()) {
				msgs->second.print();
				can_send_msgs = msgs->second.can_send_messages;
			}
		}
	};

	if (can_send_msgs) {
		// input area
		if (ImGui::InputTextMultiline("##text_input", textinput_buffer.data(), textinput_buffer.capacity(), {-15.f, -1.f},
		                              ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine)) {
			textinput_callback(std::string_view(textinput_buffer.data()));
			textinput_buffer[0] = '\0';
			ImGui::SetKeyboardFocusHere(-1);
		}
	} else {
		ImGui::TextUnformatted("No connected user selected.");
	}
}

void display::main_gui::update_peers_area() {

	constexpr char contact_text[] = "Contacts";
	ImVec2 text_size = ImGui::CalcTextSize(contact_text);
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvailWidth() - text_size.x) / 2);
	ImGui::TextUnformatted(contact_text);

	ImGui::Separator();

	std::lock_guard lg(msg_mutex);

	for (auto&& item : messages) {
		std::string text = item.first;
		int count = new_messages[text];
		bool is_selected = focused_user && focused_user.value() == item.first;
		if (count > 0) {
			text += " (" + std::to_string(count) + ')';
			ImGui::PushStyleColor(ImGuiCol_Header, imgui_color(colors::alert, 0.75f));
			is_selected = true;
		}

		if (item.second.can_send_messages) {
			ImGui::PushStyleColor(ImGuiCol_Text, imgui_color(colors::peer_connected, 0.75f));
		} else {
			ImGui::PushStyleColor(ImGuiCol_Text, imgui_color(colors::peer_disconnected, 0.75f));
		}

		if (ImGui::Selectable(text.data(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
			focused_user = item.first;
			new_messages[item.first] = 0;
		}

		ImGui::PopStyleColor(count > 0 ? 2 : 1);
	}
}

void display::main_gui::update_menu_bar() {
	auto theme_entry = [this](const char* theme_name, theme_fnct theme) {
		if (ImGui::MenuItem(theme_name, nullptr, current_theme == theme)) {
			new_theme = theme;
		}
	};

	Scoped(MenuBar()) {
		Scoped(Menu("Options")) {
			Scoped(Menu("Color theme")) {
				theme_entry("Cherry", theme::cherry);
				theme_entry("Dark", theme::dark);
				theme_entry("Itamago Dark", theme::dark_itamago);
				theme_entry("Itamago Light", theme::light_itamago);
				theme_entry("Microsoft Light", theme::MicrosoftLight);
				theme_entry("Unity Engine 4", theme::UE4);
			};
			if (ImGui::MenuItem("Quit")) {
				window.close();
			}
		};
	};

}
