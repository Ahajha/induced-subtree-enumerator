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
		bool excluded;
		unsigned effectiveDegree;
		unsigned excludedNeighbors;
		
		subTreeVertex() : induced(false), excluded(false),
			effectiveDegree(0), excludedNeighbors(0) {}
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
	
	// A vertex is excluded if its effective degree is at least
	// 2 and it is not induced, or if all of its neighbors are
	// excluded (and it is not induced).
	// If either of these is true, marks i as excluded.
	// Recursively marks any other vertices that should be excluded.
	void exclude(vertexID i);
	
	// Removes the exclusion marker from a vertex, if both of the
	// above conditions are false.
	// Recurisively unmarks other vertices.
	void unexclude(vertexID i);
};

#endif
