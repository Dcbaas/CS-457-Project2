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

    Packet RoutingTable::arpSearch(char* ipAddress){

        return NULL;
    } 
}
