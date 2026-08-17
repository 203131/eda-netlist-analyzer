/**
 * @file main.cpp
 * @brief 主程式進入點：解析 Verilog 網表、建構 DAG 電路圖並執行靜態時序分析 (STA)
 */
#include "Parser.hpp"
#include "Netlist.hpp"
#include "STA.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    // 檢查命令列參數，確保使用者提供了 Verilog 檔案路徑
    if (argc < 2) {
        std::cout << "Usage: ./netlist_analyzer <verilog_file>\n";
        return 1;
    }

    std::string filename = argv[1];
    eda::NetlistParser parser(filename);

    if (!parser.parse()) {
        std::cout << "Parsing failed!\n";
        return 1;
    }
    parser.print_summary();

    // 建構 DAG 電路圖
    eda::NetlistGraph graph;
    parser.build_graph(graph);

    // 執行靜態時序分析 (STA)：拓樸排序 + 關鍵路徑計算
    eda::STAEngine sta_engine;
    eda::TimingResult result = sta_engine.run_sta(graph);

    std::cout << "\n========================================\n";
    std::cout << " Static Timing Analysis (STA) Result\n";
    std::cout << "========================================\n";

    if (result.critical_path.empty()) {
        std::cout << "[Error] 時序分析失敗（電路圖存在組合邏輯環路，不符合 DAG 規則）\n";
        return 1;
    }

    std::cout << "[Max Delay]      : " << result.max_delay << " ns\n";
    std::cout << "[Critical Path]  : ";
    for (size_t i = 0; i < result.critical_path.size(); ++i) {
        std::cout << result.critical_path[i];
        if (i + 1 < result.critical_path.size()) std::cout << " -> ";
    }
    std::cout << "\n========================================\n";

    return 0;
}
