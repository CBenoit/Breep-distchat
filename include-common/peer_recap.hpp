#ifndef SYSDIST_SERVER_PEER_RECAP_HPP
#define SYSDIST_SERVER_PEER_RECAP_HPP

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <breep/util/serialization.hpp>
#include <breep/util/type_traits.hpp>

#include "uuid_serialize.hpp"
#include "commands.hpp"

class peer_recap {
public:
	peer_recap() = default;
	peer_recap(std::string name, const boost::uuids::uuid& id = boost::uuids::nil_uuid(), connection_state st = connection_state::unknown_error)
			: peer_recap(id, name, st) {}
	peer_recap(const boost::uuids::uuid& id, std::string name = "", connection_state st = connection_state::unknown_error)
			: username(std::move(name)), uuid(id), state(st) {}

	const std::string& name() const noexcept { return username; }
	const boost::uuids::uuid& id() const noexcept { return uuid; }
	bool should_accept() const noexcept { return state == connection_state::accepted; }

	void set_state(connection_state st) { state = st; }

private:
	std::string username{};
	boost::uuids::uuid uuid{boost::uuids::nil_uuid()};
	connection_state state{connection_state::unknown_error};

	BREEP_ENABLE_SERIALIZATION(peer_recap, username, uuid, state)
};

BREEP_DECLARE_TYPE(peer_recap)

#endif //SYSDIST_SERVER_PEER_RECAP_HPP
