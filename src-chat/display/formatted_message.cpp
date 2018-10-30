
#include <display/formatted_message.hpp>
#include <display/main_gui.hpp>

#include "display/formatted_message.hpp"

namespace {
	std::hash<std::string> str_hasher{};

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
				static_cast<float>(static_cast<unsigned int>(color) >> 8u & 0xFF) / 255.f,
				static_cast<float>(static_cast<unsigned int>(color) >> 0u & 0xFF) / 255.f,
				1.f
		};
	}

	ImVec4 author_color(const std::string& str) {
		return imgui_color(all_colors[str_hasher(str) % std::size(all_colors)]);
	}
}

void display::formatted_message::print() {
	if (auto user_msg = std::get_if<user_message>(&msg)) {
		ImGui::PushTextWrapPos();
		ImGui::TextColored(author_color(user_msg->author), "%s:", user_msg->author.data());
		ImGui::SameLine();
		ImGui::TextColored(imgui_color(colors::white), "%s", user_msg->content.data());
		ImGui::PopTextWrapPos();

	} else if (auto sys_msg = std::get_if<system_message>(&msg)) {
		ImGui::PushTextWrapPos();
		ImGui::TextColored(imgui_color(colors::system), "%s", sys_msg->content.data());
		ImGui::PopTextWrapPos();

	} else {
		// NANI
	}
}
