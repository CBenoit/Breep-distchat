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
#include <map>

#include "sound_sender.hpp"
#include "connection_fields.hpp"
#include "peer_recap.hpp"

enum class connection_state : uint8_t;

class p2pchat {

public:

	template<typename T>
	using callback = std::function<void(const T&, const peer_recap&)>;
	using connection_callback = std::function<void(const peer_recap&)>;

	explicit p2pchat(unsigned short local_port);

	p2pchat(unsigned short local_port, boost::asio::ip::address_v4 connection_address, unsigned short forward_port);

	void awake();

	connection_state connect_to(const connection_fields& cfields);

	void send_voice(bool should_send = true);

	void mute_sound_input(bool muted);

	void add_sound_target(const std::string& p);

	void remove_sound_target(const std::string& p);

	template<typename T>
	void add_callback(callback<T>);

	void add_connection_callback(connection_callback);

	void add_disconnection_callback(connection_callback);

	const std::string& me() const {
		return local_name;
	}

	~p2pchat();

	template <typename T>
	void send_to(const std::string& target, T&& value);

private:

	void network_sound_input_callback(breep::tcp::netdata_wrapper<sound_buffer_t>&);

	void local_sound_input();

	std::string local_name{};

	breep::tcp::network dual_network;
	std::mutex peers_map_mutex{};
	std::unordered_map<std::string, breep::tcp::peer> peers_by_name{};
	std::unordered_map<boost::uuids::uuid, std::string, boost::hash<boost::uuids::uuid>> peers_name_by_id{};

	std::mutex connection_mutex{};
	std::mutex disconnection_mutex{};
	std::vector<connection_callback> co_listeners;
	std::vector<connection_callback> dc_listeners;

	sound_sender s_sender{};

	std::atomic_bool sending_voice{false};
	std::atomic_bool sound_input_muted{false};

	std::atomic_bool should_quit{false};
	std::mutex sound_targets_mutex{};
	std::unordered_set<std::string> sound_targets{};
	std::thread sound_sender_thread{};

};

#include "inl/p2pchat.inl"

#endif //SYSDIST_CHAT_DUAL_CONNECTION_HPP
