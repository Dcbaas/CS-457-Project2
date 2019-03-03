#include "TableConstructs.h"

#include <string>
#include <string.h>
#include <boost/algorithm/string.hpp>
#include <vector>

namespace shared{
    void parseIP(std::string& strIP, char* dest){
        std::vector<std::string> ipSplit(4);
        boost::algorithm::split(ipSplit, strIP, boost::is_any_of(".")); 

        for(auto i = 0; i < 4; ++i){
            dest[i] = std::stoi(ipSplit.at(i));
        }
    }

    uint32_t array_to_uint(char* ipAddress){
        uint32_t result;
        memcpy(&result, ipAddress, 4);
        return result;
    }

    void uint_to_array(uint32_t src, char* dest){
        memcpy(dest, &src, 4);
        return;
    }
}
