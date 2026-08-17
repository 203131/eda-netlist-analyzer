/**
 * @file Netlist.hpp
 * @brief 定義邏輯電路網表 (Netlist) 的有向無環圖 (DAG) 資料結構
 * @details 用於 EDA 靜態時序分析 (STA)，包含 Node (Gate/Port) 與 NetlistGraph 結構
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace eda {

/**
 * @brief 電路圖中的節點種類
 */
enum class NodeType {
    INPUT,   ///< 初級輸入埠 (Primary Input)
    OUTPUT,  ///< 初級輸出埠 (Primary Output)
    GATE,    ///< 邏輯閘 (AND, OR, NOT, etc.)
    WIRE     ///< 連接線 (Net)
};

/**
 * @brief 代表電路圖中的一個節點 (Logic Gate 或 Port)
 */
struct Node {
    std::string name;        ///< 節點名稱 (例如: "g1", "a", "out")
    NodeType type;           ///< 節點類型 (INPUT, OUTPUT, GATE)
    std::string gate_type;   ///< 邏輯閘種類 (例如: "and", "not", "or", "INPUT")
    double delay = 0.0;      ///< 邏輯閘本身的傳播延遲 (ns)
    
    // 時序分析 (STA) 欄位
    double arrival_time = 0.0; ///< 訊號到達時間 (Arrival Time, AT)
    int in_degree = 0;         ///< 入度 (In-degree)：用於 Topological Sort

    std::vector<std::string> fanin;  ///< 扇入節點名稱列表 (前導驅動節點)
    std::vector<std::string> fanout; ///< 扇出節點名稱列表 (後續負載節點)
};

/**
 * @brief 管理整張電路圖 (DAG) 的主要類別
 */
class NetlistGraph {
public:
    NetlistGraph() = default;

    /**
     * @brief 新增一個節點到電路圖中
     * @param node 欲新增的 Node 物件
     */
    void add_node(const Node& node);

    /**
     * @brief 在兩個節點之間建立一條有向邊 (Directed Edge)
     * @param from 來源節點名稱 (Driver)
     * @param to 目標節點名稱 (Load)
     */
    void add_edge(const std::string& from, const std::string& to);

    /**
     * @brief 取得指定名稱的節點指標
     * @param name 節點名稱
     * @return Node* 找到回傳指標，未找到則回傳 nullptr
     */
    Node* get_node(const std::string& name);

    /**
     * @brief 取得指定名稱的節點指標（唯讀版本）
     * @param name 節點名稱
     * @return const Node* 找到回傳指標，未找到則回傳 nullptr
     */
    const Node* get_node(const std::string& name) const;

    /**
     * @brief 取得所有節點的唯讀參照
     */
    const std::unordered_map<std::string, Node>& get_all_nodes() const { return nodes_; }

private:
    std::unordered_map<std::string, Node> nodes_; ///< 節點字典 (Name -> Node)
};

} // namespace eda