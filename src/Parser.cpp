/**
 * @file Parser.cpp
 * @brief 實作 Verilog 網表的字串掃描與 DAG 圖論建構邏輯
 */
#include "Parser.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>

namespace eda {

NetlistParser::NetlistParser(const std::string& filename) : m_filename(filename) {}

std::string NetlistParser::clean_token(const std::string& token) {
    std::string s = token;
    // 過濾 Verilog 語法中的標點符號與空白
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
    // 過濾 Verilog 語法中的標點符號與空白
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        ss >> token;

        // 解析初級輸入埠 (Primary Inputs，例如: input a, b;)
        if (token == "module") {
            ss >> m_module_name;
            m_module_name = clean_token(m_module_name);
        }
        else if (token == "input") {
            while (ss >> token) {
                std::string cleaned = clean_token(token);
                if (!cleaned.empty()) m_inputs.push_back(cleaned);
            }
        }

        // 解析初級輸出埠 (Primary Outputs，例如: output y;)
        else if (token == "output") {
            while (ss >> token) {
                std::string cleaned = clean_token(token);
                if (!cleaned.empty()) m_outputs.push_back(cleaned);
            }
        }


        else if (token == "wire") {
            while (ss >> token) {
                std::string cleaned = clean_token(token);
                if (!cleaned.empty()) m_wires.push_back(cleaned);
            }
        }

        // 解析邏輯閘原語 (Gate Primitives，例如: and g1 (y, a, b);)
        else if (token == "AND" || token == "OR" || token == "NOT" || token == "NAND" || token == "NOR") {
            Gate g;
            g.type = token;
            ss >> g.name;

            std::string pin;
            bool is_first = true;
            while (ss >> pin) {
                std::string cleaned = clean_token(pin);
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

double NetlistParser::lookup_gate_delay(const std::string& gate_type) const {
    // 簡化標準元件庫 (Standard Cell Library) 之單位傳播延遲對照表 (ns)
    static const std::unordered_map<std::string, double> kDelayTable = {
        {"NOT",  0.10},
        {"NAND", 0.15},
        {"NOR",  0.15},
        {"AND",  0.20},
        {"OR",   0.20},
    };
    auto it = kDelayTable.find(gate_type);
    if (it != kDelayTable.end()) {
        return it->second;
    }
    // 未知閘型：給予保守預設延遲，避免時序分析中斷
    return 0.10;
}

void NetlistParser::build_graph(NetlistGraph& graph) const {
    // 1. 建立所有初級輸入節點 (Primary Input)，訊號到達時間起始為 0
    for (const auto& input_name : m_inputs) {
        Node node;
        node.name = input_name;
        node.type = NodeType::INPUT;
        node.gate_type = "INPUT";
        node.delay = 0.0;
        graph.add_node(node);
    }

    // 2. 建立所有邏輯閘節點，節點名稱採用其輸出腳位 (Output Pin)，
    //    使下游 Gate 能直接以訊號名稱查找驅動來源
    for (const auto& gate : m_gates) {
        Node node;
        node.name = gate.output_pin;
        node.type = NodeType::GATE;
        node.gate_type = gate.type;
        node.delay = lookup_gate_delay(gate.type);
        graph.add_node(node);
    }

    // 3. 依每個 Gate 的輸入腳位建立有向邊 (Fanin Signal -> Gate Output)
    for (const auto& gate : m_gates) {
        for (const auto& fanin_signal : gate.inputs) {
            graph.add_edge(fanin_signal, gate.output_pin);
        }
    }
}

void NetlistParser::print_summary() const {
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

} // namespace eda
