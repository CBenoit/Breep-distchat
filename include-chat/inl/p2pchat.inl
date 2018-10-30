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

inline p2pchat::p2pchat(unsigned short local_port)
		: dual_network{local_port}, sound_sender_thread{[this]() { local_sound_input(); }} {

	dual_network.set_log_level(breep::log_level::debug);
	dual_network.add_data_listener<sound_buffer_t>([this](auto& value) { network_sound_input_callback(value); });

	dual_network.add_disconnection_listener([this](auto&, const breep::tcp::peer& p) {
		std::optional<peer_recap> pr;

		peers_map_mutex.lock();
		if (auto it = peers_name_by_id.find(p.id()) ; it != peers_name_by_id.end()) {
			peers_by_name.erase(it->second);
			sound_targets_mutex.lock();
			sound_targets.erase(it->second);
			sound_targets_mutex.unlock();
			peers_name_by_id.erase(it);
			pr = peer_recap(it->second, p.id());
		}
		peers_map_mutex.unlock();

		if (pr) {
			std::lock_guard lg(disconnection_mutex);
			for (auto&& listener : dc_listeners) {
				listener(*pr);
			}
		}
	});

	dual_network.add_data_listener<peer_recap>([this](breep::tcp::netdata_wrapper<peer_recap>& recap) {
		peers_map_mutex.lock();
		peers_by_name.emplace(recap.data.name(), recap.source);
		peers_name_by_id.emplace(recap.source.id(), recap.data.name());
		peers_map_mutex.unlock();

		std::lock_guard lg(connection_mutex);
		for (auto&& listener : co_listeners) {
			listener(recap.data);
		}
	});
}

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

inline void p2pchat::local_sound_input() {
	while (!should_quit) {
		if (sending_voice) {
			std::scoped_lock sl(sound_targets_mutex, peers_map_mutex);
			s_sender.update_sample();
			for (auto&& sound_target : sound_targets) {
				if (auto it = peers_by_name.find(sound_target) ; it != peers_by_name.end()) {
					s_sender.send_sample_to(dual_network, it->second);
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(90));
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
