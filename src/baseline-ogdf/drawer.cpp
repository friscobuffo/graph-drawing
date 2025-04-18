#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/GraphList.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/orthogonal/OrthoLayout.h>
#include <ogdf/planarity/EmbedderMinDepthMaxFaceLayers.h>
#include <ogdf/planarity/PlanarSubgraphFast.h>
#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/planarity/RemoveReinsertType.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>
#include <ogdf/planarity/VariableEmbeddingInserter.h>
#include <ogdf/basic/LayoutStatistics.h>
#include "../core/graph/graph.hpp"
#include "../orthogonal/drawing_builder.hpp"
#include "../orthogonal/orthogonal_algorithms.hpp"
#include "../core/graph/graphs_algorithms.hpp"
#include "drawer.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <set>

void set_horizontal_direction(Shape *shape, int id_source, int x_source, int id_target, int x_target) {
    if (x_source < x_target) {
        shape->set_direction(id_source, id_target, Direction::RIGHT);
        shape->set_direction(id_target, id_source, Direction::LEFT);
    }
    else if (x_source > x_target) {
        shape->set_direction(id_source, id_target, Direction::LEFT);
        shape->set_direction(id_target, id_source, Direction::RIGHT);
    }
}

void set_vertical_direction(Shape *shape, int id_source, int y_source, int id_target, int y_target) {
    if (y_source < y_target) {
        shape->set_direction(id_source, id_target, Direction::DOWN);
        shape->set_direction(id_target, id_source, Direction::UP);
    }
    else if (y_source > y_target) {
        shape->set_direction(id_source, id_target, Direction::UP);
        shape->set_direction(id_target, id_source, Direction::DOWN);
    }
}

int compute_area_1(const ogdf::GraphAttributes &GA, const ogdf::Graph &G) {
    std::set<double> x_coords;
    std::set<double> y_coords;
    for (ogdf::node v : G.nodes) {
        double x = GA.x(v);
        double y = GA.y(v);
        x_coords.insert(x);
        y_coords.insert(y);
    }
    for (ogdf::edge e : G.edges) {
        int bend_size = GA.bends(e).size();
        if (bend_size > 2) {
            std::vector<ogdf::DPoint> bend_vec;
            for (auto& elem : GA.bends(e))
                bend_vec.push_back(elem);
            for (size_t j = 1; j < bend_vec.size() - 1; ++j) {
                double x_source = bend_vec[j - 1].m_x;
                double y_source = bend_vec[j - 1].m_y;
                double x_bend = bend_vec[j].m_x;
                double y_bend = bend_vec[j].m_y;
                double x_target = bend_vec[bend_vec.size() - 1].m_x;
                double y_target = bend_vec[bend_vec.size() - 1].m_y;
                if (x_source != x_bend && x_target != x_bend)
                    x_coords.insert(x_bend);
                if (y_source != y_bend && y_target != y_bend)
                    y_coords.insert(y_bend);
            }
        }
    }
    // std::cout << "coor before: ";
    // for (const auto coor : x_coords)
    // {
    //     std::cout << coor << " ";
    // }
    std::cout << std::endl;
        std::set<double> x_coords_approx;
        for (auto it = x_coords.begin(); std::next(it) != x_coords.end(); ++it) {
        auto next_it = std::next(it);
        double diff = *next_it - *it;
        if (diff > 10) {
            x_coords_approx.insert(*it);
            if(std::next(next_it) == x_coords.end())
                x_coords_approx.insert(*next_it);
        }        
    }
        std::set<double> y_coords_approx;
        for (auto it = y_coords.begin(); std::next(it) != y_coords.end(); ++it) {
        auto next_it = std::next(it);
        double diff = *next_it - *it;
        if (diff > 10)  {
            y_coords_approx.insert(*it);
            if(std::next(next_it) == y_coords.end())
                y_coords_approx.insert(*next_it);
        }
    }
    // std::cout << "coor after: ";
    // for (const auto coor : x_coords_approx)
    // {
    //     std::cout << coor << " ";
    // }
    // std::cout << std::endl;
    int x = x_coords_approx.size() - 1;
    int y = y_coords_approx.size() - 1;
    return x * y;
}

int compute_area_2(Shape *shape, ColoredNodesGraph *colored_graph) {
    BuildingResult *result = build_nodes_positions(*shape, *colored_graph);
    // node_positions_to_svg(*result->positions, *colored_graph);

    return compute_total_area(*result->positions, *colored_graph);
}

int compute_area_from_shape(const ogdf::GraphAttributes &GA, ogdf::Graph &G) {
    Shape *shape = new Shape();
    ColoredNodesGraph *colored_graph = new ColoredNodesGraph();
    int bend_id = G.nodes.size();
    for (int i = 0; i < G.nodes.size(); ++i)
        colored_graph->add_node(Color::BLACK);
    for (ogdf::edge e : G.edges) {
        auto source = e->source();
        int bend_size = GA.bends(e).size();
        if (bend_size > 2) {
            std::vector<ogdf::DPoint> bendVec;
            for (auto& elem : GA.bends(e))
                bendVec.push_back(elem);
            for (size_t j = 1; j < bendVec.size(); ++j) {
                int source_id = bend_id - 1;
                int target_id = bend_id;
                if (j == 1) {
                    source_id = source->index();
                    colored_graph->add_node(Color::BLACK);
                }
                if (j == bendVec.size() - 1) {
                    source_id = bend_id - 1;
                    target_id = e->target()->index();
                    bend_id--;
                }
                if (j != 1 && j != bendVec.size() - 1)
                    colored_graph->add_node(Color::BLACK);
                set_horizontal_direction(shape, source_id, bendVec[j - 1].m_x, target_id, bendVec[j].m_x);
                set_vertical_direction(shape, source_id, bendVec[j - 1].m_y, target_id, bendVec[j].m_y);
                bend_id++;
                colored_graph->add_edge(source_id, target_id);
                colored_graph->add_edge(target_id, source_id);
            }
        }
        else {
            auto target = e->target();
            set_horizontal_direction(shape, source->index(), GA.x(source), target->index(), GA.x(target));
            set_vertical_direction(shape, source->index(), GA.y(source), target->index(), GA.y(target));
            colored_graph->add_edge(source->index(), target->index());
            colored_graph->add_edge(target->index(), source->index());
        }
    }
    return compute_area_2(shape, colored_graph);
    // std::cout << shape->to_string() << std::endl;
    // std::cout << colored_graph->to_string() << std::endl;
}

int count_crossings(const ogdf::GraphAttributes &GA, const ogdf::LayoutStatistics &stats) {
    int crossings = 0;
    for (auto &elem : stats.numberOfCrossings(GA)) {
        crossings += elem;
    }
    return crossings / 2;
}

int count_bends(const ogdf::GraphAttributes &GA, const ogdf::Graph &G) {
    int bends = 0;
    for (ogdf::edge e : G.edges) {
        int bend_size = GA.bends(e).size();
        if (bend_size > 2) {
            int i = bend_size - 2;
            bends += i;
        }
    }
    return bends;
}

OGDFResult create_drawing(
    const SimpleGraph& graph,
    const std::string svg_output_filename,
    const std::string gml_output_filename
) {
    ogdf::Graph G;
    ogdf::GraphAttributes GA(
        G, ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::nodeType |
        ogdf::GraphAttributes::edgeGraphics | ogdf::GraphAttributes::edgeType |
        ogdf::GraphAttributes::nodeLabel | ogdf::GraphAttributes::nodeStyle |
        ogdf::GraphAttributes::nodeTemplate
    );
    std::vector<ogdf::node> nodes;
    for (int i = 0; i < graph.size(); ++i)
        nodes.push_back(G.newNode());
    for (int i = 0; i < graph.size(); ++i) {
        for (const auto& edge : graph.get_node(i).get_edges()) {
            int j = edge.get_to();
            if (i < j)
                G.newEdge(nodes[i], nodes[j]);
        }
    }
    // if (!ogdf::GraphIO::read(GA, G, input_file, ogdf::GraphIO::readGML))
    //     std::cerr << "Could not read " << input_file << std::endl;

    for (ogdf::node v : G.nodes) {
        GA.width(v) /= 2;
        GA.height(v) /= 2;
        GA.label(v) = std::to_string(v->index());
    }

    ogdf::PlanarizationLayout pl;

    ogdf::SubgraphPlanarizer *crossMin = new ogdf::SubgraphPlanarizer;
    ogdf::PlanarSubgraphFast<int> *ps = new ogdf::PlanarSubgraphFast<int>;
    ps->runs(100);
    ogdf::VariableEmbeddingInserter *ves = new ogdf::VariableEmbeddingInserter;
    ves->removeReinsert(ogdf::RemoveReinsertType::All);

    crossMin->setSubgraph(ps);
    crossMin->setInserter(ves);
    pl.setCrossMin(crossMin);

    ogdf::EmbedderMinDepthMaxFaceLayers *emb = new ogdf::EmbedderMinDepthMaxFaceLayers;
    pl.setEmbedder(emb);

    ogdf::OrthoLayout *ol = new ogdf::OrthoLayout;
    ol->separation(20.0);
    ol->cOverhang(0.4);
    pl.setPlanarLayouter(ol);

    pl.call(GA);
    ogdf::LayoutStatistics stats;
    if (svg_output_filename != "")
        ogdf::GraphIO::write(GA, svg_output_filename, ogdf::GraphIO::drawSVG);
    if (gml_output_filename != "")
        ogdf::GraphIO::write(GA, gml_output_filename, ogdf::GraphIO::writeGML);

    return {
        count_crossings(GA, stats),
        count_bends(GA, G),
        compute_area_1(GA, G)};
}


