#include "domain.h"
#include <iostream>
#include <cassert>

namespace tc {
namespace detail {

std::string_view trimSpaces(std::string_view line) {
    auto start = line.find_first_not_of(' ');
    if (start == std::string_view::npos) {
        return {};
    }
    auto end = line.find_last_not_of(' ');

    return line.substr(start, end - start + 1);
}

//
// read from line  bus stop "Stop X: latitude, longitude, D1m to stop1, D2m to stop2, ...
// the distance info block "Dm to stop,"
std::istream& operator >>(std::istream &is, DistanceInfo &di) {
    char ch;
    di.destination.resize(0);
    // read distance and char 'm'
    is >> di.distance >> ch;
    assert(ch == 'm');
    // read word "to"
    std::string to;
    is >> to;
    using namespace std::string_literals;
    assert(to == "to"s);
    // read ' '
    is.get(ch);
    while (ch == ' ') { // eat spaces
        is.get(ch);
    };
    // first character has been read
    di.destination += ch;
    // read all chars until ','
    while (is.good() && is.peek() != ',') {
        is.get(ch);
        if (!is.eof()) {
            di.destination += ch;
        }
    };

    return is;
}

} //namespace detail

} // namespace tc
