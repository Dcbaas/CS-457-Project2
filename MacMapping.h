#ifndef                     MAC_MAPPING_H
#define                     MAC_MAPPING_H

namespace shared{
    typedef struct MacMapping{
        char ipAddress[4];
        char macAddress[6];
    } MacMapping;

    bool ipCompare(char* lhs, char* rhs);
}
#endif
