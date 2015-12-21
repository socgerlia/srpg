#pragma once

namespace rrpc{

enum class direction : uint8_t{
	none = 0,
	request,
	response
};
using session_id_type = uint8_t;
using operation_id_type = uint8_t;
using interface_id_type = uint8_t;
using method_id_type = uint8_t;
using arity_type = uint8_t;

struct header{
	direction dir;
	session_id_type sessionId;
	operation_id_type operationId;

	interface_id_type interfaceId;
	method_id_type methodId;
	arity_type arity;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version){
		ar & dir;
		ar & sessionId;
		ar & operationId;

		if(dir == direction::request){
			ar & interfaceId;
			ar & methodId;
			ar & arity;
		}
	}	
};

inline std::ostream& operator<<(std::ostream& os, const header& h){
	os
		<< (h.dir == direction::request ? "->" : "<-")
		<< " {sessionId: " << (int)h.sessionId
		<< ", operationId: " << (int)h.operationId
		<< ", interfaceId: " << (int)h.interfaceId
		<< ", methodId: " << (int)h.methodId
		<< ", arity: " << (int)h.arity
		<< "}"
	;
	return os;
}

} // end namespace rrpc
