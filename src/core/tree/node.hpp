#ifndef MY_TREE_NODE_H
#define MY_TREE_NODE_H

#include <concepts>
#include <iostream>
#include <string>
#include <vector>

#include "../utils.hpp"

template <typename T>
concept TreeNodeTrait = requires(T node, const T constNode) {
    requires PrintTrait<T>;
    
    { node.get_index() } -> std::same_as<int>;
    { node.set_parent(0) } -> std::same_as<void>;
    { node.get_parent() } -> std::same_as<int>;
    { node.set_index(0) } -> std::same_as<void>;
    { constNode.get_childs() } -> std::same_as<const std::vector<int>&>;
    { node.add_child(0) } -> std::same_as<void>;
};

struct TreeNode {
private:
    int m_index{-1};
    int m_parent{-1};
    std::vector<int> m_childs{};
public:
    TreeNode() {}
    int get_index() const { return m_index; }
    void set_index(int index) { m_index = index; }
    void set_parent(int parent) { m_parent = parent; }
    int get_parent() const { return m_parent; }
    const std::vector<int>& get_childs() const { return m_childs; }
    void add_child(int index) { m_childs.push_back(index); }
    std::string to_string() const {
        std::string result = "Node " + std::to_string(m_index);
        result += " parent: " + std::to_string(m_parent) + " childs: ";
        for (auto& child : m_childs)
            result += "(" + std::to_string(child) + ")";
        return result;
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(TreeNodeTrait<TreeNode>);

#endif