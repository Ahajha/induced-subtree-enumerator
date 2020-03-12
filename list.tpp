#define INIT_CELL_COUNT 10

template<class T>
list<T>::list()
{
	firs = last = nullptr;
	
	cache = nullptr
	
	for (unsigned i = 0; i < INIT_CELL_COUNT; i++)
	{
		cache = new list_cell(nullptr,cache);
	}
}

~list();

void push_back(T item);
void push_front(T item);

void remove_first(T item);
void remove_last(T item);

bool empty();

T pop_front();
T pop_back();
