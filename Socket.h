#ifndef                         SOCKET_H
#define                         SOCKET_H

#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <memory>
#include <vector>

namespace host{
    class HostSocket{
    public:
        HostSocket();
        ~HostSocket();
    private:
        int packetSocket{0};
        struct ifaddrs* ifaddr;
        std::unique_ptr<struct ifaddrs> temp;
    };
}
#endif
