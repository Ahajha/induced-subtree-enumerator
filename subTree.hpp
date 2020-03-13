#ifndef SUBTREE_HPP
#define SUBTREE_HPP

#include <array>
#include <vector>
#include <iostream>

#include "graph.hpp"

extern const Graph G;

// Represents an induced subtree
struct Subtree
{
	struct subTreeVertex
	{
		bool induced;
		unsigned effectiveDegree;
		
		subTreeVertex() : induced(false), effectiveDegree(0) {}
	};
	
	// Each index is either enabled or disabled, and includes its
	// effective degree (which is cnt)
	
	unsigned numInduced;
	unsigned numExcluded;
	
	vertexID root;
	
	std::array<subTreeVertex, numVertices> vertices;
	
	Subtree(vertexID r);
	
	unsigned cnt(vertexID i) const { return vertices[i].effectiveDegree; }
	bool     has(vertexID i) const { return vertices[i].induced;         }
	
	// Does nothing if the graph would be invalidated
	bool add(vertexID i);
	
	void rem(vertexID i);
	
	bool exists(vertexID i) const { return i != EMPTY && vertices[i].induced; }
	
	void print() const;
	
	void writeToFile(std::string filename) const;
	
	// Returns true iff i does not have 4 neighbors in any plane.
	bool validate(vertexID i) const;
	
	// Returns true iff there is at least one block whose
	// faces cannot be accessed externally.
	bool hasEnclosedSpace() const;
};

#endif
