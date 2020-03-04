#include "subTree.hpp"
#include <fstream>
#include <queue>

// The maximum degree is 6 for a given
// vertex, and the indexes of the array correspond to
// the following directions.

// These are not arbitrary, they are in order of ascending ID.

// Up and down     : z axis
// North and south : y axis
// East and west   : x axis

#define NORTH 4
#define EAST  3
#define UP    5
#define SOUTH 1
#define WEST  2
#define DOWN  0

// These are used to print to the file
#define BLOCK_PRESENT 'X'
#define BLOCK_MISSING '_'

/*
coord = x + SIZEX * y + (SIZEX*SIZEY) * z
      = SIZEX * (y + SIZEY * z) + x

so x = coord mod SIZEX

(coord div SIZEX) = y + SIZEY * z
so y = (coord div SIZEX) mod SIZEY

(coord div SIZEX) = y + SIZEY * z
so z = (coord div SIZEX) div SIZEY = coord div (SIZEX*SIZEY)
*/

int get_x(int coord) { return coord % SIZEX; }
int get_y(int coord) { return (coord / SIZEX) % SIZEY; }
int get_z(int coord) { return coord / (SIZEX*SIZEY); }

// Returns the index of the vertex to a
// given direction of it, or EMPTY if it does not exist.
int _west (int i) { return (get_x(i) == 0)         ? EMPTY : i - 1;             }
int _east (int i) { return (get_x(i) == SIZEX - 1) ? EMPTY : i + 1;             }
int _south(int i) { return (get_y(i) == 0)         ? EMPTY : i - SIZEX;         }
int _north(int i) { return (get_y(i) == SIZEY - 1) ? EMPTY : i + SIZEX;         }
int _down (int i) { return (get_z(i) == 0)         ? EMPTY : i - (SIZEX*SIZEY); }
int _up   (int i) { return (get_z(i) == SIZEZ - 1) ? EMPTY : i + (SIZEX*SIZEY); }

/* IMPORTANT: the += 2 idea only works for odds */
int hasPillar(vertexID i)
{
	return (get_x(i) + get_y(i)) % 2 == 0;
}

// For the checkerboard branch, return true if the x or y is on the outer shell.
bool onOuterShell(vertexID i)
{
	// If i has an 'empty' neighbor, then it is
	// on the outer shell.
	/*
	for (vertexID x : G.vertices[i].neighbors)
	{
		if (x == EMPTY) return true;
	}
	*/
	return
		G.vertices[i].neighbors[NORTH] == EMPTY ||
		G.vertices[i].neighbors[SOUTH] == EMPTY ||
		G.vertices[i].neighbors[EAST ] == EMPTY ||
		G.vertices[i].neighbors[WEST ] == EMPTY;
}

Graph::Graph()
{
	for (vertexID i = 0; i < numVertices; i++)
	{
		vertices[i].neighbors[WEST ] = _west (i);
		vertices[i].neighbors[EAST ] = _east (i);
		
		vertices[i].neighbors[SOUTH] = _south(i);
		vertices[i].neighbors[NORTH] = _north(i);
		
		vertices[i].neighbors[DOWN ] = _down (i);
		vertices[i].neighbors[UP   ] = _up   (i);
	}
	
	/* IMPORTANT: the += 2 idea only works for odds */
	
	for (vertexID i = 0; i < (numVertices/2); i++)
	{
		if (!hasPillar(i))
		{
			vertices[i].neighbors[UP] = EMPTY;
			vertices[_up(i)].neighbors[DOWN] = EMPTY;
		}
	}
}

bool Subtree::add(vertexID i)
{
	/*
	vertices[i].induced = true;
	
	// This should have one neighbor, we need to validate the neighbor
	for (const vertexID x : G.vertices[i].neighbors)
	{
		if (vertices[x].induced)
		{
			++vertices[x].effectiveDegree;
			
			if (validate(x)) break;
			else
			{
				// Undo changes made and report that this is invalid
				--vertices[x].effectiveDegree;
				vertices[i].induced = false;
				return false;
			}
		}
	}
	
	++numInduced;

	for (const vertexID x : G.vertices[i].neighbors)
	{
		// Ignore the induced vertex, its degree has already been increased.
		if (x != EMPTY && vertices[x].induced)
			++vertices[x].effectiveDegree;
	}
	return true;
	*/
	
	/* This is a simpler implementation, though may not be as fast */
	
	vertices[i].induced = true;
	
	++numInduced;
	
	for (const vertexID x : G.vertices[i].neighbors)
	{
		if (x != EMPTY) ++vertices[x].effectiveDegree;
	}
	
	for (const vertexID x : G.vertices[i].neighbors)
	{
		if (exists(x))
		{
			if (validate(x) && doesNotEnclose(x))
			{
				return true;
			}
			else
			{
				rem(i);
				return false;
			}
		}
	}
	
	// This shouldn't happen, but exists just to make the compiler happy.
	return true;
}

void Subtree::rem(vertexID i)
{
	vertices[i].induced = false;
	
	--numInduced;
	
	for (const vertexID x : G.vertices[i].neighbors)
	{
		if (x != EMPTY) --vertices[x].effectiveDegree;
	}
}

void Subtree::print() const
{
	std::cout << "Subgraph: ";
	for (vertexID x = 0; x < numVertices; x++)
	{
		if (has(x)) std::cout << x << ' ';
	}
	std::cout << std::endl;
}

void Subtree::writeToFile(std::string filename) const
{
	std::ofstream file(filename);

	file << SIZEX << ' ' << SIZEY << ' ' << SIZEZ << std::endl << std::endl;
	
	vertexID x = 0;
	for (unsigned i = 0; i < SIZEZ; i++)
	{
		for (unsigned j = 0; j < SIZEY; j++)
		{
			for (unsigned k = 0; k < SIZEX; k++)
			{
				file << (vertices[x++].induced ? BLOCK_PRESENT : BLOCK_MISSING);
			}
			file << std::endl;
		}
		file << std::endl;
	}
}

Subtree::Subtree(vertexID r) : numInduced(0), root(r), vertices()
{
	for (vertexID x = 0; x < numVertices; x++)
	{
		vertices[x].induced = false;
		vertices[x].effectiveDegree = 0;
	}
	add(r);
}

Subtree::Subtree(const Subtree& S)
	: numInduced(S.numInduced), root(S.root), vertices(S.vertices) {}

bool Subtree::validate(vertexID i) const
{
	if (cnt(i) != 4)
		return cnt(i) < 4;
	
	// Ensure all axis have at least one neighbor
	return
		( exists(_west (i)) || exists(_east (i)) ) &&
		( exists(_north(i)) || exists(_south(i)) ) &&
		( exists(_down (i)) || exists(_up   (i)) );
}

bool Subtree::connectedToColumns() const
{
	// Start at the first position that would have a pillar
	for (vertexID i = 0; i < (numVertices/2); i++)
	{
		if (hasPillar(i) && !has(i) && !has(_up(i))) return false;
	}
	
	return true;
}

// For checkerboard graphs, we can just check if
// the block beneath/above it exists or not.
bool Subtree::doesNotEnclose(vertexID i) const
{
	if (onOuterShell(i) || hasPillar(i)) return true;
	
	if (i < (numVertices/2))
	{
		return !has(_up(i));
	}
	else
	{
		return !has(_down(i));
	}
}

bool Subtree::hasEnclosedSpace() const
{
	// Start at the first position that would not have a pillar
	for (vertexID i = 0; i < (numVertices/2); i++)
	{
		if (!hasPillar(i) && !onOuterShell(i) && has(i) && has(_up(i))) return true;
	}
	
	return false;
	/*
	// enum to mark each vertex
	enum label { induced, empty, empty_connected };
	
	// Each vertex is labeled one of the above
	std::array<label, numVertices> vertex_labels;
	
	// Initial label is either induced or empty
	for (vertexID x = 0; x < numVertices; x++)
	{
		vertex_labels[x] = has(x) ? induced : empty;
	}
	
	// Queue for breadth-first search
	std::queue<vertexID> toBeVisited;
	
	// For each vertex touching the outer shell of the cube,
	// queue for searching
	for (vertexID x = 0; x < numVertices; x++)
	{
		if (onOuterShell(x))
		{
			toBeVisited.push(x);
		}
	}
	
	// Keep track of the number of connected empty vertices
	unsigned numConnected = 0;
	
	while (!toBeVisited.empty())
	{
		vertexID x = toBeVisited.front();
		toBeVisited.pop();
		
		if (vertex_labels[x] == empty)
		{
			vertex_labels[x] = empty_connected;
			
			++numConnected;
			
			// Queue all of x's neighbors
			for (vertexID y : G.vertices[x].neighbors)
			{
				toBeVisited.push(y);
			}
		}
	}
	
	// If the graph has enclosed space, then there will
	// be vertices not accounted for in this formula
	return numInduced + numConnected != numVertices;
	*/
}
