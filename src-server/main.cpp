#include <breep/network/tcp.hpp>
#include <boost/uuid/uuid.hpp>

#include "peer_recap.hpp"
#include "commands.hpp"
#include "peer_info.hpp"

int main(int, char*[]) {
	std::ios::sync_with_stdio(false);

	constexpr auto scan_interval = std::chrono::seconds(15);

	uint16_t chat_port;
	std::cout << "Chat server port: ";
	std::cin >> chat_port;
	breep::tcp::network chat_network(chat_port);

	uint16_t auth_port;
	std::cout << "Authentication server port: ";
	std::cin >> auth_port;
	breep::tcp::network auth_network(auth_port);

	auth_network.set_log_level(breep::log_level::debug);
	chat_network.set_log_level(breep::log_level::debug);

	std::unordered_map<std::string, peer_info> existing_peers;
	std::mutex connected_peers_mutex;
	std::unordered_map<boost::uuids::uuid, peer_recap, boost::hash<boost::uuids::uuid>> connected_peers_uuids;

	std::mutex pending_peers_mutex;
	std::unordered_map<boost::uuids::uuid, peer_recap, boost::hash<boost::uuids::uuid>> pending_peers;

	auth_network.add_data_listener<create_account>([&](breep::tcp::netdata_wrapper<create_account>& data) {

		connected_peers_mutex.lock();
		const auto connected_count = connected_peers_uuids.count(data.source.id());
		connected_peers_mutex.unlock();

		pending_peers_mutex.lock();
		const auto pending_count = pending_peers.count(data.source.id());
		pending_peers_mutex.unlock();

		if (connected_count || pending_count) {
			data.network.send_object_to(data.source, connection_results::user_already_connected);

		} else if (existing_peers.count(data.data.username)) {
			data.network.send_object_to(data.source, connection_results::user_already_exists);

		} else {
			existing_peers.emplace(data.data.username, peer_info{data.source.id(), data.data.username, data.data.password});

			auto peer = peer_recap(data.source.id(), data.data.username, connection_state::accepted);
			chat_network.send_object(peer);

			pending_peers_mutex.lock();
			pending_peers.emplace(data.source.id(), peer);
			pending_peers_mutex.unlock();

			data.network.send_object_to(data.source, connection_results::accepted(chat_port));
		}
	});

	auth_network.add_data_listener<connect_account>([&](breep::tcp::netdata_wrapper<connect_account>& data) {

		connected_peers_mutex.lock();
		const auto connected_count = connected_peers_uuids.count(data.source.id());
		connected_peers_mutex.unlock();

		pending_peers_mutex.lock();
		const auto pending_count = pending_peers.count(data.source.id());
		pending_peers_mutex.unlock();

		if (connected_count || pending_count) {
			data.network.send_object_to(data.source, connection_results::user_already_connected);
			return;
		}
		try {
			peer_info& pi = existing_peers.at(data.data.username);
			if (pi.password_matches(data.data.password)) {

				auto peer = peer_recap(data.source.id(), data.data.username, connection_state::accepted);
				chat_network.send_object(peer);

				pending_peers_mutex.lock();
				pending_peers.emplace(data.source.id(), peer);
				pending_peers_mutex.unlock();

				data.network.send_object_to(data.source, connection_results::accepted(chat_port));
			} else {
				data.network.send_object_to(data.source, connection_results::bad_password);
			}
		} catch (const std::out_of_range& e) {
			data.network.send_object_to(data.source, connection_results::no_such_account);
		}
	});

	chat_network.add_disconnection_listener([&connected_peers_uuids, &connected_peers_mutex](breep::tcp::network&, const breep::tcp::peer& p) {
		std::lock_guard lg(connected_peers_mutex);
		connected_peers_uuids.erase(p.id());
	});

	chat_network.add_connection_listener([&pending_peers, &pending_peers_mutex, &connected_peers_uuids, &connected_peers_mutex](breep::tcp::network& n, const breep::tcp::peer& p) {

		connected_peers_mutex.lock();
		for (auto&& peers_pair : connected_peers_uuids) {
			n.send_object_to(p, peers_pair.second);
		}
		pending_peers_mutex.lock();
		connected_peers_uuids.insert(pending_peers.extract(p.id()));
		pending_peers_mutex.unlock();
		connected_peers_mutex.unlock();
	});

	chat_network.set_connection_predicate([&pending_peers, &pending_peers_mutex](const breep::tcp::peer& p) {
		std::lock_guard ls(pending_peers_mutex);
		return pending_peers.count(p.id()) != 0;
	});

	chat_network.awake();
	auth_network.awake();

	while (chat_network.is_running() && auth_network.is_running()) {

		pending_peers_mutex.lock();
		auto it = pending_peers.begin();
		auto end = pending_peers.end();

		while (it != end) {
			if (it->second.should_accept()) {
				it->second.set_state(connection_state::unknown_error);
				++it;
			} else {
				chat_network.send_object(it->second);
				auto curr = it;
				++it;
				pending_peers.erase(curr);
			}
		}
		pending_peers_mutex.unlock();

		std::this_thread::sleep_for(scan_interval);
	}
}
