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
    
    { node.getIndex() } -> std::convertible_to<int>;
    { node.setParent(0) } -> std::same_as<void>;
    { node.setIndex(0) } -> std::same_as<void>;
    { constNode.getChilds() } -> std::same_as<const std::vector<size_t>&>;
    { node.addChild(0) } -> std::same_as<void>;
};

struct TreeNode {
private:
    int m_index{-1};
    int m_parent{-1};
    std::vector<size_t> m_childs{};
public:
    TreeNode() {}
    int getIndex() const { return m_index; }
    void setIndex(int index) { m_index = index; }
    void setParent(int parent) { m_parent = parent; }
    const std::vector<size_t>& getChilds() const { return m_childs; }
    void addChild(size_t index) { m_childs.push_back(index); }
    std::string toString() const {
        std::string result = "Node " + std::to_string(m_index);
        result += " parent: " + std::to_string(m_parent) + " childs: ";
        for (auto& child : m_childs)
            result += "(" + std::to_string(child) + ")";
        return result;
    }
    void print() const { std::cout << toString() << std::endl; }
};

static_assert(TreeNodeTrait<TreeNode>);

#endif