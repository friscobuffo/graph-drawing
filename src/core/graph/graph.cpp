#include "core/graph/graph.hpp"

#include <iostream>

#include "core/utils.hpp"

GraphEdge::GraphEdge(int id, const GraphNode& from, const GraphNode& to,
                     const Graph& owner)
    : m_id(id), m_from(from), m_to(to), m_graph_owner(owner) {
  if (&from.get_owner() != &owner || &to.get_owner() != &owner)
    throw std::runtime_error(
        "GraphEdge constructor: nodes do not belong to the same graph");
}

const std::string GraphEdge::to_string() const {
  return "Edge[" + m_from.to_string() + " -> " + m_to.to_string() + "]";
}

std::string GraphNode::to_string() const {
  std::string result = "Node " + std::to_string(m_id) + " neighbors:";
  for (const auto& edge : get_edges())
    result += " " + std::to_string(edge.get_to().get_id());
  return result;
}

const GraphNode& Graph::add_node(int id) {
  if (id < 0)
    throw std::runtime_error("Graph::add_node: id must be non-negative");
  if (has_node(id))
    throw std::runtime_error(
        "Graph::add_node: node with this id already exists");
  GraphNode* node = new GraphNode(id, *this);
  m_nodeid_to_node_map[id] = std::unique_ptr<GraphNode>(node);
  m_nodeid_to_edgesid[id] = std::unordered_set<int>();
  m_nodeid_to_neighborsid_to_edgesid[id] = std::unordered_map<int, int>();
  m_nodeid_to_incoming_edgesid_map[id] = std::unordered_set<int>();
  return *node;
}

int Graph::get_number_of_edges() const {
  int count = 0;
  for (const auto& node : get_nodes()) count += node.get_degree();
  return count;
}

const std::string Graph::to_string() const {
  std::string result = "Graph:\n";
  for (const auto& node : get_nodes()) result += node.to_string() + "\n";
  return result;
}

const GraphNode& Graph::get_node_by_id(int id) const {
  if (!has_node(id))
    throw std::runtime_error("Graph::get_node_by_id: node not found");
  return *m_nodeid_to_node_map.at(id);
}

const GraphNode& Graph::add_node() {
  while (has_node(m_next_node_id)) m_next_node_id++;
  return add_node(m_next_node_id++);
}

const GraphEdge& Graph::add_edge(int from_id, int to_id) {
  if (!has_node(from_id) || !has_node(to_id))
    throw std::runtime_error("Graph::add_edge: node not found");
  if (has_edge(from_id, to_id))
    throw std::runtime_error("Graph::add_edge: edge already exists");
  if (from_id == to_id)
    throw std::runtime_error("Graph::add_edge: cannot add self-loop");
  auto& from_node = m_nodeid_to_node_map.at(from_id);
  auto& to_node = m_nodeid_to_node_map.at(to_id);
  GraphEdge* edge =
      new GraphEdge(m_next_edge_id++, *from_node, *to_node, *this);
  m_nodeid_to_edgesid[from_id].insert(edge->get_id());
  m_nodeid_to_neighborsid_to_edgesid[from_id][to_id] = edge->get_id();
  m_edgeid_to_edge_map[edge->get_id()] = std::unique_ptr<GraphEdge>(edge);
  m_nodeid_to_incoming_edgesid_map[to_id].insert(edge->get_id());
  return *edge;
}

bool Graph::has_edge(int from_id, int to_id) const {
  if (!has_node(from_id) || !has_node(to_id))
    throw std::runtime_error("Graph::has_edge: node not found");
  return m_nodeid_to_neighborsid_to_edgesid.at(from_id).contains(to_id);
}

const GraphEdge& Graph::get_edge(int from_id, int to_id) const {
  if (!has_node(from_id) || !has_node(to_id))
    throw std::runtime_error("Graph::get_edge: node not found");
  if (!has_edge(from_id, to_id))
    throw std::runtime_error("Graph::get_edge: edge not found");
  int edge_id = get_edge_id(from_id, to_id);
  return get_edge_by_id(edge_id);
}

int Graph::get_edge_id(int from_id, int to_id) const {
  if (!has_node(from_id) || !has_node(to_id))
    throw std::runtime_error("Graph::get_edge_id: node not found");
  if (!has_edge(from_id, to_id))
    throw std::runtime_error("Graph::get_edge_id: edge not found");
  return m_nodeid_to_neighborsid_to_edgesid.at(from_id).at(to_id);
}

const GraphEdge& Graph::get_edge_by_id(int edge_id) const {
  if (!m_edgeid_to_edge_map.contains(edge_id))
    throw std::runtime_error("Graph::get_edge: edge not found");
  return *m_edgeid_to_edge_map.at(edge_id);
}

void Graph::add_undirected_edge(int from_id, int to_id) {
  add_edge(from_id, to_id);
  add_edge(to_id, from_id);
}

void Graph::remove_edge(int from_id, int to_id) {
  if (!has_edge(from_id, to_id))
    throw std::runtime_error("Graph::remove_edge: edge not found");
  int edge_id = get_edge_id(from_id, to_id);
  m_nodeid_to_edgesid[from_id].erase(edge_id);
  m_nodeid_to_neighborsid_to_edgesid[from_id].erase(to_id);
  m_edgeid_to_edge_map.erase(edge_id);
  m_nodeid_to_incoming_edgesid_map[to_id].erase(edge_id);
}

void Graph::remove_undirected_edge(int from_id, int to_id) {
  remove_edge(from_id, to_id);
  remove_edge(to_id, from_id);
}

void Graph::remove_node(int id) {
  if (!has_node(id))
    throw std::runtime_error("Graph::remove_node: node not found");
  std::vector<int> edges_to_remove;
  for (const auto& edge : get_edges_of_node(id))
    edges_to_remove.push_back(edge.get_to().get_id());
  for (int neighbor_id : edges_to_remove) remove_edge(id, neighbor_id);
  edges_to_remove.clear();
  for (int edge_id : m_nodeid_to_incoming_edgesid_map[id])
    edges_to_remove.push_back(get_edge_by_id(edge_id).get_from().get_id());
  for (int neighbor_id : edges_to_remove) remove_edge(neighbor_id, id);
  m_nodeid_to_node_map.erase(id);
  m_nodeid_to_edgesid.erase(id);
  m_nodeid_to_neighborsid_to_edgesid.erase(id);
  m_nodeid_to_incoming_edgesid_map.erase(id);
}

bool GraphAttributes::has_attribute(Attribute attribute) const {
  return mattribute_to_node.contains(attribute);
}

void GraphAttributes::add_attribute(Attribute attribute) {
  if (has_attribute(attribute))
    throw std::runtime_error(
        "GraphAttributes::add_attribute: already has this attribute");
  mattribute_to_node[attribute] = {};
}

bool GraphAttributes::has_attribute_by_id(Attribute attribute, int id) const {
  if (!has_attribute(attribute))
    throw std::runtime_error(
        "GraphAttributes::has_attribute_by_id: does not have attribute");
  return mattribute_to_node.at(attribute).contains(id);
}

const std::any& GraphAttributes::get_attribute_by_id(Attribute attribute,
                                                     int id) const {
  if (!has_attribute_by_id(attribute, id))
    throw std::runtime_error(
        "GraphAttributes::get_attribute_by_id: does not have attribute");
  return mattribute_to_node.at(attribute).at(id);
}

void GraphAttributes::set_node_color(int node_id, Color color) {
  if (has_attribute_by_id(Attribute::NODES_COLOR, node_id))
    throw std::runtime_error(
        "GraphAttributes::set_node_color: the node already has color");
  mattribute_to_node.at(Attribute::NODES_COLOR)[node_id] = color;
}

void GraphAttributes::set_chain_edges(int key,
                                      const std::tuple<int, int>& edge) {
  if (!has_attribute(Attribute::CHAIN_EDGES))
    add_attribute(Attribute::CHAIN_EDGES);

  auto& chain_edges = mattribute_to_node[Attribute::CHAIN_EDGES];

  if (chain_edges.find(key) == chain_edges.end()) {
    chain_edges[key] = std::vector<std::tuple<int, int>>{};
  }

  auto& any_value = chain_edges[key];
  auto& vec = std::any_cast<std::vector<std::tuple<int, int>>&>(any_value);
  vec.push_back(edge);
}

std::vector<std::tuple<int, int>> GraphAttributes::get_chain_edges(int key) {
  if (!has_attribute_by_id(Attribute::CHAIN_EDGES, key))
    throw std::runtime_error(
        "GraphAttributes::get_chain_edges: the chain doesn't exist");

  const std::any& any_value =
      mattribute_to_node.at(Attribute::CHAIN_EDGES).at(key);
  return std::any_cast<const std::vector<std::tuple<int, int>>&>(any_value);
}

Color GraphAttributes::get_node_color(int node_id) const {
  if (!has_attribute_by_id(Attribute::NODES_COLOR, node_id))
    throw std::runtime_error(
        "GraphAttributes::get_node_color: the node does not have a color");
  return std::any_cast<Color>(
      mattribute_to_node.at(Attribute::NODES_COLOR).at(node_id));
}

void GraphAttributes::set_edge_any_label(int edge_id, const std::any& label) {
  if (has_attribute_by_id(Attribute::EDGES_ANY_LABEL, edge_id))
    throw std::runtime_error(
        "GraphAttributes::set_edge_any_label: the edge already has a label");
  mattribute_to_node.at(Attribute::EDGES_ANY_LABEL)[edge_id] = label;
}

const std::any& GraphAttributes::get_edge_any_label(int edge_id) const {
  if (!has_attribute_by_id(Attribute::EDGES_ANY_LABEL, edge_id))
    throw std::runtime_error(
        "GraphAttributes::get_edge_any_label: the edge does not have a label");
  return mattribute_to_node.at(Attribute::EDGES_ANY_LABEL).at(edge_id);
}