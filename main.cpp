// pipelineDP orthopteroid@gmail.com, MIT license
// based upon "Optimal Route Location for Pipelines Carrying Liquid And Gas In Two Phase Flow", U.Shamir, 1969

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <limits>
#include <valarray>
#include <cassert>
#include <set>

using Node = uint16_t;
using Edge = std::pair<Node, Node>;

struct StripInfo { /* input */ float x, y, z; int rc, lt, tt; int pb, pe; /* modified */ Node n;};
struct EdgeInfo { float cost, len, hill, dp; };
struct NodeInfo { float x, y, z; int strip, index; };

struct PathStep { Node path_node; float cost, len, hill, dp; };
struct PathStepStat { float cost_min, cost_max, len_min, len_max, hill_min, hill_max, min_dp, max_dp; };

using InputStrip = std::vector<StripInfo>;

const float fMAX = std::numeric_limits<float>::max();
const int iMAX = std::numeric_limits<int>::max();
const Node nMAX = std::numeric_limits<Node>::max();

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

float pipe_cost = 6;
std::array<float, 5> land_costs = { 0, 1.5, 0.8, 2.5, 0.2, }; // none, water, swamp, rock, soil
std::array<float, 3> tree_costs = { 0, 0, 0.55, }; // none, small, large

PressureLoss pressure_loss = flanigans_method(880, 815);

int main()
{
    Node nCount = 0;
    std::vector<NodeInfo> node_info;
    std::map<Edge, EdgeInfo> edge_info;
    std::vector< std::deque<Node> > bwd_linkage;
    std::vector< std::deque<Node> > fwd_linkage;
    std::vector<PathStepStat> node_path_stats;
    //
    std::deque<Node> visit_stack;
    std::vector<bool> fwd_visit_marking;

    auto edge_length = [&](Node f, Node t) -> float
    {
        const float dx = node_info[t].x - node_info[f].x;
        const float dy = node_info[t].y - node_info[f].y;
        const float dz = node_info[t].z - node_info[f].z;
        return std::sqrt( dx * dx + dy * dy + dz * dz );
    };

    auto edge_hill = [&](Node f, Node t) -> float
    {
        const float dz = node_info[t].z - node_info[f].z;
        return dz > 0 ? dz : 0;
    };

    auto elev_gain = [&](float f_elev, Node f, Node t) -> float
    {
        const float dz = node_info[t].z - node_info[f].z;
        return f_elev + (dz > 0 ? dz : 0);
    };

    std::cout << "Parsing input data...\n";

    for(int s = 0; s < input_strips.size(); ++s)
        for (int i = 0; i < input_strips[s].size(); ++i)
            input_strips[s][i].n = nCount++;

    bwd_linkage.resize(nCount);
    fwd_linkage.resize(nCount);
    node_info.resize(nCount);
    node_path_stats.resize(nCount);
    fwd_visit_marking.resize(nCount);

    node_info[0] = {input_strips[0][0].x, input_strips[0][0].y, input_strips[0][0].z, 0, 0};
    for(int s = 1; s < input_strips.size(); ++s) // nb: start at 1
        for(int i = 0; i < input_strips[s].size(); ++i)
        {
            const StripInfo& info = input_strips[s][i];
            const Node t = input_strips[s][i].n;
            node_info[t] = {info.x, info.y, info.z, s, i};
            for(int j = info.pb; j <= info.pe; ++j) // nb: inclusive end
            {
                const Node f = input_strips[s - 1][j].n;
                assert( node_info[f].strip == node_info[t].strip -1 ); // 'strip' topology constraint
                bwd_linkage[t].push_back(f);
                fwd_linkage[f].push_back(t);
                //
                edge_info.insert(
                {
                    {f, t},
                    {
                        edge_length(f, t) * (land_costs[info.lt] + tree_costs[info.tt] + pipe_cost),
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
    const PathStepStat pssInit = {fMAX, -fMAX, fMAX, -fMAX, fMAX, -fMAX, fMAX, -fMAX};
    std::fill(node_path_stats.begin(), node_path_stats.end(), pssInit);
    node_path_stats[0] = {0,0,0,0,0,0,0,0};
    visit_stack = {0};
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        for(const Node &t: fwd_linkage[f])
        {
            const PathStepStat& pss_f = node_path_stats[f];
            PathStepStat& pss_t = node_path_stats[t];
            const EdgeInfo& ei = edge_info[{f, t}];
            pss_t.cost_min = std::min(pss_t.cost_min, pss_f.cost_min + ei.cost);
            pss_t.cost_max = std::max(pss_t.cost_max, pss_f.cost_max + ei.cost);
            pss_t.len_min = std::min(pss_t.len_min, pss_f.len_min + ei.len);
            pss_t.len_max = std::max(pss_t.len_max, pss_f.len_max + ei.len);
            pss_t.hill_min = std::min(pss_t.hill_min, pss_f.hill_min + ei.hill);
            pss_t.hill_max = std::max(pss_t.hill_max, pss_f.hill_max + ei.hill);
            pss_t.min_dp = std::min(pss_t.min_dp, pss_f.min_dp + ei.dp);
            pss_t.max_dp = std::max(pss_t.max_dp, pss_f.max_dp + ei.dp);
            visit_stack.push_back(t);
        };
    }

    std::cout << "Node limits on lengths, hills and pressures:\n";

    std::cout << "0 (0,0) 0 0, 0 0, 0 0\n";
    for(Node t = 1; t < nCount; ++t) // nb: start at 1
    {
        PathStepStat& pss_t = node_path_stats[t];
        std::cout << t << " (" << node_info[t].strip << ',' << node_info[t].index << ") ";
        std::cout << pss_t.len_min << ' ' << pss_t.len_max << ", ";
        std::cout << pss_t.hill_min << ' ' << pss_t.hill_max << ", ";
        std::cout << pss_t.min_dp << ' ' << pss_t.max_dp << '\n';
    }

    std::cout << "Node Path Limits and Linkages:\n";

    visit_stack = {0};
    std::fill(fwd_visit_marking.begin(), fwd_visit_marking.end(), false);
    std::cout << "f mncost mxcost mnlen mxlen mnhill mxhill mndp mxdp : (t cost len hill dp) ...\n";
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        PathStepStat& pss = node_path_stats[f];
        std::cout << f << " (" << node_info[f].strip << ',' << node_info[f].index << ") ";
        std::cout << pss.cost_min << " " << pss.cost_max << " ";
        std::cout << pss.len_min << " " << pss.len_max << " ";
        std::cout << pss.hill_min << " " << pss.hill_max << " ";
        std::cout << pss.min_dp  << " " << pss.max_dp << " ";
        if( fwd_linkage[f].size() > 0)
            std::cout << ": ";
        for(const Node &t : fwd_linkage[f])
        {
            auto ei = edge_info[{f, t}];
            std::cout << "(" << t << " " << ei.cost << " " << ei.len << " " << ei.hill << " " << ei.dp << ") ";
            visit_stack_unique_visitation(t);
        };
        std::cout << "\n";
    }

    ///////////////
    std::deque<PathStep> soln;

    auto print_solution = [&]()
    {
        // calc total hill-climb
        soln[0].hill = 0;
        for(int i = 1; i < soln.size(); ++i)
            soln[i].hill = elev_gain(soln[i - 1].hill, soln[i - 1].path_node, soln[i].path_node);

        std::cout << "Route (nodes): ";
        for(const PathStep &ps: soln)
        { std::cout << ps.path_node << ' '; };
        std::cout << "\nRoute (strip,index): ";
        for(const PathStep &ps: soln)
        { std::cout << "(" << node_info[ps.path_node].strip << "," << node_info[ps.path_node].index << ") "; };
        std::cout << "\nCumulative cost: ";
        for(const PathStep &ps: soln)
        { std::cout << ps.cost << ' '; };
        std::cout << "\nCumulative length: ";
        for(const PathStep &ps: soln)
        { std::cout << ps.len << ' '; };
        std::cout << "\nCumulative hills: ";
        for(const PathStep &ps: soln)
        { std::cout << ps.hill << ' '; };
        std::cout << "\nCumulative pressureloss: ";
        for(const PathStep &ps: soln)
        { std::cout << ps.dp << ' '; };
        std::cout << "\n";
    };

    auto minimize = [&](const std::string& title, std::function<float(const PathStep& ps)> psa)
    {
        std::cout << "\nMinimizing " << title << "\n";
        soln.clear();

        // forward pass: start at inlet and work towards outlet to accumulate using the psa()
        std::vector<PathStep> fwd_accum(fwd_linkage.size(), {nMAX, fMAX, fMAX, fMAX, fMAX} );
        fwd_accum[0] = {0,0,0,0,0};
        visit_stack = {0};
        std::fill(fwd_visit_marking.begin(), fwd_visit_marking.end(), false);
        while (!visit_stack.empty())
        {
            Node f = visit_stack_pop();
            for(const Node &t: fwd_linkage[f])
            {
                PathStep step_accum = {nMAX, fMAX, fMAX, fMAX, fMAX};
                step_accum.path_node = t; // remember t as on the minimum_path (if selected)
                step_accum.cost = fwd_accum[f].cost + edge_info[{f, t}].cost;
                step_accum.len = fwd_accum[f].len + edge_info[{f, t}].len;
                step_accum.dp = fwd_accum[f].dp + edge_info[{f, t}].dp;
                if (psa(step_accum) < psa(fwd_accum[t]))
                    fwd_accum[t] = step_accum;
                visit_stack.push_back(t);
            };
        }

        // backward pass: start at outlet and work towards inlet following the route of smallest-value psa()
        soln.push_front(fwd_accum[nCount - 1]);
        Node t = nCount - 1;
        while (t != 0)
        {
            PathStep step_min = {nMAX, fMAX, fMAX, fMAX, fMAX};
            for(const Node &f: bwd_linkage[t])
            {
                if (psa(fwd_accum[f]) < psa(step_min))
                    step_min = fwd_accum[f];
            };
            soln.push_front(step_min);
            t = step_min.path_node; // change to node in previous strip (node f)
        }

        print_solution();
    };

    // path-step attribute accessors
    auto psa_cost = [](const PathStep& ps) -> float { return ps.cost; };
    auto psa_length = [](const PathStep& ps) -> float { return ps.len; };
    auto psa_pressureloss = [](const PathStep& ps) -> float { return ps.dp; };

    minimize("cost", psa_cost);
    minimize("length", psa_length);
    minimize("pressureloss", psa_pressureloss);

    // forward pass: start at inlet and summarize feasible accum pl towards outlet
    const uint16_t solution_tol = 10; // tenths etc. exponentially increases running time.
    std::vector< std::set<uint16_t> > fwd_feasible(nCount);
    fwd_feasible[0].insert(0);
    visit_stack = {0};
    while (!visit_stack.empty())
    {
        Node f = visit_stack_pop();
        for(const Node &t: fwd_linkage[f])
        {
            for(const uint16_t &v: fwd_feasible[f])
            {
                fwd_feasible[t].insert(v + uint16_t(floorf(edge_info[{f, t}].dp * solution_tol)));
            };
            visit_stack.push_back(t);
        };
    }

    // find the route that results in the desired pressure-loss remainder
    auto solve_pressure = [&](const float pl_target)
    {
        std::cout << "\nSolving for pressureloss of " << pl_target << "\n";
        soln.clear();

        // moving backwards, scan each pl table of incoming nodes to find the closest pressure loss to pl_remaining
        std::deque<Node> bwd_route;
        float pl_remainder = pl_target;
        Node t = nCount - 1;
        bwd_route.push_front(t);
        while (t != 0)
        {
            std::pair<Node, float> sel = {nMAX, fMAX};
            for(const Node &f: bwd_linkage[t])
            {
                for(const uint16_t &v: fwd_feasible[f])
                {
                    const float pl_test = float(v) / float(solution_tol) + edge_info[{f, t}].dp;
                    if(sel.first == nMAX)
                        sel = {f, pl_test};
                    else if (fabsf(pl_remainder - pl_test) < fabsf(pl_remainder - sel.second))
                        sel = {f, pl_test};
                };
            };
            assert(sel.first != nMAX);

            bwd_route.push_front(sel.first);
            pl_remainder -= edge_info[{sel.first, t}].dp; // reduce the remaining pressure loss
            t = sel.first;
        }

        // trace forward to build the cumulative solution from the stored route
        Node f = 0;
        PathStep step_accum = {0,0,0,0,0};
        for(const Node &t: bwd_route)
        {
            step_accum.path_node = t; // remember the t-route
            step_accum.cost += edge_info[{f, t}].cost;
            step_accum.len += edge_info[{f, t}].len;
            step_accum.hill += edge_info[{f, t}].hill;
            step_accum.dp += edge_info[{f, t}].dp;
            soln.push_back(step_accum);
            f = t;
        };

        print_solution();
    };

    solve_pressure(38);
    solve_pressure(39);
    solve_pressure(40);
    solve_pressure(41);
    solve_pressure(42);
    solve_pressure(43);

    return 0;
}
