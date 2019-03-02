#ifndef                         ROUTING_TABLE_H
#define                         ROUTING_TABLE_H

#include "RoutingItem.h"
#include "Packet.h"
#include "MacMapping.h"

#include <string>
#include <forward_list>

namespace shared{
    class RoutingTable{
    public:
        RoutingTable(std::string filename);
        void addHomeAddr(char* ipAddress);
        char* findMacAddress(char* ipAddress);
        std::string arpSearch(char* ipAddress);
        bool isHome(char* destIp);
        char* isRouteForward(std::string interfaceName);

        
    private:
        std::forward_list<RoutingItem> routingTable;
        std::forward_list<shared::MacMapping> macMappings;
        std::forward_list<char*> homeAddrs;
        
        
    };
}

#endif
