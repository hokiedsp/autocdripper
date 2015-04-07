#include "utils.h"

#include <cstring>
#include <algorithm>

void cleanup_upc(std::string &upc)
{
    upc.erase(std::remove_if(upc.begin(),
                        upc.end(),
                        [](const char& s){ return NULL==strchr("0123456789",s); }),
              upc.end());
}
