#include <iostream>
using std::cout;
using std::endl;

#include "utf8fcns.h"

#include <boost/regex.hpp>
#include <boost/regex/icu.hpp>

static const std::string wordinit_pattern("(\\w)(\\w*)");
static const boost::u32regex wordinit_exp = boost::make_u32regex(wordinit_pattern);

static const std::string space_pattern("\\s");
static const boost::u32regex space_exp = boost::make_u32regex(space_pattern);

int main()
{
    std::string x(u8"laura (ElizaBeth) Hughes-Ikuma1 Сергей Васильевич Рахманинов 伊熊 (kesh)　威 ");

    cout << "x:" << x << "[" << utf8fcns::len(x) << "]" << endl;
    cout << "abbr(x): " << utf8fcns::abbr(x) << endl;
    cout << "cap(x): " << utf8fcns::cap(x) << endl;
    cout << "cap2(x): " << utf8fcns::cap2(x) << endl;
    cout << "caps(x): " << utf8fcns::caps(x) << endl;
    cout << "caps2(x): " << utf8fcns::caps2(x) << endl;
    cout << "cut(x,8): " << utf8fcns::cut(x,8) << endl;
    cout << "cut(x,72): " << utf8fcns::cut(x,72) << endl;
    cout << "cutwords(x,8): " << utf8fcns::cutwords(x,8) << endl;
    cout << "cutwords(x,72): " << utf8fcns::cutwords(x,72) << endl;
}

//    @"(\b[a-zA-Z])[a-zA-Z]* ?"
