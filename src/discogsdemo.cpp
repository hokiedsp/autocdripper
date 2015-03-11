#include <iostream>
#include <fstream>
#include <iomanip>
#include <exception>
#include <string>
#include <memory>
#include <deque>

#include "CDbDiscogs.h"

using std::string;
using std::exception;
using std::cout;
using std::endl;

int main(int argc, const char *argv[])
{
    try
    {
        std::deque<string> ids;
        CDbDiscogs dc;

        ids.push_back("3224333"); // rachmaninov symphony box set
        ids.push_back("530300");  // beethoven 9th
        ids.push_back("2449413"); // jobim songbook
        ids.push_back("1879353"); // joshua redman

        dc.Query(ids);

        dc.Query(ids[2],2);

        //cout << dc.NumberOfTracks() << endl;
        //cout << dc.NumberOfDiscs() << endl;

        //dc.PrintJSON(0);

        std::unique_ptr<SDbrBase> cdr(dc.Retrieve());
        cout << *cdr << endl;

    }
    catch (exception& e)
    {
        cout << "An exception thrown" << endl;
        cout << e.what() << endl;
        return 1;
    }

    return 0;
}
