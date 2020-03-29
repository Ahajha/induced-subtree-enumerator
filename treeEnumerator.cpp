#include "subTree.hpp"
#include "indexedList.hpp"
#include "CTPL/ctpl_stl.h"

#include <stack>
#include <iostream>
#include <ctime>
#include <thread>
#include <mutex>

#define NUM_THREADS (std::thread::hardware_concurrency())

// Since there is only one base graph, we can let it be global.
const Graph G;

// Thread pool
ctpl::thread_pool pool(NUM_THREADS);

// Maximum size graph seen so far
unsigned largestTree = 0;

// File to write the best graph seen so far to
std::string outfile;

// The start time of the program
clock_t start_time;

// Grid of indexedLists, used to store the border elements as they are removed,
// then swapped back to restore. A call to branch can find the list it should
// use by going to lists[id][S.numInduced].
std::vector<std::array<indexedList<defs::numVertices>, defs::numVertices>> lists(NUM_THREADS);

std::vector<unsigned long long> numLeaves(NUM_THREADS);
bool lastWasNew = false;

// Returns the number of thread-seconds since the start of the program.
float threadSeconds()
{
	return (float)(clock()-start_time)/(CLOCKS_PER_SEC);
}

// Updates the border of S after adding x.
void update(Subtree& S, indexedList<defs::numVertices>& border,
	defs::vertexID x, std::stack<defs::action>& previous_actions)
{
	for (defs::vertexID y : G.vertices[x].neighbors)
	{
		// Pushes the current action, will need
		// to do the opposite action to reverse.
		if (S.cnt(y) > 1)
		{
			// This is a fix for the base algorithm, it will
			// not work without this.
			if (border.remove(y))
			{
				previous_actions.push({defs::rem,y});
			}
		}
		else if (y > S.root && !S.has(y))
		{
			border.push_front(y);
			previous_actions.push({defs::add,y});
		}
	}
}

// Restores the border of S after removing x.
void restore(indexedList<defs::numVertices>& border,
	std::stack<defs::action>& previous_actions)
{
	while (true)
	{
		defs::action act = previous_actions.top();
		previous_actions.pop();
		
		if (act.type == defs::stop)
		{
			return;
		}
		
		if (act.type == defs::add)
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
void branch(int id, Subtree& S, indexedList<defs::numVertices>& border,
	std::stack<defs::action>& previous_actions)
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
			defs::vertexID x = border.pop_front();
			
			// Push it onto a temporary list. This is a fix
			// to the base algorithm, it will not work without this
			// (along with the swap below)
			lists[id][S.numInduced].push_back(x);
			
			// Ensure the addition would be valid
			if(S.add(x))
			{
				previous_actions.push({defs::stop,0});
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
		
		swap(border, lists[id][S.numInduced]);
	}
}

int main(int num_args, char** args)
{
	if (num_args != 2)
	{
		std::cerr << "usage: " << args[0] << " <outfile>" << std::endl;
		exit(1);
	}
	
	outfile = args[1];
	
	start_time = clock();
	
	for (defs::vertexID x = 0; x < defs::numVertices; x++)
	{
		// Makes a subgraph with one vertex, its root.
		Subtree S(x);
		
		indexedList<defs::numVertices> border;
		
		std::stack<defs::action> previous_actions;
		
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
		
		std::clog << "\r" << threadSeconds() << " thread-seconds elapsed, "
			<< total << " leaves encountered" << std::flush;
		
		lastWasNew = false;
		
		mutex.unlock();
	}
	
	std::clog << std::endl << "Largest size = " << largestTree << std::endl;
}
