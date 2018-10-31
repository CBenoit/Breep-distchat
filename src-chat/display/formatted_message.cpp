
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

	ImVec4 author_color(const std::string& str) {
		return imgui_color(all_colors[str_hasher(str) % std::size(all_colors)]);
	}
}

void display::formatted_message::print() {
	if (auto user_msg = std::get_if<user_message>(&msg)) {
		ImGui::PushTextWrapPos();
		ImGui::PushStyleColor(ImGuiCol_Text, author_color(user_msg->author));
		ImGui::TextUnformatted(user_msg->author.data(), user_msg->author.data() + user_msg->author.size());
		ImGui::SameLine();
		ImGui::PopStyleColor();
		ImGui::TextUnformatted(": ");
		ImGui::SameLine();
		ImGui::TextUnformatted(user_msg->content.data(), user_msg->content.data() + user_msg->content.size());
		ImGui::PopTextWrapPos();

	} else if (auto sys_msg = std::get_if<system_message>(&msg)) {
		ImGui::PushTextWrapPos();
		ImGui::PushStyleColor(ImGuiCol_Text, imgui_color(colors::system));
		ImGui::TextUnformatted(sys_msg->content.data(), sys_msg->content.data() + sys_msg->content.size());
		ImGui::PopStyleColor();
		ImGui::PopTextWrapPos();

	} else {
		// NANI
	}
}
