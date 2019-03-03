#pragma once

#include "TableConstructs.h"

#include <map>
#include <forward_list>
#include <string>


typedef int SocketFD;
namespace shared{
    class RoutingManager{
    public:
        RoutingManager(std::string filename);
//        ~RoutingManager();

        void addMacMapping(std::string interfaceName, uint8_t* macAddress);
        void addIpMapping(std::string interfaceName, uint8_t* ipAddress);
        void addSocketMapping(std::string interfaceName, SocketFD socket);
        uint8_t* getMacAddress(std::string interfaceName) const;
        uint8_t* getIpAddress(std::string interfaceName) const; 
        SocketFD getSocketName(std::string interfaceName) const;

        bool isHome(uint8_t* ipAddress);
    private:
        //The routing table is used to find where a packet needs to go.
        std::forward_list<struct TableItem> routingTable;
        
        //The forwarding table tells exacly where a spcific packet is supposed to go.
        std::forward_list<struct ForwardingData> forwardingTable;

        //A mapping of a home MAC address to an interface name.
        std::map<std::string, uint8_t*> homeMacMapping;

        //A mapping of a home IP address to an interface name.
        std::map<std::string, uint8_t*> homeIpMapping;

        //A mapping of a Socket File Descriptor to an interface name.
        std::map<std::string, SocketFD> socketMapping;
    };
}
