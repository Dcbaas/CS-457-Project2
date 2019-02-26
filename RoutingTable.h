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
        Packet arpSearch(char* ipAddress);
        bool isHome(char* destIp);
    private:
        //Table of all prefixes
//        std::map<char*, RoutingItem> table;
        //Map mapping all IP addresses to a mac address.
 //       std::map<char*, char*> ipMacMapping;
        //A linked list of the routers home addresses to indicate that a message was for it.

        std::forward_list<RoutingItem> routingTable;
        std::forward_list<shared::MacMapping> macMappings;
        std::forward_list<char*> homeAddrs;
        
        
    };
}

#endif
