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

#include <list>
#include <array>

#include <AL/al.h>
#include <AL/alc.h>
#include <audio_source.hpp>
#include <display/connection_gui.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/container_hash/extensions.hpp>

#include "sound_def.hpp"
#include "sound_sender.hpp"
#include "flow_controller.hpp"
#include "display/main_gui.hpp"
#include "p2pchat.hpp"
#include "peer_recap.hpp"
#include "connection_fields.hpp"
#include "display/formatted_message.hpp"

BREEP_DECLARE_TYPE(std::string)

void sender();

void receiver();

int main(int argc, char* argv[]) {

	connection_fields fields;
	for (display::connection_gui cgui; !fields; fields = cgui.show(fields)) {}

	display::main_gui gui;
	p2pchat chat{*fields.local_port};

	audio_source::init();

	chat.add_connection_callback([&gui](const peer_recap& pr) {
		gui.add_user(pr.name());
		gui.add_message(pr.name(), display::system_message(pr.name() + " connected."));
	});

	chat.add_disconnection_callback([&gui](const peer_recap& pr) {
		gui.add_message(pr.name(), display::system_message(pr.name() + " disconnected."));
		gui.remove_user(pr.name());
	});

	chat.add_callback<std::string>([&gui](const std::string& data, const peer_recap& source) {
		gui.add_message(source.name(), display::user_message{source.name(), data});
	});

	if (connection_state st = chat.connect_to(fields); st != connection_state::accepted) {
		std::cout << "Failed to connect.\n";
		switch (st) {
			case connection_state::no_such_account:
				std::cout << "Reason: no_such_account\n";
				break;
			case connection_state::bad_password:
				std::cout << "Reason: bad_password\n";
				break;
			case connection_state::user_already_exists:
				std::cout << "Reason: user_already_exists\n";
				break;
			case connection_state::user_already_connected:
				std::cout << "Reason: user_already_connected\n";
				break;
			case connection_state::unknown_error:
				[[fallthrough]];
			default:
				std::cout << "Reason: unknown_error\n";
				break;
		}
		return 1;
	}

	gui.set_textinput_callback([&gui, &chat](std::string_view v) {
		std::string s(v);
		gui.add_message(*gui.get_focused_name(), display::user_message(chat.me(), v));

		if (gui.get_focused_name()) {
			chat.send_to(gui.get_focused_name().value(), s);
		}
	});

	while (gui.display());

	return 0;
}
