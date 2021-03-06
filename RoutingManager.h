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

        //TODO make all the strings use references  
        void addMacMapping(std::string interfaceName, uint8_t* macAddress);
        void addIpMapping(std::string interfaceName, uint8_t* ipAddress);
        void addSocketMapping(std::string interfaceName, SocketFD socket);
        
        //This will find an already established forwarding for a given IP will return 
        //nullptr if nothing is found.
        struct ForwardingData* findForwarding(uint8_t* ipAddress);

        //Adds a forwarding to the managers list. The data is delibrately pass by copy because 
        //we want it added to the linked list its in.
        void addForwarding(struct ForwardingData data);

        //Find the matching prefix for the given ip and return the interface name.
        //Return an empty string if nothing is found.
        std::string findRouting(uint8_t* ipAddress);

        bool hasRouterForward(uint8_t* ipAddress);
        uint8_t* getRouterForward(uint8_t* ipAddress);

        //TODO make these all use refrences. 
        uint8_t* getMacAddress(std::string interfaceName) const;
        uint8_t* getIpAddress(std::string interfaceName) const; 
        SocketFD getSocketName(std::string interfaceName) const;

        bool isHomeIp(uint8_t* ipAddress);
        bool isHomeMac(uint8_t* macAddress);
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

        //Compares a prefix 
        bool prefixCompare(uint8_t* rhs, uint8_t* lhs, int prefixLength);

        //Compares a whole IP address
        bool ipCompare(uint8_t* rhs, uint8_t* lhs);

        bool macCompare(uint8_t* rhs, uint8_t* lhs);

        bool forwardingCompare(struct ForwardingData& lhs, struct ForwardingData& rhs);
        
    };

}

