
#include <peer_info.hpp>

#include "peer_info.hpp"

peer_info::peer_info(const boost::uuids::uuid& id, std::string username, const std::string& password)
		: uuid(id), password_hash(hasher(password)), username(std::move(username)) {

}

bool peer_info::password_matches(const std::string& s) const {
	return hasher(s) == password_hash;
}

const std::string& peer_info::name() const {
	return username;
}

const boost::uuids::uuid& peer_info::id() const {
	return uuid;
}
