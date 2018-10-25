#include <breep/network/tcp.hpp>
#include <boost/uuid/uuid.hpp>

#include "peer_recap.hpp"
#include "commands.hpp"
#include "peer_info.hpp"


int main(int, char*[]) {
	unsigned short port;
	std::cout << "Listening port: ";
	std::cin >> port;
	breep::tcp::network network(port);

	std::unordered_map<std::string, peer_info> existing_peers;
	std::unordered_set<boost::uuids::uuid, boost::hash<boost::uuids::uuid>> connected_peers_uuids;

	network.add_data_listener<create_account>([&existing_peers, & connected_peers_uuids](breep::tcp::netdata_wrapper<create_account>& data) {
		if (connected_peers_uuids.count(data.source.id())) {
			data.network.send_object(std::make_pair(connection_state::refused, peer_recap(data.source.id())));
		} else if (existing_peers.count(data.data.username)){
			data.network.send_object(std::make_pair(connection_state::refused, peer_recap(data.data.username)));
		} else {
			existing_peers.emplace(data.data.username, peer_info{data.source.id(), data.data.username, data.data.password});
			connected_peers_uuids.emplace(data.source.id());
			data.network.send_object(std::make_pair(connection_state::accepted, peer_recap(data.source.id(), data.data.username)));
		}
	});
	network.add_data_listener<connect_account>([](breep::tcp::netdata_wrapper<connect_account>& data) {
	});
}
