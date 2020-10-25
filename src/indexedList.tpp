#include "indexedList.hpp"

#include <random>
#include <iostream>

template<class T, T N>
constexpr indexedList<T,N>::indexedList() :
	numItems(0), list(), head(defs::EMPTY), tail(defs::EMPTY) {}

template<class T, T N>
constexpr bool indexedList<T,N>::remove(T x)
{
	if (!list[x].inList) return false;
	
	list[x].inList = false;
	
	if (list[x].next != defs::EMPTY)
		list[list[x].next].prev = list[x].prev;
	else // Tail of the list
		tail = list[x].prev;
	
	if (list[x].prev != defs::EMPTY)
		list[list[x].prev].next = list[x].next;
	else // Head of the list
		head = list[x].next;
	
	//list[x].next = EMPTY;
	//list[x].prev = EMPTY;
	
	--numItems;
	
	return true;
}

template<class T, T N>
constexpr void indexedList<T,N>::push_front(T x)
{
	list[x].inList = true;
	list[x].next = head;
	list[x].prev = defs::EMPTY;
	
	if (head == defs::EMPTY)
	{
		tail = x;
	}
	else
	{
		list[head].prev = x;
	}
	
	head = x;
	
	++numItems;
}

template<class T, T N>
constexpr void indexedList<T,N>::push_back(T x)
{
	list[x].inList = true;
	list[x].next = defs::EMPTY;
	list[x].prev = tail;
	
	if (tail == defs::EMPTY)
	{
		head = x;
	}
	else
	{
		list[tail].next = x;
	}
	
	tail = x;
	
	++numItems;
}

template<class T, T N>
constexpr T indexedList<T,N>::pop_front()
{
	T x = head;
	remove(head);
	return x;
}

template<class T, T N>
constexpr T indexedList<T,N>::pop_back()
{
	T x = tail;
	remove(tail);
	return x;
}

template<class T, T N>
constexpr bool indexedList<T,N>::empty() const
{
	return head == defs::EMPTY;
}

template<class T, T N>
constexpr void swap(indexedList<T,N>& list1, indexedList<T,N>& list2)
{
	// Loop follows the second list, since this constantly swaps the
	// next item information into the other list.
	for (T x = list1.head; x != defs::EMPTY; x = list2.list[x].next)
	{
		std::swap(list1.list[x],list2.list[x]);
	}
	
	// Now follow list2 again, but only swap if list1 did not already swap.
	// The cell has been swapped if either the cell in list 2 is not induced
	// (since it should be) or if the cell in both lists are induced.
	// (!list2.list[x].inList || list1.list[x].inList), which negates to
	// (list2.list[x].inList && !list1.list[x].inList).
	// After the (potential) swap, the next cell will always be in list1.
	for (T x = list2.head; x != defs::EMPTY; x = list1.list[x].next)
	{
		if (list2.list[x].inList && !list1.list[x].inList)
		{
			std::swap(list1.list[x],list2.list[x]);
		}
	}
	
	// Finally, swap the heads and tails.
	std::swap(list1.head,list2.head);
	std::swap(list1.tail,list2.tail);
}

template<class T, T N>
constexpr bool indexedList<T,N>::exists(T x) const
{
	return list[x].inList;
}

template<class T, T N>
constexpr void indexedList<T,N>::clear()
{
	head = tail = defs::EMPTY;
}

template<class T, T N>
constexpr T indexedList<T,N>::size() const
{
	return numItems;
}

template<class T, T N>
constexpr T indexedList<T,N>::removeRandom()
{
	T toRemove = rand() % numItems;
	
	T valueToRemove = head;
	for (T i = 0; i < toRemove; i++)
		valueToRemove = list[valueToRemove].next;
	
	remove(valueToRemove);
	
	return valueToRemove;
}

template<class T, T N>
void indexedList<T,N>::print() const
{
	/*
	for (T x = head; x != EMPTY; x = list[x].next)
	{
		std::cout << x << " ";
	}
	std::cout << std::endl;
	*/
	std::cout << "(default cells omitted)" << std::endl;
	for (unsigned i = 0; i < N; i++)
	{
		if (list[i].inList || list[i].next != defs::EMPTY || list[i].prev != defs::EMPTY)
			std::cout << "vertex " << i << ", " << (list[i].inList ? "in" : "not in") << ", "
				<< "next = " << list[i].next << ", prev = " << list[i].prev << std::endl;
	}
	std::cout << "head = " << head << ", tail = " << tail
		<< ", size = " << numItems << std::endl;
}