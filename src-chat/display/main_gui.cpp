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
#include <sound_sender.hpp>


#include "display/main_gui.hpp"
#include "display/imgui_helper.hpp"
#include "display/imgui_theming.hpp"
#include "audio_source.hpp"

namespace {
	std::atomic<bool> instantiaded{false};

	template <typename T>
	bool try_erase(std::set<T>& s, const T& val) {
		return s.erase(val) > 0;
	}
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

	Scoped(Child(ImGui::GetID("Chat_area"), ImVec2{-(peer_name_max_x_size + 45.f + call_state_width), 0.f})) {
		update_chat_area();
	};

	ImGui::SameLine();
	ImGui::VerticalSeparator();
	ImGui::SameLine();

	Scoped(Child(ImGui::GetID("Left_panel"))) {
		Scoped(Child(ImGui::GetID("Peers_area"), ImVec2{-(call_state_width - 10.f), ImGui::GetContentRegionAvail().y / 2})) {
			update_peers_area();
		};
		ImGui::SameLine();
		Scoped(Child(ImGui::GetID("Call_area"), ImVec2{0.f, ImGui::GetContentRegionAvail().y / 2})) {
			update_call_state();
		};

		ImGui::Separator();

		Scoped(Child(ImGui::GetID("Disconnected_peers_area"))) {
			update_dc_peers_area();
		};
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
		ImGui::PushItemWidth(-15.f);
		if (ImGui::InputText("##text_input", textinput_buffer.data(), textinput_buffer.capacity()
				, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
			textinput_callback(std::string_view(textinput_buffer.data()));
			textinput_buffer[0] = '\0';
			ImGui::SetKeyboardFocusHere(-1);
		}
		ImGui::PopItemWidth();
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
		if (!item.second.can_send_messages) {
			continue;
		}
		print_peer(item.first);
	}
}

void display::main_gui::update_dc_peers_area() {

	constexpr char disconncted_peers[] = "Disconnected peers";
	ImVec2 text_size = ImGui::CalcTextSize(disconncted_peers);
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvailWidth() - text_size.x) / 2);
	ImGui::TextUnformatted(disconncted_peers);

	ImGui::Separator();

	std::lock_guard lg(msg_mutex);
	for (auto&& item : messages) {
		if (item.second.can_send_messages) {
			continue;
		}
		print_peer(item.first);
	}
}

void display::main_gui::print_peer(const std::string& peer) {
	std::string text = peer;
	int count = new_messages[peer];
	bool is_selected = focused_user && focused_user.value() == peer;
	if (count > 0) {
		text += " (" + std::to_string(count) + ')';
		ImGui::PushStyleColor(ImGuiCol_Header, imgui_color(colors::alert, 0.75f));
		is_selected = true;
	}

	if (ImGui::Selectable(text.data(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
		focused_user = peer;
		new_messages[peer] = 0;
	}

	if (count > 0) {
		ImGui::PopStyleColor();
	}
}

void display::main_gui::update_call_state() {

	constexpr char calls_text[] = "calls";
	ImVec2 text_size = ImGui::CalcTextSize(calls_text);
	ImGui::SetCursorPosX((ImGui::GetContentRegionAvailWidth() - text_size.x) / 2);
	ImGui::TextUnformatted(calls_text);

	ImGui::Separator();

	std::scoped_lock sl(msg_mutex, call_mutex);
	for (auto&& item : messages) {
		ImGui::PushID(item.first.data());
		if (item.second.can_send_messages) {
			if (requests_in.count(item.first) > 0) {
				Scoped(Child("Accept_child", ImVec2{ImGui::GetContentRegionAvailWidth() / 2, 0.f})) {
					if (ImGui::Selectable("Accept")) {
						ongoing_calls.insert(requests_in.extract(item.first));
						call_request_callback(item.first, true);
						lockless_add_message(item.first, system_message("Call accepted."));
						call_starting_callback(item.first);
					}
				};
				ImGui::SameLine();
				Scoped(Child("Deny_child")) {
					if (ImGui::Selectable("Deny")) {
						requests_in.erase(item.first);
						call_request_callback(item.first, false);
						lockless_add_message(item.first, system_message("Call denied."));
					}
				};

			} else if (requests_out.count(item.first) > 0) {
				if (ImGui::Selectable("Calling...")) {
					requests_out.erase(item.first);
					call_request_callback(item.first, false);
					lockless_add_message(item.first, system_message("Call cancelled"));
				}

			} else if (ongoing_calls.count(item.first) > 0) {
				if (ImGui::Selectable("Ongoing")) {
					ongoing_calls.erase(item.first);
					call_request_callback(item.first, false);
					lockless_add_message(item.first, system_message("Call ended."));
					call_ending_callback(item.first);
				}

			} else {
				if (ImGui::Selectable("Call")) {
					requests_out.insert(item.first);
					call_request_callback(item.first, true);
					lockless_add_message(item.first, system_message("Calling..."));
				}
			}
		}
		ImGui::PopID();
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
			if (ImGui::MenuItem("Mute speakers", nullptr, sound_muted)) {
				sound_muted = !sound_muted;
				snd_muting_callback(sound_muted);
			}
			if (ImGui::MenuItem("Mute microphone", nullptr, mic_muted)) {
				mic_muted = !mic_muted;
				mic_muting_callback(mic_muted);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) {
				window.close();
			}
		};
	};

}

void display::main_gui::call_request_input(const std::string& source, const display::call_request& call) {

	std::lock_guard lg(call_mutex);
	if (call.rq_value) {
		if (ongoing_calls.insert(requests_out.extract(source)).inserted) {
			add_message(source, system_message("Call accepted."));
			call_starting_callback(source);

		} else if (requests_in.count(source) == 0) {
			requests_in.insert(source);
			add_message(source, system_message("Incomming call..."));

		}


	} else {
		if (try_erase(ongoing_calls, source)) {
			add_message(source, system_message("Call ended."));
			call_ending_callback(source);

		} else if (try_erase(requests_out, source)) {
			add_message(source, system_message("Call denied."));

		} else if (try_erase(requests_in, source)){
			add_message(source, system_message("Call cancelled."));
		}
	}
}

void display::main_gui::lockless_add_message(const std::string& source, display::formatted_message&& msg) {
	auto source_messages = messages.find(source);
	if (source_messages != messages.end()) {
		source_messages->second.emplace_back(std::move(msg));
	}
	if (!focused_user || source != *focused_user) {
		++new_messages[source];
	}
}
