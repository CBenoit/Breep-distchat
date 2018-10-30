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

#include <sound_def.hpp>
#include <audio_source.hpp>
#include <commands.hpp>
#include <breep/network/tcp.hpp>

#include "peer_recap.hpp"

inline p2pchat::p2pchat(unsigned short local_port, boost::asio::ip::address_v4 connection_address,
                        unsigned short forward_port)
		: p2pchat(local_port) {
	dual_network.connect(connection_address, forward_port);
}

inline void p2pchat::awake() {
	dual_network.awake();
}

inline void p2pchat::send_voice(bool should_send) {
	sending_voice = should_send;
}

inline void p2pchat::mute_sound_input(bool muted) {
	sound_input_muted = muted;
}

inline void p2pchat::network_sound_input_callback(breep::tcp::netdata_wrapper<sound_buffer_t>& value) {
	if (!sound_input_muted) {
		audio_source::play(value.data);
	}
}

inline p2pchat::~p2pchat() {
	should_quit = true;
	dual_network.disconnect();
	if (sound_sender_thread.joinable()) {
		sound_sender_thread.join();
	}
}

template<typename T>
void p2pchat::add_callback(callback <T> cb) {
	dual_network.add_data_listener<T>([this, c = std::move(cb)](breep::tcp::netdata_wrapper<T>& value) {

		std::optional<peer_recap> pr;
		peers_map_mutex.lock();
		if (auto it = peers_name_by_id.find(value.source.id()) ; it != peers_name_by_id.end()) {
			pr = peer_recap(it->second, value.source.id(), connection_state::accepted);
		}
		peers_map_mutex.unlock();

		if (pr) {
			c(value.data, *pr);
		}
	});
}

inline void p2pchat::add_connection_callback(connection_callback cb) {
	std::lock_guard lg{connection_mutex};
	co_listeners.emplace_back(std::move(cb));
}

inline void p2pchat::add_disconnection_callback(connection_callback cb) {
	std::lock_guard lg{disconnection_mutex};
	dc_listeners.emplace_back(std::move(cb));
}

inline void p2pchat::add_sound_target(const std::string& p) {
	std::lock_guard lg(sound_targets_mutex);
	sound_targets.emplace(p);
}

inline void p2pchat::remove_sound_target(const std::string& p) {
	std::lock_guard lg(sound_targets_mutex);
	sound_targets.erase(p);
}

template <typename T>
void p2pchat::send_to(const std::string& target, T&& value) {
	std::lock_guard lg(peers_map_mutex);
	if (auto it = peers_by_name.find(target) ; it != peers_by_name.end()) {
		dual_network.send_object_to<T>(it->second, value);
	}
}
