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

struct StripInfo { float x, y, z; int rc, lt, tt; int pb, pe; /**/ Node n;};
using InputStrip = std::vector<StripInfo>;

struct EdgeInfo { float cost, len, hill, dp; };

struct XYZ { float x, y, z; };
struct PathStep { Node from_node; float cost, len, hill, dp; };
struct PathStepStat { float min_cost, max_cost, min_len, max_len, min_hill, max_hill, min_dp, max_dp; };

const int nptINCR = 10;
const int nptSIZE = nptINCR +1;

// used to dimension a node's L vs H pressure curves
struct NodePressureTable {
    float scale_len[nptSIZE];
    float scale_hill[nptSIZE];
    float table[nptSIZE /* len */ ][nptSIZE /* hill */ ];
};

const float fMAX = std::numeric_limits<float>::max();
const Node nMAX = std::numeric_limits<Node>::max();

const PathStep psMAX = {nMAX, fMAX, fMAX, fMAX, fMAX};
const PathStep psZERO = {nMAX, 0, 0, 0, 0};
const PathStepStat pssInit = {fMAX, -fMAX, fMAX, -fMAX, fMAX, -fMAX, fMAX, -fMAX};

enum : int {NoLand = 0, Water, Swamp, Rock, Soil};
enum : int {NoTrees = 0, SmallTrees, LargeTrees};

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
    const double U = 31194 * QG * Z / (Pav * D * D); // superficial velocity
    const double F1_logterm = std::log(U / std::pow(R, 0.32));
    const double F1 = std::exp(-.07464 * F1_logterm * F1_logterm + .4772 * F1_logterm - .8003); // friction loss efficiency factor
    const double C = 20500 / (std::pow(SG, .46) * std::pow(T+460,.54)); // friction factor for pressure loss
    return
            {
                    float(std::pow(QG * 1E6 / ( C * std::pow(D, 2.6182) * F1 ), 1.853) / ( 2 * Pav ) / ftPerMile),
                    float(SL * 3.06 / (144 * (U + 3.06)))
            };
}

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

PressureLoss pressure_loss = flanigans_method(880, 815);

int main()
{
    Node nCount = 0;
    std::vector<XYZ> node_pos;
    std::map<Edge, EdgeInfo> edge_info;
    std::vector< std::deque<Node> > bwd_linkage;
    std::vector< std::deque<Node> > fwd_linkage;
    std::vector<NodePressureTable> node_pressure_tables;
    std::vector<PathStepStat> node_path_stats;
    //
    std::deque<Node> visit_stack;
    std::vector<bool> fwd_visit_marking;

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

    std::cout << "Parsing input data...\n";

    for(int s = 0; s < input_strips.size(); ++s)
        for (int i = 0; i < input_strips[s].size(); ++i)
            input_strips[s][i].n = nCount++;

    bwd_linkage.resize(nCount);
    fwd_linkage.resize(nCount);
    node_pos.resize(nCount);
    node_path_stats.resize(nCount);
    node_pressure_tables.resize(nCount);
    fwd_visit_marking.resize(nCount);

    node_pos[0] = {input_strips[0][0].x, input_strips[0][0].y, input_strips[0][0].z};
    for(int s = 1; s < input_strips.size(); ++s) // nb: start at 1
        for(int i = 0; i < input_strips[s].size(); ++i)
        {
            const StripInfo& info = input_strips[s][i];
            const Node t = input_strips[s][i].n;
            node_pos[t] = {info.x, info.y, info.z};
            for(int j = info.pb; j <= info.pe; ++j) // nb: inclusive end
            {
                const Node f = input_strips[s - 1][j].n;
                bwd_linkage[t].push_back(f);
                fwd_linkage[f].push_back(t);
                //
                edge_info.insert(
                {
                    {f, t},
                    {
                        edge_length(f, t) * (land_costs[info.lt] + tree_costs[info.tt]),
                        edge_length(f, t),
                        edge_hill(f, t),
                        pressure_loss.alpha * edge_length(f, t) + pressure_loss.beta * edge_hill(f, t)
                    }
                });
            }
        }

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

    std::cout << "Computing Path min & max...\n";

    // find path limits for each node
    std::fill(node_path_stats.begin(), node_path_stats.end(), pssInit);
    node_path_stats[0] = {0, 0, 0, 0, 0, 0};
    visit_stack = {0};
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t)
        {
            const PathStepStat& pss_f = node_path_stats[f];
            PathStepStat& pss_t = node_path_stats[t];
            const EdgeInfo& ei = edge_info[{f, t}];
            pss_t.min_cost = std::min(pss_t.min_cost, pss_f.min_cost + ei.cost);
            pss_t.max_cost = std::max(pss_t.max_cost, pss_f.max_cost + ei.cost);
            pss_t.min_len = std::min(pss_t.min_len, pss_f.min_len + ei.len);
            pss_t.max_len = std::max(pss_t.max_len, pss_f.max_len + ei.len);
            pss_t.min_hill = std::min(pss_t.min_hill, pss_f.min_hill + ei.hill);
            pss_t.max_hill = std::max(pss_t.max_hill, pss_f.max_hill + ei.hill);
            pss_t.min_dp = std::min(pss_t.min_dp, pss_f.min_dp + ei.dp);
            pss_t.max_dp = std::max(pss_t.max_dp, pss_f.max_dp + ei.dp);
            visit_stack.push_back(t);
        });
    }

    // tailor a specialized pressure table for each node
    for(Node t = 1; t < nCount; ++t) // nb: start at 1
    {
        NodePressureTable& npt = node_pressure_tables[t];
        const PathStepStat& pss = node_path_stats[t];
        const float dl = (pss.max_len - pss.min_len) / nptINCR;
        const float dh = (pss.max_hill - pss.min_hill) / nptINCR;
        for(int i = 0; i < nptSIZE; ++i)
        {
            npt.scale_len[i] = pss.min_len + float(i) * dl;
            npt.scale_hill[i] = pss.min_hill + float(i) * dh;
        }
        for(int i = 0; i < nptSIZE; ++i) // len
            for(int j = 0; j < nptSIZE; ++j) // hill
                npt.table[i][j] = pressure_loss.alpha * npt.scale_len[i] + pressure_loss.beta * npt.scale_hill[j];
    }

    std::cout << "Node Path Limits and Linkages:\n";

    visit_stack = {0};
    std::fill(fwd_visit_marking.begin(), fwd_visit_marking.end(), false);
    std::cout << "f mncost mxcost mnlen mxlen mnhill mxhill mndp mxdp : (t cost len hill dp) ...\n";
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        PathStepStat& pss = node_path_stats[f];
        auto nl = node_pressure_tables[f];
        std::cout << f << " ";
        std::cout << pss.min_cost  << " " << pss.max_cost << " ";
        std::cout << pss.min_len  << " " << pss.max_len << " ";
        std::cout << pss.min_hill  << " " << pss.max_hill << " ";
        std::cout << pss.min_dp  << " " << pss.max_dp << " ";
        if( fwd_linkage[f].size() > 0)
            std::cout << ": ";
        std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t)
        {
            auto ei = edge_info[{f, t}];
            std::cout << "(" << t << " " << ei.cost << " " << ei.len << " " << ei.hill << " " << ei.dp << ") ";
            visit_stack_unique_visitation(t);
        });
        std::cout << "\n";
    }

    ///////////////

    auto solve = [&](const std::string& title, std::function<float(const PathStep& ps)> psa, std::function<void(PathStep& ps, Node f, float v)> pss, std::function<float(const EdgeInfo& ei)> eia)
    {
        std::vector<PathStep> fwd_min(fwd_linkage.size(), psMAX );

        // forward pass: start at inlet and work towards outlet
        fwd_min[0] = psZERO;
        visit_stack = {0};
        std::fill(fwd_visit_marking.begin(), fwd_visit_marking.end(), false);
        while (!visit_stack.empty())
        {
            Node f = visit_stack_pop();
            std::for_each(fwd_linkage[f].begin(), fwd_linkage[f].end(), [&](Node t)
            {
                auto v = psa(fwd_min[f]) + eia(edge_info[{f, t}]);
                if (v < psa(fwd_min[t]))
                {
                    pss(fwd_min[t], f, v);
                    fwd_min[t].cost = fwd_min[f].cost + edge_info[{f, t}].cost;
                }
                visit_stack.push_back(t);
            });
        }

        // backward pass: start at outlet and work towards inlet
        std::deque<PathStep> bwd_min;
        Node t = nCount - 1;
        PathStep hack;
        pss(hack, t, psa(fwd_min[t]));
        bwd_min.push_front(hack);
        while (t != 0)
        {
            PathStep step_min = psMAX;
            std::for_each(bwd_linkage[t].begin(), bwd_linkage[t].end(), [&](Node f)
            {
                auto v = psa(fwd_min[f]);
                if (v < psa(step_min))
                {
                    pss(step_min, f, v);
                    step_min.cost = fwd_min[f].cost;
                }
            });
            bwd_min.push_front(step_min);
            t = step_min.from_node;
        }

        std::cout << title;
        std::for_each(bwd_min.begin(), bwd_min.end(), [&](PathStep& ps)
        { std::cout << "(" << ps.from_node << ' ' << psa(ps) << " " << ps.cost << ") "; });
        std::cout << "total cost " << bwd_min.back().cost << "\n";
    };

    std::cout << "Solving...\n";

    // path step accessors and setters for datatypes
    auto psa_cost = [](const PathStep& ps) -> float { return ps.cost; };
    auto pss_cost = [](PathStep& ps, Node f, float v) -> void { ps.from_node = f; ps.cost = v; };
    auto eia_cost = [](const EdgeInfo& ei) -> float { return ei.cost; };
    auto psa_length = [](const PathStep& ps) -> float { return ps.len; };
    auto pss_length = [](PathStep& ps, Node f, float v) -> void { ps.from_node = f; ps.len = v; };
    auto eia_length = [](const EdgeInfo& ei) -> float { return ei.len; };
    auto psa_headloss = [](const PathStep& ps) -> float { return ps.dp; };
    auto pss_headloss = [](PathStep& ps, Node f, float v) -> void { ps.from_node = f; ps.dp = v; };
    auto eia_headloss = [](const EdgeInfo& ei) -> float { return ei.dp; };

    solve("Optimal route with minimal cost:\n", psa_cost, pss_cost, eia_cost);
    solve("Optimal route with minimal length:\n", psa_length, pss_length, eia_length);
    solve("Optimal route with minimal headloss:\n", psa_headloss, pss_headloss, eia_headloss);

    return 0;
}
