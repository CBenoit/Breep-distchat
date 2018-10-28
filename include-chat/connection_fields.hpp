#ifndef SYSDIST_SERVER_CONNECTION_FIELDS_HPP
#define SYSDIST_SERVER_CONNECTION_FIELDS_HPP

#include <optional>
#include <string>
#include <boost/asio/ip/address.hpp>

struct connection_fields {

	explicit operator bool () const {
		return account_creation && username && password && local_port && remote_address && remote_port;
	}

	std::optional<bool> account_creation{};
	std::optional<std::string> username{};
	std::optional<std::string> password{};
	std::optional<unsigned short> local_port{};
	std::optional<boost::asio::ip::address_v4> remote_address{};
	std::optional<unsigned short> remote_port{};
};

#endif //SYSDIST_SERVER_CONNECTION_FIELDS_HPP
