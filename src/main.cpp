#include "Parser.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./netlist_analyzer \n";
        return 1;
    }

    std::string filename = argv[1];
    NetlistParser parser(filename);

    if (parser.parse()) {
        parser.printSummary();
    } else {
        std::cout << "Parsing failed!\n";
        return 1;
    }

    return 0;
}