// pipelineDP orthopteroid@gmail.com, MIT license
// based upon "Optimal Route Location for Pipelines Carrying Liquid And Gas In Two Phase Flow", U.Shamir, 1969

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <limits>
#include <valarray>

struct StripInfo { float x, y, z; int rc, lt, tt; int pb, pe; };
using InputStrip = std::vector<StripInfo>;

using Node = uint16_t;
using Edge = std::pair<Node, Node>;

struct EdgeInfo { float edge_cost; };

struct XYZ { float x, y, z; };
struct PathStep { Node from_node; float path_cost; };

const int nptINCR = 10;
const int nptSIZE = nptINCR +1;

// used to dimension a node's L vs H pressure curves
struct NodePressureTable {
    float min_len, max_len, min_hill, max_hill;
    float scale_len[nptSIZE];
    float scale_hill[nptSIZE];
    float table[nptSIZE][nptSIZE];
};

const float fMAX = std::numeric_limits<float>::max();
const Node nMAX = std::numeric_limits<Node>::max();

const PathStep psMAX = {nMAX, fMAX};
const PathStep psZERO = {nMAX, 0};
const NodePressureTable npsInit = {fMAX, -fMAX, fMAX, -fMAX /* ... */};

enum : int {NoLand = 0, Water, Swamp, Rock, Soil};
enum : int {NoTrees = 0, SmallTrees, LargeTrees};

/////////

// UMaALtd Report p55-p57 (Jumping Pound sample problem)
// nb: edge length computed from node coordinates
// nb: river crossings ignored
std::vector<InputStrip> input_strips =
{
    {
        {0, 0, 395, -1, -1, -1, -1, -1, },
    },
    {
        {6550, -3240, 403, 0, Swamp, SmallTrees,0, 0, },
        {5850, -2245, 402, 0, Swamp,SmallTrees,0, 0, },
        {5220, -1407, 395, 0, Swamp,SmallTrees,0, 0, },
        {4560, -332, 413, 0, Swamp,SmallTrees,0, 0, },
        {3820, -665, 445, 0, Swamp,SmallTrees,0, 0, },
        {3150, -1660, 448, 0, Swamp,SmallTrees,0, 0, },
    },
    {
        {8800, -3650, 340, 0, Swamp, SmallTrees, 0, 2, },
        {8060, -2160, 320, 0, Soil, SmallTrees, 0, 3, },
        {7395, -1050, 318, 0, Swamp, SmallTrees, 0, 3, },
        {6650, -166, 315, 0, Soil, SmallTrees, 1, 4, },
        {5810, 1080, 342, 0, Rock, SmallTrees, 3, 5, },
        {4980, 2245, 340, 0, Soil, SmallTrees, 4, 5, },
    },
    {
        {14200, -4230, 170, 0, Swamp, LargeTrees, 0, 2, },
        {12600, -3240, 185, 0, Swamp, LargeTrees, 0, 2, },
        {11300, -2160, 195, 0, Swamp, LargeTrees, 0, 3, },
        {10400, -995, 200, 0, Swamp, SmallTrees, 1, 4, },
        {9450, -83, 225, 0, Swamp, SmallTrees, 1, 4, },
        {8550, 747, 252, 0, Swamp, LargeTrees, 2, 5, },
        {7550, 1660, 270, 0, Swamp, LargeTrees, 3, 5, },
        {6650, 2245, 276, 0, Swamp, LargeTrees, 4, 5, },
    },
    {
        {16050, -4325, 188, 0, Swamp, SmallTrees, 0, 1, },
        {15450, -3820, 200, 0, Swamp, SmallTrees, 0, 1, },
        {14480, -2825, 220, 0, Swamp, SmallTrees, 0, 2, },
        {13470, -1660, 240, 0, Swamp, LargeTrees, 1, 3, },
        {12800, -415, 300, 0, Swamp, LargeTrees, 2, 5, },
        {12170, 830, 250, 0, Swamp, LargeTrees, 3, 6, },
        {11750, 1910, 210, 0, Swamp, SmallTrees, 4, 7, },
        {11400, 2990, 190, 0, Swamp, SmallTrees, 5, 7, },
    },
    {
        {19500, -4150, 100, 0, Rock, SmallTrees, 0, 2, },
        {18850, -3245, 100, 0, Rock, SmallTrees, 0, 3, },
        {18300, -2480, 100, 0, Rock, SmallTrees, 0, 3, },
        {17900, -1660, 100, 0, Rock, LargeTrees, 1, 4, },
        {16900, -581, 110, 0, Swamp, LargeTrees, 2, 5, },
        {16050, 333, 120, 0, Swamp, LargeTrees, 3, 6, },
        {15380, 1165, 125, 0, Swamp, LargeTrees, 3, 7, },
        {14800, 1995, 150, 0, Swamp, LargeTrees, 4, 7, },
        {14150, 2825, 175, 0, Swamp, LargeTrees, 5, 7, },
    },
    {
        {22600, -3750, 70, 0, Swamp, LargeTrees, 0, 2, },
        {22100, -2820, 73, 0, Swamp, LargeTrees, 0, 3, },
        {21450, -1990, 95, 0, Swamp, LargeTrees, 0, 4, },
        {20900, -1285, 125, 0, Swamp, SmallTrees, 1, 5, },
        {20200, -498, 170, 0, Swamp, SmallTrees, 2, 6, },
        {19800, 166, 220, 0, Swamp, LargeTrees, 2, 6, },
        {19350, 748, 275, 0, Water, LargeTrees, 3, 7, },
        {18800, 1245, 350, 0, Water, LargeTrees, 4, 8, },
        {18300, 2035, 395, 0, Water, LargeTrees, 5, 8, },
    },
    {
        {25300, -2745, 35, 0, Swamp, SmallTrees, 0, 2, },
        {24900, -2245, 35, 0, Rock, SmallTrees, 0, 2, },
        {24600, -1700, 35, 0, Rock, SmallTrees, 0, 4, },
        {24200, -995, 40, 0, Rock, SmallTrees, 1, 5, },
        {23800, -249, 70, 0, Rock, SmallTrees, 1, 6, },
        {23300, 665, 75, 0, Rock, LargeTrees, 2, 8, },
        {22700, 1415, 75, 0, Swamp, LargeTrees, 4, 8, },
    },
    {
        {30900, -1785, 25, 0, Swamp, SmallTrees, 0, 3, },
        {26900, -1285, 25, 0, Swamp, LargeTrees, 0, 4, },
        {26550, -665, 65, 0, Swamp, LargeTrees, 0, 5, },
        {26300, -166, 90, 0, Swamp, LargeTrees, 1, 6, },
        {26000, 374, 120, 0, Swamp, SmallTrees, 2, 6, },
        {25500, 1080, 150, 0, Swamp, SmallTrees, 3, 6, },
    },
    {
        {30500, 0, -10, 0, Swamp, SmallTrees, 0, 5, },
    },
};

std::array<float, 5> land_costs = { 0, 1.5, 0.8, 2.5, 0.2, }; // none, water, swamp, rock, soil
std::array<float, 3> tree_costs = { 0, 0, 0.55, }; // none, small, large

// "Two Phase Gathering Systems", The Oil and Gas Journal, Flanigan, 1958
// pressure loss due to pipe length and pressure loss due to 'hills' (+ve vertical change)
// empirical curve-fit method, for more complexity use Baker's method (Pipeline Engineer Handbook, Baker, 1960, ppH67)
struct PressureLoss { float alpha, beta; /* dP for length and height in psi / foot */ };
PressureLoss flanigans_method(float inlet_pressure, float outlet_pressure)
{
    const double ftPerMile = 5280;
    const double D = 12.17; // diameter
    const double QG = 73; // gas flow rate in MMCFPD
    const double R = 7.3; // gas / oil ratio
    const double SG = .7; // gas gravity
    const double SL = 51.2; // liquid gravity
    const double T = 90; // avg line temperature (buried pipe)
    const double Z = .867; // gas compressibilty factor
    const double Pav = (inlet_pressure + outlet_pressure) / 2; // avg pressure
    const double U = 31194.f * QG * Z / Pav * D * D; // superficial velocity
    const double F1_logterm = std::log(U / std::pow(R, 0.32));
    const double F1 = std::exp(-.7464 * F1_logterm * F1_logterm + .4772 * F1_logterm - .8003); // friction loss efficiency factor
    const double C = 20500 / (std::pow(SG, .46) * std::pow(T+460,.54)); // friction factor for pressure loss
    return
    {
        float(std::pow(QG * 1E6 / ( C * std::pow(D, 2.6182) * F1 ), 1.853) / ( 2 * Pav ) / ftPerMile),
        float(SL * 3.06 / (144 * U * 3.06))
    };
}

int main()
{
    std::vector< std::vector<Node> > input_strip_numbering(input_strips.size());
    std::vector<NodePressureTable> node_pressure_tables;
    std::vector<XYZ> node_pos;
    std::vector< std::deque<Node> > bwd_linkage;
    std::vector< std::deque<Node> > fwd_linkage;
    std::map<Edge, EdgeInfo> edge_cost;

    auto edge_length = [&](Node f, Node t) -> float
    {
        const float dx = node_pos[t].x - node_pos[f].x;
        const float dy = node_pos[t].y - node_pos[f].y;
        const float dz = node_pos[t].z - node_pos[f].z;
        return std::sqrt( dx * dx + dy * dy + dz * dz );
    };

    auto edge_hill = [&](Node f, Node t) -> float
    {
        const float dz = node_pos[t].z - node_pos[f].z;
        return dz > 0 ? dz : 0;
    };

    PressureLoss pressure_loss = flanigans_method(880, 815);

    std::cout << "Parsing input data...\n";
    Node n;
    int s, i, j;
    for(n = 0, s = 0; s < input_strips.size(); ++s)
    {
        input_strip_numbering[s].resize(input_strips[s].size());
        for (i = 0; i < input_strips[s].size(); ++i)
            input_strip_numbering[s][i] = n + i;
        n += input_strips[s].size();
    }
    bwd_linkage.resize(n );
    fwd_linkage.resize(n );
    node_pressure_tables.resize(n);

    n = 1; // nb: start at 1
    StripInfo& info = input_strips[0][0];
    node_pos.push_back({info.x, info.y, info.z}); // position
    for(s = 1; s < input_strips.size(); ++s) // nb: start at 1
        for(i = 0; i < input_strips[s].size(); ++i, ++n)
        {
            info = input_strips[s][i];
            node_pos.push_back({info.x, info.y, info.z}); // position
            for(j = info.pb; j <= info.pe; ++j) // nb: inclusive end
            {
                Node pn = input_strip_numbering[s - 1][j];
                bwd_linkage[n].push_back(pn);
                fwd_linkage[pn].push_back(n);
            }
            for(j = info.pb; j <= info.pe; ++j) // nb: inclusive end
            {
                Node pn = input_strip_numbering[s - 1][j];
                edge_cost.insert({{input_strip_numbering[s - 1][j], n}, {edge_length(pn, n) * (land_costs[info.lt] + tree_costs[info.tt])}});
            }
        }

    std::deque<Node> visit_stack;
    auto visit_stack_pop = [&]()
    { Node t = visit_stack.front(); visit_stack.pop_front(); return t; };

    std::vector<bool> fwd_visit_marking(fwd_linkage.size());
    auto visit_stack_unique_visitation = [&] (Node n)
    {
        if (!fwd_visit_marking[n])
        {
            fwd_visit_marking[n] = true;
            visit_stack.push_back(n);
        }
    };

    std::cout << "Computing Node L & H pressure tables...\n";
    std::fill(node_pressure_tables.begin(), node_pressure_tables.end(), npsInit);
    node_pressure_tables[0] = {0, 0, 0, 0};
    visit_stack = {0};
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t)
        {
            float len = edge_length(f, t);
            float hill = edge_hill(f, t);
            node_pressure_tables[t].min_len = std::min(node_pressure_tables[t].min_len, node_pressure_tables[f].min_len + len);
            node_pressure_tables[t].max_len = std::max(node_pressure_tables[t].max_len, node_pressure_tables[f].max_len + len);
            node_pressure_tables[t].min_hill = std::min(node_pressure_tables[t].min_hill, node_pressure_tables[f].min_hill + hill);
            node_pressure_tables[t].max_hill = std::max(node_pressure_tables[t].max_hill, node_pressure_tables[f].max_hill + hill);
            visit_stack.push_back(t);
        });
    }

    // tailor a specialized table for each node
    std::for_each(node_pressure_tables.begin(), node_pressure_tables.end(), [&](NodePressureTable& npt)
    {
        if(npt.max_len == 0)
            return;
        float dl = (npt.max_len - npt.min_len) / nptINCR;
        float dh = (npt.max_hill - npt.min_hill) / nptINCR;
        for(i = 0; i < nptSIZE; ++i)
        {
            npt.scale_len[i] = npt.min_len + i * dl;
            npt.scale_hill[i] = npt.min_hill + i * dh;
        }
        for(i = 0; i < nptSIZE; ++i) // len
            for(j = 0; j < nptSIZE; ++j) // hill
                npt.table[i][j] = pressure_loss.alpha * npt.scale_len[i] + pressure_loss.beta * npt.scale_hill[j];
    });

    std::cout << "Node Pressure Limits and Linkages:\n";
    visit_stack = {0};
    std::fill(fwd_visit_marking.begin(), fwd_visit_marking.end(), false);
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        auto nl = node_pressure_tables[f];
        std::cout << f;
        std::cout << " mnlen " << nl.min_len << " mxlen " << nl.max_len << " mnhill " << nl.min_hill << " mxhill " << nl.max_hill;
        if( fwd_linkage[f].size() > 0)
            std::cout << " : ";
        std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t)
        {
            std::cout << t;
            std::cout << " cost " << edge_cost[{f, t}].edge_cost << " ";
            visit_stack_unique_visitation(t);
        });
        std::cout << "\n";
    }

    std::vector<PathStep> fwd_local_min(fwd_linkage.size(), psMAX );

    // accumulate minimum forward pass
    // start at node 0
    fwd_local_min[0] = psZERO;
    visit_stack = {0};
    std::fill(fwd_visit_marking.begin(), fwd_visit_marking.end(), false);
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t)
        {
            auto local_cost = fwd_local_min[f].path_cost + edge_cost[{f, t}].edge_cost;
            if (local_cost < fwd_local_min[t].path_cost)
                fwd_local_min[t] = {f, local_cost};
            visit_stack_unique_visitation(t);
        });
    }

    // select minimum backward pass
    // start at node 'bwd_linkage.size() -1', end at node 0
    std::deque<PathStep> bwd_global_min;
    Node t = bwd_linkage.size() - 1;
    bwd_global_min.push_front({t, fwd_local_min[t].path_cost} );
    while (t != 0)
    {
        PathStep min_route_cost = psMAX;
        std::for_each(bwd_linkage[t].begin(), bwd_linkage[t].end(), [&](Node f)
        {
            if (fwd_local_min[f].path_cost < min_route_cost.path_cost)
                min_route_cost = {f, fwd_local_min[f].path_cost};
        });
        bwd_global_min.push_front(min_route_cost);
        t = min_route_cost.from_node;
    }

    std::cout << "Optimal route and cumulative path cost:\n";
    std::for_each(bwd_global_min.begin(), bwd_global_min.end(), [&](PathStep& nc)
    { std::cout << nc.from_node << ' ' << nc.path_cost << "\n"; });
    return 0;
}
