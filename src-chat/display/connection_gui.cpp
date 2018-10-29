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

#include <SFML/Window/Event.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

#include "display/connection_gui.hpp"
#include "display/imgui_helper.hpp"
#include "connection_fields.hpp"

namespace default_values {
    namespace {
        constexpr unsigned short remote_port = 1234;
        constexpr unsigned short local_port = remote_port + 1;
        constexpr char user_name[] = "";
        constexpr char password[] = "";
        constexpr char addr[] = "127.0.0.1";
    }
}
namespace max_sizes {
	namespace {
		constexpr size_t username = 256;
		constexpr size_t password = 256;
		constexpr size_t addr = 16;
	}
}

namespace {
	template<auto N, auto M>
	void test_and_set(char (&field)[N], const char (&char_value)[M], const std::optional<std::string>& str_value) {
		static_assert(N >= M);
		if (str_value) {
			std::copy(str_value->begin(),str_value->end(), field);
		} else {
			std::copy(char_value, char_value + M, field);
		}
	}

	template<auto N, auto M>
	void test_and_set(char (&field)[N], const char (&char_value)[M], const std::optional<boost::asio::ip::address_v4>& addr_value) {
		static_assert(N >= M);
		if (addr_value) {
			std::string string_addr = addr_value->to_string();
			std::copy(string_addr.begin(), string_addr.end(), field);
		} else {
			std::copy(char_value, char_value + M, field);
		}
	}

	void test_size_and_set(const char* str, std::optional<std::string>& value) {
		size_t len = std::strlen(str);
		if (len) {
			value = std::string(str, len);
		}
	}

	void test_size_and_set(const char* str, std::optional<boost::asio::ip::address_v4>& value) {
		boost::system::error_code ec{};
		value = boost::asio::ip::address_v4::from_string(str, ec);
		if (ec) {
			value = {};
		}
	}

	void test_if_port_and_set(int port, std::optional<unsigned short>& value) {
		if (port >= 0 && port < std::numeric_limits<unsigned short>::max()) {
			value = static_cast<unsigned short>(port);
		}
	}
}

display::connection_gui::connection_gui()
		: window{sf::VideoMode(400, 160), "Clonnect login"}
		, frame_flags{ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings}
		, clk{}
{
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);
	ImGui::GetIO().IniFilename = nullptr; // disable .ini saving

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.f;
}

display::connection_gui::~connection_gui() {
	window.close();
	ImGui::SFML::Shutdown();
}

connection_fields display::connection_gui::show(const connection_fields& fields) {

    using namespace std::string_literals;
    bool keep_looping = true;

    int local_port = fields.local_port ? fields.local_port.value() : default_values::local_port;
    int remote_port = fields.remote_port ? fields.remote_port.value() : default_values::remote_port;

    char username[max_sizes::username];
    test_and_set(username, default_values::user_name, fields.username);

    char password[max_sizes::password];
    test_and_set(password, default_values::password, fields.password);

    char remote_addr[max_sizes::addr];
    test_and_set(remote_addr, default_values::addr, fields.remote_address);

	connection_fields ans;

    do {
        sf::Event ev{};
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
            	std::exit(1);
            }

            if ((ev.type != sf::Event::KeyPressed && ev.type != sf::Event::KeyReleased) ||
                ev.key.code != sf::Keyboard::Unknown)
                ImGui::SFML::ProcessEvent(ev);
        }

        ImGui::SFML::Update(window, clk.restart());

        ImGui::SetNextWindowSize(
                ImVec2{static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y)});
        ImGui::SetNextWindowPos({});
        ImGui::Begin("Main", nullptr, frame_flags);

        // TODO HERE
        ImGui::Text("      Username: "); ImGui::SameLine();
        ImGui::InputText("##Username", username, std::size(username));

        ImGui::Text("      Password: "); ImGui::SameLine();
	    ImGui::InputText("##Password", password, std::size(password), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsNoBlank);

	    ImGui::Text("Remote address: "); ImGui::SameLine();
	    ImGui::InputText("##Remote address", remote_addr, std::size(remote_addr));

	    ImGui::Text("   Remote port: "); ImGui::SameLine();
	    ImGui::InputInt("##Remote port", &remote_port, 0, 0);

	    ImGui::Text("    Local port: "); ImGui::SameLine();
	    ImGui::InputInt("##Local port", &local_port, 0, 0);

        if (ImGui::Button("Connect")) {
        	ans.account_creation = false;
        	keep_looping = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Create account")) {
        	ans.account_creation = true;
        	keep_looping = false;
        }

        ImGui::End();
        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    } while (keep_looping);

	test_size_and_set(username, ans.username);
	test_size_and_set(password, ans.password);
	test_size_and_set(remote_addr, ans.remote_address);
	test_if_port_and_set(local_port, ans.local_port);
	test_if_port_and_set(remote_port, ans.remote_port);

    return ans;
}
