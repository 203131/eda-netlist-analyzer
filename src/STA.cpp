/**
 * @file STA.cpp
 * @brief 實作 Kahn's Algorithm 拓樸排序與靜態時序分析 (STA) 計算邏輯
 */

#include "STA.hpp"
#include <queue>
#include <iostream>
#include <algorithm>

namespace eda {

bool STAEngine::run_topological_sort(const NetlistGraph& graph, std::vector<std::string>& topo_order) {
    topo_order.clear();
    const auto& nodes = graph.get_all_nodes();
    
    // 建立暫存入度表與處理 Queue
    std::unordered_map<std::string, int> in_degree_map;
    std::queue<std::string> q;

    // 1. 初始化入度表，找出所有入度為 0 的初級輸入點 (Primary Inputs)
    for (const auto& [name, node] : nodes) {
        in_degree_map[name] = node.in_degree;
        if (node.in_degree == 0) {
            q.push(name);
        }
    }

    // 2. Kahn's 演算法主迴圈
    while (!q.empty()) {
        std::string curr = q.front();
        q.pop();
        topo_order.push_back(curr);

        const Node* node_ptr = graph.get_node(curr);
        if (!node_ptr) continue;

        // 走訪該節點的所有扇出 (Fanout)，降低對應入度
        for (const auto& fanout_name : node_ptr->fanout) {
            in_degree_map[fanout_name]--;
            // 當扇出節點的前導依賴全數處理完畢 (In-degree == 0)，放入 Queue
            if (in_degree_map[fanout_name] == 0) {
                q.push(fanout_name);
            }
        }
    }

    // 3. 檢查環路 (Cycle Detection)
    if (topo_order.size() != nodes.size()) {
        std::cerr << "[Error] 電路圖中存在組合邏輯環路 (Combinational Loop)，不符合 DAG 規則！\n";
        return false;
    }

    return true;
}

TimingResult STAEngine::run_sta(NetlistGraph& graph) {
    TimingResult result;

    // 1. 執行拓樸排序，確保訊號計算依據 Dependency Ordering 推進
    if (!run_topological_sort(graph, result.topo_order)) {
        return result;
    }

    // 追溯 Critical Path 用之父節點字典 (Child -> Parent)
    std::unordered_map<std::string, std::string> prev_node;

    // 2. 正向動態規劃計算到達時間：AT(v) = delay(v) + max_{u in Fanin(v)} AT(u)
    for (const std::string& name : result.topo_order) {
        Node* curr = graph.get_node(name);
        if (!curr) continue;

        double max_fanin_at = 0.0;
        std::string critical_parent = "";
        bool found_parent = false;

        for (const std::string& fanin_name : curr->fanin) {
            Node* parent = graph.get_node(fanin_name);
            // 首個有效 fanin 一律先納入候選，之後才用嚴格 > 篩選出真正的最大值，
            // 避免所有 fanin 的 Arrival Time 同為 0.0（例如皆為 Primary Input）時
            // critical_parent 永遠抓不到父節點，導致 Critical Path 反推時斷在半路。
            if (parent && (!found_parent || parent->arrival_time > max_fanin_at)) {
                max_fanin_at = parent->arrival_time;
                critical_parent = fanin_name;
                found_parent = true;
            }
        }

        // 當前節點 Arrival Time 累積算子
        curr->arrival_time = curr->delay + max_fanin_at;

        if (!critical_parent.empty()) {
            prev_node[name] = critical_parent;
        }
    }

    // 3. 搜尋整張網表中 Arrival Time 最大的終點節點
    std::string critical_end_node = "";
    double max_circuit_delay = -1.0;

    for (const auto& [name, node] : graph.get_all_nodes()) {
        if (node.arrival_time > max_circuit_delay) {
            max_circuit_delay = node.arrival_time;
            critical_end_node = name;
        }
    }

    result.max_delay = max_circuit_delay;

    // 4. 回溯 (Backtrack) 構建關鍵路徑
    std::string curr_backtrack = critical_end_node;
    while (!curr_backtrack.empty()) {
        result.critical_path.push_back(curr_backtrack);
        if (prev_node.find(curr_backtrack) != prev_node.end()) {
            curr_backtrack = prev_node[curr_backtrack];
        } else {
            curr_backtrack = "";
        }
    }

    // 反轉路徑序列，使其維持 Primary Input -> Primary Output 的邏輯順序
    std::reverse(result.critical_path.begin(), result.critical_path.end());

    return result;
}

} // namespace eda