#include "Graph.h"
#include "DataStructures.h"
#include "Utils.h"
#include <queue>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <algorithm>

namespace tdenum {

Graph::Graph() : numberOfNodes(0), numberOfEdges(0), isRandomGraph(false) {}

Graph::Graph(int numberOfNodes) : numberOfNodes(numberOfNodes), numberOfEdges(0),
		neighborSets(numberOfNodes), isRandomGraph(false) {}

void Graph::reset(int n) {
    numberOfNodes = n;
    numberOfEdges = 0;
    neighborSets.clear();
    neighborSets.resize(n);
}

void Graph::randomize(double pr) {

    // I need to know if I'm a random graph!
    isRandomGraph = true;
    p = pr;

    // Remove all edges
    for (int i=0; i<numberOfNodes; ++i) {
        neighborSets[i].clear();
    }

    NodeSet nodes = getNodesVector();
    double d;
    TRACE(TRACE_LVL__NOISE, "In, p=" << pr);
    pr *= RAND_MAX;
    for (int i=0; i<numberOfNodes; ++i) {
        for (int j=i+1; j<numberOfNodes; ++j) {
            d = rand();
            TRACE(TRACE_LVL__NOISE, "Got d=" << (double)d/RAND_MAX);
            if (d <= pr) {
                TRACE(TRACE_LVL__NOISE, "Adding edge..");
                addEdge(nodes[i],nodes[j]);
            }
        }
    }
}
bool Graph::isRandom() const {
    return isRandomGraph;
}
double Graph::getP() const {
    if (!isRandom()) {
        TRACE(TRACE_LVL__ERROR, "Called getP() on non-random graph!");
        return -1;
    }
    return p;
}


void Graph::removeAllButFirstK(int k) {
    // Remove respective edges from neighbor sets.
    // While doing so, recalculate the number of edges using the formula:
    // sum_v(d(v))=2E
    int E = 0;
    for (int u=0; u<k; ++u) {
        for (int v=k; v<numberOfNodes; ++v) {
            neighborSets[u].erase(v);
        }
        E += neighborSets[u].size();
    }
    // Update the number of nodes and edges
    if (E%2) {
        TRACE(TRACE_LVL__ERROR, "WTF");
    }
    numberOfNodes = k;
    numberOfEdges = E/2;
}

void Graph::randomNodeRename() {
    // Shuffle node names:
    vector<Node> f = getNodesVector();
    std::random_shuffle(f.begin(), f.end());
    // Use f as a mapping between nodes to build the new
    // neighbor sets.
    vector< set<Node> > newNeighbors(numberOfNodes);
    for (Node u=0; u<numberOfNodes; ++u) {
        for (auto v=neighborSets[u].begin(); v!=neighborSets[u].end(); ++v) {
            newNeighbors[f[u]].insert(f[*v]);
        }
    }
    neighborSets = newNeighbors;
}

void Graph::addClique(const set<Node>& newClique) {
	for (set<Node>::iterator i = newClique.begin(); i != newClique.end(); ++i) {
		Node v = *i;
		for (set<Node>::iterator j = newClique.begin(); j != newClique.end(); ++j) {
			Node u = *j;
			if (u < v) {
				addEdge(u, v);
			}
		}
	}
}

void Graph::addClique(const vector<Node>& newClique) {
	for (vector<Node>::const_iterator i = newClique.begin(); i != newClique.end(); ++i) {
		Node v = *i;
		for (vector<Node>::const_iterator j = newClique.begin(); j != newClique.end(); ++j) {
			Node u = *j;
			if (u < v) {
				addEdge(u, v);
			}
		}
	}
}

void Graph::addEdge(Node u, Node v) {
	if (!isValidNode(u) || !isValidNode(v) || neighborSets[u].count(v)>0) {
		return;
	}
	neighborSets[u].insert(v);
	neighborSets[v].insert(u);
	numberOfEdges++;
}

void Graph::saturateNodeSets(const set< set<Node> >& s) {
	for (set< set<Node> >::iterator i = s.begin(); i != s.end(); ++i) {
		addClique(*i);
	}
}

// Adds edges that will make the given node sets cliques
void Graph::saturateNodeSets(const set< vector<Node> >& s) {
	for (set< vector<Node> >::iterator i = s.begin(); i != s.end(); ++i) {
		addClique(*i);
	}
}

bool Graph::isValidNode(Node v) const {
	if (v<0 || v>=numberOfNodes) {
		cout << "Invalid input" << endl;
		return false;
	}
	return true;
}


/*
 * Returns the set of nodes in the graph
 */
set<Node> Graph::getNodes() const {
	set<Node> nodes;
	for (Node i=0; i<numberOfNodes; i++) {
		nodes.insert(i);
	}
	return nodes;
}

/*
 * Returns the vector of nodes in the graph
 */
vector<Node> Graph::getNodesVector() const {
	vector<Node> nodes;
	for (Node i=0; i<numberOfNodes; i++) {
		nodes.insert(nodes.end(), i);
	}
	return nodes;
}

/*
 * Returns the number of edges in the graph
 */
int Graph::getNumberOfEdges() const {
	return numberOfEdges;
}

/*
 * Returns the number of nodes in the graph
 */
int Graph::getNumberOfNodes() const {
	return numberOfNodes;
}

int Graph::d(Node v) const {
    if (!isValidNode(v)) {
        cout << "Error: requesting degree of invalid node" << endl;
        return -1;
    }
    return neighborSets[v].size();
}
/*
 * Returns the set of neighbors of the given node
 */
const set<Node>& Graph::getNeighbors(Node v) const {
	if (!isValidNode(v)) {
		cout << "Error: Requesting access to invalid node" << endl;
		return neighborSets[0];
	}
	return neighborSets[v];
}

vector<bool> Graph::getNeighborsMap(Node v) const {
	vector<bool> result(numberOfNodes, false);
	for (set<Node>::iterator j = neighborSets[v].begin(); j != neighborSets[v].end(); ++j) {
		result[*j] = true;
	}
	return result;
}

/*
 * Returns the set of neighbors of nodes in the given node set without returning
 * nodes that are in the input node set
 */
NodeSet Graph::getNeighbors(const set<Node>& inputSet) const {
	NodeSetProducer neighborsProducer(numberOfNodes);
	for (set<Node>::const_iterator i = inputSet.begin(); i != inputSet.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return NodeSet();
		}
		for (set<Node>::iterator j = neighborSets[v].begin(); j != neighborSets[v].end(); ++j) {
			neighborsProducer.insert(*j);
		}
	}
	for (set<Node>::const_iterator i = inputSet.begin(); i != inputSet.end(); ++i) {
		neighborsProducer.remove(*i);
	}
	return neighborsProducer.produce();
}

NodeSet Graph::getNeighbors(const vector<Node>& inputSet) const {
	NodeSetProducer neighborsProducer(numberOfNodes);
	for (vector<Node>::const_iterator i = inputSet.begin(); i != inputSet.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return NodeSet();
		}
		for (set<Node>::iterator j = neighborSets[v].begin(); j != neighborSets[v].end(); ++j) {
			neighborsProducer.insert(*j);
		}
	}
	for (vector<Node>::const_iterator i = inputSet.begin(); i != inputSet.end(); ++i) {
		neighborsProducer.remove(*i);
	}
	return neighborsProducer.produce();
}

bool Graph::areNeighbors(Node u, Node v) const {
	return neighborSets[u].find(v) != neighborSets[u].end();
}

vector<NodeSet> Graph::getComponents(const set<Node>& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (set<Node>::iterator i = removedNodes.begin(); i != removedNodes.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return vector<NodeSet>();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	return getComponentsAux(visitedList, numberOfUnhandeledNodes);
}

vector<NodeSet> Graph::getComponents(const NodeSet& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (Node v : removedNodes) {
		if (!isValidNode(v)) {
			return vector<NodeSet>();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	return getComponentsAux(visitedList, numberOfUnhandeledNodes);
}

vector<NodeSet> Graph::getComponentsAux(vector<int> visitedList, int numberOfUnhandeledNodes) const {
	vector<NodeSet> components;
	// Finds a new component in each iteration
	while (numberOfUnhandeledNodes > 0) {
		queue<Node> bfsQueue;
		NodeSetProducer componentProducer(visitedList.size());
		// Initialize the queue to contain a node not handled
		for (Node i=0; i<numberOfNodes; i++) {
			if (visitedList[i]==0) {
				bfsQueue.push(i);
				visitedList[i]=1;
				componentProducer.insert(i);
				numberOfUnhandeledNodes--;
				break;
			}
		}
		// BFS through the component
		while (!bfsQueue.empty()) {
			Node v = bfsQueue.front();
			bfsQueue.pop();
			for (set<Node>::iterator i = neighborSets[v].begin();
					i != neighborSets[v].end(); ++i) {
				Node u = *i;
				if (visitedList[u]==0) {
					bfsQueue.push(u);
					visitedList[u]=1;
					componentProducer.insert(u);
					numberOfUnhandeledNodes--;
				}
			}
		}
		components.push_back(componentProducer.produce());
	}
	return components;
}

vector<int> Graph::getComponentsMap(const vector<Node>& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (vector<Node>::const_iterator i = removedNodes.begin(); i != removedNodes.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return vector<int>();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	int currentComponent = 1;
	while (numberOfUnhandeledNodes > 0) {
			queue<Node> bfsQueue;
			// Initialize the queue to contain a node not handled
			for (Node i=0; i<numberOfNodes; i++) {
				if (visitedList[i]==0) {
					bfsQueue.push(i);
					visitedList[i]=currentComponent;
					numberOfUnhandeledNodes--;
					break;
				}
			}
			// BFS through the component
			while (!bfsQueue.empty()) {
				Node v = bfsQueue.front();
				bfsQueue.pop();
				for (set<Node>::iterator i = neighborSets[v].begin();
						i != neighborSets[v].end(); ++i) {
					Node u = *i;
					if (visitedList[u]==0) {
						bfsQueue.push(u);
						visitedList[u]=currentComponent;
						numberOfUnhandeledNodes--;
					}
				}
			}
			currentComponent++;
		}
	return visitedList;
}

// Returns the nodes reachable from v after removing removedNodes.
// Uses a BFS from v, where nodes in removedNodes are not processed.
set<Node> Graph::getComponent(Node v, const set<Node>& removedNodes) {
	queue<Node> q;
	vector<bool> insertedNodes = vector<bool>(numberOfNodes);
	set<Node> component;

	// Mark removedNodes as inserted to avoid processing them
	for (Node removed : removedNodes) {
		insertedNodes[removed] = true;
	}

	// Initialize the BFS with v
	component.insert(v);
	q.push(v);
	insertedNodes[v] = true;
	// BFS through the component
	while (!q.empty()) {
		v = q.front();
		q.pop();
		const set<Node>& neighbors = getNeighbors(v);
		for (Node neighbor : neighbors) {
			if (!insertedNodes[neighbor]) {
				q.push(neighbor);
				insertedNodes[neighbor] = true;
				component.insert(neighbor);
			}
		}
	}
	return component;
}


NodeSet Graph::getAdjacent(const NodeSet& C, const NodeSet& K) const {
    NodeSet K2;
    auto adjacent = getNeighbors(C);
    std::set_intersection(adjacent.begin(), adjacent.end(),
                        K.begin(), K.end(),
                        std::back_inserter(K2));
    return K2;
}

BlockVec Graph::getBlocks(const set<Node>& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (set<Node>::iterator i = removedNodes.begin(); i != removedNodes.end(); ++i) {
		Node v = *i;
		if (!isValidNode(v)) {
			return BlockVec();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	return getBlocksAux(visitedList, numberOfUnhandeledNodes);
}

BlockVec Graph::getBlocks(const NodeSet& removedNodes) const {
	vector<int> visitedList(numberOfNodes, 0);
	for (Node v : removedNodes) {
		if (!isValidNode(v)) {
			return BlockVec();
		}
		visitedList[v] = -1;
	}
	int numberOfUnhandeledNodes = numberOfNodes - removedNodes.size();
	return getBlocksAux(visitedList, numberOfUnhandeledNodes);
}

BlockVec Graph::getBlocksAux(vector<int> visitedList, int numberOfUnhandeledNodes) const {
	BlockVec blocks;
	// Finds a new component in each iteration
	Node unhandeledID = 0;
	while (numberOfUnhandeledNodes > 0) {
		queue<Node> bfsQueue;
		NodeSetProducer sepProducer(visitedList.size()), 
						compProducer(visitedList.size());
		// Initialize the queue to contain a node not handled
		for (; unhandeledID<numberOfNodes; unhandeledID++) {
			if (visitedList[unhandeledID] == 0) {
				bfsQueue.push(unhandeledID);
				visitedList[unhandeledID] = 1;
				compProducer.insert(unhandeledID);
				numberOfUnhandeledNodes--;
				unhandeledID++;
				break;
			}
		}
		// BFS through the component
		while (!bfsQueue.empty()) {
			Node v = bfsQueue.front();
			bfsQueue.pop();
			for (set<Node>::iterator it = neighborSets[v].begin();
				it != neighborSets[v].end(); ++it) {
				Node u = *it;
				if (visitedList[u] == 0) {
					bfsQueue.push(u);
					visitedList[u] = 1;
					compProducer.insert(u);
					numberOfUnhandeledNodes--;
				}
				else if (visitedList[u] == -1) {
					sepProducer.insert(u);
				}
			}
		}
		blocks.push_back(BlockPtr(new Block(sepProducer.produce(), compProducer.produce(),getNumberOfNodes())));
	}
	return blocks;
}
string Graph::str() const {
    ostringstream oss;
	for (Node v=0; v<getNumberOfNodes(); v++) {
        auto neighbors = getNeighbors(v);
		oss << v << " has neighbors: {";
		for (set<Node>::iterator jt = neighbors.begin(); jt!=neighbors.end(); ++jt) {
			oss << *jt << ",";
		}
        if (neighbors.size() > 0) {
            oss << "\b"; // Remove trailing space
        }
		oss << "}" << endl;
	}
	return oss.str();
}
void Graph::print() const {
    cout << str();
}

ostream& operator<<(ostream& os, const Graph& g) {
    os << g.str();
    return os;
}

const NodeSet& Block::getNodeSetUnion(const NodeSet& sep, const NodeSet& comp) {
	NodeSet* result = new NodeSet(sep.size() + comp.size());
	std::set_union(sep.begin(), sep.end(),
		comp.begin(), comp.end(),
		result->begin());
	return *result;
}

const vector<bool>& Block::getFullNodeVector(const NodeSet& nodes, int numNodes) {
	vector<bool>* result = new vector<bool>(numNodes, false);
	for (auto n = nodes.begin(); n != nodes.end(); n++)
		(*result)[*n] = true;
	return *result;
}

bool Block::includesNodes(const NodeSet& toCheck) const {
	for (auto n = toCheck.begin(); n != toCheck.end(); n++)
		if (!fullNodes[*n])
			return false;
	return true;
}

} /* namespace tdenum */

