/**
 * @file Netlist.cpp
 * @brief 實作 NetlistGraph 的圖論操作方法
 */

#include "Netlist.hpp"
#include <iostream>

namespace eda {

void NetlistGraph::add_node(const Node& node) {
    if (nodes_.find(node.name) == nodes_.end()) {
        nodes_[node.name] = node;
    }
}

void NetlistGraph::add_edge(const std::string& from, const std::string& to) {
    if (nodes_.find(from) == nodes_.end() || nodes_.find(to) == nodes_.end()) {
        std::cerr << "[Error] 無法建立連線: " << from << " -> " << to << " (節點不存在)\n";
        return;
    }

    // 建立雙向鄰接關係 (Adjacency List)
    nodes_[from].fanout.push_back(to);
    nodes_[to].fanin.push_back(from);

    // 更新目標節點入度 (In-degree)
    nodes_[to].in_degree++;
}

Node* NetlistGraph::get_node(const std::string& name) {
    auto it = nodes_.find(name);
    if (it != nodes_.end()) {
        return &(it->second);
    }
    return nullptr;
}

const Node* NetlistGraph::get_node(const std::string& name) const {
    auto it = nodes_.find(name);
    if (it != nodes_.end()) {
        return &(it->second);
    }
    return nullptr;
}

} // namespace eda