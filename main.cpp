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

struct EdgeCost { float fixed, parameterized, total; };
using EdgeMap = std::map<Edge, EdgeCost>;
using DAG = std::vector< std::deque<Node> >;

struct XYZ { float x, y, z; };
struct NodeAndCost { Node node; float cost; };

const NodeAndCost nacMAX = {std::numeric_limits<uint16_t>::max(), std::numeric_limits<float>::max()};
const NodeAndCost nacZERO = {std::numeric_limits<uint16_t>::max(), 0};

// determine cost of an edge from # of river crossings and % of different terrain type over a length in feet
constexpr EdgeCost FillEdgeCost(
        const int riv_c,
        const float wat_p, const float swa_p, const float roc_p, const float soi_p, const float str_p, const float btr_p
)
{
    return {riv_c * 10000.0f, wat_p * 1.5f + swa_p * .8f + roc_p * 2.5f + soi_p * .2f + str_p * 0.0f + btr_p * .55f, 0.0f };
}

template<class T>
T deque_front_pop(std::deque<T>& deque) { T t = deque.front(); deque.pop_front(); return t; }

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
    { {0,1}, FillEdgeCost(0, 0, 0, 0, 1., 0, 0) },
    { {0,2}, FillEdgeCost(0, 0, 0, 0, 1., 0, 0) },
    { {1,3}, FillEdgeCost(0, 0, 0, 0, 1., 0, 0) },
    { {2,3}, FillEdgeCost(0, 0, 0, 0, 1., 0, 0) },
};

int main()
{
    std::deque<Node> child_nodes;

    auto NodeDist =[&](const Node a, const Node b) -> float
    {
        float dx = node_pos[a].x - node_pos[b].x;
        float dy = node_pos[a].y - node_pos[b].y;
        return sqrt( dx * dx + dy * dy );
    };

    DAG back_linkage;
    back_linkage.resize(fwd_linkage.size() );

    // init individual edge costs
    for(auto it = edge_cost.begin(); it != edge_cost.end(); ++it)
        it->second.total = it->second.fixed + it->second.parameterized * NodeDist( it->first.first, it->first.second );

    std::cout << "Linkages and costs:\n";
    child_nodes = {0};
    while (!child_nodes.empty()) {
        Node f = deque_front_pop(child_nodes);
        std::cout << f << ": ";
        std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t) {
            std::cout << t << " " << edge_cost[{f, t}].total << " ";
            if(child_nodes.front() != t)
                child_nodes.push_back(t);
        });
        std::cout << "\n";
    }

    // init local minima vector
    std::vector<NodeAndCost> fwd_local_min;
    fwd_local_min.resize(fwd_linkage.size() );
    std::fill(fwd_local_min.begin(), fwd_local_min.end(), nacMAX );
    fwd_local_min[0] = nacZERO;

    // accumulate minimum forward pass
    // start at node 0
    child_nodes = {0};
    while (!child_nodes.empty()) {
        Node f = deque_front_pop(child_nodes);
        std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t) {
            auto local_cost = fwd_local_min[f].cost + edge_cost[{f, t}].total;
            if (local_cost < fwd_local_min[t].cost)
                fwd_local_min[t] = {f, local_cost};
            if(child_nodes.front() != t)
                child_nodes.push_back(t);
            back_linkage[t].push_back(f);
        });
    }

    // select minimum backward pass
    // start at node 'back_linkage.size() -1', end at node 0
    std::deque<NodeAndCost> back_global_min;
    Node t = back_linkage.size() -1;
    back_global_min.push_front( {t, fwd_local_min[t].cost} );
    while (t != 0) {
        NodeAndCost min_route_cost = nacMAX;
        std::for_each(back_linkage[t].begin(), back_linkage[t].end(), [&](Node f) {
            if (fwd_local_min[f].cost < min_route_cost.cost)
                min_route_cost = {f, fwd_local_min[f].cost};
        });
        back_global_min.push_front(min_route_cost);
        t = min_route_cost.node;
    }

    std::cout << "Optimal route and cumulative cost:\n";
    std::for_each(back_global_min.begin(), back_global_min.end(), [&](NodeAndCost& nc)
        { std::cout << nc.node << ' ' << nc.cost << "\n"; });
    return 0;
}
