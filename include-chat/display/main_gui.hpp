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

#include <SFML/Graphics/RenderWindow.hpp>
#include <mutex>
#include <functional>
#include <imgui.h>
#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/container_hash/extensions.hpp>

namespace display {
	enum class colors {
		red = 0xDC143C,
		blue = 0x00BFFF,
		green = 0x00FF7F,
		cyan = 0xE0FFFF,
		pink = 0xFF69B4,
		yellow = 0xFAFAD2,
		white = 0xFFFFF0,
		system = 0x6495ED
	};

	struct message {
	    message(int position_, std::string_view author_, std::string_view content_)
	        : position(position_), author(author_), content(content_) {}
        int position;
	    std::string author;
	    std::string content;
    };

	struct sys_message {
	    sys_message(int position_, std::string_view content_) : position(position_), content(content_) {}
        int position;
	    std::string content;
	};

	bool is_instanciated();

	template <typename T>
    using uuid_map = std::unordered_map<boost::uuids::uuid, T, boost::hash<boost::uuids::uuid>>;

	class main_gui {

		using theme_fnct = void(*)();

	public:
		main_gui();
		~main_gui();

		/* return is_open */
		bool display();

		bool is_open() { return window.isOpen(); }

		void system_message(std::string_view message) {
			std::lock_guard lg(msg_mutex);
			auto sys_msgs = sys_messages.find(focused_uuid);
			if (sys_msgs != sys_messages.end()) {
                sys_msgs->second.emplace_back(sys_messages.size() + messages.size(), message);
			}
		}

		void add_message(std::string_view author, std::string_view msg) {
			std::lock_guard lg(msg_mutex);
			auto msgs = messages.find(focused_uuid);
			if (msgs != messages.end()) {
                msgs->second.emplace_back(sys_messages.size() + messages.size(), author, msg);
			}
		}

		void set_textinput_callback(std::function<void(std::string_view)> tic) {
			textinput_callback = std::move(tic);
		}

		void add_user(boost::uuids::uuid uuid, std::string_view username) {
            uuids_names.emplace(uuid, username);
		}

		void remove_user(boost::uuids::uuid uuid) {
		    uuids_names.erase(uuid);
		}

		boost::uuids::uuid& get_focused_uuid() {
            return focused_uuid;
		}

	private:
		void update_frame();
		void update_chat_frame();

		theme_fnct new_theme{nullptr};
		theme_fnct current_theme{nullptr};

		sf::RenderWindow window;
		sf::Clock clk;
		const int frame_flags;

		uuid_map<std::vector<message>> messages{};
		uuid_map<std::vector<sys_message>> sys_messages{};
		std::mutex msg_mutex{};

		std::string textinput_buffer{};

		std::function<void(std::string_view)> textinput_callback{};

		uuid_map<std::string> uuids_names{};
		boost::uuids::uuid focused_uuid{boost::uuids::nil_uuid()};
	};
}

#endif //SYSDIST_CHAT_UI_HPP
