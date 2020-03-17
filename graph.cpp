#include "graph.hpp"

int Graph::get_x(vertexID coord) { return coord % SIZEX; }
int Graph::get_y(vertexID coord) { return (coord / SIZEX) % SIZEY; }
int Graph::get_z(vertexID coord) { return coord / (SIZEX*SIZEY); }

vertexID Graph::_west (vertexID i)
	{ return (get_x(i) == 0)         ? EMPTY : i - 1;             }
	
vertexID Graph::_east (vertexID i)
	{ return (get_x(i) == SIZEX - 1) ? EMPTY : i + 1;             }

vertexID Graph::_south(vertexID i)
	{ return (get_y(i) == 0)         ? EMPTY : i - SIZEX;         }

vertexID Graph::_north(vertexID i)
	{ return (get_y(i) == SIZEY - 1) ? EMPTY : i + SIZEX;         }

vertexID Graph::_down (vertexID i)
	{ return (get_z(i) == 0)         ? EMPTY : i - (SIZEX*SIZEY); }

vertexID Graph::_up   (vertexID i)
	{ return (get_z(i) == SIZEZ - 1) ? EMPTY : i + (SIZEX*SIZEY); }

Graph::graphVertex::graphVertex(vertexID i)
{
	enabled = true;

	directions[WEST ] = _west (i);
	directions[EAST ] = _east (i);
	
	directions[SOUTH] = _south(i);
	directions[NORTH] = _north(i);
	
	directions[DOWN ] = _down (i);
	directions[UP   ] = _up   (i);
	
	// Neighbors need to be in ascending order of ID
	for (vertexID n : directions)
		if (n != EMPTY)
			neighbors.push_back(n);
}

Graph::Graph() : numDisabled(0)
{
	for (vertexID i = 0; i < numVertices; i++)
	{
		vertices[i] = graphVertex(i);
	}
}

void Graph::disable(vertexID i)
{
	vertices[i].enabled = false;
	
	vertices[vertices[i].directions[EAST ]].directions[WEST ] = EMPTY;
	vertices[vertices[i].directions[WEST ]].directions[EAST ] = EMPTY;
	
	vertices[vertices[i].directions[SOUTH]].directions[NORTH] = EMPTY;
	vertices[vertices[i].directions[NORTH]].directions[SOUTH] = EMPTY;
	
	vertices[vertices[i].directions[DOWN ]].directions[UP   ] = EMPTY;
	vertices[vertices[i].directions[UP   ]].directions[DOWN ] = EMPTY;
	
	for (vertexID n : vertices[i].neighbors)
	{
		//vertices[n].neighbors.remove(i);
		
		// The way I have to do this seems dumb, but since this
		// isn't a critical part of the code (performance wise)
		// this isn't too much of an issue.
		auto iter = vertices[n].neighbors.begin();
		while (*iter != i) ++iter;
		vertices[n].neighbors.erase(iter);
	}
	
	++numDisabled;
}
