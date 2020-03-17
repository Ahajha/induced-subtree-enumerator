#include "subTree.hpp"
#include "indexedList.hpp"
#include "CTPL/ctpl_stl.h"

#include <stack>
#include <iostream>
#include <ctime>
#include <thread>
#include <mutex>
#include <fstream>

#define NUM_THREADS (std::thread::hardware_concurrency())

enum action_type
{
	add,
	rem,
	stop
};

struct action
{
	action_type type;
	vertexID v;
};

// Since there is only one base graph, we can let it be global.
Graph G;

// This is for functions needing the original graph.
const Graph G_orig;

// Thread pool
ctpl::thread_pool pool(NUM_THREADS);

// Maximum size graph seen so far
unsigned largestTree = 0;

// File to read the starting graph, an induced subgraph of a 3D lattice.
// File to write the best graph seen so far to
std::string infile, outfile;

// The start time of the program
clock_t start_time;

// Grid of indexedLists, used to store the border elements as they are removed,
// then swapped back to restore. A call to branch can find the list it should
// use by going to lists[id][S.numInduced].
std::vector<std::array<indexedList<numVertices>, numVertices>> lists(NUM_THREADS);

std::vector<unsigned long long> numLeaves(NUM_THREADS);
bool lastWasNew = false;

// Returns the number of thread-seconds since the start of the program.
float threadSeconds()
{
	return (float)(clock()-start_time)/(CLOCKS_PER_SEC);
}

// Updates the border of S after adding x.
void update(Subtree& S, indexedList<numVertices>& border,
	vertexID x, std::stack<action>& previous_actions)
{
	for (vertexID y : G.vertices[x].neighbors)
	{
		// Pushes the current action, will need
		// to do the opposite action to reverse.
		if (S.cnt(y) > 1)
		{
			// This is a fix for the base algorithm, it will
			// not work without this.
			if (border.remove(y))
			{
				previous_actions.push({rem,y});
			}
		}
		else if (y > S.root && !S.has(y))
		{
			border.push_front(y);
			previous_actions.push({add,y});
		}
	}
}

// Restores the border of S after removing x.
void restore(indexedList<numVertices>& border,
	std::stack<action>& previous_actions)
{
	while (true)
	{
		action act = previous_actions.top();
		previous_actions.pop();
		
		if (act.type == stop)
		{
			return;
		}
		
		if (act.type == add)
		{
			border.remove(act.v);
		}
		else /* act.type == rem */
		{
			border.push_front(act.v);
		}
	}
}

// After confirming S has a greater number of blocks than seen before,
// prints to clog If S does not have enclosed space, updates
// largestTree and writes the result to outfile, does neither if S
// does have enclosed space. Relies on the mutex below for thread safety.

std::mutex mutex;

void checkCandidate(Subtree S)
{	
	mutex.lock();
	
	if (S.numInduced > largestTree)
	{
		if (!lastWasNew)
		{
			std::cout << std::endl;
			lastWasNew = true;
		}
		
		if (S.hasEnclosedSpace())
		{
			std::clog << S.numInduced
				<< " vertices with enclosed space, found at "
				<< threadSeconds() << " thread-seconds" << std::endl;
		}
		else
		{
			largestTree = S.numInduced;
			
			S.writeToFile(outfile);
			
			std::clog << largestTree << " vertices, found at " <<
				threadSeconds() << " thread-seconds" << std::endl;
		}
	}
	
	mutex.unlock();
}

// Performs the bulk of the algorithm described in the paper.
void branch(int id, Subtree& S, indexedList<numVertices>& border,
	std::stack<action>& previous_actions)
{
	// We only consider subtrees without children to be good candidates,
	// since any children of this tree would be better candidates.
	if (border.empty())
	{
		if (S.numInduced > largestTree)
		{
			checkCandidate(S);
		}
		++numLeaves[id];
	}
	else
	{
		do
		{
			// Get and remove the first element
			vertexID x = border.pop_front();
			
			// Push it onto a temporary list. This is a fix
			// to the base algorithm, it will not work without this
			// (along with the swap below)
			lists[id][S.numInduced].push_back(x);
			
			// Ensure the addition would be valid
			if(S.add(x))
			{
				previous_actions.push({stop,0});
				update(S,border,x,previous_actions);
				
				if (pool.n_idle() != 0)
				{
					pool.push(branch,S,border,previous_actions);
				}
				else
				{
					branch(id,S,border,previous_actions);
				}
				
				restore(border,previous_actions);
				
				S.rem(x);
			}
		}
		while (!border.empty());
		
		std::swap(border, lists[id][S.numInduced]);
	}
}

int main(int num_args, char** args)
{
	if (num_args != 3)
	{
		std::cerr << "usage: " << args[0] << " <infile> <outfile>" << std::endl;
		exit(1);
	}
	
	infile  = args[1];
	outfile = args[2];
	
	// Read the input file and disable any vertices that are not induced in the
	// original subgraph.
	std::ifstream in(infile);
	
	char symbol;
	vertexID x = 0;
	for (unsigned i = 0; i < SIZEZ; i++)
	{
		for (unsigned j = 0; j < SIZEY; j++)
		{
			for (unsigned k = 0; k < SIZEX; k++)
			{
				in >> symbol;
				
				if (symbol == '_')
				{
					G.disable(x);
				}
				++x;
			}
		}
	}
	
	in.close();
	
	// Start the enumeration
	start_time = clock();
	
	for (vertexID x = 0; x < numVertices; x++)
	{
		if (!G.vertices[x].enabled) continue;
	
		// Makes a subgraph with one vertex, its root.
		Subtree S(x);
		
		indexedList<numVertices> border;
		
		std::stack<action> previous_actions;
		
		update(S,border,x,previous_actions);
		
		pool.push(branch,S,border,previous_actions);
	}
	
	// Wait for all threads to finish
	while (pool.n_idle() < NUM_THREADS)
	{
		std::this_thread::sleep_for (std::chrono::seconds(1));
		
		unsigned long long total = 0;
		for (unsigned i = 0; i < NUM_THREADS; i++) total += numLeaves[i];
		
		mutex.lock();
		
		std::cout << "\r" << threadSeconds() << " thread-seconds elapsed, "
			<< total << " leaves encountered";
		
		std::cout.flush();
		
		lastWasNew = false;
		
		mutex.unlock();
	}
	
	std::clog << threadSeconds() << " thread-seconds" << std::endl;
	
	std::clog << "Largest size = " << largestTree << std::endl;
}
