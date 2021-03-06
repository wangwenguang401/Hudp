#ifndef HEADER_NET_SOCKETMANAGER
#define HEADER_NET_SOCKETMANAGER

#include <memory>
#include <unordered_map>

#include "CommonType.h"
#include "CommonFlag.h"
#include "ISocketManager.h"

namespace hudp {

    class CSocket;
    // not thread safe
    class CSocketManagerImpl : public CSocketManager {
    public:
        CSocketManagerImpl();
        ~CSocketManagerImpl();
    
        bool IsSocketExist(const HudpHandle& handle);
        // return a socket, create one if not exist.
        std::shared_ptr<CSocket> GetSocket(const HudpHandle& handle);
        // remove a socket directly
        bool DeleteSocket(const HudpHandle& handle);
        // send close msg to remote
        bool CloseSocket(const HudpHandle& handle);

    private:
        std::mutex _mutex;
        std::unordered_map<HudpHandle, std::shared_ptr<CSocket>> _socket_map;
    };
}

#endif