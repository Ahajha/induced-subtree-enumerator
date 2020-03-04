// Step 1: Implement
// Step 2: Optimize

// IN THAT ORDER

#include "subTree.hpp"
#include "CTPL/ctpl_stl.h"

#include <list>
#include <stack>
#include <iostream>
#include <ctime>
#include <thread>
#include <mutex>

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
const Graph G = Graph();

// Thread pool
ctpl::thread_pool pool(NUM_THREADS);

// Maximum size graph seen so far
unsigned largestTree = 0;

// File to write the best graph seen so far to
std::string outfile;

clock_t start_time;

// Returns the number of thread-seconds since the start of the program.
float threadSeconds()
{
	return (float)(clock()-start_time)/(CLOCKS_PER_SEC);
}

void printBorder(std::list<vertexID> border)
{
	std::cout << "Border:";
	for (vertexID i : border)
	{
		std::cout << ' ' << i;
	}
	std::cout << std::endl;
}

void printStack(std::stack<action> previous_actions)
{
	std::stack<action> temp(previous_actions);
	
	while(!temp.empty())
	{
		action act = temp.top();
		if (act.type == stop)
			std::cout << "stop ";
		else
			std::cout << "{" << ((act.type == rem) ? "rem" : "add")
				<< "," << act.v << "} ";
		
		temp.pop();
	}
	std::cout << std::endl;
}


void update(Subtree& S, std::list<vertexID>& border,
	vertexID x, std::stack<action>& previous_actions)
{
	for (vertexID y : G.vertices[x].neighbors)
	{
		if (y == EMPTY) continue;
		
		// Pushes the current action, will need
		// to do the opposite action to reverse.
		if (S.cnt(y) > 1)
		{
			// border.remove(y), slighly optimized
			const auto end = border.end();
			for (auto iter = border.begin(); iter != end; ++iter)
			{
				if (y == *iter)
				{
					border.erase(iter);
					previous_actions.push({rem,y});
					break;
				}
			}
		}
		else if (y > S.root && !S.has(y))
		{
			border.push_front(y);
			previous_actions.push({add,y});
		}
	}
}

void restore(std::list<vertexID>& border,
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

void checkCandidate(Subtree S)
{
	static std::mutex mutex;
	
	mutex.lock();
	
	if (S.numInduced > largestTree)
	{
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

void branch(int id, Subtree& S, std::list<vertexID>& border,
	std::stack<action>& previous_actions)
{
	// We only consider subtrees without children to be good candidates,
	// since any children of this tree would be better candidates.
	if (border.empty())
	{
		if (S.numInduced > largestTree && S.connectedToColumns())
		{
			checkCandidate(S);
		}
	}
	else
	{
		std::list<vertexID> border_copy;
	
		do
		{
			// Get and remove the first element
			vertexID x = border.front();
			border.pop_front();
			border_copy.push_back(x);
			
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
		
		std::swap(border, border_copy);
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
	
	for (vertexID x = 0; x < numVertices; x++)
	{
		// Makes a subgraph with one vertex, its root.
		Subtree S(x);
		
		std::list<vertexID> border;
		
		std::stack<action> previous_actions = std::stack<action>();
		
		update(S,border,x,previous_actions);
		
		branch(-1,S,border,previous_actions);
		
		// Would like to make main wait for all threads to finish,
		// but using pool.wait(true) disallows any future threads
		// from spawning. For now, let the main continue as is.
	}
	
	// This is not the most elegant solution, but it will work for now
	while (pool.n_idle() < NUM_THREADS)
	{
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}
	
	std::clog << threadSeconds() << " thread-seconds" << std::endl;
	
	std::clog << "Largest size = " << largestTree << std::endl;
}
