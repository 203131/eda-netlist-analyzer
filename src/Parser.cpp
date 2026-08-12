#include "Parser.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

NetlistParser::NetlistParser(const std::string& filename) : m_filename(filename) {}

std::string NetlistParser::cleanToken(const std::string& token) {
    std::string s = token;
    s.erase(std::remove_if(s.begin(), s.end(), [](char c) {
        return c == ',' || c == ';' || c == '(' || c == ')';
    }), s.end());
    return s;
}

bool NetlistParser::parse() {
    std::ifstream file(m_filename);
    if (!file.is_open()) {
        std::cerr << "[Error] Cannot open file: " << m_filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "module") {
            ss >> m_module_name;
            m_module_name = cleanToken(m_module_name);
        } 
        else if (token == "input") {
            while (ss >> token) {
                std::string cleaned = cleanToken(token);
                if (!cleaned.empty()) m_inputs.push_back(cleaned);
            }
        } 
        else if (token == "output") {
            while (ss >> token) {
                std::string cleaned = cleanToken(token);
                if (!cleaned.empty()) m_outputs.push_back(cleaned);
            }
        } 
        else if (token == "wire") {
            while (ss >> token) {
                std::string cleaned = cleanToken(token);
                if (!cleaned.empty()) m_wires.push_back(cleaned);
            }
        } 
        else if (token == "AND" || token == "OR" || token == "NOT" || token == "NAND" || token == "NOR") {
            Gate g;
            g.type = token;
            ss >> g.name;
            
            std::string pin;
            bool is_first = true;
            while (ss >> pin) {
                std::string cleaned = cleanToken(pin);
                if (!cleaned.empty()) {
                    if (is_first) {
                        g.output_pin = cleaned;
                        is_first = false;
                    } else {
                        g.inputs.push_back(cleaned);
                    }
                }
            }
            m_gates.push_back(g);
        }
    }
    file.close();
    return true;
}

void NetlistParser::printSummary() const {
    std::cout << "========================================\n";
    std::cout << " Netlist Summary: " << m_module_name << "\n";
    std::cout << "========================================\n";
    std::cout << "[Inputs]  : ";
    for (const auto& in : m_inputs) std::cout << in << " ";
    std::cout << "\n[Outputs] : ";
    for (const auto& out : m_outputs) std::cout << out << " ";
    std::cout << "\n[Wires]   : ";
    for (const auto& w : m_wires) std::cout << w << " ";
    std::cout << "\n[Gates]   : " << m_gates.size() << " instances found:\n";
    for (const auto& g : m_gates) {
        std::cout << "  - " << g.type << " (" << g.name << ") -> Out: " << g.output_pin << " | In: ";
        for (const auto& in : g.inputs) std::cout << in << " ";
        std::cout << "\n";
    }
    std::cout << "========================================\n";
}