#include "MacMapping.h"

namespace shared{
    bool ipCompare(char* rhs, char* lhs){
        for(auto i = 0; i < 4; ++i){
            if(rhs[i] != lhs[i]){
                return false;
            }
        }
        return true;
    }
}
