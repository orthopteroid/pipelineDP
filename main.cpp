// pipelineDP orthopteroid@gmail.com, MIT license
// based upon "Optimal Route Location for Pipelines Carrying Liquid And Gas In Two Phase Flow", U.Shamir, 1969

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <limits>
#include <valarray>

using Node = uint16_t;
using Edge = std::pair<Node, Node>;

struct EdgeInfo { float edge_cost; };
using EdgeMap = std::map<Edge, EdgeInfo>;
using DAG = std::vector< std::deque<Node> >;

struct XYZ { float x, y, z; };
struct NodeAndCost { Node node; float path_cost; };

const NodeAndCost nacMAX = {std::numeric_limits<uint16_t>::max(), std::numeric_limits<float>::max()};
const NodeAndCost nacZERO = {std::numeric_limits<uint16_t>::max(), 0};

enum : int {NoTrees = 0, SmallTrees, LargeTrees};
enum : int {Water = 1, Swamp, Rock, Soil};

EdgeInfo edge_info(const int rc, const int lt, const int tt, const float length)
{
    static std::array<float, 3> tree_costs = { 0, 0, 0.55, }; // none, small, large
    static std::array<float, 5> land_costs = { 0, 1.5, 0.8, 2.5, 0.2, }; // none, water, swamp, rock, soil
    return {rc * 10000 + length * (land_costs[lt] + tree_costs[tt]) };
}

/////////

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
