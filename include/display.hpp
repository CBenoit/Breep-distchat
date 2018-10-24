#ifndef SYSDIST_CHAT_UI_HPP
#define SYSDIST_CHAT_UI_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <mutex>
#include <functional>
#include <imgui.h>
#include <iostream>

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

	bool is_intiantiated();

	class gui {
	public:
		gui();
		~gui();

		/* return is_open */
		bool display();

		bool is_open() { return window.isOpen(); }

		void system_message(std::string_view message) {
			std::lock_guard lg(msg_mutex);
			system_messages.emplace_back(system_messages.size() + messages.size(), message);
		}

		void add_message(std::string_view author, std::string_view msg) {
			std::lock_guard lg(msg_mutex);
			messages.emplace_back(system_messages.size() + messages.size(), author, msg);
		}

		void set_textinput_callback(std::function<void(std::string_view)> tic) {
			textinput_callback = std::move(tic);
		}

	private:
		void update_frame();
		void update_chat_frame();

		sf::RenderWindow window;
		sf::Clock clk;
		const int frame_flags;

		std::vector<std::tuple<int, std::string, std::string>> messages{};
		std::vector<std::tuple<int, std::string>> system_messages{};
		std::mutex msg_mutex{};

		std::string textinput_buffer{};

		std::function<void(std::string_view)> textinput_callback{};
	};
}


#endif //SYSDIST_CHAT_UI_HPP
