#ifndef SYSDIST_SERVER_FORMATTED_MESSAGE_HPP
#define SYSDIST_SERVER_FORMATTED_MESSAGE_HPP

#include <string_view>
#include <string>
#include <variant>

namespace display {
	struct user_message {
		user_message(std::string_view author_, std::string_view content_)
				: author(author_), content(content_) {}
		std::string author;
		std::string content;
	};

	struct system_message {
		explicit system_message(std::string_view content_) : content(content_) {}
		std::string content;
	};

	class formatted_message {
	public:
		formatted_message(user_message m) : msg(std::move(m)) {}
		formatted_message(system_message m) : msg(std::move(m)) {}

		void print();

	private:
		std::variant<system_message, user_message> msg;
	};
}

#endif //SYSDIST_SERVER_FORMATTED_MESSAGE_HPP
