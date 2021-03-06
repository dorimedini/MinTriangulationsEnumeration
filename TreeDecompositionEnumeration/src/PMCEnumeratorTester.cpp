#include "ChordalGraph.h"
#include "DataStructures.h"
#include "GraphProducer.h"
#include "GraphReader.h"
#include "GraphStats.h"
#include "MinimalTriangulationsEnumerator.h"
#include "PMCEnumeratorTester.h"
#include "PMCEnumerator.h"
#include "StatisticRequest.h"
#include "Utils.h"
#include <unistd.h>

namespace tdenum {


PMCEnumeratorTester::PMCEnumeratorTester() :
    TestInterface("PMC Tester")
    #define X(test) , flag_##test(true)
    PMC_TEST_TABLE
    #undef X
    {}

#define X(_func) \
PMCEnumeratorTester& PMCEnumeratorTester::set_##_func() { flag_##_func = true; return *this; } \
PMCEnumeratorTester& PMCEnumeratorTester::unset__##_func() { flag_##_func = false; return *this; } \
PMCEnumeratorTester& PMCEnumeratorTester::set_only_##_func() { clear_all(); return set_##_func(); }
PMC_TEST_TABLE
#undef X

PMCEnumeratorTester& PMCEnumeratorTester::go() {
    //set_only_crosscheck();
    #define X(_func) if (flag_##_func) {DO_TEST(_func);}
    PMC_TEST_TABLE
    #undef X
    return *this;
}
PMCEnumeratorTester& PMCEnumeratorTester::set_all() {
    // Note to self: no good code comes after 1:00am
    #define X(_func) flag_##_func = true;
    PMC_TEST_TABLE
    #undef X
    return *this;
}
PMCEnumeratorTester& PMCEnumeratorTester::clear_all() {
    #define X(_func) flag_##_func = false;
    PMC_TEST_TABLE
    #undef X
    return *this;
}

/**
 * Generic test initialization.
 *
 * Declares/assumes variables 'g' and 'pmce' in the current scope.
 */
#define SETUP_SR() \
    StatisticRequest sr; \
    sr.set_single_pmc_alg(PMCALG_ENUM_NORMAL) \
      .set_pmc()

#define SETUP(_n) \
    SETUP_SR(); \
    Graph g(_n); \
    PMCEnumerator pmce(g); \
    ASSERT_NO_THROW(pmce.set_algorithm(pmc_alg))

#define RESET(_n) \
    g.reset(_n); \
    pmce.reset(g); \
    ASSERT_NO_THROW(pmce.set_algorithm(pmc_alg))

PMCAlg pmc_alg = PMCAlg();

bool PMCEnumeratorTester::sanity() const {
    SETUP(0);
    RESET(1);
    RESET(10);
    return true;
}

bool PMCEnumeratorTester::trivialgraphs() const {
    SETUP(0);
    ASSERT(pmce.is_pmc(NodeSet(), g));
    ASSERT_EQ(pmce.get(/*sr*/), NodeSetSet());
    RESET(1);
    NodeSet nodeset = g.getNodesVector();
    ASSERT(!pmce.is_pmc(NodeSet(), g));
    ASSERT(pmce.is_pmc(nodeset, g));
    NodeSetSet nss;
    nss.insert(nodeset);
    ASSERT_EQ(pmce.get(/*sr*/), nss);
    return true;
}

bool PMCEnumeratorTester::randomgraphs() const {
    SETUP(0);
    TRACE(TRACE_LVL__TEST, ""); // Newline
    for (int i=2; i<FAST_GRAPH_SIZE; ++i) {
        RESET(i);
        g.randomize(0.5);
        TRACE(TRACE_LVL__TEST, "Random graph #" << i-1 << "/" << FAST_GRAPH_SIZE-2 << ":");
        TRACE(TRACE_LVL__TEST, g.str());
        TRACE(TRACE_LVL__TEST, "Resulting PMCs:\n" << pmce.get(/*sr*/));
    }
    return true;
}

bool PMCEnumeratorTester::smallknowngraphs() const {
    // For n=2, there are two possible graphs: sparse or clique.
    // Both graphs are already triangulated, so the PMCs should
    // be {a},{b} for the sparse graph and {a,b} for the clique.
    SETUP(2);
    NodeSet a, b, ab, c, ac, abc;
    NodeSetSet G1_0_PMCs, G1_1_PMCs,
            G2_0_PMCs, G2_1_PMCs, G2_2_PMCs, G2_3_PMCs;
    a.push_back(0);
    b.push_back(1);
    ab.push_back(0);
    ab.push_back(1);
    c.push_back(2);
    ac.push_back(0);
    ac.push_back(2);
    abc.push_back(0);
    abc.push_back(1);
    abc.push_back(2);

    TRACE(TRACE_LVL__TEST, "2-Graphs...");
    G1_0_PMCs = pmce.get(/*sr*/);
    TRACE(TRACE_LVL__TEST, "Got PMCs " << G1_0_PMCs << " for graph " << pmce.get_graph());
    ASSERT_EQ(G1_0_PMCs.size(), (unsigned)2);
    ASSERT(G1_0_PMCs.isMember(a));
    ASSERT(G1_0_PMCs.isMember(b));
    g.addEdge(0,1);
    pmce.reset(g);
    G1_1_PMCs = pmce.get(/*sr*/);
    TRACE(TRACE_LVL__TEST, "With Got PMCs: " << G1_1_PMCs);
    ASSERT_EQ(G1_1_PMCs.size(), (unsigned)1);
    ASSERT(G1_1_PMCs.isMember(ab));

    // For n=3, there are four different graphs (up to isomorphism),
    // differentiated by the number of edges m.
    // In any case, the graph is already triangulated (no cycles of length
    // 4 or more) so PMCs are actual maximal cliques.
    // m=0: {a},{b},{c}
    // m=1: {a,b}, {c}
    // m=2: {a,b}, {a,c}
    // m=3: {a,b,c}
    TRACE(TRACE_LVL__TEST, "3-Graphs...");
    RESET(3);
    G2_0_PMCs = pmce.get(/*sr*/);
    TRACE(TRACE_LVL__TEST, "Got PMCs: " << G2_0_PMCs);
    ASSERT_EQ(G2_0_PMCs.size(), (unsigned)3);
    ASSERT(G2_0_PMCs.isMember(a));
    ASSERT(G2_0_PMCs.isMember(b));
    ASSERT(G2_0_PMCs.isMember(c));
    g.addEdge(0, 1);
    pmce.reset(g);
    G2_1_PMCs = pmce.get(/*sr*/);
    TRACE(TRACE_LVL__TEST, "Got PMCs: " << G2_1_PMCs);
    ASSERT_EQ(G2_1_PMCs.size(), (unsigned)2);
    ASSERT(G2_1_PMCs.isMember(ab));
    ASSERT(G2_1_PMCs.isMember(c));
    TRACE(TRACE_LVL__TEST, "Adding (0,2) to G which is now:\n" << g);
    g.addEdge(0, 2);
    pmce.reset(g);
    TRACE(TRACE_LVL__TEST, "Added. G=\n" << g);
    G2_2_PMCs = pmce.get(/*sr*/);
    TRACE(TRACE_LVL__TEST, "Got PMCs: " << G2_2_PMCs);
    ASSERT_EQ(G2_2_PMCs.size(), (unsigned)2);
    ASSERT(G2_2_PMCs.isMember(ab));
    ASSERT(G2_2_PMCs.isMember(ac));
    g.addEdge(1, 2);
    pmce.reset(g);
    G2_3_PMCs = pmce.get(/*sr*/);
    TRACE(TRACE_LVL__TEST, "Got PMCs: " << G2_3_PMCs);
    ASSERT_EQ(G2_3_PMCs.size(), (unsigned)1);
    ASSERT(G2_3_PMCs.isMember(abc));

    return true;
}

bool PMCEnumeratorTester::fourgraphs() const {
    /**
     * There are 11 different graphs up to isomorphism. Sort them
     * by their number of edges:
     * ============================================================
     * 0:   O   O
     *
     *      O   O
     * ============================================================
     * 1:   O---O
     *
     *      O   O
     * ============================================================
     * 2:   O---O   O   O
     *              |
     *      O---O   O---O
     * ============================================================
     * 3:   O---O   O   O   O---O
     *      |       |  /    |  /
     *      |       |_/     |_/
     *      O---O   O---O   O   O
     * ============================================================
     * 4:   O---O   O---O
     *      |   |   |  /
     *      |   |   |_/
     *      O---O   O---O
     * ============================================================
     * 5:   O---O
     *      |  /|
     *      |_/ |
     *      O---O
     * ============================================================
     * 6:   O---O   (4-clique)
     *      |\ /|
     *      |_V_|
     *      O---O
     * ============================================================
     *
     */
    SETUP(4);
    NodeSetSet pmcs;
    NodeSet a={0},b={1},c={2},d={3};
    NodeSet ab={0,1},ac={0,2},ad={0,3},bc={1,2},bd={1,3},cd={2,3};
    NodeSet abc={0,1,2},abd={0,1,3},acd={0,2,3},bcd={1,2,3};
    NodeSet abcd={0,1,2,3};


    // G0 Has one variation - an independent set.
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)4);
    ASSERT(pmcs.isMember(a));
    ASSERT(pmcs.isMember(b));
    ASSERT(pmcs.isMember(c));
    ASSERT(pmcs.isMember(d));

    // G1 has one variant as well.
    g.addEdge(1,2);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)3);
    ASSERT(pmcs.isMember(a));
    ASSERT(pmcs.isMember(bc));
    ASSERT(pmcs.isMember(d));
    g.reset(4);

    // G2 has two variants.
    // Start with disjoint pairs of edges (resulting in two 2-PMCs)
    g.addEdge(0,1);
    g.addEdge(2,3);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)2);
    ASSERT(pmcs.isMember(ab));
    ASSERT(pmcs.isMember(cd));
    g.reset(4);
    // Try one node of degree 2. We should get two  2-PMCs
    // and one 1-PMC
    g.addEdge(1,2);
    g.addEdge(1,3);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)3);
    ASSERT(pmcs.isMember(a));
    ASSERT(pmcs.isMember(bc));
    ASSERT(pmcs.isMember(bd));
    g.reset(4);

    // G3 has the most variants, but all of them are already
    // triangulated so PMCs are simply maximal cliques.
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,3);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)3);
    ASSERT(pmcs.isMember(ab));
    ASSERT(pmcs.isMember(bc));
    ASSERT(pmcs.isMember(cd));
    g.reset(4);
    g.addEdge(0,1);
    g.addEdge(0,2);
    g.addEdge(0,3);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)3);
    ASSERT(pmcs.isMember(ab));
    ASSERT(pmcs.isMember(ac));
    ASSERT(pmcs.isMember(ad));
    g.reset(4);
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,0);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)2);
    ASSERT(pmcs.isMember(abc));
    ASSERT(pmcs.isMember(d));
    g.reset(4);

    // G4 is the only case where the graph itself isn't triangulated,
    // and even then only the first of two instances is such.
    // In the untriangulated case, every 3 vertices is a PMC because
    // there are two minimal triangulations (if G is a square a-b-c-d-a,
    // then we can add a-c or b-d to triangulate), each one yielding
    // a pair of 3-PMCs.
    // In the other case - one 3-PMC and one 2-PMC.
    g.addEdge(0,1);
    g.addEdge(1,2);
    g.addEdge(2,3);
    g.addEdge(3,0);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    TRACE(TRACE_LVL__TEST, "Got pmcs:" << pmcs);
    TRACE(TRACE_LVL__TEST, "alg=" << pmce.get_alg().str());
    ASSERT_EQ(pmcs.size(), (unsigned)4);
    ASSERT(pmcs.isMember(abc));
    ASSERT(pmcs.isMember(abd));
    ASSERT(pmcs.isMember(acd));
    ASSERT(pmcs.isMember(bcd));
    g.reset(4);
    g.addEdge(0,1);
    g.addEdge(0,2);
    g.addEdge(0,3);
    g.addEdge(1,2);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)2);
    ASSERT(pmcs.isMember(abc));
    ASSERT(pmcs.isMember(ad));
    g.reset(4);

    // For G5, there are two 3-PMCs (already triangulated)
    g.addEdge(0,1);
    g.addEdge(0,2);
    g.addEdge(0,3);
    g.addEdge(1,2);
    g.addEdge(1,3);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)2);
    ASSERT(pmcs.isMember(abc));
    ASSERT(pmcs.isMember(abd));
    g.reset(4);

    // G6 is a clique. Only one 4-PMC
    g.addClique(abcd);
    pmce.reset(g);
    pmcs=pmce.get(/*sr*/);
    ASSERT_EQ(pmcs.size(), (unsigned)1);
    ASSERT(pmcs.isMember(abcd));

    return true;
}

bool PMCEnumeratorTester::cliqueswithtails() const {
    // Make graphs of type v---K where K is a clique
    // and v is connected to a single node in K.
    // The graph is already triangulated, and there should be two
    // cliques: K and {v,u} where u is v's neighbor in K.
    int max_clique_size = 10;
    NodeSetSet pmcs;
    NodeSet always_pmc({0,1});
    SETUP(1);
    for (int i=3; i<max_clique_size; ++i) {
        RESET(i+1);
        NodeSet K;
        // v=0
        for (int j=1; j<=i; ++j) {
            K.push_back(j);
        }
        set<NodeSet> for_saturation;
        for_saturation.insert(K);
        g.saturateNodeSets(for_saturation);
        g.addEdge(0,1);
        pmce.reset(g);
        pmcs = pmce.get(/*sr*/);
        ASSERT(pmcs.isMember(always_pmc));
        ASSERT(pmcs.isMember(K));
        ASSERT_EQ(pmcs.size(),(unsigned)2);
    }
    return true;
}

bool PMCEnumeratorTester::independentsets() const {
    /**
     * Independent sets are triangulated, and each node is a PMC.
     */
    SETUP(1);
    NodeSetSet pmcs;
    for (int i=1; i<5; ++i) {
        RESET(i);
        pmcs = pmce.get(/*sr*/);
        for (int j=0; j<i; ++j) {
            ASSERT(pmcs.isMember(NodeSet({j})));
        }
        ASSERT_EQ(pmcs.size(), (unsigned)i);
    }
    return true;
}

bool PMCEnumeratorTester::twoedgesindependentsubgraphs() const {
    /**
     * Looks like:
     *
     * 0---2---1
     *
     * The order is important, so G1 and G2 both contain nodes with
     * no edges, and only G3=G will have non-trivial cliques.
     */
    SETUP(3);
    NodeSet ac({0,2}),bc({1,2});
    g.addEdge(0,2);
    g.addEdge(1,2);
    pmce.reset(g);
    NodeSetSet pmcs = pmce.get(/*sr*/);
    ASSERT(pmcs.isMember(ac));
    ASSERT(pmcs.isMember(bc));
    ASSERT_EQ(pmcs.size(), (unsigned)2);
    return true;
}

bool PMCEnumeratorTester::triangleonstilts() const {
    /**
     * This graph looks like:
     *
     *   0-----3
     *         |\
     *         | \
     *         |  2
     *         | /
     *         |/
     *   1-----4
     *
     * The PMCs should be {{0,3},{4,2,3},{1,4}}.
     */
    SETUP(5);
    NodeSet ad({0,3}), cde({2,3,4}), ae({1,4});
    g.addEdge(0,3);
    g.addEdge(3,2);
    g.addEdge(4,3);
    g.addEdge(2,4);
    g.addEdge(1,4);
    pmce.reset(g);
    NodeSetSet pmcs = pmce.get(/*sr*/);
    ASSERT(pmcs.isMember(ad));
    ASSERT(pmcs.isMember(cde));
    ASSERT(pmcs.isMember(ae));
    ASSERT_EQ(pmcs.size(), (unsigned)3);

    TRACE(TRACE_LVL__TEST, "Graph before random rename:" << endl << g);
    g.randomNodeRename();
    TRACE(TRACE_LVL__TEST, "Graph After random rename:" << endl << g);
    pmce.reset(g);
    pmcs = pmce.get(/*sr*/);
    TRACE(TRACE_LVL__TEST, "Got pmcs:" << endl << pmcs);
    ASSERT_EQ(pmcs.size(), (unsigned)3);
    return true;
}

bool PMCEnumeratorTester::noamsgraphs() const {
    /**
     * Two graphs:
     * 0--1--4--2--3
     * 0--4--1--3--2
     */
    // First graph's PMCs:
    NodeSet ab({0,1}), be({1,4}), ce({2,4}), cd({2,3});
    // Second graph's PMCs (two of which are also the first graph's PMCs):
    NodeSet ae({0,4}), bd({1,3});
    NodeSetSet pmcs;

    SETUP(5);
    g.addEdge(0,1);
    g.addEdge(4,1);
    g.addEdge(4,2);
    g.addEdge(3,2);
    pmce.reset(g);
    pmcs = pmce.get(/*sr*/);
    ASSERT(pmcs.isMember(ab));
    ASSERT(pmcs.isMember(be));
    ASSERT(pmcs.isMember(ce));
    ASSERT(pmcs.isMember(cd));
    ASSERT_EQ(pmcs.size(), (unsigned)4);

    RESET(5);
    g.addEdge(0,4);
    g.addEdge(1,4);
    g.addEdge(1,3);
    g.addEdge(2,3);
    pmce.reset(g);
    pmcs = pmce.get(/*sr*/);
    ASSERT(pmcs.isMember(ae));
    ASSERT(pmcs.isMember(be));
    ASSERT(pmcs.isMember(bd));
    ASSERT(pmcs.isMember(cd));
    ASSERT_EQ(pmcs.size(), (unsigned)4);

    return true;
}

bool algorithmconsistency_aux(bool pmc_parallel = false) {
    SETUP_SR();
    auto gs = GraphProducer()
                .add_random({2,4,6,8,10,12,14,16,18,20},{0.3,0.5,0.7}, true)
                .get();
    for (unsigned i=0; i<gs.size(); ++i) {
        cout << UTILS__REPLACE_STREAM(i+1 << "/" << gs.size());
        const Graph& g = gs[i].get_graph();
        NodeSetSet pmcs;
        bool found_pmcs = false;
        for (int alg = PMCAlg::first(); alg<PMCAlg::last(); ++alg) {
            TRACE(TRACE_LVL__TEST, "Running " << PMCAlg(alg).str()
                                    << " with the following graph:" << endl << g);
            PMCEnumerator pmce(g);
            pmce.set_algorithm(PMCAlg(alg));
            if (pmc_parallel) {
                pmce.enable_parallel();
            }
            NodeSetSet these_pmcs = pmce.get(/*sr*/);
            if (found_pmcs && these_pmcs != pmcs) {
                cout << utils__replace_string();
                UTILS__ASSERT_PRINT("WRONG! Algorithm " << PMCAlg(alg).str() <<
                                    " got it wrong. Calculated pmcs:" << these_pmcs << endl <<
                                    "The PMCs first calculated were:" << pmcs << endl);
                return false;
            }
            else if (!found_pmcs) {
                pmcs = these_pmcs;
                found_pmcs = true;
            }
        }
    }
    cout << utils__replace_string();
    return true;
}
bool PMCEnumeratorTester::algorithmconsistencysync() const {
    return algorithmconsistency_aux(false);
}
bool PMCEnumeratorTester::algorithmconsistencyparallel() const {
    return algorithmconsistency_aux(true);
}

bool crosscheck_aux(const GraphStats& gs) {
    // Iterate over all maximal cliques in all triangulations.
    // Make sure each one is in the NodeSetSet returned by the
    // PMCEnumerator, and vice-versa.
    SETUP_SR();
    TRACE_NO_NL(TRACE_LVL__ALWAYS, "Cross-checking graph '" << gs.get_text() << "'... ");
    NodeSetSet found;
    const Graph& g = gs.get_graph();
    PMCEnumerator pmce(g);
    pmce.set_algorithm(PMCALG_ENUM_NORMAL);
    NodeSetSet pmcs = pmce.get(/*sr*/);
    MinimalTriangulationsEnumerator enumerator(g, NONE, UNIFORM, SEPARATORS);
    while (enumerator.hasNext()) {
        ChordalGraph triangulation = enumerator.next();
        set<NodeSet> cliques = triangulation.getMaximalCliques();
        for (auto it=cliques.begin(); it != cliques.end(); ++it) {
            found.insert(*it);
        }
    }
    TRACE(TRACE_LVL__TEST, "===PMCs found===:" << endl << found);
    TRACE(TRACE_LVL__TEST, "===Actual PMCs===:" << endl << pmcs);
    if (found != pmcs) {
        cout << "FAILED   ";
        UTILS__ASSERT_PRINT("The PMCs calculated are incorrect:" << endl
                     << "Found:" << endl
                     << found << endl
                     << "Returned by the PMC enumerator:" << endl
                     << pmcs);
        return false;
    }
    cout << "PASSED" << endl;
    return true;
}
bool PMCEnumeratorTester::crosscheck() const {
    // Print each failure on a separate line
    bool all_passed = true;
    GraphProducer gp;
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    gp.add_dir(DATASET_DIR_BASE+DATASET_DIR_DEADEASY);
    gp.add_dir(DATASET_DIR_BASE+DATASET_DIR_EASY);
    for (auto gs: gp.get()) {
        all_passed = (all_passed && crosscheck_aux(gs));
    }
    return all_passed;
}


}



using namespace tdenum;


