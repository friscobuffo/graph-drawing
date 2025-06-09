#include <iostream>

#include "baseline-ogdf/drawer.hpp"
#include "config/config.hpp"
#include "core/graph/file_loader.hpp"
#include "core/graph/generators.hpp"
#include "core/graph/graph.hpp"
#include "orthogonal/drawing_builder.hpp"
#include "orthogonal/drawing_stats.hpp"

std::unique_ptr<Graph> prova0() {
  std::unique_ptr<Graph> graph = std::make_unique<Graph>();
  for (int i = 0; i < 12; ++i) graph->add_node(i);
  graph->add_undirected_edge(0, 1);
  graph->add_undirected_edge(3, 1);
  graph->add_undirected_edge(3, 0);  // aggiunto ora
  graph->add_undirected_edge(3, 2);
  graph->add_undirected_edge(0, 2);
  graph->add_undirected_edge(3, 4);
  graph->add_undirected_edge(4, 5);
  graph->add_undirected_edge(5, 6);
  graph->add_undirected_edge(7, 6);
  graph->add_undirected_edge(0, 8);
  graph->add_undirected_edge(0, 7);
  graph->add_undirected_edge(0, 10);
  graph->add_undirected_edge(10, 11);
  graph->add_undirected_edge(3, 11);
  graph->add_undirected_edge(3, 9);
  graph->add_undirected_edge(8, 9);
  return std::move(graph);
}

std::unique_ptr<Graph> prova1() {
  std::unique_ptr<Graph> graph = std::make_unique<Graph>();
  for (int i = 0; i < 16; ++i) graph->add_node(i);
  graph->add_undirected_edge(0, 1);
  graph->add_undirected_edge(1, 2);
  graph->add_undirected_edge(2, 3);
  graph->add_undirected_edge(3, 0);
  graph->add_undirected_edge(4, 5);
  graph->add_undirected_edge(5, 6);
  graph->add_undirected_edge(6, 2);
  graph->add_undirected_edge(7, 8);
  graph->add_undirected_edge(8, 9);
  graph->add_undirected_edge(9, 11);
  graph->add_undirected_edge(10, 11);
  graph->add_undirected_edge(6, 12);
  graph->add_undirected_edge(12, 10);
  graph->add_undirected_edge(10, 2);
  graph->add_undirected_edge(13, 14);
  graph->add_undirected_edge(14, 7);
  graph->add_undirected_edge(13, 0);
  graph->add_undirected_edge(15, 13);
  graph->add_undirected_edge(15, 4);
  graph->add_undirected_edge(0, 7);
  graph->add_undirected_edge(3, 8);
  graph->add_undirected_edge(2, 9);
  graph->add_undirected_edge(1, 5);
  graph->add_undirected_edge(4, 0);
  return std::move(graph);
}

void cut_vertices_stats(std::string& folder_path) {
  auto txt_files = collect_txt_files(folder_path);
  int pattern_1_times = 0;
  int pattern_2_times = 0;
  int pattern_3_times = 0;
  for (const auto& entry_path : txt_files) {
    std::unique_ptr<Graph> graph = load_graph_from_txt_file(entry_path);
    auto biconnected_components = compute_biconnected_components(*graph);
    for (const int cutvertex : biconnected_components.cutvertices) {
      int node_degree = graph->get_node_by_id(cutvertex).get_degree();
      if (node_degree == 4) {
        int components_with_this_cutvertex = 0;
        for (auto& component : biconnected_components.components)
          if (component->has_node(cutvertex)) components_with_this_cutvertex++;
        if (components_with_this_cutvertex == 3)
          pattern_2_times++;
        else if (components_with_this_cutvertex == 2) {
          for (auto& component : biconnected_components.components) {
            if (component->has_node(cutvertex)) {
              int deg = component->get_node_by_id(cutvertex).get_degree();
              if (deg == 2) pattern_1_times++;
              break;
            }
          }
        }
      } else if (node_degree == 3) {
        int components_with_this_cutvertex = 0;
        for (auto& component : biconnected_components.components)
          if (component->has_node(cutvertex)) components_with_this_cutvertex++;
        if (components_with_this_cutvertex == 2) pattern_3_times++;
      }
    }
  }
  std::cout << "pattern 1: " << pattern_1_times << "\n";
  std::cout << "pattern 2: " << pattern_2_times << "\n";
  std::cout << "pattern 3: " << pattern_3_times << "\n";
}

int main() {
  // auto graph = generate_connected_random_graph(20, 35);
  // auto graph_rg = load_graph_from_txt_file("rome_2/grafo113.28.txt");
  // auto graph_rg = load_graph_from_txt_file("rome_2/grafo115.30.txt");
  // auto graph_rg = load_graph_from_txt_file("rome_2/grafo3470.41.txt");
  // auto result_ogdf = create_drawing(*graph_rg, "rome_ogdf.svg");
  // graph_rg->print();
  // auto result_rg = make_orthogonal_drawing_sperimental(*graph_rg);
  // node_positions_to_svg(result_rg.positions, *result_rg.augmented_graph,
  //                       result_rg.attributes, "rome.svg");
  // auto graph = generate_connected_random_graph(15, 23);
  // auto result = make_orthogonal_drawing_sperimental(*graph);
  // node_positions_to_svg(result.positions, *result.augmented_graph,
  //                       result.attributes, "daje.svg");

  // std::unique_ptr<Graph> graph0 = prova0();
  // result = make_orthogonal_drawing_sperimental(*graph0);
  // node_positions_to_svg(result.positions, *result.augmented_graph,
  //                       result.attributes, "daje0.svg");
  // std::unique_ptr<Graph> graph1 = prova1();
  // result = make_orthogonal_drawing_sperimental(*graph1);
  // node_positions_to_svg(result.positions, *result.augmented_graph,
  //                       result.attributes, "daje1.svg");
  // auto graph_rg = load_graph_from_txt_file("rome_2/grafo11549.35.txt");
  auto graph_rg = load_graph_from_txt_file("rome_2/grafo149.41.txt");
  auto result_rg = make_orthogonal_drawing_sperimental(*graph_rg);
  node_positions_to_svg(result_rg.positions, *result_rg.augmented_graph,
                        result_rg.attributes, "rome2.svg");
  std::cout << "crossings: " << compute_total_crossings(result_rg) << "\n";
  std::cout << "area: " << compute_total_area(result_rg) << "\n";

  // prova_special();
  return 0;
  // Config config("config.txt");
  // const std::string& filename = config.get("output_svg_shape_metrics");
  // auto graph = load_graph_from_txt_file(config.get("input_graph_file"));

  // auto result_ogdf = create_drawing(*graph, config.get("output_svg_ogdf"));
  // std::cout << "OGDF:\n";
  // std::cout << "Area: " << result_ogdf.area << "\n";
  // std::cout << "Crossings: " << result_ogdf.crossings << "\n";
  // std::cout << "Bends: " << result_ogdf.bends << "\n";
  // std::cout << "Total edge length: " << result_ogdf.total_edge_length <<
  // "\n";

  // srand(0);
  // auto result = make_orthogonal_drawing_sperimental(*graph);
  // node_positions_to_svg(result.positions, *result.augmented_graph,
  //                       result.attributes, filename);
  // auto stats = compute_all_orthogonal_stats(result);
  // std::cout << "Shape metrics:\n";
  // std::cout << "Area: " << stats.area << "\n";
  // std::cout << "Crossings: " << stats.crossings << "\n";
  // std::cout << "Bends: " << stats.bends << "\n";
  // std::cout << "Total edge length: " << stats.total_edge_length << "\n";

  // // std::string graphs_folder = "generated-graphs";
  // // cut_vertices_stats(graphs_folder);

  // return 0;
}