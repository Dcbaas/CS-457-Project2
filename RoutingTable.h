#ifndef                         ROUTING_TABLE_H
#define                         ROUTING_TABLE_H

#include <array>
#include <string>
#include <map>
#include <tuple>

namespace shared{
    typedef std::string InterfaceName;
    typedef std::array<char, 4> IpAddress;
    typedef std::array<char, 6> MacAddress;


    class RoutingTable{
    public:
        RoutingTable(std::string filename);
        char* findMacAddress(char* ipAddress);
        char* findMacAddress(InterfaceName name);
        void addInterface(InterfaceName name, char* prefix, int prefixLen, char* routerForward, bool isRouterForward, char* MacAddress = nullptr);
        
    private:
        //Tuple has interface name, its ipAddress, if it towards another router, 
        //it has the ip address of that as well and a flag to indicate it has that data. 
        std::map<InterfaceName, std::tuple<InterfaceName, IpAddress, int, IpAddress, bool>> routingTable;
        //Maps an Ip address to a mac address. 
        std::map<IpAddress, MacAddress> linkLayerMap;
    };
}

#endif
