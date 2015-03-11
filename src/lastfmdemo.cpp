#include <iostream>
#include <fstream>
#include <iomanip>
#include <exception>
#include <string>
#include <memory>
#include <deque>

#include "CDbLastFm.h"

using std::string;
using std::exception;
using std::cout;
using std::endl;

int main(int argc, const char *argv[])
{
    try
    {
        std::deque<string> ids;
        CDbLastFm lf("0691e8527e395f789d23e4e91b0be8fc");

        //ids.push_back("f4046797-7569-4d09-8ffe-072db1b382d5"); // rachmaninov symphony box set
        //ids.push_back("530300");  // beethoven 9th
        ids.push_back("a98737df-b536-496a-93b3-b3eaecf9ad49"); // jobim songbook
        ids.push_back("879909d6-1d83-4cfa-b260-8064afe2a756"); // joshua redman

        lf.Query(ids);

        lf.PrintJSON(0);
        //cout << lf.NumberOfTracks() << endl;
        //cout << lf.NumberOfDiscs() << endl;

        std::unique_ptr<SDbrBase> cdr(lf.Retrieve());
        cout << *cdr << endl;

        std::vector<unsigned char> imdata = lf.FrontData();

        std::ofstream FILE("front.jpg");
        FILE.write((char*)imdata.data(),imdata.size());

    }
    catch (exception& e)
    {
        cout << "An exception thrown" << endl;
        cout << e.what() << endl;
        return 1;
    }

    return 0;
}
