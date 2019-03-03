#pragma once

#include <string>

namespace shared{
    void parseIP(std::string& strIP, char* dest);
    uint32_t array_to_uint(char* ipAddress);
    void uint_to_array(uint32_t src, char* dest);
    
    struct TableItem{
        uint8_t prefix[4];
        int prefixLength;
        uint8_t fowardingAddress[4]; //Can be null depening on input.
        std::string interfaceName;
    };

    /**
     * This struct is to save time when a routing is found for a given address. 
     * 
     * It contians:
     * 
     * 1. The final destination IP
     * 2. The next IMEDIATE destination MAC address
     * 3. The Socket to foward the data on
     * 4. The name of the interface to foward the data on.
     **/
    struct ForwardingData{
        uint8_t ipAddress[4];
        uint8_t macAddress[6];
        int sendingSocket;
        std::string interfaceName;
    };
}
