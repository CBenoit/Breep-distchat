#ifndef SYSDIST_SERVER_COMMANDS_HPP
#define SYSDIST_SERVER_COMMANDS_HPP

#include <breep/util/serialization.hpp>

// Send to server to create an account
struct create_account {
	std::string username{};
	std::string password{};

	BREEP_ENABLE_SERIALIZATION(create_account, username, password)
};
// answer: connection_result


// Send to server to try to connect
struct connect_account {
	std::string username{};
	std::string password{};

	BREEP_ENABLE_SERIALIZATION(connect_account, username, password)
};
// answer: connection_result

enum class connection_state : uint8_t {
	accepted,
	no_such_account,
	bad_password,
	user_already_exists,
	user_already_connected,
	unknown_error,
};

struct connection_result {
	explicit connection_result(connection_state st = connection_state::user_already_connected, unsigned short p = 0)
			: state{st}, port{p} {}

	connection_state state;
	unsigned short port;

	BREEP_ENABLE_SERIALIZATION(connection_result, state, port)
};

namespace connection_results {
	inline const connection_result no_such_account{connection_state::no_such_account};
	inline const connection_result bad_password{connection_state::bad_password};
	inline const connection_result user_already_exists{connection_state::user_already_exists};
	inline const connection_result user_already_connected{connection_state::user_already_connected};

	inline connection_result accepted(unsigned short p) {
		return connection_result(connection_state::accepted, p);
	}
}

BREEP_DECLARE_TYPE(connection_result)
BREEP_DECLARE_TYPE(connection_state)
BREEP_DECLARE_TYPE(connect_account)
BREEP_DECLARE_TYPE(create_account)

#endif //SYSDIST_SERVER_COMMANDS_HPP
