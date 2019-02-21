#ifndef                         ROUTING_TABLE_H
#define                         ROUTING_TABLE_H

#include "RoutingItem.h"
#include "Packet.h"

#include <string>
#include <map>

namespace shared{
    class RoutingTable{
    public:
        RoutingTable(std::string filename);
        void addItem(RoutingItem item);
        char* findMacAddress(char* ipAddress);
        Packet arpSearch(char* ipAddress);

    private:
        std::map<char*, RoutingItem> table;
    };
}

#endif
