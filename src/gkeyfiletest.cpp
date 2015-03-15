#include <stdexcept>
#include <iostream>

#include "CGKeyFileAutoCDRipper.h"

#define CONFFILE "/home/kesh/Documents/autocdripper/src/autocdripper.conf"

using std::cout;
using std::endl;

int main(void)
{
    try
    {
        CGKeyFileAutoCDRipper conf(CONFFILE);
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
    }
    return 0;
}
