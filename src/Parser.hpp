#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include <iostream>

struct Gate {
    std::string type;
    std::string name;
    std::string output_pin;
    std::vector<std::string> inputs;
};

class NetlistParser {
public:
    NetlistParser(const std::string& filename);
    bool parse();
    void printSummary() const;

private:
    std::string m_filename;
    std::string m_module_name;
    std::vector<std::string> m_inputs;
    std::vector<std::string> m_outputs;
    std::vector<std::string> m_wires;
    std::vector<Gate> m_gates;

    std::string cleanToken(const std::string& token);
};

#endif // PARSER_HPP