// Step 1: Implement
// Step 2: Optimize

// IN THAT ORDER

#include "subTree.hpp"
#include "CTPL-master/ctpl_stl.h"

#include <list>
#include <stack>
#include <iostream>
#include <ctime>
#include <thread>
#include <mutex>

#define NUM_THREADS 15

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
			// Minor todo: optimize the removal of the element
			
			unsigned old_size = border.size();
			border.remove(y);
			
			// If the element actually got removed, record that
			if (border.size() != old_size)
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

// Forward declaration, since this and branch are mutually recursive
void spawn_thread(int id,Subtree S, std::list<vertexID> border,
	std::stack<action> previous_actions);

void branch(int id, Subtree& S, std::list<vertexID>& border,
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
					Subtree S_copy(S);
					std::list<vertexID> border_copy(border);
					std::stack<action> prev_actions_copy(previous_actions);
					
					pool.push(spawn_thread,S_copy,border_copy,prev_actions_copy);
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

// Problem:

// Program does not use all threads given. One initial problem I noticed
// is that the values passed by reference to the threads were local,
// and thus were deleted potentially before the threads even saw them.

// My thoughts are to create a global array of NUM_THREADS parameter sets,
// and each time a thread is dispatched it is told what set of parameters to use.

// TODO

void spawn_thread(int id, Subtree S, std::list<vertexID> border,
	std::stack<action> previous_actions)
{
	Subtree S_copy(S);
	std::list<vertexID> border_copy(border);
	std::stack<action> prev_actions_copy(previous_actions);
	
	//pool.push(branch,S,border,prev_actions_copy);
	
	branch(id,S,border,prev_actions_copy);
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
	
	//ctpl::thread_pool(NUM_THREADS);
	
	for (vertexID x = 0; x < numVertices; x++)
	{
		// Makes a subgraph with one vertex, its root.
		Subtree S(x);
		
		std::list<vertexID> border;
		
		std::stack<action> previous_actions = std::stack<action>();
		
		update(S,border,x,previous_actions);
		
		//pool.push(branch,S,border,previous_actions);
		
		branch(-1,S,border,previous_actions);
		
		// Wait for all the threads to finish, to allow the current subtree
		// of branch() to use all the threads.
		pool.stop(true);
	}
	
	// Also divide by the number of threads here since clock() reports
	// thread-seconds, instead of raw seconds.
	std::clog << threadSeconds() << " thread-seconds" << std::endl;
	
	std::clog << "Largest size = " << largestTree << std::endl;
}
