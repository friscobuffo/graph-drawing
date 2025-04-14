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
#include "drawer.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <set>

namespace fs = std::filesystem;
using namespace ogdf;

int compute_area(const GraphAttributes &GA, const Graph &G)
{
    std::set<double> x_coords;
    std::set<double> y_coords;
    for (node v : G.nodes)
    {
        double x = GA.x(v);
        double y = GA.y(v);

        x_coords.insert(x);
        y_coords.insert(y);
    }
    int x = 0;
    int y = 0;
    for (int i = 0; i < x_coords.size(); i++)
    {
        x++;
    }
    for (int i = 0; i < y_coords.size(); i++)
    {
        y++;
    }
    int area = x * y;
    return area;
}

int count_crossings(const GraphAttributes &GA, const LayoutStatistics &stats)
{
    int crossings = 0;
    for (auto &elem : stats.numberOfCrossings(GA))
    {
        crossings += elem;
    }
    return crossings / 2;
}

int count_bends(const GraphAttributes &GA, const Graph &G)
{
    int bends = 0;
    for (edge e : G.edges)
    {
        int bend_size = GA.bends(e).size();
        while (bend_size > 2)
        {
            bends++;
            bend_size--;
        }
    }
    return bends;
}

OGDFResult create_drawing(const std::string input_file)
{
    Graph G;
    GraphAttributes GA(G,
                       GraphAttributes::nodeGraphics | GraphAttributes::nodeType |
                           GraphAttributes::edgeGraphics | GraphAttributes::edgeType);

    if (!GraphIO::read(GA, G, input_file, GraphIO::readGML))
    {
        std::cerr << "Could not read " << input_file << std::endl;
    }

    for (node v : G.nodes)
    {
        GA.width(v) /= 2;
        GA.height(v) /= 2;
    }

    PlanarizationLayout pl;

    SubgraphPlanarizer *crossMin = new SubgraphPlanarizer;
    PlanarSubgraphFast<int> *ps = new PlanarSubgraphFast<int>;
    ps->runs(100);
    VariableEmbeddingInserter *ves = new VariableEmbeddingInserter;
    ves->removeReinsert(RemoveReinsertType::All);

    crossMin->setSubgraph(ps);
    crossMin->setInserter(ves);
    pl.setCrossMin(crossMin);

    EmbedderMinDepthMaxFaceLayers *emb = new EmbedderMinDepthMaxFaceLayers;
    pl.setEmbedder(emb);

    OrthoLayout *ol = new OrthoLayout;
    ol->separation(20.0);
    ol->cOverhang(0.4);
    pl.setPlanarLayouter(ol);

    pl.call(GA);
    LayoutStatistics stats;

    std::string output_file = input_file.substr(input_file.find_last_of("/\\") + 1);
    output_file = output_file.substr(0, output_file.find_last_of('.'));
    std::string output_file_gml = std::format("test-graphs/output/ogdf/{}.gml", output_file);
    std::string output_file_svg = std::format("test-graphs/output/ogdf/{}.svg", output_file);
    GraphIO::write(GA, output_file_gml, GraphIO::writeGML);
    GraphIO::write(GA, output_file_svg, GraphIO::drawSVG);

    return {
        count_crossings(GA, stats),
        count_bends(GA, G),
        compute_area(GA, G)};
}


