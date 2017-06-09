#include "DatasetStatisticsGenerator.h"
#include "MinimalTriangulationsEnumerator.h"
#include "DataStructures.h"
#include "DirectoryIterator.h"
#include "Utils.h"
#include "TestUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <math.h>
using std::ofstream;
using std::endl;
using std::ostringstream;
using std::setw;

namespace tdenum {

DatasetStatisticsGenerator::DatasetStatisticsGenerator(const string& outputfile, int flds) :
                            outfilename(outputfile), fields(flds), graphs_computed(0),
                            max_text_len(10)/* Needs to be as long as "Graph text" */ {
    omp_init_lock(&lock);
}
DatasetStatisticsGenerator::~DatasetStatisticsGenerator() {
    omp_destroy_lock(&lock);
}

string DatasetStatisticsGenerator::header(bool csv) const {

    ostringstream oss;
    string delim(csv ? "," : "|");

    // Header
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);
    oss << setw(max_text_len) << "Graph text";
    if (fields & DSG_COMP_N) {
        oss << delim << "Nodes ";
    }
    if (fields & DSG_COMP_M) {
        oss << delim << "Edges ";
    }
    if (fields & DSG_COMP_MS) {
        oss << delim << "Minimal separators";
    }
    if (fields & DSG_COMP_PMC) {
        oss << delim << "PMCs    ";
    }
    if (fields & DSG_COMP_TRNG) {
        oss << delim << "Minimal triangulations";
    }
    oss << endl;
    if (!csv) {
        oss << string(max_text_len+7+7+14+9+23, '=') << endl;
    }

    return oss.str();
}
string DatasetStatisticsGenerator::str(unsigned int i, bool csv) const {

    // Input validation
    if (i>=g.size() || !valid[i]) {
        return "INVALID INPUT\n";
    }

    ostringstream oss;
    string delim(csv ? "," : "|");
    oss.setf(std::ios_base::left, std::ios_base::adjustfield);

    // Identifying text
    if (csv) {
        // Use quotes to escape commas and such, for CSV.
        // If the user sends opening quotes this won't work but
        // danger is my middle name
        oss << "\"" << text[i] << "\"";
    }
    else {
        oss << setw(max_text_len) << text[i];
    }

    // Fields
    if (fields & DSG_COMP_N) {
        oss << delim << setw(6) << n[i];
    }
    if (fields & DSG_COMP_M) {
        oss << delim << setw(6) << m[i];
    }
    if (fields & DSG_COMP_MS) {
        oss << delim << setw(18) << ms[i];
    }
    if (fields & DSG_COMP_PMC) {
        oss << delim << setw(8) << pmcs[i];
    }
    if (fields & DSG_COMP_TRNG) {
        oss << delim << setw(22) << triangs[i];
    }
    oss << endl;

    // That's it!
    return oss.str();
}
string DatasetStatisticsGenerator::str(bool csv) const {

    ostringstream oss;
    string delim(csv ? "," : "|");

    // Header
    oss << header(csv);

    // Data
    for (unsigned int i=0; i<g.size(); ++i) {
        // Don't print invalid rows
        if (!valid[i]) continue;
        oss << str(i,csv);
    }

    // That's it!
    return oss.str();
}


void DatasetStatisticsGenerator::dump_line(unsigned int i) {
    ofstream outfile;
    omp_set_lock(&lock);
    outfile.open(outfilename, ios::out | ios::app);
    if (!outfile.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << outfilename << "'");
        omp_unset_lock(&lock);
        return;
    }
    outfile << str(i, true);
    omp_unset_lock(&lock);
}
void DatasetStatisticsGenerator::dump_header() {
    ofstream outfile;
    omp_set_lock(&lock);
    outfile.open(outfilename, ios::out | ios::trunc);
    if (!outfile.good()) {
        TRACE(TRACE_LVL__ERROR, "Couldn't open file '" << outfilename << "'");
        omp_unset_lock(&lock);
        return;
    }
    outfile << header(true);
    omp_unset_lock(&lock);
}

void DatasetStatisticsGenerator::print_progress(bool verbose)  {

    // This needs to be locked...
    omp_set_lock(&lock);

    // Construct and print the string
    ostringstream oss;
    oss << "\rN/M";
    if (fields & DSG_COMP_MS) oss << "/MS";
    if (fields & DSG_COMP_PMC) oss << "/PMCS";
    if (fields & DSG_COMP_TRNG) oss << "/TRNG";
    for(unsigned int i=0; i<graphs_in_progress.size(); ++i) {
        unsigned int j = graphs_in_progress[i];
        oss << " | " << n[j] << "/" << m[j];
        if (fields & DSG_COMP_MS) oss << "/" << ms[j];
        if (fields & DSG_COMP_PMC) oss << "/" << pmcs[j];
        if (fields & DSG_COMP_TRNG) oss << "/" << triangs[j];
    }
    cout << oss.str();

    // Unlock
    omp_unset_lock(&lock);

}

void DatasetStatisticsGenerator::add_graph(const Graph& graph, const string& txt) {

    // Add vector elements
    g.push_back(graph);
    text.push_back(txt);
    valid.push_back(false);
    n.push_back(0);
    m.push_back(0);
    ms.push_back(0);
    pmcs.push_back(0);
    triangs.push_back(0);

    // Update max text length
    if (max_text_len < strlen(txt.c_str())) {
        max_text_len = strlen(txt.c_str());
    }
}
void DatasetStatisticsGenerator::add_graph(const string& filename, const string& text) {
    Graph g = GraphReader::read(filename);
    add_graph(g, text == "" ? filename : text);
}

void DatasetStatisticsGenerator::compute(unsigned int i, bool verbose) {

    // Add this graph to the  list of 'in progress' graphs.
    // This is a shared resource, so lock it.
    omp_set_lock(&lock);
    graphs_in_progress.push_back(i);
    omp_unset_lock(&lock);

    // Basics
    if (fields & DSG_COMP_N) {
        n[i] = g[i].getNumberOfNodes();
    }
    if (fields & DSG_COMP_M) {
        m[i] = g[i].getNumberOfEdges();
    }

    // Separators
    if (fields & DSG_COMP_MS) {
        ms[i] = 0;
        MinimalSeparatorsEnumerator mse(g[i], UNIFORM);
        while(mse.hasNext()) {
            ++ms[i];
            print_progress(verbose);
            mse.next();
        }
    }

    // PMCs
    if (fields & DSG_COMP_PMC) {
        PMCEnumerator pmce(g[i]);
        NodeSetSet nss = pmce.get();
        pmcs[i] = nss.size();
        print_progress(verbose);
    }

    // Triangulations
    if (fields & DSG_COMP_TRNG) {
        triangs[i] = 0;
        MinimalTriangulationsEnumerator enumerator(g[i], NONE, UNIFORM, SEPARATORS);
        while (enumerator.hasNext()) {
            ++triangs[i];
            print_progress(verbose);
            enumerator.next();
        }
    }

    // That's it for this one!
    valid[i] = true;

    // Some cleanup.
    // Lock
    omp_set_lock(&lock);

    // Update the graphs in progress (under lock)
    REMOVE_FROM_VECTOR(graphs_in_progress, i);
    graphs_computed++;

    // Print
    if (verbose) {

        // We may need to clear the line (if progress was printed)..
        cout << "\r" << string(50+max_text_len, ' ') << "\r";

        // Get the time and output.
        char s[1000];
        time_t t = time(NULL);
        struct tm * p = localtime(&t);
        strftime(s, 1000, "%c", p);
        cout << s << ": Done computing graph " << graphs_computed+1 << "/" << g.size() << ","
                  << "'" <<text[i] << "'" << endl;
    }

    // Unlock
    omp_unset_lock(&lock);

}
void DatasetStatisticsGenerator::compute(bool verbose) {

    // Start a new file
    dump_header();

    // Dump all data, calculate the missing data.
    // If possible, parallelize this
#pragma omp parallel for
    for (unsigned int i=0; i<g.size(); ++i) {
        if (!valid[i]) {
            compute(i, verbose);
        }
        dump_line(i);
    }
}

void DatasetStatisticsGenerator::print() const {
    cout << str(false);
}

}



