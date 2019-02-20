#include "RoutingItem.h"

#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>


namespace shared{
    RoutingItem::RoutingItem(std::string prefix, std::string routeForward, 
            std::string interfaceName): interfaceName(interfaceName){
        std::vector<std::string> prefixSplit;

        boost::algorithm::split(prefixSplit, prefix, boost::is_any_of("/"));

        prefixLength = std::stoi(prefixSplit[1]);
        //Convert the ip to an array
        convertIPToArr(this->prefix, prefixSplit[0]);
        //Convert the route forward.
        convertIPToArr(this->routeForward, routeForward);
    }

    bool RoutingItem::convertIPToArr(char* dest, std::string address){

        if(address == "-"){
            return false;
        }
        std::vector<std::string> ipSplit(4);

        boost::algorithm::split(ipSplit, address, boost::is_any_of("."));

        for(auto i = 0; i < 4; ++i){
            dest[i] = std::stoi(ipSplit.at(i));
        }

        return true;
    }

}
