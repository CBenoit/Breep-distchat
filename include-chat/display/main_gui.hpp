#ifndef SYSDIST_CHAT_UI_HPP
#define SYSDIST_CHAT_UI_HPP

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
#include <mutex>
#include <functional>
#include <iostream>
#include <boost/uuid/uuid.hpp>

#include <boost/uuid/nil_generator.hpp>
#include <boost/container_hash/extensions.hpp>

#include <breep/util/serialization.hpp>

#include <imgui.h>

#include <SFML/Graphics/RenderWindow.hpp>

#include "formatted_message.hpp"
#include "sound_def.hpp"

namespace display {
	enum class colors {
		red = 0xDC143C,
		blue = 0x00BFFF,
		green = 0x00FF7F,
		cyan = 0xE0FFFF,
		pink = 0xFF69B4,
		yellow = 0xFFD700,
		white = 0xFFFFF0,
		alert = 0x801341,
		peer_connected = 0x32CD32,
		peer_disconnected = 0xDC143C,
		system = 0x6495ED
	};

	ImVec4 imgui_color(colors color, float alpha = 1.f);

	bool is_instanciated();

	struct call_request {
		call_request() = default;
		call_request(bool b) : rq_value(b) {}
		bool rq_value{false};
		BREEP_ENABLE_SERIALIZATION(call_request, rq_value)
	};

	class main_gui {

		using theme_fnct = void (*)();
		struct msg_list {
			void print() {
				for (auto&& msg : messages) {
					msg.print();
				}
			}

			template <typename... Args>
			void emplace_back(Args&&... args) {
				messages.emplace_back(std::forward<Args>(args)...);
			}

			std::vector<formatted_message> messages{};
			bool can_send_messages{};
		};

	public:
		main_gui();

		~main_gui();

		/* return is_open */
		bool display();

		bool is_open() { return window.isOpen(); }

		void sound_input(const std::string& source, const sound_buffer_t& sound);

		void call_request_input(const std::string& source, const call_request& call);

		void add_message(const std::string& source, formatted_message&& msg) {
			std::lock_guard lg{msg_mutex};
			lockless_add_message(source, std::move(msg));
		}

		void set_textinput_callback(std::function<void(std::string_view)> tic) {
			textinput_callback = std::move(tic);
		}

		void set_call_request_callback(std::function<void(const std::string&, const call_request&)> crc) {
			call_request_callback = std::move(crc);
		}

		void add_user(const std::string& username) {
			std::string name_and_count(username);
			name_and_count += " (000)";

			auto size = ImGui::CalcTextSize(name_and_count.data());
			peer_name_max_x_size = std::max(peer_name_max_x_size, size.x);
			peer_name_y_size = size.y;

			std::scoped_lock lock{msg_mutex};

			messages[username].can_send_messages = true;
			new_messages.insert(std::make_pair(username, 0));
		}

		void remove_user(const std::string& username) {
			std::scoped_lock lock{msg_mutex, call_mutex};
			auto source_messages = messages.find(username);
			if (source_messages != messages.end()) {
				source_messages->second.can_send_messages = false;
			}
			++new_messages[username];
			ongoing_calls.erase(username);
			requests_in.erase(username);
			requests_out.erase(username);
		}

		const std::optional<std::string>& get_focused_name() const {
			return focused_user;
		}

	private:
		void update_frame();
		void update_chat_area();
		void update_peers_area();
		void update_dc_peers_area();
		void update_menu_bar();
		void update_call_state();

		void print_peer(const std::string& peer);

		void lockless_add_message(const std::string& source, formatted_message&& msg);

		// Display variables //
		float peer_name_max_x_size{50.f};
		float peer_name_y_size{0.f};
		float call_state_width{130.f};

		theme_fnct new_theme{nullptr};
		theme_fnct current_theme{nullptr};

		sf::RenderWindow window;
		sf::Clock clk;
		const int frame_flags;

		std::optional<std::string> focused_user{};

		// Network input //
		std::mutex msg_mutex{};
		std::unordered_map<std::string, msg_list> messages{};
		std::unordered_map<std::string, int> new_messages{};

		// Gui input //
		std::string textinput_buffer{};
		std::function<void(std::string_view)> textinput_callback{};
		std::function<void(const std::string&, const call_request&)> call_request_callback{[](auto&&...){}};

		// Sound management //
		std::atomic<bool> sound_muted{false};
		std::atomic<bool> mic_muted{false};
		std::mutex call_mutex{};
		std::set<std::string> ongoing_calls{};
		std::set<std::string> requests_in{};
		std::set<std::string> requests_out{};

	};
}

BREEP_DECLARE_TYPE(display::call_request)

#endif //SYSDIST_CHAT_UI_HPP
