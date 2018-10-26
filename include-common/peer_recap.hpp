#ifndef SYSDIST_SERVER_PEER_RECAP_HPP
#define SYSDIST_SERVER_PEER_RECAP_HPP

#include <string>
#include <boost/uuid/uuid.hpp>
#include <breep/util/serialization.hpp>
#include <breep/util/type_traits.hpp>

class peer_recap {
public:
	peer_recap() = default;
	explicit peer_recap(std::string name) : username(std::move(name)) {}
	explicit peer_recap(const boost::uuids::uuid& id) : uuid(id) {}
	peer_recap(std::string name, const boost::uuids::uuid& id) : username(std::move(name)), uuid(id) {}
	peer_recap(const boost::uuids::uuid& id, std::string name) : username(std::move(name)), uuid(id) {}

	const std::string& name() const { return username; }
	const boost::uuids::uuid& id() const { return uuid; }

private:
	std::string username{};
	boost::uuids::uuid uuid{boost::uuids::nil_uuid()};

	BREEP_ENABLE_SERIALIZATION(peer_recap, username, uuid)
};

BREEP_DECLARE_TYPE(peer_recap)

#endif //SYSDIST_SERVER_PEER_RECAP_HPP
