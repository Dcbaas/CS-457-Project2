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

        while(std::getline(line, tableFile)){
            boost::algorithm::split(split, line, boost::is_space());
        }
    }
}
