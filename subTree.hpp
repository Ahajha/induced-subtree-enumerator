#ifndef SUBTREE_HPP
#define SUBTREE_HPP

#include <array>
#include <vector>
#include <iostream>

typedef int vertexID;

constexpr vertexID EMPTY = -1; 
constexpr unsigned numVertices = SIZEX*SIZEY*SIZEZ;

struct graphVertex
{
	std::array<vertexID, 6> neighbors;
};

struct Graph
{
	// Index of a given vertex is its ID
	std::array<graphVertex, numVertices> vertices;
	
	Graph();
};

extern const Graph G;

struct subTreeVertex
{
	bool induced;
	unsigned effectiveDegree;
	
	subTreeVertex() : induced(false), effectiveDegree(0) {}
};

// Represents an induced subtree
struct Subtree
{
	// Each index is either enabled or disabled, and includes its
	// effective degree (which is cnt)
	
	unsigned numInduced;
	unsigned numExcluded;
	
	vertexID root;
	
	std::array<subTreeVertex, numVertices> vertices;
	
	Subtree(vertexID r);
	
	// Copy constructor
	Subtree(const Subtree& S);
	
	unsigned cnt(vertexID i) const { return vertices[i].effectiveDegree; }
	bool     has(vertexID i) const { return vertices[i].induced;         }
	
	// Does nothing if the graph would be invalidated
	bool add(vertexID i);
	
	void rem(vertexID i);
	
	bool exists(vertexID i) const { return i != EMPTY && vertices[i].induced; }
	
	void print() const;
	
	void writeToFile(std::string filename) const;
	
	// This currently isn't doing anything. Every vertex we add is of degree 1.
	// We need to check its neighbor.
	bool validate(vertexID i) const;
	
	// Returns true iff there is at least one block whose
	// faces cannot be accessed externally.
	bool hasEnclosedSpace() const;
};

#endif
