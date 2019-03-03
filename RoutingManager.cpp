#include "RoutingManager.h"
#include "TableConstructs.h"

#include <map>
#include <forward_list>
#include <string>
#include <ifstream>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace shared{
    RoutingManager(std::string filename){
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


        }
    }
}
