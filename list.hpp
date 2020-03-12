#ifndef LIST_HPP
#define LIST_HPP

/*
Implementation of a doubly linked list.

This list class' purpose is to cache the cells it creates.
When it 'deletes' a cell, it actually just moves it to a cache,
and then grabs from the cache (or creates a new one if not
available) when it wants to 'create' a cell. The caches are
local, rather than static, for thread safety.
*/

template<class T>
class list
{
	public:
	
	list();
	
	// Should probably use full rule of 5.5
	// Will also need a swap function
	~list();
	
	void push_back(T item);
	void push_front(T item);
	
	void remove_first(T item);
	void remove_last(T item);
	
	bool empty();
	
	T pop_front();
	T pop_back();
	
	private:
	
	struct list_cell
	{
		T item;
		list cell *forw, *back;
		
		list_cell(list_cell *f, list_cell *b, T it)
		{
			forw = f; back = b; item = it;
		}
		
		// Used for the 'empty' cells
		list_cell(list_cell *f, list_cell *b)
		{
			forw = f; back = b;
		}
	};
	
	list_cell *firs, *last;
	
	list_cell *cache;
};

#include "list.tpp"

#endif
