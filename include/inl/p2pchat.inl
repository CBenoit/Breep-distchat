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

#include <p2pchat.hpp>
#include <sound_def.hpp>
#include <audio_source.hpp>

#include "p2pchat.hpp"

inline p2pchat::p2pchat(unsigned short local_port)
		: dual_network{local_port}
		, sound_sender_thread{[this](){local_sound_input();}}
{
	dual_network.add_data_listener<sound_buffer_t>([this](auto& value) { network_sound_input_callback(value); });
	dual_network.add_connection_listener([this](auto&, const breep::tcp::peer& p) {
		std::lock_guard lg(peers_map_mutex);
		peers_map.emplace(p.id(), p);
	});
	dual_network.add_disconnection_listener([this](auto&, const breep::tcp::peer& p) {
		peers_map_mutex.lock();
		peers_map.erase(p.id());
		peers_map_mutex.unlock();
		std::lock_guard lg(sound_targets_mutex);
		sound_targets.erase(p.id());
	})
}

inline p2pchat::p2pchat(unsigned short local_port, boost::asio::ip::address_v4 connection_address,
                                 unsigned short forward_port)
		: p2pchat(local_port)
{
	dual_network.connect(connection_address, forward_port);
}

inline void p2pchat::send_voice(bool should_send) {
	sending_voice = true;
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
			std::scoped_lock sl(sound_targets_mutex, peers_map_mutex)
			for (auto&& sound_target : sound_targets) {
				s_sender.send_sample_to(dual_network, peers_map.at(sound_target));
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(90));
	}
}

inline p2pchat::~p2pchat() {
	should_quit = true;
	if (sound_sender_thread.joinable()) {
		sound_sender_thread.join();
	}
}

template<typename T>
void p2pchat::send_to(const breep::tcp::peer& target, const T& value) {
	static_assert(!std::is_same_v<std::remove_cv_t<T>, char*>, "You should not send char*")
	dual_network.send_object_to(target, value);
}

template<typename T>
void p2pchat::add_callback(callback<T> cb) {
	return dual_network.add_data_listener([c = std::move(cb)] (breep::tcp::netdata_wrapper<T>& value) {
		c(value.data, value.source);
	})
}

inline void p2pchat::remove_callback(const p2pchat::callback_id& cb_id) {
	return dual_network.remove_data_listener(cb_id);
}

inline p2pchat::connection_callback_id p2pchat::add_connection_callback(connection_callback cb) {
	return dual_network.add_connection_listener([this, c = std::move(cb)](auto&, const breep::tcp::peer& peer) {
		c(*this, peer);
	});
}

inline bool p2pchat::remove_connection_callback(const connection_callback_id& ccb_id) {
	return dual_network.remove_connection_listener(ccb_id);
}

inline p2pchat::connection_callback_id p2pchat::add_disconnection_callback(connection_callback cb) {
	return dual_network.add_disconnection_listener([this, c = std::move(cb)](auto&, const breep::tcp::peer& peer) {
		c(peer);
	});
}

inline bool p2pchat::remove_disconnection_callback(const connection_callback_id& dcb_id) {
	return dual_network.remove_disconnection_listener(ccb_id);
}

inline bool p2pchat::connect_to(boost::asio::ip::address_v4 addr, unsigned short forward_port) {
	return dual_network.connect(addr, forward_port);
}

inline void p2pchat::add_sound_target(const breep::tcp::peer& p) {
	add_sound_target(p.id());
}

inline void p2pchat::add_sound_target(const boost::uuids::uuid& p) {
	std::lock_guard lg(sound_targets_mutex);
	sound_targets.emplace(p);
}

inline void p2pchat::remove_sound_target(const breep::tcp::peer& p) {
	remove_sound_target(p.id());
}

inline void p2pchat::remove_sound_target(const boost::uuids::uuid& p) {
	std::lock_guard lg(sound_targets_mutex);
	sound_targets.erase(p)
}
