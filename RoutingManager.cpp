#include "RoutingManager.h"
#include "TableConstructs.h"

#include <map>
#include <forward_list>
#include <string>
#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace shared{
    RoutingManager::RoutingManager(std::string filename){
        std::ifstream tableFile;
        tableFile.open(filename);

        if(!tableFile){
            throw 15;
        }

        std::vector<std::string> split;
        std::string line;

        while(std::getline(tableFile, line)){
            boost::algorithm::split(split, line, boost::is_space());

            //Split the prefix from the prefix length
            std::vector<std::string> prefixSplit;

            boost::algorithm::split(prefixSplit, split[0], boost::is_any_of("/"));

            uint8_t* prefix = new uint8_t[4];
            int prefixLength = std::stoi(prefixSplit[1]);

            uint8_t* forwardingIP = new uint8_t[4];
            bool hasForwardIP = false; 
            //Extract the prefix
            parseIP(prefixSplit[0], prefix);

            //Extract the forwarding ip if there is something there.
            if(split[1] != "-"){
                parseIP(split[1], forwardingIP);
                hasForwardIP = true;
            }

            struct TableItem tableItem;

            memcpy(tableItem.prefix, prefix, 4);
            tableItem.prefixLength = prefixLength;
            memcpy(tableItem.fowardingAddress, forwardingIP, 4);

            tableItem.interfaceName = split[2];

            routingTable.push_front(tableItem);

        }

        tableFile.close();
    }

    struct ForwardingData* RoutingManager::findForwarding(uint8_t* ipAddress){
        for(auto forwardingIt = forwardingTable.begin(); forwardingIt != forwardingTable.end(); 
                ++forwardingIt){
           if(ipCompare(ipAddress, forwardingIt->ipAddress)){
               return &(*forwardingIt);
           }
        }
        
        return nullptr;
    }


    void RoutingManager::addForwarding(struct ForwardingData data){
        //Make sure this new forwarding isn't already in the list.
        //If it is, discard it.
        for(auto forwardDataIt = forwardingTable.begin(); forwardDataIt != forwardingTable.end();
                ++forwardDataIt){
            //If this is a match return to discard
            if(forwardingCompare(data, *forwardDataIt)){
                return;
            }
        }

        //No match was found add to list
        forwardingTable.push_front(data);
    }

    std::string RoutingManager::findRouting(uint8_t* ipAddress){
        for(auto tableIt = routingTable.begin(); tableIt != routingTable.end(); ++tableIt){
            if(prefixCompare(ipAddress, tableIt->prefix, tableIt->prefixLength)){
                return tableIt->interfaceName;
            }
        }
        //Return an empty string on failing to find anything;
        return "";
    }

    void RoutingManager::addMacMapping(std::string interfaceName, uint8_t* macAddress){
        homeMacMapping.emplace(interfaceName, macAddress);
    }

    void RoutingManager::addIpMapping(std::string interfaceName, uint8_t* ipAddress){
        homeIpMapping.emplace(interfaceName, ipAddress);
    }

    void RoutingManager::addSocketMapping(std::string interfaceName, SocketFD socket){
        socketMapping.emplace(interfaceName, socket);
    }


    //This is an unsafe operation but I don't care.
    uint8_t* RoutingManager::getMacAddress(std::string interfaceName) const{
        uint8_t* macAddress = homeMacMapping.find(interfaceName)->second;
        return macAddress;
    }

    //This is an unsafe operation but screw it.
    uint8_t* RoutingManager::getIpAddress(std::string interfaceName) const{
        uint8_t* ipAddress = homeIpMapping.find(interfaceName)->second;
        return ipAddress;
 
    }

    SocketFD RoutingManager::getSocketName(std::string interfaceName) const{
        return socketMapping.find(interfaceName)->second;
    }
            

    bool RoutingManager::isHomeIp(uint8_t* ipAddress){
        uint32_t rhs, lhs;
        memcpy(&rhs, ipAddress, 4);
        for(auto ipIt = homeIpMapping.begin(); ipIt != homeIpMapping.end(); ++ipIt){
           memcpy(&lhs, ipIt->second,4); 
           if(rhs == lhs){
               return true;
           }
        }
        return false;
    }

    bool RoutingManager::prefixCompare(uint8_t* rhs, uint8_t* lhs, int prefixLength){
        unsigned int byteLength = prefixLength/8;

        for(int i = 0; i < byteLength; ++i){
            if(rhs[i] != lhs[i]){
                return false;
            }
        }

        return true;
    }

    bool RoutingManager::ipCompare(uint8_t* rhs, uint8_t* lhs){
        int left, right;

        memcpy(&left, lhs, 4);
        memcpy(&right, rhs, 4);

        return right == left;
    }


    //Returns true if the mac address are idential
    bool RoutingManager::macCompare(uint8_t* rhs, uint8_t* lhs){
        for(int i = 0; i < 6; ++i){
            if(rhs[i] != lhs[i]){
                return false;
            }
        }

        return true;
    }

    bool RoutingManager::forwardingCompare(struct ForwardingData& lhs, struct ForwardingData& rhs){
        //Compare the ips
        bool equalIP = ipCompare(lhs.ipAddress, rhs.ipAddress);

        //Compare both source and destination mac addresses
        bool equalMacSource = macCompare(lhs.sourceMacAddress, rhs.sourceMacAddress);
        bool equalMacDest = macCompare(lhs.destinationMacAddress, rhs.destinationMacAddress);

        bool equalSocekts = lhs.sendingSocket == rhs.sendingSocket;

        bool equalInterfaceNames = lhs.interfaceName == rhs.interfaceName;

        return equalIP && equalMacSource && equalMacDest && equalSocekts && equalInterfaceNames;
    }
}

