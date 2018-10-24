#ifndef SYSDIST_CHAT_UI_HPP
#define SYSDIST_CHAT_UI_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <mutex>
#include <functional>
#include <imgui.h>
#include <iostream>

namespace display {
	bool is_intiantiated();

	class gui {
	public:
		gui();
		~gui();

		/* return is_open */
		bool display();

		bool is_open() { return window.isOpen(); }

		void system_message(std::string_view message, const ImVec4& color) {
			// TODO
			std::cerr << "TODO: " << __FILE__ << ':' << __LINE__ << '\n';
		}

		void add_message(std::string author, std::string msg) {
			std::lock_guard lg(msg_mutex);
			messages.emplace_back(std::move(author), std::move(msg));
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

		std::vector<std::pair<std::string, std::string>> messages{};
		std::mutex msg_mutex{};

		std::string textinput_buffer{};

		std::function<void(std::string_view)> textinput_callback{};
	};
}


#endif //SYSDIST_CHAT_UI_HPP
