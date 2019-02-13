#include "Socket.h"

#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <memory>
#include <vector>
#include <iostream>

namespace host{
    HostSocket::HostSocket(){
        if(getifaddrs(&ifaddr) == -1){
            std::cerr << "Error with creating ifaddr" << std::endl;
            //Make this a better exception.
            throw 1;
        }

        //Do temp stuff
        //

        packetSocket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if(packetSocket < 0){
            std::cerr << "Error initalizing Host Socket" << std::endl;
            throw 2;
        }
    }
}
