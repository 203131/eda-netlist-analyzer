/**
 * @file Parser.hpp
 * @brief 定義 Verilog Netlist 語法解析器介面
 */

#pragma once

#include "Netlist.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace eda {

/**
 * @brief 代表一個邏輯閘原語 (Gate Primitive) 的解析結果
 */
struct Gate {
    std::string type;                  ///< 邏輯閘種類 (例如: "AND", "OR", "NOT")
    std::string name;                  ///< 實例名稱 (例如: "g1")
    std::string output_pin;            ///< 輸出腳位名稱
    std::vector<std::string> inputs;   ///< 輸入腳位名稱列表
};

/**
 * @brief Verilog 網表解析器類別
 */
class NetlistParser {
public:
    NetlistParser(const std::string& filename);

    /**
     * @brief 解析建構時指定的 Verilog 檔案，擷取模組、埠與邏輯閘原語
     * @return true 代表解析成功，false 代表檔案無法開啟或語法錯誤
     */
    bool parse();

    /**
     * @brief 將解析結果 (Module、Ports、Gates) 以格式化文字輸出至 stdout
     */
    void print_summary() const;

    /**
     * @brief 將解析結果轉換為 NetlistGraph DAG，供 STAEngine 進行時序分析
     * @details 每個 Primary Input 與每個 Gate 各對應一個節點（Gate 節點以其輸出腳位命名，
     *          供下游 Gate 以訊號名稱查找），並依 Gate 輸入腳位建立有向邊 (Fanin -> Gate)。
     *          Gate 延遲採用 lookup_gate_delay() 提供的簡化標準元件庫 (Standard Cell) 數值，
     *          並非讀取真實 Liberty/SDF 時序檔案。
     * @param graph 輸出參數：欲填入節點與邊的 NetlistGraph 物件
     */
    void build_graph(NetlistGraph& graph) const;

private:
    std::string m_filename;
    std::string m_module_name;
    std::vector<std::string> m_inputs;
    std::vector<std::string> m_outputs;
    std::vector<std::string> m_wires;
    std::vector<Gate> m_gates;

    /**
     * @brief 輔助方法：清除字串中的標點符號 (如分號 ';', 括號 '()', 逗號 ',')
     * @param token 原始字串標記
     * @return std::string 清理後的乾淨字串
     */
    std::string clean_token(const std::string& token);

    /**
     * @brief 輔助方法：依邏輯閘種類查表取得簡化標準元件庫的傳播延遲
     * @param gate_type 邏輯閘種類 (例如: "AND", "NOT")
     * @return double 對應的傳播延遲 (ns)；未知閘型則回傳保守預設值
     */
    double lookup_gate_delay(const std::string& gate_type) const;
};

} // namespace eda
