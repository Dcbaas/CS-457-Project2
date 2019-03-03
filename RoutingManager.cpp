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

            char prefix[4];
            int prefixLength = std::stoi(prefixSplit[1]);

            char forwardingIP[4];
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
    }
}
