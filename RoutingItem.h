#ifndef                     ROUTING_ITEM_H
#define                     ROUTING_ITEM_H

#include <string>
#include <vector>

namespace shared{
    class RoutingItem{
    public:
        RoutingItem(std::string prefix, std::string routeForward, std::string interfaceName);
        char prefix[4];
        char prefixLength;
        char routeForward[4];
        std::string interfaceName;
        char macAddress[6];
    private:
        bool convertIPToArr(char* dest, std::string address);
        bool convertMACToArr(char* dest, std::string address);
        
    };
}
#endif
