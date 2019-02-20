#include "RoutingTable.h"

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <sstream>
#include <array>
#include <string>
#include <map>
#include <tuple>
#include <vector>

namespace shared{
    RoutingTable::RoutingTable(std::string filename){
        std::ifstream table(filename);

        if(!table){
            throw 5;
        }
        std::string line;

        while(std::getline(table, line)){
            std::vector<std::string> splitData;

            boost::algorithm::split(splitData, line, boost::is_space);


        }
    }
}
