
using rsp = void*;

extern "C"{

RRPC_API rsp rrpc_open_session(int policy);
RRPC_API void rrpc_connect(rsp p, const char* ip, unsigned short port);
RRPC_API void rrpc_send(rsp p);

}
