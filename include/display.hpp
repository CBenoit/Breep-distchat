#ifndef SYSDIST_CHAT_UI_HPP
#define SYSDIST_CHAT_UI_HPP

#include <SFML/Graphics/RenderWindow.hpp>

namespace display {
	bool is_intiantiated();

	class gui {
	public:
		gui();
		~gui();

		/* return is_open */
		bool display();

		bool is_open() { return window.isOpen(); }

	private:
		void update_frame();

		sf::RenderWindow window;
		sf::Clock clk;
		const int frame_flags;
	};
}


#endif //SYSDIST_CHAT_UI_HPP
