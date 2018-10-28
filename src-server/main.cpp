#include <breep/network/tcp.hpp>
#include <boost/uuid/uuid.hpp>

#include "peer_recap.hpp"
#include "commands.hpp"
#include "peer_info.hpp"


int main(int, char*[]) {

	unsigned short chat_port;
	std::cout << "Chat server port: ";
	std::cin >> chat_port;
	breep::tcp::network chat_network(chat_port);

	unsigned short auth_port;
	std::cout << "Authentication server port: ";
	std::cin >> auth_port;
	breep::tcp::network auth_network(auth_port);

	std::unordered_map<std::string, peer_info> existing_peers;
	std::mutex connected_peers_mutex;
	std::unordered_set<boost::uuids::uuid, boost::hash<boost::uuids::uuid>> connected_peers_uuids;

	auth_network.add_data_listener<create_account>([&](breep::tcp::netdata_wrapper<create_account>& data) {
		std::lock_guard lg(connected_peers_mutex);
		if (connected_peers_uuids.count(data.source.id())) {
			data.network.send_object_to(data.source, std::make_pair(connection_state::refused, peer_recap(data.source.id())));
		} else if (existing_peers.count(data.data.username)){
			data.network.send_object_to(data.source, std::make_pair(connection_state::refused, peer_recap(data.data.username)));
		} else {
			existing_peers.emplace(data.data.username, peer_info{data.source.id(), data.data.username, data.data.password});
			connected_peers_uuids.emplace(data.source.id());
			data.network.send_object_to(data.source, connection_state::accepted);
			chat_network.send_object(peer_recap(data.source.id(), data.data.username));
		}
	});

	auth_network.add_data_listener<connect_account>([&](breep::tcp::netdata_wrapper<connect_account>& data) {
		std::lock_guard lg(connected_peers_mutex);
		if (connected_peers_uuids.count(data.source.id())) {
			data.network.send_object(std::make_pair(connection_state::refused, peer_recap(data.source.id())));
			return;
		}
		try {
			peer_info& pi = existing_peers.at(data.data.username);
			if (pi.password_matches(data.data.password)) {
				data.network.send_object_to(data.source, connection_state::accepted);
				chat_network.send_object(peer_recap(data.data.username, data.source.id()));
			} else {
				data.network.send_object_to(data.source, std::make_pair(connection_state::refused, peer_recap{}));
			}
		} catch (const std::out_of_range& e) {
			data.network.send_object_to(data.source, std::make_pair(connection_state::refused, peer_recap(data.data.username)));
		}
	});

	chat_network.add_disconnection_listener([&connected_peers_uuids, &connected_peers_mutex](breep::tcp::network&, const breep::tcp::peer& p) {
		std::lock_guard lg(connected_peers_mutex);
		connected_peers_uuids.erase(p.id());
	});

	chat_network.awake();
	auth_network.sync_awake();
}
