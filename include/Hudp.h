#ifndef HEADER_INCLUDE_HUDP
#define HEADER_INCLUDE_HUDP

#include "HudpFlag.h"

namespace hudp {

    // init library
    void Init();
    
    // start thread and recv with ip and port
    bool Start(const std::string& ip,uint16_t port, const recv_back& func);

    void Join();

    // send msg
    bool SendTo(const HudpHandle& handle, uint16_t flag, std::string& msg);
    bool SendTo(const HudpHandle& handle, uint16_t flag, const char* msg, uint32_t len);

    // destroy socket. release resources
    void Close(const HudpHandle& handle);

}

#endif
