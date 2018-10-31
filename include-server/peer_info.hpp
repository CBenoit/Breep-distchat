#ifndef SYSDIST_SERVER_PEER_INFO_HPP
#define SYSDIST_SERVER_PEER_INFO_HPP

#include <boost/uuid/uuid.hpp>
#include <string>

class peer_info {
public:

	peer_info(const boost::uuids::uuid& id, std::string username_, const std::string& password);

	bool password_matches(const std::string& s) const;

	const std::string& name() const;

	const boost::uuids::uuid& id() const;

private:
	static constexpr std::hash<std::string> hasher{};
	const boost::uuids::uuid uuid;
	const std::size_t password_hash;
	const std::string username;
};


#endif //SYSDIST_SERVER_PEER_INFO_HPP
