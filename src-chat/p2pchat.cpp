#include "p2pchat.hpp"
#include "commands.hpp"
#include "peer_recap.hpp"

p2pchat::p2pchat(unsigned short local_port)
		: dual_network{local_port}, sound_sender_thread{[this]() { local_sound_input(); }} {

	dual_network.set_log_level(breep::log_level::debug);
}

void p2pchat::local_sound_input() {
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


connection_state p2pchat::connect_to(const connection_fields& cfields) {
	if (!cfields) {
		return connection_state::unknown_error;
	}

	connection_result state(connection_state::unknown_error);

	dual_network.add_data_listener<connection_result>([&state](breep::tcp::netdata_wrapper<connection_result>& data) {
		data.network.remove_data_listener<connection_result>(data.listener_id);
		state = data.data;
		data.network.disconnect();
	});

	if (!dual_network.connect(*cfields.remote_address, *cfields.remote_port)) {
		return connection_state::unknown_error;
	}

	if (*cfields.account_creation) {
		create_account ca;
		ca.password = *cfields.password;
		ca.username = *cfields.username;
		dual_network.send_object(ca);
	} else {
		connect_account ca;
		ca.password = *cfields.password;
		ca.username = *cfields.username;
		dual_network.send_object(ca);
	}
	dual_network.join();

	if (state.state != connection_state::accepted) {
		return state.state;
	}

	setup_listeners();
	if (!dual_network.connect(*cfields.remote_address, state.port)) {
		clear_listeners();
		return connection_state::unknown_error;
	}
	local_name = *cfields.username;
	return connection_state::accepted;
}

void p2pchat::setup_listeners() {

	dual_network.set_connection_predicate([this](const breep::tcp::peer& new_peer) {
		if (server_id) {
			std::cerr << "Incomming: " << new_peer.id() << ": " << std::boolalpha << (pending_peers.count(new_peer.id()) > 0) << '\n';
			return pending_peers.count(new_peer.id()) > 0;
		}
		return true;
	});

	dual_network.add_connection_listener([this](breep::tcp::network& n, const breep::tcp::peer& new_peer) {
		if (server_id) {
			std::cerr << "Connecting: " << new_peer.id() << ": " << std::boolalpha << (pending_peers.count(new_peer.id()) > 0) << '\n';
			auto it = pending_peers.find(new_peer.id());
			if (it != pending_peers.end()) {
				peers_map_mutex.lock();
				peers_by_name.emplace(it->second.name(), new_peer);
				peers_name_by_id.emplace(new_peer.id(), it->second.name());
				peers_map_mutex.unlock();

				std::lock_guard lg(connection_mutex);
				for (auto&& listener : co_listeners) {
					listener(it->second);
				}

				pending_peers.erase(it);
			} else {
				unmapped_peers.emplace(new_peer.id(), new_peer);
				if (unmapped_peers.size() == 1) {
					n.send_object_to(server_id.value(), get_info(new_peer.id()));
				}
			}

		} else {
			server_id.emplace(new_peer);
		}
	});

	dual_network.add_data_listener<peer_recap>([this](breep::tcp::netdata_wrapper<peer_recap>& recap) {
		std::cerr << "Got peer recap: " << recap.data.name() << "\t" << recap.data.id() << ": " << std::boolalpha << (unmapped_peers.count(recap.data.id()) > 0) << '\n';
		if (auto it = unmapped_peers.find(recap.data.id()) ; it != unmapped_peers.end()) {
			peers_map_mutex.lock();
			peers_by_name.emplace(recap.data.name(), it->second);
			peers_name_by_id.emplace(recap.data.id(), recap.data.name());
			peers_map_mutex.unlock();

			std::lock_guard lg(connection_mutex);
			for (auto&& listener : co_listeners) {
				listener(recap.data);
			}

			unmapped_peers.erase(it);

			if (!unmapped_peers.empty()) {
				recap.network.send_object_to(server_id.value(), get_info(unmapped_peers.begin()->first));
			}
		} else {
			pending_peers.emplace(recap.data.id(), recap.data);
		}
	});

	dual_network.add_data_listener<sound_buffer_t>([this](auto& value) { network_sound_input_callback(value); });

	dual_network.add_disconnection_listener([this](auto&, const breep::tcp::peer& p) {
		std::optional<peer_recap> pr;

		peers_map_mutex.lock();
		if (auto it = peers_name_by_id.find(p.id()) ; it != peers_name_by_id.end()) {
			peers_by_name.erase(it->second);

			sound_targets_mutex.lock();
			sound_targets.erase(it->second);
			sound_targets_mutex.unlock();

			pr = peer_recap(std::move(it->second), p.id());
			peers_name_by_id.erase(it);
		}
		peers_map_mutex.unlock();

		pending_peers.erase(p.id());

		if (pr) {
			std::lock_guard lg(disconnection_mutex);
			for (auto&& listener : dc_listeners) {
				listener(*pr);
			}
		}
	});
}

void p2pchat::clear_listeners() {
	dual_network.clear_any();
	dual_network.remove_connection_predicate();
}
