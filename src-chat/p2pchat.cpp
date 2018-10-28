#include "p2pchat.hpp"
#include "commands.hpp"
#include "peer_recap.hpp"

bool p2pchat::connect_to(const connection_fields& cfields) {
	if (!cfields) {
		return false;
	}

	using refused = std::pair<connection_state, peer_recap>;
	using accepted = std::pair<connection_state, unsigned short>;

	std::optional<refused>  r_value;
	std::optional<accepted> a_value;

	dual_network.set_unlistened_type_listener([](auto&&...) {
		std::cout << "OH NOES!!\n";
	});

	dual_network.add_data_listener<refused>([&r_value](breep::tcp::netdata_wrapper<refused>& data){
		r_value = data.data;
		data.network.remove_data_listener<refused>(data.listener_id);
		data.network.disconnect();
	});
	dual_network.add_data_listener<accepted>([&a_value](breep::tcp::netdata_wrapper<accepted>& data){
		a_value = data.data;
		data.network.remove_data_listener<accepted>(data.listener_id);
		data.network.disconnect();
	});

	if (!dual_network.connect(*cfields.remote_address, *cfields.remote_port)) {
		return false;
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

	if (r_value) {
		// TODO: extract refusal explaination
		return false;
	}

	if (!a_value) {
		// FIXME: do something (unexcepted)
		return false;
	}

	return dual_network.connect(*cfields.remote_address, a_value->second);
}
