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

#include <sound_def.hpp>
#include <sound_sender.hpp>
#include <flow_controller.hpp>
#include <display.hpp>
#include <p2pchat.hpp>

BREEP_DECLARE_TYPE(std::string)

void sender();
void receiver();

int main(int argc,char* argv[]) {

	display::gui gui;
	audio_source::init();

	unsigned short p;
	std::cout << "Local port: ";
	std::cin >> p;
	p2pchat chat(p);

	std::unique_ptr<breep::tcp::peer> peer = nullptr;
	chat.add_connection_callback([&peer, &gui](const breep::tcp::peer& lp) {
		gui.system_message("Connected: " + lp.id_as_string());
		peer = std::make_unique<breep::tcp::peer>(lp);
	});

	chat.add_callback<std::string>([&gui](const std::string& data, const breep::tcp::peer& source) {
		gui.add_message(source.id_as_string(), data);
	});

	std::cout << "Connect to remote [0/1]: ";
	std::cin >> p;
	if (p == 1) {
		std::string addr;
		std::cout << "Remote ipv4 address: ";
		std::cin >> addr;
		std::cout << "Remote port: ";
		std::cin >> p;
		if (!chat.connect_to(boost::asio::ip::address_v4::from_string(addr), p)) {
			throw "Failed to connect.\n";
		}
	} else {
		chat.awake();
	}

	gui.set_textinput_callback([&gui, &chat, &peer](std::string_view v) {
		std::string s(v);
		gui.add_message(chat.me().id_as_string().data(), s);
		if (peer) {
			chat.send_to(*peer, s);
		}
	});

	while (gui.display());

    /*if (argc >= 2) {
        if (std::string(argv[1]) == "sender") {
            sender();
            return 0;
        } else if (std::string(argv[1]) == "receiver") {
            receiver();
            return 0;
        }
    }*/

	return 0;
}

void receiver() {

	breep::tcp::network network(1233);
	network.set_log_level(breep::log_level::info);

	network.add_data_listener<sound_buffer_t>([](breep::tcp::netdata_wrapper<sound_buffer_t>& value) {
		audio_source::play(value.data);
	});

	network.sync_connect(boost::asio::ip::address_v4::loopback(), 1234);

}

void sender() {

	breep::tcp::network network(1234);
	network.set_log_level(breep::log_level::info);
	network.awake();

	sound_sender ssender{};

	volatile bool b{true};
	while (b) {
		ssender.send_sample(network);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
