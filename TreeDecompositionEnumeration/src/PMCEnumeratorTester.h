#ifndef POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED
#define POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED

#include "DirectoryIterator.h"
#include "Graph.h"
#include "PMCEnumerator.h"
#include "TestInterface.h"

// Graphs with more nodes may cause slow tests
#define FAST_GRAPH_SIZE 10

#define PMC_TEST_TABLE \
    /* Make sure we don't have runtime errors... */ \
    X(sanity) \
    /* Graphs with no more than one vertex */ \
    X(trivialgraphs) \
    /* With n<=FAST_GRAPH_SIZE, output some random graphs . \
     * This just checks for basic runtime errors.. */ \
    X(randomgraphs) \
    /* Graphs of size 2 and 3, where the answer is easily checked */ \
    X(smallknowngraphs) \
    /* Graphs of size 4 (there are 11 such graphs up to isomorphism) */ \
    X(fourgraphs) \
    /* Test cliques with an added node connected to one other node */ \
    X(cliqueswithtails) \
    /* Try independent sets. */ \
    X(independentsets) \
    /* Graph with nodes a,b,c and edges (ac),(bc), IN THAT ORDER, so G\{c} \
       becomes an independent set */ \
    X(twoedgesindependentsubgraphs) \
    /* This fails for some reason... special graph, see implementation \
       for details */ \
    X(triangleonstilts) \
    /* Two specific graphs Noam wanted to test. */ \
    X(noamsgraphs) \
    /* Run with random graphs, make sure the same PMC sets are returned. \
       Test both synchronous and asynchronous modes in the PMC enumerator. */ \
    X(algorithmconsistencysync) \
    X(algorithmconsistencyparallel) \
    /* Uses existing datasets and Nofar's code to cross-check the PMC \
       algorithm with Nofar's version */ \
    X(crosscheck)

typedef enum {
#define X(func) PMCENUM_TEST_NAME__##func,
    PMC_TEST_TABLE
#undef X
    PMCENUM_TEST_NAME__LAST
} PMCEnumeratorTesterFunctions;

namespace tdenum {

class PMCEnumeratorTester : public TestInterface {
private:

    // Calls all tests with flag_ values set to true.
    PMCEnumeratorTester& go();

public:

    // Define all functions and on/off flags.
    // On/off flags are to be set directly, e.g:
    //     PMCEnumeratorTester p(false);
    //
    #define X(_func) \
        PMCEnumeratorTester& set_##_func(); \
        PMCEnumeratorTester& unset__##_func(); \
        PMCEnumeratorTester& set_only_##_func(); \
        bool _func() const; \
        bool flag_##_func;
    PMC_TEST_TABLE
    #undef X

    // Calls all test functions, unless start=false.
    PMCEnumeratorTester();

    // Sets / clears all flags
    PMCEnumeratorTester& set_all();
    PMCEnumeratorTester& clear_all();

};

}
#endif // POTENTIALMAXIMALCLIQUESENUMERATORTESTER_H_INCLUDED
