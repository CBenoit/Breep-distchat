#ifndef SYSDIST_CHAT_DUAL_CONNECTION_HPP
#define SYSDIST_CHAT_DUAL_CONNECTION_HPP

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

#include <boost/asio/ip/address.hpp>
#include <breep/network/tcp.hpp>
#include <utility>

#include "sound_sender.hpp"
#include "connection_fields.hpp"

class p2pchat {

public:
	template <typename T>
	using callback = std::function<void(const T&, const breep::tcp::peer&)>;
	using callback_id = breep::type_listener_id;
	using connection_callback = std::function<void(const breep::tcp::peer&)>;
	using connection_callback_id = breep::listener_id;

	explicit p2pchat(unsigned short local_port);
	p2pchat(unsigned short local_port, boost::asio::ip::address_v4 connection_address, unsigned short forward_port);

	void awake();

	bool connect_to(boost::asio::ip::address_v4, unsigned short forward_port);

	bool connect_to(const connection_fields& cfields);

	void send_voice(bool should_send = true);
	void mute_sound_input(bool muted);

	void add_sound_target(const breep::tcp::peer& p);
	void add_sound_target(const boost::uuids::uuid& p);
	void remove_sound_target(const breep::tcp::peer& p);
	void remove_sound_target(const boost::uuids::uuid& p);

	template<typename T>
	void send_to(const breep::tcp::peer&, const T& value);

	template<typename T>
	callback_id add_callback(callback<T>);

	/* returns true if a listener was removed */
	bool remove_callback(const callback_id& cb_id);

	connection_callback_id add_connection_callback(connection_callback);

	/* returns true if a listener was removed */
	bool remove_connection_callback(const connection_callback_id& ccb_id);

	connection_callback_id add_disconnection_callback(connection_callback);

	/* returns true if a listener was removed */
	bool remove_disconnection_callback(const connection_callback_id& dcb_id);

	const breep::tcp::peer& me() {
		return dual_network.self();
	}

	~p2pchat();

private:

	void network_sound_input_callback(breep::tcp::netdata_wrapper<sound_buffer_t>&);
	void local_sound_input();

	breep::tcp::network dual_network;
	std::mutex peers_map_mutex{};
	std::unordered_map<boost::uuids::uuid, breep::tcp::peer, boost::hash<boost::uuids::uuid>> peers_map{};

	sound_sender s_sender{};

	std::atomic_bool sending_voice{false};
	std::atomic_bool sound_input_muted{false};

	std::atomic_bool should_quit{false};
	std::mutex sound_targets_mutex{};
	std::unordered_set<boost::uuids::uuid, boost::hash<boost::uuids::uuid>> sound_targets{};
	std::thread sound_sender_thread{};

};

#include "inl/p2pchat.inl"

#endif //SYSDIST_CHAT_DUAL_CONNECTION_HPP
