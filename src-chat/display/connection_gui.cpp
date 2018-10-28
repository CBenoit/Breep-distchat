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

#include "display/connection_gui.hpp"
#include "display/imgui_helper.hpp"

display::connection_gui::connection_gui()
		: window{sf::VideoMode(1280, 720), "Clonnect login"}
		, frame_flags{ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		              ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar}
		, clk{}
{
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);
	ImGui::GetIO().IniFilename = nullptr; // disable .ini saving
}

display::connection_gui::~connection_gui() {
	ImGui::SFML::Shutdown();
}

#define USERNAME_MAX_SIZE 256
#define PASSWORD_MAX_SIZE 256
#define ADDR_MAX_SIZE 16

std::optional<display::connection_fields> display::connection_gui::show() {
    int local_port = 1234;
    int remote_port = 1234;
    char username[USERNAME_MAX_SIZE] = "User";
    char password[PASSWORD_MAX_SIZE] = "";
    char remote_addr[ADDR_MAX_SIZE] = "";

    while (is_open()) {
        sf::Event ev{};
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                window.close();
                return {};
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

        ImGui::InputText("Username", username, USERNAME_MAX_SIZE);
        ImGui::InputInt("Local port", &local_port);
        ImGui::InputText("Remote address", remote_addr, ADDR_MAX_SIZE);
        ImGui::InputInt("Remote port", &remote_port);
        ImGui::InputText("Password", password, PASSWORD_MAX_SIZE);

        if (ImGui::Button("Connect")) {
            window.close();
        }

        ImGui::End();
        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    connection_fields fields;
    fields.remote_address = boost::asio::ip::address_v4::from_string(remote_addr);
    fields.remote_port = static_cast<unsigned short>(remote_port);
    fields.local_port = static_cast<unsigned short>(local_port);
    fields.username = std::string(username);
    fields.password = std::string(password);


    return {fields};
}
