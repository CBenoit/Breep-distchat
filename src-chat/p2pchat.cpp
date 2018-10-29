#include "p2pchat.hpp"
#include "commands.hpp"
#include "peer_recap.hpp"

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
		return connection_state ::unknown_error;
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

	if (!dual_network.connect(*cfields.remote_address, state.port)) {
		return connection_state::unknown_error;
	}
	return connection_state::accepted;
}
