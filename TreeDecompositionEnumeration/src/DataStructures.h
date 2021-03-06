/*
 * DataStructures.h
 *
 *  Created on: Oct 17, 2016
 *      Author: tmp
 */

#ifndef DATASTRUCTURES_H_
#define DATASTRUCTURES_H_

#include <vector>
#include <set>
#include <algorithm>
#include <string>
#include <memory>

using namespace std;

namespace tdenum {

/**
 * Basics
 */
typedef int Node;
typedef vector<Node> NodeSet; // sorted vector of node names
typedef NodeSet MinimalSeparator;

/**
 * Noam's structure
 */
struct Block {
	const MinimalSeparator S;
	const NodeSet C;
	const NodeSet nodes;
	const vector<bool> fullNodes;
	Block(const MinimalSeparator& sep, const NodeSet& comp, int numNodes) :
		S(sep), C(comp), nodes(getNodeSetUnion(sep,comp)), fullNodes(getFullNodeVector(nodes, numNodes)) {}
	static const NodeSet getNodeSetUnion(const NodeSet&, const NodeSet&);
	static const vector<bool> getFullNodeVector(const NodeSet&, int);
	bool includesNodes(const NodeSet&) const;
};

typedef shared_ptr<Block> BlockPtr;
typedef vector<BlockPtr> BlockVec;

/**
 * Utility functions
 */
string str_nodeset(const NodeSet&);
void print(const NodeSet&);

/*
 * A maximum heap. Can be used for Maximum Cardinality Search.
 */
class IncreasingWeightNodeQueue {
	vector<int> weight;
	set< pair<int,Node> > queue;
public:
	IncreasingWeightNodeQueue(int numberOfNodes);
	// Increase the weight of the node by 1.
	// Node is assumed to be an integer between 0 and numberOfNodes-1.
	void increaseWeight(Node v);
	// Returns the weight of the node. This is also available after pop.
	// Node is assumed to be an integer between 0 and numberOfNodes-1.
	int getWeight(Node v);
	// True if the structure is empty.
	bool isEmpty();
	// Returns and removes the node with the maximal weight.
	Node pop();
};

/*
 * A minimum heap.
 */
class WeightedNodeSetQueue {
	set< pair<int, NodeSet > > queue;
public:
	// True if the structure is empty.
	bool isEmpty();
	// True if the structure contains the given node set with the given weight.
	bool isMember(const NodeSet& nodeSet, int weight);
	// Adds the given node set to the structure with the given weight.
	void insert(const NodeSet& nodeSet, int weight);
	// Returns the maximal weighted node set, and removes it from the structure.
	NodeSet pop();
};

class NodeSetSet {
	set< NodeSet > sets;
public:
    NodeSetSet(const set<NodeSet>& = set<NodeSet>());
    // True if the structure contains the given node set.
	bool isMember(const NodeSet& nodeSet) const;
	bool operator==(const NodeSetSet& nss) const;
	bool operator!=(const NodeSetSet& nss) const;
	// Adds / removes the given node set to / from the structure.
	void insert(const NodeSet& nodeSet);
	void remove(const NodeSet& nodeSet);
	NodeSetSet unify(const NodeSetSet& other) const;
	// Prints out the NodeSets
	string str() const;
	// std::set methods
	unsigned int size() const;
	bool empty() const;
	void clear();
	set<NodeSet>::iterator begin() const;
	set<NodeSet>::iterator end() const;
	set<NodeSet>::iterator find(const NodeSet& nodeSet) const;
	operator set<NodeSet>() const { return sets; }
	friend ostream& operator<<(ostream& os, const NodeSetSet&);
};


/*
 * Constructs a subset of nodes in linear time in the size of the original set.
 */
class NodeSetProducer {
	vector<bool> isMember;
	int numMembers;
public:
	NodeSetProducer(int sizeOfOriginalNodeSet);
	// Adds the given node to the subset.
	// Node is assumed to be an integer between 0 and sizeOfOriginalNodeSet-1.
	void insert(Node v);
	// Removes the given node from the subset.
	// Node is assumed to be an integer between 0 and sizeOfOriginalNodeSet-1.
	void remove(Node v);
	// Returns the subset containing all added nodes, sorted ascending.
	NodeSet produce();
};

} /* namespace tdenum */

#endif /* DATASTRUCTURES_H_ */
