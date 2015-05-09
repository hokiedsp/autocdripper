#include <iostream>
using std::cout;
using std::endl;

#include "utf8fcns.h"

int main()
{
    std::string test1(u8"Сергей Васильевич Рахманинов 伊熊　威 Laura (Elizabeth) Hughes-Ikuma1 ");

    cout << test1 << endl << utf8fcns::abbr(test1) << endl;


}

//    @"(\b[a-zA-Z])[a-zA-Z]* ?"
