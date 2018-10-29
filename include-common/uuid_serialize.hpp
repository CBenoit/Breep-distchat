#ifndef SYSDIST_SERVER_UUID_SERIALIZE_HPP
#define SYSDIST_SERVER_UUID_SERIALIZE_HPP

#include <boost/uuid/uuid.hpp>
#include <breep/util/serialization.hpp>

breep::serializer& operator<<(breep::serializer& s, const boost::uuids::uuid& id);

inline breep::serializer& operator<<(breep::serializer& s, const boost::uuids::uuid& id) {
	for (const uint8_t& c : id.data) {
		s << c;
	}
	return s;
}

breep::deserializer& operator>>(breep::deserializer& d, boost::uuids::uuid& id);

inline breep::deserializer& operator>>(breep::deserializer& d, boost::uuids::uuid& id) {
	for (uint8_t& c : id.data) {
		d >> c;
	}
	return d;
}

#endif //SYSDIST_SERVER_UUID_SERIALIZE_HPP
