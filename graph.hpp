#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <array>
#include <vector>

typedef int vertexID;

constexpr vertexID EMPTY = -1;
constexpr unsigned numVertices = SIZEX*SIZEY*SIZEZ;

/*
In this case, a Graph is a cubic lattice, with size given
by SIZEX, SIZEY, and SIZEZ.
*/

class Graph
{
	public:
	
	struct graphVertex
	{
		// Neighbors are the vertices it actually
		// touches, directions are indexed according to
		// the direction enum below, EMPTY means there
		// is no vertex in a given direction.
		
		bool enabled;
		std::vector<vertexID> neighbors;
		std::array <vertexID, 6> directions;
		
		graphVertex() {}
		graphVertex(vertexID);
	};
	
	enum direction
	{
		// These are not arbitrary, they are in order of ascending ID.
		// Up and down     : z axis
		// North and south : y axis
		// East and west   : x axis
		UP    = 5,
		DOWN  = 0,
		NORTH = 4,
		SOUTH = 1,
		EAST  = 3,
		WEST  = 2
	};
	
	// Index of a given vertex is its ID
	std::array<graphVertex, numVertices> vertices;
	
	unsigned numDisabled;
	
	Graph();
	
	void disable(vertexID);
	
	private:
	
	static int get_x(vertexID);
	static int get_y(vertexID);
	static int get_z(vertexID);
	
	// Returns the index of the vertex in a given
	// direction, or EMPTY if such a vertex does
	// not exist.
	static vertexID _west (vertexID);
	static vertexID _east (vertexID);
	static vertexID _north(vertexID);
	static vertexID _south(vertexID);
	static vertexID _down (vertexID);
	static vertexID _up   (vertexID);
};

#endif
