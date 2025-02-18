#ifndef MY_TREE_H
#define MY_TREE_H

#include <vector>
#include <concepts>
#include <iostream>
#include <string>
#include <memory>

#include "../utils.hpp"
#include "node.hpp"

template <typename T>
concept TreeTrait = requires(T tree, const T constTree) {
    requires PrintTrait<T>;
    typename T::TreeNodeType;
    requires TreeNodeTrait<typename T::TreeNodeType>;
    
    { constTree.getNodes() } -> std::same_as<const Container<typename T::TreeNodeType>&>;
    { tree.addNode(std::declval<typename T::TreeNodeType*>()) } -> std::same_as<void>;
    { tree.addChild(0, 0) } -> std::same_as<void>;
    { constTree.size() } -> std::convertible_to<size_t>;
};

template <typename T>
struct Tree {
    using TreeNodeType = T;
private:
    Container<T> m_nodes;
public:
    Tree() {}
    const Container<T>& getNodes() const { return m_nodes; }
    void addNode(T* node) {
        node->setIndex(size());
        m_nodes.addElement(std::unique_ptr<T>(node));
    }
    void addChild(size_t parent, size_t child) {
        m_nodes[parent].addChild(child);
        m_nodes[child].setParent(parent);
    }
    size_t size() const { return m_nodes.size(); }
    std::string to_string() const {
        std::string result = "Tree: {";
        for (auto& node : m_nodes)
            result += "["+node.to_string()+"] ";
        return result + "}";
    }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(TreeTrait<Tree<TreeNode>>);

struct SimpleTree {
    using TreeNodeType = TreeNode;
private:
    Tree<TreeNode> m_tree;
public:
    SimpleTree() {}
    const Container<TreeNode>& getNodes() const { return m_tree.getNodes(); }
    void addNode(TreeNode* node) { m_tree.addNode(node); }
    void addNode() { m_tree.addNode(new TreeNode()); }
    void addChild(size_t parent, size_t child) { m_tree.addChild(parent, child); }
    size_t size() const { return m_tree.size(); }
    std::string to_string() const { return m_tree.to_string(); }
    void print() const { std::cout << to_string() << std::endl; }
};

static_assert(TreeTrait<SimpleTree>);

#endif