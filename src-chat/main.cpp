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
#include <breep/network/tcp.hpp>
#include <audio_source.hpp>
#include <display/connection_gui.hpp>

#include "sound_def.hpp"
#include "sound_sender.hpp"
#include "flow_controller.hpp"
#include "display/main_gui.hpp"
#include "p2pchat.hpp"

BREEP_DECLARE_TYPE(std::string)

void sender();
void receiver();

int main(int argc,char* argv[]) {
    std::optional<display::connection_fields> fields = display::connection_gui().show();

    std::unique_ptr<p2pchat> chat = nullptr;
    if (fields.has_value()) {
        chat = std::make_unique<p2pchat>(fields.value().local_port);
    } else {
        chat = std::make_unique<p2pchat>(1234);
    }

    display::main_gui gui;
    audio_source::init();

	std::unique_ptr<breep::tcp::peer> peer = nullptr;
	chat->add_connection_callback([&peer, &gui](const breep::tcp::peer& lp) {
		gui.system_message("Connected: " + lp.id_as_string());
		peer = std::make_unique<breep::tcp::peer>(lp);
	});

	chat->add_disconnection_callback([&peer, &gui](const breep::tcp::peer& lp) {
		gui.system_message("Disconnected: " + lp.id_as_string());
		peer = nullptr;
	});

	chat->add_callback<std::string>([&gui](const std::string& data, const breep::tcp::peer& source) {
		gui.add_message(source.id_as_string(), data);
	});

	if (fields.has_value()) {
        if (!chat->connect_to(fields.value().remote_address, fields.value().remote_port)) {
            throw "Failed to connect.\n";
        }
    } else {
	    chat->awake();
	}

	gui.set_textinput_callback([&gui, &chat, &peer](std::string_view v) {
		std::string s(v);
		gui.add_message(chat->me().id_as_string().data(), s);
		if (peer) {
			chat->send_to(*peer, s);
		}
	});

	while (gui.display());

	return 0;
}