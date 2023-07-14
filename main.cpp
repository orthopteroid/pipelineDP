// pipelineDP orthopteroid@gmail.com, MIT license
// based upon "Optimal Route Location for Pipelines Carrying Liquid And Gas In Two Phase Flow", U.Shamir, 1969

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <limits>
#include <valarray>

struct StripInfo { float x, y, z; int rc, lt, tt; int ps, pe; };
using InputStrip = std::vector<StripInfo>;

using Node = uint16_t;
using Edge = std::pair<Node, Node>;

struct EdgeInfo { float edge_cost; };
using EdgeMap = std::map<Edge, EdgeInfo>;
using DAG = std::vector< std::deque<Node> >;

struct XYZ { float x, y, z; };
struct NodeAndCost { Node node; float path_cost; };

const NodeAndCost nacMAX = {std::numeric_limits<uint16_t>::max(), std::numeric_limits<float>::max()};
const NodeAndCost nacZERO = {std::numeric_limits<uint16_t>::max(), 0};

enum : int {NoLand = 0, Water, Swamp, Rock, Soil};
enum : int {NoTrees = 0, SmallTrees, LargeTrees};

EdgeInfo edge_info(const int rc, const int lt, const int tt, const float length)
{
    static std::array<float, 3> tree_costs = { 0, 0, 0.55, }; // none, small, large
    static std::array<float, 5> land_costs = { 0, 1.5, 0.8, 2.5, 0.2, }; // none, water, swamp, rock, soil
    return {rc * 10000 + length * (land_costs[lt] + tree_costs[tt]) };
}

/////////

// UMaALtd Report p55-p57 (Jumping Pound sample problem)
// nb: edge length computed from node coordinates
// nb: river crossings ignored
std::vector<InputStrip> inputf =
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
        {25500, 1080, 1500, 0, Swamp, SmallTrees, 3, 6, },
    },
    {
        {30500, 0, -10, 0, Swamp, SmallTrees, 0, 5, },
    },
};

std::vector<XYZ> node_pos =
{
    /* 0 */ {0, 0, 0},
    /* 1 */ {1, 1, 0},
    /* 2 */ {1, 2, 0},
    /* 3 */ {2, 3, 0},
};

DAG fwd_linkage =
{
    /* 0 */ { {1, 2} },
    /* 1 */ { {3} },
    /* 2 */ { {3} },
    /* 3 */ { /* end */ },
};

EdgeMap edge_cost =
{
    { {0,1}, edge_info(0, Soil, SmallTrees, 4500) },
    { {0,2}, edge_info(0, Soil, SmallTrees, 6000) },
    { {1,3}, edge_info(0, Soil, SmallTrees, 5800) },
    { {2,3}, edge_info(0, Soil, SmallTrees, 3500) },
};

int main()
{
    std::deque<Node> visit_stack;
    std::vector<bool> fwd_visit_marking(fwd_linkage.size());

    auto visit_stack_pop = [&]()
    { Node t = visit_stack.front(); visit_stack.pop_front(); return t; };

    auto visit_stack_unique_visitation = [&] (Node n)
    {
        if (!fwd_visit_marking[n])
        {
            fwd_visit_marking[n] = true;
            visit_stack.push_back(n);
        }
    };

    DAG bwd_linkage;
    bwd_linkage.resize(fwd_linkage.size() );

    std::cout << "Linkages and costs:\n";
    visit_stack = {0};
    std::fill(fwd_visit_marking.begin(), fwd_visit_marking.end(), false);
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        std::cout << f << ": ";
        std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t)
        {
            std::cout << t << " " << edge_cost[{f, t}].edge_cost << " ";
            visit_stack_unique_visitation(t);
        });
        std::cout << "\n";
    }

    std::vector<NodeAndCost> fwd_local_min(fwd_linkage.size(), nacMAX );

    // accumulate minimum forward pass
    // start at node 0
    fwd_local_min[0] = nacZERO;
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
            bwd_linkage[t].push_back(f);
        });
    }

    // select minimum backward pass
    // start at node 'bwd_linkage.size() -1', end at node 0
    std::deque<NodeAndCost> bwd_global_min;
    Node t = bwd_linkage.size() - 1;
    bwd_global_min.push_front({t, fwd_local_min[t].path_cost} );
    while (t != 0)
    {
        NodeAndCost min_route_cost = nacMAX;
        std::for_each(bwd_linkage[t].begin(), bwd_linkage[t].end(), [&](Node f)
        {
            if (fwd_local_min[f].path_cost < min_route_cost.path_cost)
                min_route_cost = {f, fwd_local_min[f].path_cost};
        });
        bwd_global_min.push_front(min_route_cost);
        t = min_route_cost.node;
    }

    std::cout << "Optimal route and cumulative path_cost:\n";
    std::for_each(bwd_global_min.begin(), bwd_global_min.end(), [&](NodeAndCost& nc)
    { std::cout << nc.node << ' ' << nc.path_cost << "\n"; });
    return 0;
}
