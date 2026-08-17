/**
 * @file STA.hpp
 * @brief 靜態時序分析 (Static Timing Analysis) 引擎介面
 * @details 實作 Kahn's 演算法拓樸排序與 Critical Path 關鍵路徑計算
 */

#pragma once

#include "Netlist.hpp"
#include <vector>
#include <string>

namespace eda {

/**
 * @brief 靜態時序分析結果資料結構
 */
struct TimingResult {
    double max_delay = 0.0;                    ///< 整張電路網表的最大時序延遲 (ns)
    std::vector<std::string> critical_path;   ///< 最長時序路徑節點鏈 (Input -> Gate -> Output)
    std::vector<std::string> topo_order;      ///< 拓樸排序節點序列
};

/**
 * @brief 靜態時序分析核心引擎
 */
class STAEngine {
public:
    STAEngine() = default;

    /**
     * @brief 使用 Kahn's 演算法對 DAG 電路圖進行拓樸排序
     * @param graph 電路網表圖
     * @param topo_order 輸出參數：存放拓樸排序結果的字串向量
     * @return true 拓樸排序成功 (合法 DAG)，false 代表電路存在組合邏輯環路 (Cycle)
     */
    bool run_topological_sort(const NetlistGraph& graph, std::vector<std::string>& topo_order);

    /**
     * @brief 執行靜態時序分析並計算 Critical Path
     * @param graph 電路網表圖 (會更新各節點之 arrival_time 數值)
     * @return TimingResult 包含最大延遲與關鍵路徑節點列表
     */
    TimingResult run_sta(NetlistGraph& graph);
};

} // namespace eda