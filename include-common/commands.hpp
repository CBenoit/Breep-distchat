#ifndef SYSDIST_SERVER_COMMANDS_HPP
#define SYSDIST_SERVER_COMMANDS_HPP

#include <breep/util/serialization.hpp>

#include "peer_recap.hpp"

struct create_account {
	std::string username;
	std::string password;
	BREEP_ENABLE_SERIALIZATION(create_account, username, password)
};

struct connect_account {
	std::string username;
	std::string password;
	BREEP_ENABLE_SERIALIZATION(connect_account, username, password)
};

enum class connection_state : char {
	refused,
	accepted
};

BREEP_DECLARE_TEMPLATE(std::pair)

BREEP_DECLARE_TYPE(connection_state)
BREEP_DECLARE_TYPE(connect_account)
BREEP_DECLARE_TYPE(create_account)

#endif //SYSDIST_SERVER_COMMANDS_HPP
