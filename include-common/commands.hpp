#ifndef SYSDIST_SERVER_COMMANDS_HPP
#define SYSDIST_SERVER_COMMANDS_HPP

#include <breep/util/serialization.hpp>

#include "peer_recap.hpp"

// Send to server to create an account
struct create_account {
	std::string username{};
	std::string password{};
	BREEP_ENABLE_SERIALIZATION(create_account, username, password)
};
// answer (broadcasted):
// std::pair<connection_state, peer_recap>
// with connection_state == accepted if all green.
//
// Otherwise:
// if peer_recap.name() is not empty, then the requested name is already mapped to an existing peer
// if peer_recap.id().is_nil() is false, then the source's id is already mapped to a connected peer


// Send to server to try to connect
struct connect_account {
	std::string username{};
	std::string password{};
	BREEP_ENABLE_SERIALIZATION(connect_account, username, password)
};
// answer (broadcasted):
// std::pair<connection_state, peer_recap>
// with connection_state == accepted if all green
//
// Otherwise:
// if peer_recap.name() is not empty, then the requested account name does not exist
// if peer_recap.id().is_nil() is false, then the source's id is already mapped to a connected peer
// if peer_recap.name() is empty and peer_recap.id().is_nil() is true, then the password is not correct

enum class connection_state : char {
	refused,
	accepted
};

BREEP_DECLARE_TEMPLATE(std::pair)

BREEP_DECLARE_TYPE(connection_state)
BREEP_DECLARE_TYPE(connect_account)
BREEP_DECLARE_TYPE(create_account)

#endif //SYSDIST_SERVER_COMMANDS_HPP
