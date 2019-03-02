#include "RoutingTable.h"

#include <boost/algorithm/string.hpp>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

namespace shared{
    RoutingTable::RoutingTable(std::string filename){
        std::ifstream tableFile;
        tableFile.open(filename);

        if(!tableFile){
            throw 15;
        }

        std::vector<std::string> split;
        std::string line;

        while(std::getline(tableFile, line)){
            boost::algorithm::split(split, line, boost::is_space());
            //Add the item to the list
            routingTable.push_front(shared::RoutingItem(split[0], split[1], split[2]));
        }
    }

    void RoutingTable::addHomeAddr(char* ipAddress){
	printf("Adding the address: %d.%d.%d.%d\n", ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);
        homeAddrs.push_front(ipAddress);
    }

    char* RoutingTable::findMacAddress(char* ipAddress){
        for(auto mappingIt = macMappings.begin(); mappingIt != macMappings.end(); ++mappingIt){
            if(shared::ipCompare(mappingIt->ipAddress, ipAddress)){
                //return the mac address
                return mappingIt->macAddress;
            }
        }
        return NULL;
    }

    std::string RoutingTable::arpSearch(char* ipAddress){
        for(auto routeIt = routingTable.begin(); routeIt != routingTable.end(); ++ routeIt){
            bool found = true;
            for(int ipElement = 0; ipElement < routeIt->prefixLength/8; ++ipElement){
                if(ipAddress[ipElement] != routeIt->prefix[ipElement]){
                    found = false; 
                    break;
                }
            }
            if(found){
                return routeIt->interfaceName;
            }
        }

        return NULL;
    } 

    bool RoutingTable::isHome(char* destIp){
        for(auto homeIt = homeAddrs.begin(); homeIt != homeAddrs.end(); ++homeIt){
            char* temp = *homeIt;
            if(temp[0] == destIp[0] && temp[1] == destIp[1] && temp[2] == destIp[2] &&
                    temp[3] == destIp[3]){
                delete[] destIp;
                return true;
            }
        }
        delete[] destIp;
        return false;
    }

    char* RoutingTable::isRouteForward(std::string interfaceName){
        for(auto foundIT = routingTable.begin(); foundIT != routingTable.end(); ++foundIT){
            if(foundIT->interfaceName == interfaceName){
                if(foundIT->routeForward[0] != '-'){
                    return foundIT->routeForward;
                }
                else
                    break;
            }
        }
        return NULL;
    }
}
